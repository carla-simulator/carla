"""Server settings, all from ``COSMOS_*`` environment variables (or CLI flags).

===========================  =====================  ===================================================
variable                     default                meaning
===========================  =====================  ===================================================
COSMOS_STATE                 /state                 tokens, SQLite, blobs, job inputs/results
COSMOS_HOST / COSMOS_PORT    0.0.0.0 / 8000         API bind address
COSMOS_PROFILE               auto                   runtime profile name or ``auto`` (GPU detection)
COSMOS_PROFILES_DIR          <package>/../profiles  where ``*.yaml`` profiles live
COSMOS_MODELS_DIR            /models                baked model artifacts (workers read this)
COSMOS_IMAGE_VARIANT         <detected>             which weights are baked in: ``nano`` | ``full`` |
                                                    ``none``; default reads ``<models>/hf/ARTIFACTS_IMAGE``,
                                                    then ``CARLA_COSMOS_IMAGE_VARIANT`` (set by the image)
COSMOS_GUARDRAILS            1                      pass ``0`` to disable model guardrails
COSMOS_TOKEN                 -                      bootstrap token accepted at start (scripts, CI)
COSMOS_BLOB_TTL_HOURS        72                     unreferenced blobs older than this are deleted
COSMOS_JOB_TTL_HOURS         168                    finished jobs (and results) older than this go
COSMOS_GC_INTERVAL_S         600                    garbage-collection period
COSMOS_LOG_LEVEL             INFO
COSMOS_RUN_DIR               <state>/run            worker sockets
COSMOS_VENV_<NAME>           /opt/venvs/<name>      python venv for a worker venv name (api,
                                                    transfer25, cosmos3); falls back to the
                                                    running interpreter when the path is missing
===========================  =====================  ===================================================
"""

from __future__ import annotations

import os
import sys
from dataclasses import dataclass, field
from pathlib import Path

PACKAGE_DIR = Path(__file__).resolve().parent
DEFAULT_PROFILES_DIR = PACKAGE_DIR.parent / "profiles"


# AF_UNIX socket paths are limited to ~108 bytes; leave room for "/<worker>.sock".
MAX_RUN_DIR_LEN = 80


def default_run_dir(state_dir: Path) -> Path:
    """``<state>/run`` unless that would make socket paths too long (dev checkouts)."""
    run = state_dir / "run"
    if len(str(run.resolve())) <= MAX_RUN_DIR_LEN:
        return run
    import hashlib
    import tempfile

    tag = hashlib.sha1(str(state_dir.resolve()).encode()).hexdigest()[:8]
    for base in (Path(tempfile.gettempdir()), Path("/tmp")):
        cand = base / f"cosmos-run-{tag}"
        if len(str(cand)) <= MAX_RUN_DIR_LEN:
            return cand
    return run  # will fail loudly at bind time


def _env(name: str, default: str) -> str:
    return os.environ.get(f"COSMOS_{name}", default)


def detect_image_variant(models_dir: Path) -> str:
    """Which weights this image carries: ``nano``, ``full`` or ``none``.

    The marker written by ``prefetch.py`` / ``compose_image.py`` is the truth (a ``-nomodels``
    image says ``none`` even though its ``CARLA_COSMOS_IMAGE_VARIANT`` env says ``nano``); outside
    the image the env var, then ``none``.
    """
    marker = models_dir / "hf" / "ARTIFACTS_IMAGE"
    try:
        value = marker.read_text().strip()
    except OSError:
        value = os.environ.get("CARLA_COSMOS_IMAGE_VARIANT", "").strip()
    return value if value in ("nano", "full") else "none"


def _env_bool(name: str, default: bool) -> bool:
    v = os.environ.get(f"COSMOS_{name}")
    if v is None:
        return default
    return v.strip().lower() not in ("0", "false", "no", "off", "")


@dataclass
class Settings:
    state_dir: Path = field(default_factory=lambda: Path(_env("STATE", "/state")))
    host: str = field(default_factory=lambda: _env("HOST", "0.0.0.0"))
    port: int = field(default_factory=lambda: int(_env("PORT", "8000")))
    profile: str = field(default_factory=lambda: _env("PROFILE", "auto"))
    profiles_dir: Path = field(default_factory=lambda: Path(_env("PROFILES_DIR", str(DEFAULT_PROFILES_DIR))))
    models_dir: Path = field(default_factory=lambda: Path(_env("MODELS_DIR", "/models")))
    image_variant: str = field(default_factory=lambda: _env("IMAGE_VARIANT", ""))
    guardrails: bool = field(default_factory=lambda: _env_bool("GUARDRAILS", True))
    bootstrap_token: str | None = field(default_factory=lambda: os.environ.get("COSMOS_TOKEN") or None)
    blob_ttl_hours: float = field(default_factory=lambda: float(_env("BLOB_TTL_HOURS", "72")))
    job_ttl_hours: float = field(default_factory=lambda: float(_env("JOB_TTL_HOURS", "168")))
    gc_interval_s: float = field(default_factory=lambda: float(_env("GC_INTERVAL_S", "600")))
    log_level: str = field(default_factory=lambda: _env("LOG_LEVEL", "INFO"))
    run_dir: Path | None = field(default_factory=lambda: Path(v) if (v := os.environ.get("COSMOS_RUN_DIR")) else None)

    def __post_init__(self) -> None:
        if self.run_dir is None:
            self.run_dir = default_run_dir(self.state_dir)
        if not self.image_variant:
            self.image_variant = detect_image_variant(self.models_dir)

    # derived paths --------------------------------------------------------------
    @property
    def tokens_file(self) -> Path:
        return self.state_dir / "tokens.json"

    @property
    def initial_token_file(self) -> Path:
        return self.state_dir / "initial_token.txt"

    @property
    def db_file(self) -> Path:
        return self.state_dir / "cosmos.sqlite"

    @property
    def blobs_dir(self) -> Path:
        return self.state_dir / "blobs"

    @property
    def jobs_dir(self) -> Path:
        return self.state_dir / "jobs"

    def ensure_dirs(self) -> None:
        for d in (self.state_dir, self.blobs_dir, self.jobs_dir, self.run_dir):
            d.mkdir(parents=True, exist_ok=True)

    def venv_python(self, venv: str) -> str:
        """Interpreter for a worker venv; the current interpreter outside the image."""
        p = Path(os.environ.get(f"COSMOS_VENV_{venv.upper()}", f"/opt/venvs/{venv}")) / "bin" / "python"
        return str(p) if p.exists() else sys.executable
