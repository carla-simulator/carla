"""HTTP client for the carla-cosmos server.

::

    client = CosmosClient("http://node:8000", token="cc_...")      # or COSMOS_URL / COSMOS_TOKEN
    client.wait_ready()
    job = client.submit_clip(Clip.load("clips/abc"), "cosmos3-nano",
                             prompt="golden hour, wet asphalt",
                             controls={"depth": "clip", "seg": "clip", "edge": "derive"})
    info = job.wait(on_progress=lambda i: print(i.progress, i.message))
    stored = job.download()          # <results root>/<clip_id>/<job_id>/ + job.json (results.py)

Uploads are content-addressed: files are hashed locally, the server is asked
which blobs it lacks (``POST /v1/blobs/check``) and only those are streamed.
Requests are validated against the backend contract *before* any upload.
"""

from __future__ import annotations

import contextlib
import hashlib
import logging
import os
import tempfile
import time
import zipfile
from pathlib import Path
from typing import TYPE_CHECKING, Any, Callable, Iterable, Sequence

import httpx

from .clip import Clip
from .contracts import (
    BackendContract,
    ClipManifest,
    ControlInput,
    JobInfo,
    JobRequest,
    JobSubmission,
    ModelInfo,
    Priority,
    ResultManifest,
    validate_request,
)

if TYPE_CHECKING:  # pragma: no cover
    from .results import StoredJob

log = logging.getLogger(__name__)

DEFAULT_URL = "http://localhost:8000"
CHUNK = 4 << 20


class CosmosError(Exception):
    """Server returned an error (or the request failed local validation)."""

    def __init__(self, status: int, detail: Any, errors: list[str] | None = None) -> None:
        self.status = status
        self.detail = detail
        self.errors = errors or []
        msg = f"HTTP {status}: {detail}" if status else str(detail)
        if self.errors:
            msg += "\n  - " + "\n  - ".join(self.errors)
        super().__init__(msg)


class JobFailed(CosmosError):
    """The job ended ``failed`` or ``cancelled``."""

    def __init__(self, info: JobInfo) -> None:
        self.info = info
        super().__init__(0, f"job {info.id} {info.status}: {info.error or ''}")


class CosmosClient:
    def __init__(self, url: str | None = None, token: str | None = None, timeout: float = 120.0,
                 transport: httpx.BaseTransport | None = None) -> None:
        self.url = (url or os.environ.get("COSMOS_URL") or DEFAULT_URL).rstrip("/")
        self.token = token or os.environ.get("COSMOS_TOKEN") or _read_token_file(os.environ.get("COSMOS_TOKEN_FILE"))
        headers = {"Authorization": f"Bearer {self.token}"} if self.token else {}
        self._http = httpx.Client(base_url=self.url, headers=headers, timeout=httpx.Timeout(timeout, connect=10.0),
                                  transport=transport)

    def close(self) -> None:
        self._http.close()

    def __enter__(self) -> "CosmosClient":
        return self

    def __exit__(self, *exc) -> None:
        self.close()

    # -- plumbing ---------------------------------------------------------------------------
    def _request(self, method: str, path: str, **kw) -> httpx.Response:
        try:
            r = self._http.request(method, path, **kw)
        except httpx.HTTPError as exc:
            raise CosmosError(0, f"{method} {self.url}{path} failed: {exc}") from exc
        if r.status_code >= 400:
            detail: Any = r.text
            errors: list[str] = []
            try:
                body = r.json()
                detail = body.get("detail", body)
                if isinstance(detail, dict) and "errors" in detail:
                    errors = list(detail["errors"])
                    detail = "request rejected"
            except ValueError:
                pass
            raise CosmosError(r.status_code, detail, errors)
        return r

    # -- health -------------------------------------------------------------------------------
    def health(self) -> dict[str, Any]:
        return self._request("GET", "/v1/health/live").json()

    def ready(self) -> tuple[bool, dict[str, Any]]:
        """``(ready, details)`` — does not raise on 503."""
        try:
            r = self._http.get("/v1/health/ready")
        except httpx.HTTPError as exc:
            return False, {"error": str(exc)}
        try:
            body = r.json()
        except ValueError:
            body = {"error": r.text}
        return r.status_code == 200, body

    def wait_ready(self, timeout: float = 1800.0, poll: float = 2.0,
                   on_wait: Callable[[dict[str, Any]], None] | None = None) -> dict[str, Any]:
        """Block until every worker is loaded; raises :class:`CosmosError` on timeout."""
        t0 = time.monotonic()
        while True:
            ok, body = self.ready()
            if ok:
                return body
            if on_wait:
                on_wait(body)
            if time.monotonic() - t0 > timeout:
                raise CosmosError(503, f"server not ready after {timeout:.0f}s: {body}")
            time.sleep(poll)

    def status(self) -> dict[str, Any]:
        return self._request("GET", "/v1/status").json()

    def retention(self, refresh: bool = False) -> dict[str, float] | None:
        """How long the server keeps outputs (``{"job_ttl_hours": .., "blob_ttl_hours": ..}``).

        ``None`` when the server publishes no retention policy (older servers).
        Cached: this is a server setting, it does not change while it runs.
        """
        cached = getattr(self, "_retention_cache", None)
        if cached is None or refresh:
            try:
                cached = dict(self.status().get("retention") or {})
            except CosmosError:
                cached = {}
            self._retention_cache = cached
        return cached or None

    # -- models ---------------------------------------------------------------------------------
    def models(self) -> dict[str, ModelInfo]:
        return {k: ModelInfo.model_validate(v) for k, v in self._request("GET", "/v1/models").json().items()}

    def contract(self, backend: str) -> BackendContract:
        return ModelInfo.model_validate(self._request("GET", f"/v1/models/{backend}").json()).contract

    # -- blobs --------------------------------------------------------------------------------------
    def missing_blobs(self, ids: Iterable[str]) -> list[str]:
        return self._request("POST", "/v1/blobs/check", json={"ids": list(ids)}).json()["missing"]

    def upload(self, path: str | Path, content_type: str | None = None, blob_id: str | None = None) -> str:
        """Upload a file (skipped when the server already has it).  Returns the blob id."""
        p = Path(path)
        blob_id = blob_id or sha256_file(p)
        if not self.missing_blobs([blob_id]):
            log.debug("blob %s (%s) already on server", blob_id[:12], p.name)
            return blob_id
        ct = content_type or ("video/mp4" if p.suffix == ".mp4" else "application/octet-stream")
        with p.open("rb") as f:
            self._request("PUT", f"/v1/blobs/{blob_id}", content=_iter_file(f),
                          headers={"Content-Type": ct, "X-Filename": p.name,
                                   "Content-Length": str(p.stat().st_size)})
        log.info("uploaded %s (%.1f MB)", p.name, p.stat().st_size / 1e6)
        return blob_id

    def upload_dir(self, directory: str | Path) -> str:
        """Zip a directory (deterministically) and upload it as one blob."""
        with tempfile.TemporaryDirectory() as tmp:
            z = Path(tmp) / "scene.zip"
            zip_dir(Path(directory), z)
            return self.upload(z, content_type="application/zip")

    def download_blob(self, blob_id: str, dest: str | Path) -> Path:
        dest = Path(dest)
        with self._http.stream("GET", f"/v1/blobs/{blob_id}") as r:
            if r.status_code >= 400:
                raise CosmosError(r.status_code, r.read().decode(errors="replace"))
            with dest.open("wb") as f:
                for chunk in r.iter_bytes(CHUNK):
                    f.write(chunk)
        return dest

    # -- jobs ----------------------------------------------------------------------------------------
    def submit(self, request: JobRequest, manifest: ClipManifest) -> "Job":
        """Submit a fully specified request (blob ids already uploaded)."""
        body = JobSubmission(request=request, manifest=manifest).model_dump(mode="json")
        info = JobInfo.model_validate(self._request("POST", "/v1/jobs", json=body).json())
        return Job(self, info)

    def submit_clip(self, clip: Clip, backend: str, prompt: str, controls: dict[str, str | ControlInput],
                    views: list[str] | None = None, rgb: bool | None = None, negative_prompt: str | None = None,
                    seed: int = 0, guidance: float | None = None, num_steps: int | None = None,
                    resolution: str | None = None, priority: Priority = "interactive",
                    extra: dict[str, Any] | None = None, contract: BackendContract | None = None,
                    weights: dict[str, float] | None = None,
                    mask_classes: Sequence[str | int] | None = None,
                    mask_dilate: int | None = None) -> "Job":
        """Validate, upload what is needed from ``clip`` and submit.

        ``controls`` maps control name to ``"clip"`` (upload the clip's control
        video(s)), ``"derive"`` (server derives it from RGB), ``"scene"``
        (upload the clip's ClipGT scene package) or a :class:`ControlInput`.
        ``rgb=None`` uploads RGB when the backend or a derived control needs it.

        ``weights`` sets the per-control weight (``{"depth": 1.0, "seg": 0.5}``),
        the same knob as the ``("clip", 0.5)`` tuple form of ``controls`` and
        winning over it.

        ``mask_classes`` names CARLA semantic classes (``["vehicle"]``,
        ``["car", 12]``) to remove from every input derived from the captured
        pixels: the control videos and the RGB video are blanked there before
        they are uploaded, and the mask itself is uploaded for backends whose
        controls accept one (see :mod:`carla_cosmos.mask`).  ``mask_dilate``
        overrides the default dilation in pixels.
        """
        contract = contract or self.contract(backend)
        manifest = clip.manifest
        views = views or (manifest.camera_names if len(manifest.camera_names) == 1 else [])
        if not views and contract.max_views > 1:
            views = [v for v in contract.views if v in manifest.camera_names] or manifest.camera_names
        mask_tags = _resolve_mask_classes(mask_classes)
        dilate = _default_dilate() if mask_dilate is None else int(mask_dilate)
        unknown_weights = sorted(set(weights or {}) - set(controls))
        if unknown_weights:
            raise CosmosError(0, f"weight given for control(s) {unknown_weights} that are not in the request "
                                 f"(controls: {sorted(controls)})")
        req = JobRequest(backend=backend, prompt=prompt, negative_prompt=negative_prompt, seed=seed,
                         guidance=guidance, num_steps=num_steps, resolution=resolution, priority=priority,
                         views=views, extra=extra or {},
                         mask_classes=_mask_class_names(mask_tags),
                         mask_dilate=dilate if mask_tags else 0)
        # (kind, source, setter): kind "scene" -> source is a directory; kind "video" -> (video kind, view)
        plan: list[tuple[str, Any, Callable[[str], None]]] = []

        # controls
        for name, how in controls.items():
            if isinstance(how, ControlInput):
                req.controls[name] = how
                continue
            weight = None
            if isinstance(how, tuple):
                how, weight = how
            if weights and name in weights:
                weight = float(weights[name])
            if how == "derive":
                req.controls[name] = ControlInput(derive=True, weight=weight)
            elif how == "scene":
                if clip.scene_dir is None or not clip.scene_dir.exists():
                    raise CosmosError(0, f"control '{name}': clip {manifest.clip_id} has no scene package")
                inp = ControlInput(scene="pending", weight=weight)
                req.controls[name] = inp
                plan.append(("scene", clip.scene_dir, lambda bid, i=inp: setattr(i, "scene", bid)))
            elif how == "clip":
                if len(views) == 1:
                    inp = ControlInput(blob="pending", weight=weight)
                    req.controls[name] = inp
                    plan.append(("video", (name, views[0]), lambda bid, i=inp: setattr(i, "blob", bid)))
                else:
                    inp = ControlInput(blobs={v: "pending" for v in views}, weight=weight)
                    req.controls[name] = inp
                    for v in views:
                        plan.append(("video", (name, v), lambda bid, i=inp, v=v: i.blobs.__setitem__(v, bid)))
            else:
                raise CosmosError(0, f"control '{name}': unknown source '{how}' (clip | derive | scene | ControlInput)")

        # rgb
        needs_rgb = contract.rgb_required or any(c.derive for c in req.controls.values())
        if rgb is None:
            rgb = needs_rgb
        if rgb:
            for v in views:
                req.rgb[v] = "pending"
                plan.append(("video", ("rgb", v), lambda bid, v=v: req.rgb.__setitem__(v, bid)))

        # masking: the clip videos we are about to upload get blanked, and the mask itself
        # is uploaded for backends whose controls take one (Transfer 2.5 mask_path)
        with contextlib.ExitStack() as stack:
            if mask_tags:
                wanted = {src for kind, src, _ in plan if kind == "video"}
                if not wanted:
                    raise CosmosError(0, f"mask_classes={_mask_class_names(mask_tags)} has no effect on this "
                                         f"request: it uploads no clip video to mask (controls "
                                         f"{sorted(req.controls)} are server-rendered or derived and no RGB "
                                         f"is uploaded)")
                send_mask = bool([n for n in contract.maskable_controls if n in req.controls])
                masked = self._mask_clip_videos(clip, wanted, mask_tags, dilate, send_mask, stack)
                if send_mask:
                    for v in views:
                        req.masks[v] = "pending"
                        plan.append(("mask", ("mask", v), lambda bid, v=v: req.masks.__setitem__(v, bid)))
            else:
                masked = {}

            errors = validate_request(contract, req, manifest)
            # resolve clip files only after the contract check, so the error list is about the request
            resolved: list[tuple[str, Path, Callable[[str], None]]] = []
            for kind, source, setter in plan:
                if kind == "scene":
                    resolved.append((kind, source, setter))
                    continue
                vkind, view = source
                if (vkind, view) in masked:
                    resolved.append((kind, masked[(vkind, view)], setter))
                    continue
                try:
                    resolved.append((kind, clip.video(vkind, view), setter))
                except FileNotFoundError:
                    errors.append(f"clip {manifest.clip_id} has no '{vkind}' video for view '{view}' "
                                  f"(available: {sorted(manifest.videos)})")
            if errors:
                raise CosmosError(0, f"request for '{backend}' is invalid", errors)

            for kind, path, setter in resolved:
                setter(self.upload_dir(path) if kind == "scene" else self.upload(path))
        return self.submit(req, manifest)

    def _mask_clip_videos(self, clip: Clip, wanted: set[tuple[str, str]], classes: tuple[int, ...],
                          dilate: int, mask_video: bool,
                          stack: "contextlib.ExitStack") -> dict[tuple[str, str], Path]:
        """Write masked copies of the clip videos this request uploads (into a temp dir).

        ``wanted`` is the set of ``(video kind, view)`` the upload plan needs.
        Returns ``{(kind, view): path}``, including ``("mask", view)`` when the
        mask video itself is uploaded.  Raises when a requested control cannot
        be masked, rather than uploading unmasked pixels.
        """
        from . import mask as maskmod  # cv2 + numpy: capture extra, only needed when masking

        tmp = Path(stack.enter_context(tempfile.TemporaryDirectory(prefix="carla_cosmos_mask_")))
        unmaskable = sorted({k for k, _ in wanted} - set(maskmod.MASKABLE_KINDS))
        if unmaskable:
            raise CosmosError(0, f"cannot mask clip video kind(s) {unmaskable}; "
                                 f"maskable: {list(maskmod.MASKABLE_KINDS)}")
        out: dict[tuple[str, str], Path] = {}
        for view in sorted({v for _, v in wanted}):
            kinds = sorted({k for k, v in wanted if v == view})
            try:
                written = maskmod.masked_clip_videos(clip, view, classes, kinds, tmp / _slug(view),
                                                     dilate=dilate, mask_video=mask_video)
            except maskmod.MaskError as exc:
                raise CosmosError(0, str(exc)) from exc
            missing = [k for k in kinds if k not in written]
            if missing:
                raise CosmosError(0, f"clip {clip.manifest.clip_id} has no {missing} video for view '{view}' "
                                     f"to mask")
            for kind, path in written.items():
                out[(kind, view)] = path
        return out

    def jobs(self, status: str | None = None, backend: str | None = None, mine: bool = False,
             limit: int = 100) -> list[JobInfo]:
        params: dict[str, Any] = {"limit": limit}
        if status:
            params["status"] = status
        if backend:
            params["backend"] = backend
        if mine:
            params["mine"] = "true"
        return [JobInfo.model_validate(j) for j in self._request("GET", "/v1/jobs", params=params).json()]

    def job(self, job_id: str) -> "Job":
        return Job(self, JobInfo.model_validate(self._request("GET", f"/v1/jobs/{job_id}").json()))

    def cancel(self, job_id: str) -> dict[str, Any]:
        return self._request("DELETE", f"/v1/jobs/{job_id}").json()

    def result_manifest(self, job_id: str) -> ResultManifest:
        return ResultManifest.model_validate(self._request("GET", f"/v1/jobs/{job_id}/result").json())

    def download_result_file(self, job_id: str, name: str, dest: str | Path) -> Path:
        dest = Path(dest)
        dest.parent.mkdir(parents=True, exist_ok=True)
        with self._http.stream("GET", f"/v1/jobs/{job_id}/result/{name}") as r:
            if r.status_code >= 400:
                raise CosmosError(r.status_code, r.read().decode(errors="replace"))
            with dest.open("wb") as f:
                for chunk in r.iter_bytes(CHUNK):
                    f.write(chunk)
        return dest


class Job:
    """Handle on a submitted job."""

    def __init__(self, client: CosmosClient, info: JobInfo) -> None:
        self.client = client
        self.info = info

    @property
    def id(self) -> str:
        return self.info.id

    def refresh(self) -> JobInfo:
        self.info = self.client.job(self.id).info
        return self.info

    def wait(self, poll: float = 2.0, timeout: float | None = None,
             on_progress: Callable[[JobInfo], None] | None = None, raise_on_failure: bool = True) -> JobInfo:
        """Poll until terminal.  Raises :class:`JobFailed` unless ``raise_on_failure`` is false."""
        t0 = time.monotonic()
        last: tuple[str, float, str] | None = None
        while True:
            info = self.refresh()
            key = (info.status, round(info.progress, 3), info.message)
            if on_progress and key != last:
                on_progress(info)
                last = key
            if info.terminal:
                if info.status != "done" and raise_on_failure:
                    raise JobFailed(info)
                return info
            if timeout is not None and time.monotonic() - t0 > timeout:
                raise CosmosError(0, f"job {self.id} still '{info.status}' after {timeout:.0f}s")
            time.sleep(poll)

    def cancel(self) -> dict[str, Any]:
        return self.client.cancel(self.id)

    def result(self) -> "Result":
        return Result(self.client, self.id, self.client.result_manifest(self.id))

    def download(self, out_dir: str | Path | None = None, *, names: Iterable[str] | None = None,
                 verify: bool = True, progress: Callable[[str, int, int], None] | None = None,
                 clip: "Clip | str | Path | None" = None, viewer_video: bool | None = None) -> "StoredJob":
        """Store every returned file under ``<out_dir>/<clip_id>/<job_id>/`` and record it.

        ``out_dir`` is the *results root* — ``$COSMOS_RESULTS`` or
        ``./cosmos-results`` when omitted.  Writes ``manifest.json`` (the
        server's listing) and ``job.json`` (request as submitted, timings,
        files with sizes and sha256, server status and expiry) next to the
        videos, verifies both against the server's listing and skips files
        that are already complete, so calling it twice is cheap and safe.

        The side-by-side viewer of the result is rendered to ``viewer_<layout>.mp4`` next to the
        videos unless ``viewer_video=False``; pass ``clip`` (the :class:`~carla_cosmos.Clip` this job
        was submitted from) when it does not sit next to the results root.
        """
        from .results import ResultStore

        return ResultStore(out_dir).save(self, names=names, verify=verify, progress=progress,
                                         clip=clip, viewer_video=viewer_video)


class Result:
    """Result manifest plus downloads."""

    def __init__(self, client: CosmosClient, job_id: str, manifest: ResultManifest) -> None:
        self.client = client
        self.job_id = job_id
        self.manifest = manifest

    @property
    def files(self) -> list[str]:
        return [f.name for f in self.manifest.files]

    def download(self, dest: str | Path, names: Iterable[str] | None = None) -> dict[str, Path]:
        """Download result files (all by default) into ``dest``; writes ``manifest.json`` too."""
        dest = Path(dest)
        dest.mkdir(parents=True, exist_ok=True)
        wanted = set(names) if names is not None else None
        out: dict[str, Path] = {}
        for f in self.manifest.files:
            if wanted is not None and f.name not in wanted:
                continue
            p = self.client.download_result_file(self.job_id, f.name, dest / f.name)
            if sha256_file(p) != f.sha256:
                raise CosmosError(0, f"downloaded {f.name} has a different sha256 than the manifest")
            out[f.name] = p
        (dest / "manifest.json").write_text(self.manifest.model_dump_json(indent=2))
        return out


# -- helpers --------------------------------------------------------------------------------------

def _resolve_mask_classes(specs: "Sequence[str | int] | None") -> tuple[int, ...]:
    """Class names/ids -> tag ids, as a :class:`CosmosError` on a bad name.

    ``carla_cosmos.mask`` is imported lazily: it needs the capture extras
    (numpy/OpenCV), which the plain client does not.
    """
    if not specs:
        return ()
    from . import mask as maskmod

    try:
        tags = maskmod.resolve_classes(specs)
    except maskmod.MaskError as exc:
        raise CosmosError(0, str(exc)) from exc
    if not tags:
        raise CosmosError(0, "mask_classes resolved to no class")
    return tags


def _mask_class_names(tags: tuple[int, ...]) -> list[str]:
    """Canonical names of resolved tag ids (recorded in the job request)."""
    if not tags:
        return []
    from . import mask as maskmod

    return maskmod.class_names(tags)


def _default_dilate() -> int:
    from . import mask as maskmod

    return maskmod.DEFAULT_DILATE


def _slug(view: str) -> str:
    """File-system-safe form of a camera name."""
    return view.replace(":", "_")


def sha256_file(path: str | Path) -> str:
    h = hashlib.sha256()
    with Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(CHUNK), b""):
            h.update(chunk)
    return h.hexdigest()


def zip_dir(directory: Path, dest: Path) -> Path:
    """Deterministic zip (sorted names, fixed timestamps) so equal scenes dedup to one blob."""
    with zipfile.ZipFile(dest, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for p in sorted(x for x in directory.rglob("*") if x.is_file()):
            info = zipfile.ZipInfo(str(p.relative_to(directory)), date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            zf.writestr(info, p.read_bytes())
    return dest


def _iter_file(f, chunk: int = CHUNK):
    while True:
        data = f.read(chunk)
        if not data:
            return
        yield data


def _read_token_file(path: str | None) -> str | None:
    if not path:
        return None
    try:
        return Path(path).read_text().strip() or None
    except OSError:
        return None
