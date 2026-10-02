"""Local persistence of job results — every file that comes back is kept on disk.

Layout (``results root`` = explicit path, ``$COSMOS_RESULTS`` or ``./cosmos-results``)::

    <results_root>/
      index.json                          every job this machine knows about
      <clip_id>/<job_id>/
        <view>.mp4 ...                    generated videos
        control_<hint>[_<view>].mp4       the control the model actually saw
        grid.mp4                          multi-view contact sheet (AV backend)
        viewer_grid.mp4 | viewer_single.mp4   the side-by-side viewer, as a video
        manifest.json                     the server's ResultManifest, verbatim
        job.json                          request as submitted, timings, files
                                          (size + sha256), server status, expiry

Usage::

    job = client.submit_clip(...)
    job.wait()
    stored = job.download()                 # or job.download("/data/cosmos-results")
    print(stored.directory, stored.videos)

Every download also gets a **viewer video**: the side-by-side layout of
``demos/viewer.py`` (input | control | result, or the per-camera grid) rendered
to ``viewer_<layout>.mp4`` next to the videos and recorded in ``job.json`` with
kind ``viewer``.  It is written by :mod:`carla_cosmos.viewer` through the same
compositor the interactive window uses and needs no display.  Turn it off with
``viewer_video=False`` (``--no-viewer-video`` on the CLI, ``COSMOS_VIEWER_VIDEO=0``
in the environment); it is skipped with a note when the clip the result was
generated from cannot be found, since the viewer needs its input RGB.

Downloads are verified against the server's own listing (size, then sha256) and
are idempotent: a second call re-checks the files already on disk and fetches
only what is missing or damaged.  Results on the server are garbage-collected
(``COSMOS_JOB_TTL_HOURS``, 168 h by default); the store records the expiry the
server publishes in ``/v1/status`` and warns when it is close.
"""

from __future__ import annotations

import json
import logging
import os
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import TYPE_CHECKING, Any, Callable, Iterable

from pydantic import BaseModel, Field

from .contracts import ResultManifest

if TYPE_CHECKING:  # pragma: no cover
    from .client import Job

log = logging.getLogger(__name__)

ENV_ROOT = "COSMOS_RESULTS"
DEFAULT_ROOT = "cosmos-results"
INDEX_NAME = "index.json"
JOB_JSON = "job.json"
MANIFEST_JSON = "manifest.json"
EXPIRY_WARN_HOURS = 24.0
"""Warn when a result on the server is deleted less than this many hours from now."""


def default_results_root(out: str | Path | None = None) -> Path:
    """``out``, else ``$COSMOS_RESULTS``, else ``./cosmos-results``."""
    return Path(out or os.environ.get(ENV_ROOT) or DEFAULT_ROOT).expanduser()


def parse_time(value: str | None) -> datetime | None:
    """Parse a server timestamp (``...Z``) — ``None`` when absent or unparseable."""
    if not value:
        return None
    try:
        dt = datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None
    return dt if dt.tzinfo else dt.replace(tzinfo=timezone.utc)


def _now() -> datetime:
    return datetime.now(timezone.utc)


class StoredFile(BaseModel):
    """One file kept on disk, as the server described it."""

    name: str
    size: int
    sha256: str
    kind: str = "video"
    view: str | None = None


class StoredJob(BaseModel):
    """``job.json``: what was asked for, what came back and where it lives."""

    format_version: int = 1
    job_id: str
    clip_id: str
    backend: str
    server: str
    """URL of the server the result came from."""
    status: str
    error: str | None = None
    worker: str | None = None
    prompt: str = ""
    seed: int = 0
    resolution: str | None = None
    views: list[str] = Field(default_factory=list)
    controls: dict[str, Any] = Field(default_factory=dict)
    """``ControlInput`` of every control, as submitted (blob ids, weights, masks, ...)."""
    request: dict[str, Any] = Field(default_factory=dict)
    """The whole :class:`~carla_cosmos.contracts.JobRequest`, verbatim."""
    timings: dict[str, float] = Field(default_factory=dict)
    created: str | None = None
    started: str | None = None
    finished: str | None = None
    downloaded: str = Field(default_factory=lambda: _now().isoformat())
    directory: str = ""
    files: list[StoredFile] = Field(default_factory=list)
    complete: bool = True
    """Every file of the server's listing is on disk with the right size and sha256."""
    retention_hours: float | None = None
    server_expires: str | None = None
    """When the server deletes its copy (``None``: the server publishes no TTL)."""

    @property
    def path(self) -> Path:
        return Path(self.directory)

    @property
    def videos(self) -> list[str]:
        """The videos the server returned — not the locally rendered viewer video."""
        return [f.name for f in self.files if f.name.endswith(".mp4") and f.kind != "viewer"]

    @property
    def viewer_video(self) -> str | None:
        """``viewer_grid.mp4`` / ``viewer_single.mp4``: the side-by-side viewer, rendered here."""
        return next((f.name for f in self.files if f.kind == "viewer"), None)

    @property
    def bytes(self) -> int:
        return sum(f.size for f in self.files)

    def paths(self) -> dict[str, Path]:
        """``file name -> path on disk``."""
        return {f.name: self.path / f.name for f in self.files}

    def expires_in_hours(self, now: datetime | None = None) -> float | None:
        """Hours until the server deletes its copy (negative once past)."""
        expires = parse_time(self.server_expires)
        if expires is None:
            return None
        return (expires - (now or _now())).total_seconds() / 3600.0


class IndexEntry(BaseModel):
    """One line of ``index.json`` — enough for ``carla-cosmos jobs`` without any I/O."""

    job_id: str
    clip_id: str
    backend: str
    server: str = ""
    status: str = "unknown"
    prompt: str = ""
    directory: str | None = None
    """Where the files are (absolute); ``None`` while nothing has been downloaded."""
    stored: bool = False
    files: int = 0
    bytes: int = 0
    submitted: str | None = None
    downloaded: str | None = None
    server_expires: str | None = None

    def expires_in_hours(self, now: datetime | None = None) -> float | None:
        expires = parse_time(self.server_expires)
        if expires is None:
            return None
        return (expires - (now or _now())).total_seconds() / 3600.0


class ResultStore:
    """The local results directory and its index."""

    def __init__(self, root: str | Path | None = None) -> None:
        self.root = default_results_root(root)

    def __repr__(self) -> str:  # pragma: no cover - debugging aid
        return f"ResultStore({str(self.root)!r})"

    # -- layout ---------------------------------------------------------------------------
    @property
    def index_path(self) -> Path:
        return self.root / INDEX_NAME

    def job_dir(self, clip_id: str, job_id: str) -> Path:
        return self.root / clip_id / job_id

    # -- writing ---------------------------------------------------------------------------
    def save(self, job: "Job", *, names: Iterable[str] | None = None, verify: bool = True,
             progress: Callable[[str, int, int], None] | None = None,
             warn_hours: float = EXPIRY_WARN_HOURS, clip=None,
             viewer_video: bool | None = None) -> StoredJob:
        """Download every file of a finished job (skipping files already complete).

        Raises :class:`~carla_cosmos.client.JobFailed` when the job did not
        finish successfully — there is nothing to store then.
        """
        from .client import JobFailed  # local: client imports this module

        info = job.info if job.info.terminal else job.refresh()
        if info.status != "done":
            raise JobFailed(info)
        result = job.result()
        manifest = result.manifest
        return self.save_manifest(job.client, manifest, info=info, names=names, verify=verify,
                                  progress=progress, warn_hours=warn_hours, clip=clip,
                                  viewer_video=viewer_video)

    def save_manifest(self, client, manifest: ResultManifest, info=None, *, names: Iterable[str] | None = None,
                      verify: bool = True, progress: Callable[[str, int, int], None] | None = None,
                      warn_hours: float = EXPIRY_WARN_HOURS, clip=None,
                      viewer_video: bool | None = None) -> StoredJob:
        """The download half of :meth:`save`, for a manifest already in hand."""
        from .client import CosmosError, sha256_file

        clip_id = manifest.clip.clip_id
        dest = self.job_dir(clip_id, manifest.job_id)
        dest.mkdir(parents=True, exist_ok=True)
        wanted = set(names) if names is not None else None
        selected = [f for f in manifest.files if wanted is None or f.name in wanted]
        stored: list[StoredFile] = []
        complete = wanted is None
        for i, f in enumerate(selected, 1):
            path = dest / _safe_name(f.name)
            path.parent.mkdir(parents=True, exist_ok=True)
            if _on_disk(path, f.size, f.sha256, verify):
                log.debug("keeping %s (already complete)", path)
            else:
                tmp = path.with_name(path.name + ".part")
                client.download_result_file(manifest.job_id, f.name, tmp)
                size = tmp.stat().st_size
                if size != f.size:
                    tmp.unlink(missing_ok=True)
                    raise CosmosError(0, f"{f.name}: server sent {size} bytes, its listing says {f.size}")
                if verify and sha256_file(tmp) != f.sha256:
                    tmp.unlink(missing_ok=True)
                    raise CosmosError(0, f"{f.name}: sha256 differs from the server's listing")
                os.replace(tmp, path)
                log.info("stored %s (%.1f MB)", path, f.size / 1e6)
            stored.append(StoredFile(name=f.name, size=f.size, sha256=f.sha256, kind=f.kind, view=f.view))
            if progress:
                progress(f.name, i, len(selected))
        (dest / MANIFEST_JSON).write_text(manifest.model_dump_json(indent=2))
        viewer = self.write_viewer_video(dest, manifest, clip=clip, enabled=viewer_video)
        if viewer is not None:
            stored.append(viewer)

        retention = _retention_hours(client)
        finished = getattr(info, "finished", None) or manifest.created
        expires = parse_time(finished)
        expires = (expires + timedelta(hours=retention)).isoformat() if (expires and retention) else None
        req = manifest.request
        job_record = StoredJob(
            job_id=manifest.job_id, clip_id=clip_id, backend=manifest.backend,
            server=getattr(client, "url", ""), status=getattr(info, "status", "done"),
            error=getattr(info, "error", None), worker=manifest.worker,
            prompt=req.prompt, seed=req.seed, resolution=req.resolution,
            views=list(req.views), controls={k: v.model_dump(mode="json") for k, v in req.controls.items()},
            request=req.model_dump(mode="json"), timings=dict(manifest.timings),
            created=getattr(info, "created", None), started=getattr(info, "started", None), finished=finished,
            directory=str(dest.resolve()), files=stored, complete=complete,
            retention_hours=retention, server_expires=expires,
        )
        (dest / JOB_JSON).write_text(job_record.model_dump_json(indent=2))
        self.upsert(IndexEntry(
            job_id=job_record.job_id, clip_id=clip_id, backend=job_record.backend, server=job_record.server,
            status=job_record.status, prompt=job_record.prompt, directory=job_record.directory, stored=complete,
            files=len(stored), bytes=job_record.bytes, downloaded=job_record.downloaded,
            server_expires=expires))
        left = job_record.expires_in_hours()
        if left is not None and left < warn_hours:
            log.warning("the server copy of job %s expires in %.1f h — the local copy in %s is the only one after that",
                        job_record.job_id, left, dest)
        return job_record

    def write_viewer_video(self, dest: Path, manifest: ResultManifest, clip=None,
                           enabled: bool | None = None, force: bool = False) -> StoredFile | None:
        """Render the side-by-side viewer of this result to ``<dest>/viewer_<layout>.mp4``.

        Returns the :class:`StoredFile` to record in ``job.json`` (kind ``viewer``), or ``None`` when
        the export is turned off, the clip cannot be found, or pygame/OpenCV/ffmpeg are not available.
        A missing viewer video never fails a download — the generated videos are the result.
        """
        from .client import sha256_file

        try:
            from .viewer import VIEWER_KIND, viewer_video_enabled, write_viewer_video
        except ImportError as exc:  # no pygame / no OpenCV: the videos are stored either way
            log.info("no viewer video: %s (install the 'viewer' and 'capture' extras)", exc)
            return None
        if not viewer_video_enabled(enabled):
            return None
        try:
            path = write_viewer_video(dest, clip=clip, manifest=manifest, results_root=self.root, force=force)
        except Exception as exc:  # noqa: BLE001 - a rendering problem must not lose the download
            log.warning("viewer video for job %s not written: %s", manifest.job_id, exc)
            return None
        if path is None:
            return None
        return StoredFile(name=path.name, size=path.stat().st_size, sha256=sha256_file(path), kind=VIEWER_KIND)

    def note_submitted(self, job: "Job", clip_id: str | None = None) -> IndexEntry:
        """Record a job we have submitted but not downloaded yet."""
        entry = IndexEntry(job_id=job.id, clip_id=clip_id or "", backend=job.info.backend,
                           server=getattr(job.client, "url", ""), status=job.info.status,
                           submitted=job.info.created)
        return self.upsert(entry)

    def upsert(self, entry: IndexEntry) -> IndexEntry:
        """Merge one entry into ``index.json`` (kept only-grows-more-accurate)."""
        entries = {e.job_id: e for e in self.index()}
        old = entries.get(entry.job_id)
        if old is not None:
            merged = old.model_dump()
            merged.update({k: v for k, v in entry.model_dump().items() if v not in (None, "", 0, False)})
            entry = IndexEntry.model_validate(merged)
        entries[entry.job_id] = entry
        self._write_index(entries.values())
        return entry

    def _write_index(self, entries: Iterable[IndexEntry]) -> Path:
        self.root.mkdir(parents=True, exist_ok=True)
        body = {"format_version": 1,
                "jobs": [e.model_dump() for e in sorted(entries, key=lambda e: e.job_id)]}
        tmp = self.index_path.with_name(INDEX_NAME + ".tmp")
        tmp.write_text(json.dumps(body, indent=2))
        os.replace(tmp, self.index_path)
        return self.index_path

    # -- reading ---------------------------------------------------------------------------
    def index(self) -> list[IndexEntry]:
        """Every job this store knows about (empty when there is no index yet)."""
        try:
            body = json.loads(self.index_path.read_text())
        except (OSError, ValueError):
            return []
        out = []
        for row in body.get("jobs", []):
            try:
                out.append(IndexEntry.model_validate(row))
            except ValueError:  # a row from a newer/older client: skip it, do not lose the file
                log.debug("ignoring unreadable index row %r", row)
        return out

    def get(self, job_id: str) -> IndexEntry | None:
        return next((e for e in self.index() if e.job_id == job_id), None)

    def load(self, job_id: str) -> StoredJob | None:
        """Read the ``job.json`` of a stored job."""
        entry = self.get(job_id)
        if entry is None or not entry.directory:
            return None
        try:
            return StoredJob.model_validate_json((Path(entry.directory) / JOB_JSON).read_text())
        except (OSError, ValueError):
            return None

    def rebuild_index(self) -> list[IndexEntry]:
        """Re-derive the index from the ``job.json`` files on disk."""
        entries: dict[str, IndexEntry] = {e.job_id: e for e in self.index()}
        for jj in sorted(self.root.glob("*/*/" + JOB_JSON)):
            try:
                rec = StoredJob.model_validate_json(jj.read_text())
            except (OSError, ValueError):
                continue
            entries[rec.job_id] = IndexEntry(
                job_id=rec.job_id, clip_id=rec.clip_id, backend=rec.backend, server=rec.server, status=rec.status,
                prompt=rec.prompt, directory=str(jj.parent.resolve()), stored=rec.complete, files=len(rec.files),
                bytes=rec.bytes, downloaded=rec.downloaded, server_expires=rec.server_expires)
        self._write_index(entries.values())
        return list(entries.values())


# -- helpers ------------------------------------------------------------------------------------

def _safe_name(name: str) -> str:
    """Reject result names that would escape the job directory."""
    p = Path(name)
    if p.is_absolute() or ".." in p.parts:
        raise ValueError(f"refusing result file name '{name}'")
    return name


def _on_disk(path: Path, size: int, sha256: str, verify: bool) -> bool:
    """Is this file already there, whole and correct?"""
    from .client import sha256_file

    try:
        if path.stat().st_size != size:
            return False
    except OSError:
        return False
    return sha256_file(path) == sha256 if verify else True


def _retention_hours(client) -> float | None:
    """``job_ttl_hours`` published by the server, or ``None``."""
    try:
        retention = client.retention()
    except Exception:  # noqa: BLE001 - a status call must never break a download
        return None
    if not retention:
        return None
    ttl = retention.get("job_ttl_hours")
    return float(ttl) if isinstance(ttl, (int, float)) else None
