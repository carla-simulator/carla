"""Pydantic models shared between the carla-cosmos client and server.

A :class:`BackendContract` describes what a generation backend accepts
(controls, camera views, fps, frame counts, resolution buckets).  The server
publishes its contracts on ``GET /v1/models``; the client validates a
:class:`JobRequest` against the contract and the :class:`ClipManifest` of the
clip it is about to upload, *before* moving any bytes.

The three built-in contracts encode the numbers measured in Phase 0
(``.omc/research/cosmos``):

=================  =====================================  =======  ============  ===================  ===========
backend            controls                               views    fps (src->m)  frames (at model fps) resolution
=================  =====================================  =======  ============  ===================  ===========
cosmos3-nano/super edge, blur, depth, seg, wsm             1        10/16/24/30   5..300; wsm 101*k     256/480/720
transfer2.5        edge, vis, depth, seg                   1        16 -> 16      93*k                  480/720
transfer2.5-av     hdmap_bbox (required)                   1..7     30 -> 10      29 + 28*(k-1)         720
=================  =====================================  =======  ============  ===================  ===========
"""

from __future__ import annotations

from datetime import datetime, timezone
from typing import Any, Literal

from pydantic import BaseModel, Field, field_validator

Family = Literal["cosmos3", "transfer2.5", "transfer2.5-av"]
Priority = Literal["interactive", "batch"]

AV_CAMERAS: tuple[str, ...] = (
    "camera:front:wide:120fov",
    "camera:cross:left:120fov",
    "camera:cross:right:120fov",
    "camera:rear:left:70fov",
    "camera:rear:right:70fov",
    "camera:rear:tele:30fov",
    "camera:front:tele:30fov",
)
"""The seven NVIDIA RDS-HQ camera names understood by the world-scenario renderer."""


def canonical_camera_name(name: str) -> str:
    """``camera:front:wide:120fov`` -> ``camera_front_wide_120fov`` (file-name form)."""
    return name.replace(":", "_")


# ----------------------------------------------------------------------------- contract pieces

class ControlSpec(BaseModel):
    """One control modality accepted by a backend."""

    name: str
    required: bool = False
    accepts_video: bool = True
    """The client may upload a pre-rendered control video."""
    accepts_scene: bool = False
    """The server can render the control from a ClipGT scene package."""
    server_derivable: bool = False
    """The server can derive the control from the RGB video (edge, blur/vis)."""
    weight_range: tuple[float, float] | None = (0.0, 1.0)


class FpsRule(BaseModel):
    """Accepted clip frame rates and the rate the model consumes them at."""

    source: list[int]
    model: int | None = None
    """``None`` means the model runs at the source fps."""

    def model_fps(self, source_fps: int) -> int:
        """Frame rate at which frame-count rules are evaluated."""
        return self.model if self.model is not None else source_fps


class FrameRule(BaseModel):
    """Valid frame counts ``n`` at model fps.

    ``n`` is valid iff ``min <= n <= max`` and, when ``step`` is set,
    ``n >= base and (n - base) % step == 0``.  A rule with ``when_control`` only
    applies while that control is active (e.g. Cosmos 3 ``wsm`` -> ``101*k``).
    """

    min: int = 1
    max: int | None = None
    base: int | None = None
    step: int | None = None
    when_control: str | None = None

    def allows(self, n: int) -> bool:
        """Whether ``n`` frames satisfy the rule."""
        if n < self.min or (self.max is not None and n > self.max):
            return False
        if self.step is not None:
            base = self.base if self.base is not None else 0
            return n >= base and (n - base) % self.step == 0
        return True

    def describe(self) -> str:
        """Human-readable form used in validation errors."""
        if self.step is not None:
            base = self.base if self.base is not None else 0
            form = f"{base} + {self.step}*k" if base else f"{self.step}*k"
        else:
            form = "any"
        span = f"{self.min}..{self.max}" if self.max is not None else f">={self.min}"
        return f"{form} within {span}"

    def nearest(self, n: int, count: int = 2) -> list[int]:
        """Closest valid counts around ``n`` (for error messages)."""
        lo, hi = n, n
        found: list[int] = []
        limit = (self.max or n + 1000) + 1
        while len(found) < count and (lo >= self.min or hi < limit):
            if lo >= self.min and self.allows(lo) and lo not in found:
                found.append(lo)
            if hi < limit and self.allows(hi) and hi not in found:
                found.append(hi)
            lo -= 1
            hi += 1
        return sorted(found)


class BackendContract(BaseModel):
    """What a backend accepts.  Served by ``GET /v1/models``."""

    id: str
    family: Family
    controls: list[ControlSpec]
    views: list[str] = Field(default_factory=list)
    """Allowed camera names.  Empty means one view with any camera name."""
    max_views: int = 1
    fps: FpsRule
    frames: list[FrameRule]
    resolutions: list[str]
    rgb_required: bool = True
    """Whether the backend needs the RGB video (Transfer 2.5 general does)."""
    description: str = ""

    def control(self, name: str) -> ControlSpec | None:
        """Look up a control by name."""
        return next((c for c in self.controls if c.name == name), None)

    def frame_rule(self, active_controls: set[str]) -> FrameRule:
        """Rule in force for the given controls (control-specific rule wins)."""
        for rule in self.frames:
            if rule.when_control is not None and rule.when_control in active_controls:
                return rule
        return next(r for r in self.frames if r.when_control is None)


# ----------------------------------------------------------------------------- built-in contracts

def _cosmos3(id_: str, description: str) -> BackendContract:
    return BackendContract(
        id=id_,
        family="cosmos3",
        description=description,
        controls=[
            ControlSpec(name="edge", server_derivable=True),
            ControlSpec(name="blur", server_derivable=True),
            ControlSpec(name="depth"),
            ControlSpec(name="seg"),
            ControlSpec(name="wsm", accepts_scene=True),
        ],
        fps=FpsRule(source=[10, 16, 24, 30]),
        frames=[
            FrameRule(min=101, max=303, base=0, step=101, when_control="wsm"),
            FrameRule(min=5, max=300),
        ],
        resolutions=["256", "480", "720"],
        rgb_required=False,
    )


COSMOS3_NANO = _cosmos3("cosmos3-nano", "Cosmos 3 Nano, transfer mode (vLLM-Omni)")
COSMOS3_SUPER = _cosmos3("cosmos3-super", "Cosmos 3 Super, transfer mode (vLLM-Omni)")

TRANSFER25 = BackendContract(
    id="transfer2.5",
    family="transfer2.5",
    description="Cosmos Transfer 2.5-2B general (4-branch multicontrol)",
    controls=[
        ControlSpec(name="edge", server_derivable=True),
        ControlSpec(name="vis", server_derivable=True),
        ControlSpec(name="depth", server_derivable=True),
        ControlSpec(name="seg", server_derivable=True),
    ],
    fps=FpsRule(source=[16], model=16),
    frames=[FrameRule(min=93, base=0, step=93)],
    resolutions=["480", "720"],
    rgb_required=True,
)

TRANSFER25_AV = BackendContract(
    id="transfer2.5-av",
    family="transfer2.5-av",
    description="Cosmos Transfer 2.5-2B auto/multiview (7-camera world scenario)",
    controls=[
        ControlSpec(name="hdmap_bbox", required=True, accepts_scene=True, weight_range=(0.0, 1.0)),
    ],
    views=list(AV_CAMERAS),
    max_views=7,
    fps=FpsRule(source=[30], model=10),
    frames=[FrameRule(min=29, base=29, step=28)],
    resolutions=["720"],
    rgb_required=False,
)

BUILTIN_CONTRACTS: dict[str, BackendContract] = {
    c.id: c for c in (COSMOS3_NANO, COSMOS3_SUPER, TRANSFER25, TRANSFER25_AV)
}


# ----------------------------------------------------------------------------- clip manifest

class RecorderInfo(BaseModel):
    """Recorder log a clip was captured from (deterministic replays)."""

    log: str
    start: float
    duration: float


class CameraManifest(BaseModel):
    """A camera as actually mounted on the ego (after the mounting rule)."""

    name: str
    hfov: float
    width: int
    height: int
    lens: str = "pinhole"
    t_flu: list[float]
    """Actual position, FLU metres, relative to the rear-axle-on-ground origin."""
    rpy_flu: list[float]
    """Actual roll-pitch-yaw, degrees, FLU ``xyz``."""
    attach_ue: dict[str, float]
    """Attach transform relative to the ego actor origin (UE: x, y, z, pitch, yaw, roll)."""
    shifted: bool = False
    """Whether the mounting rule moved the camera out of the ego body."""


class RigManifest(BaseModel):
    """Rig as mounted."""

    name: str
    mount: str
    cameras: list[CameraManifest]


class ClipManifest(BaseModel):
    """``manifest.json`` of a captured clip."""

    format_version: int = 1
    clip_id: str
    carla_version: str
    map: str
    weather: dict[str, float]
    rig: RigManifest
    fps: int
    frames: int
    ego_id: int
    ego_type_id: str
    rear_axle_offset_ue: list[float]
    """Rear-axle-on-ground point in the ego actor frame (UE metres)."""
    aovs: list[str]
    videos: dict[str, str]
    """``"<kind>/<camera_canonical>" -> file name`` (kind in rgb, depth, seg, edge)."""
    scene_dir: str | None = None
    recorder: RecorderInfo | None = None
    seed: int | None = None
    created: str = Field(default_factory=lambda: datetime.now(timezone.utc).isoformat())

    @property
    def camera_names(self) -> list[str]:
        """Camera names (colon form) present in the clip."""
        return [c.name for c in self.rig.cameras]

    def video(self, kind: str, camera: str) -> str | None:
        """File name of a video, or ``None`` if the clip has none."""
        return self.videos.get(f"{kind}/{canonical_camera_name(camera)}")


# ----------------------------------------------------------------------------- job request

class ControlInput(BaseModel):
    """How one control is supplied for a job."""

    blob: str | None = None
    """Blob id of an uploaded control video (single-view jobs)."""
    blobs: dict[str, str] = Field(default_factory=dict)
    """``camera name -> blob id`` of uploaded control videos (multi-view jobs)."""
    scene: str | None = None
    """Blob id of an uploaded scene package (rendered server-side)."""
    derive: bool = False
    """Ask the server to derive the control from the RGB video."""
    weight: float | None = None

    def sources(self) -> int:
        """Number of sources given (exactly one is valid)."""
        return int(self.blob is not None or bool(self.blobs)) + int(self.scene is not None) + int(self.derive)

    def blob_for(self, view: str) -> str | None:
        """Blob id supplying this control for ``view``."""
        return self.blobs.get(view, self.blob)


class JobRequest(BaseModel):
    """A generation job as submitted to ``POST /v1/jobs``."""

    backend: str
    prompt: str
    negative_prompt: str | None = None
    seed: int = 0
    guidance: float | None = None
    """Text CFG scale (backend default when ``None``)."""
    num_steps: int | None = None
    resolution: str | None = None
    """Resolution bucket from the contract (``"480"``, ``"720"``...); backend default when ``None``."""
    priority: Priority = "interactive"
    controls: dict[str, ControlInput] = Field(default_factory=dict)
    views: list[str] = Field(default_factory=list)
    """Camera names (colon form).  Empty means the clip's single camera."""
    rgb: dict[str, str] = Field(default_factory=dict)
    """``camera name -> blob id`` of RGB videos."""
    extra: dict[str, Any] = Field(default_factory=dict)
    """Backend pass-through (e.g. ``control_guidance`` for Cosmos 3)."""

    @field_validator("prompt")
    @classmethod
    def _prompt_not_empty(cls, v: str) -> str:
        if not v.strip():
            raise ValueError("prompt must not be empty (guardrails reject empty prompts)")
        return v


# ----------------------------------------------------------------------------- validation

def validate_request(contract: BackendContract, request: JobRequest,
                     manifest: ClipManifest) -> list[str]:
    """Check a request against a contract and the clip it refers to.

    Returns a list of actionable error strings; empty means valid.
    """
    errors: list[str] = []
    if request.backend != contract.id:
        errors.append(f"request targets backend '{request.backend}' but contract is '{contract.id}'")

    if request.resolution is not None and request.resolution not in contract.resolutions:
        errors.append(f"resolution '{request.resolution}' not offered by '{contract.id}' "
                      f"(accepted: {contract.resolutions})")
    errors += _validate_controls(contract, request)
    views = _validate_views(contract, request, manifest, errors)
    errors += _validate_control_views(request, views)
    errors += _validate_rgb(contract, request, views, manifest)
    errors += _validate_timing(contract, request, manifest)
    return errors


def _validate_controls(contract: BackendContract, request: JobRequest) -> list[str]:
    errors: list[str] = []
    known = {c.name for c in contract.controls}
    if not request.controls:
        errors.append(f"no controls given; '{contract.id}' accepts {sorted(known)}")
    for name, inp in request.controls.items():
        spec = contract.control(name)
        if spec is None:
            errors.append(f"control '{name}' is not supported by '{contract.id}' (supported: {sorted(known)})")
            continue
        if inp.sources() != 1:
            errors.append(f"control '{name}' needs exactly one of blob, scene or derive (got {inp.sources()})")
        if inp.scene is not None and not spec.accepts_scene:
            errors.append(f"control '{name}' cannot be rendered from a scene package on '{contract.id}'")
        if inp.blob is not None and not spec.accepts_video:
            errors.append(f"control '{name}' does not accept an uploaded video on '{contract.id}'")
        if inp.derive and not spec.server_derivable:
            errors.append(f"control '{name}' cannot be derived from RGB on '{contract.id}'; upload it")
        if inp.weight is not None:
            if spec.weight_range is None:
                errors.append(f"control '{name}' has no adjustable weight on '{contract.id}'")
            else:
                lo, hi = spec.weight_range
                if not lo <= inp.weight <= hi:
                    errors.append(f"control '{name}' weight {inp.weight} outside [{lo}, {hi}]")
    for spec in contract.controls:
        if spec.required and spec.name not in request.controls:
            errors.append(f"control '{spec.name}' is required by '{contract.id}'")
    return errors


def _validate_control_views(request: JobRequest, views: list[str]) -> list[str]:
    errors: list[str] = []
    for name, inp in request.controls.items():
        if inp.blobs:
            missing = [v for v in views if v not in inp.blobs]
            if missing:
                errors.append(f"control '{name}' has no blob for view(s) {missing}")
            extra = [v for v in inp.blobs if v not in views]
            if extra:
                errors.append(f"control '{name}' has blobs for view(s) {extra} that are not requested")
        elif inp.blob is not None and len(views) > 1:
            errors.append(f"control '{name}' uses a single blob for {len(views)} views; use per-view 'blobs'")
    return errors


def _validate_views(contract: BackendContract, request: JobRequest, manifest: ClipManifest,
                    errors: list[str]) -> list[str]:
    clip_cams = manifest.camera_names
    views = list(request.views)
    if not views:
        if len(clip_cams) == 1:
            views = clip_cams
        else:
            errors.append(f"request.views is empty but the clip has {len(clip_cams)} cameras; name the view(s)")
            return []
    if len(views) > contract.max_views:
        errors.append(f"{len(views)} views requested; '{contract.id}' accepts at most {contract.max_views}")
    if len(set(views)) != len(views):
        errors.append("duplicate camera names in request.views")
    for v in views:
        if contract.views and v not in contract.views:
            errors.append(f"view '{v}' is not one of the fixed cameras of '{contract.id}': {contract.views}")
        if v not in clip_cams:
            errors.append(f"view '{v}' is not in the clip (clip cameras: {clip_cams})")
    return views


def _validate_rgb(contract: BackendContract, request: JobRequest, views: list[str],
                  manifest: ClipManifest) -> list[str]:
    errors: list[str] = []
    needs_rgb = contract.rgb_required or any(c.derive for c in request.controls.values())
    for v in views:
        if v not in request.rgb:
            if needs_rgb:
                errors.append(f"RGB video for view '{v}' is required (backend needs RGB or a control is derived)")
        elif manifest.video("rgb", v) is None:
            errors.append(f"clip has no RGB video for view '{v}'")
    return errors


def _validate_timing(contract: BackendContract, request: JobRequest, manifest: ClipManifest) -> list[str]:
    errors: list[str] = []
    if manifest.fps not in contract.fps.source:
        errors.append(f"clip fps {manifest.fps} not accepted by '{contract.id}' (accepted: {contract.fps.source})")
        return errors
    model_fps = contract.fps.model_fps(manifest.fps)
    factor = manifest.fps // model_fps
    if manifest.fps % model_fps:
        errors.append(f"clip fps {manifest.fps} is not a multiple of model fps {model_fps}")
        return errors
    n_model = manifest.frames // factor
    rule = contract.frame_rule(set(request.controls))
    if not rule.allows(n_model):
        near = [n * factor for n in rule.nearest(n_model)]
        at = f" ({n_model} at {model_fps} fps)" if factor != 1 else ""
        errors.append(
            f"clip has {manifest.frames} frames{at}; '{contract.id}' needs {rule.describe()} "
            f"frames at {model_fps} fps"
            + (f" when '{rule.when_control}' is used" if rule.when_control else "")
            + (f"; nearest valid clip lengths: {near}" if near else "")
        )
    return errors


# ----------------------------------------------------------------------------- jobs (server <-> client)

JobStatus = Literal["queued", "preparing", "running", "done", "failed", "cancelled"]
TERMINAL_STATUSES: frozenset[str] = frozenset({"done", "failed", "cancelled"})


class JobSubmission(BaseModel):
    """Body of ``POST /v1/jobs``: the request plus the manifest of the clip it uses.

    The server re-runs :func:`validate_request` and checks that every blob id
    exists before queueing.
    """

    request: JobRequest
    manifest: ClipManifest


class JobInfo(BaseModel):
    """State of a job as returned by ``/v1/jobs``."""

    id: str
    backend: str
    status: JobStatus
    priority: Priority
    token_id: str
    created: str
    started: str | None = None
    finished: str | None = None
    progress: float = 0.0
    """0..1 while running."""
    message: str = ""
    """Last progress message from the worker (e.g. ``chunk 2/3``)."""
    error: str | None = None
    views: list[str] = Field(default_factory=list)
    worker: str | None = None
    queue_position: int | None = None
    """Jobs ahead of this one for the same backend (only while queued)."""

    @property
    def terminal(self) -> bool:
        """Whether the job will not change state any more."""
        return self.status in TERMINAL_STATUSES


class ResultFile(BaseModel):
    """One file of a job result."""

    name: str
    size: int
    sha256: str
    view: str | None = None
    kind: str = "video"
    """``video`` | ``json`` | ``other``."""


class ResultManifest(BaseModel):
    """``manifest.json`` written next to the generated videos of a job."""

    format_version: int = 1
    job_id: str
    backend: str
    worker: str
    request: JobRequest
    clip: ClipManifest
    files: list[ResultFile]
    worker_manifest: dict[str, Any] = Field(default_factory=dict)
    """Backend-specific facts: checkpoint sha256s, resolved native spec, chunking, ..."""
    timings: dict[str, float] = Field(default_factory=dict)
    """Seconds: ``queued``, ``preparing``, ``running``."""
    created: str = Field(default_factory=lambda: datetime.now(timezone.utc).isoformat())


class ModelInfo(BaseModel):
    """A backend as published by ``GET /v1/models``."""

    contract: BackendContract
    available: bool
    """A worker serving this backend is loaded and ready."""
    workers: list[str] = Field(default_factory=list)
    queued: int = 0
    scene_rendering: bool = False
    """The server can render ``accepts_scene`` controls from an uploaded scene package."""
