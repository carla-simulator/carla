"""Runtime profiles: which workers to start, on which GPUs, in which venv.

A profile is a YAML file in ``profiles/``::

    name: nano-1gpu
    description: Cosmos 3 Nano on one 80 GB GPU
    priority: 20              # auto-selection order (lower first)
    match: {min_gpus: 1, max_gpus: 1, min_memory_gib: 40}   # omit for manual-only
    workers:
      - name: cosmos3-nano
        type: cosmos3          # see WORKER_TYPES
        backends: [cosmos3-nano]
        gpus: [0]
        args: ["--tp", "1"]

``COSMOS_PROFILE=auto`` picks the lowest-priority profile whose ``match``
accepts the detected GPUs; a name picks that file.
"""

from __future__ import annotations

import logging
import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

import yaml

log = logging.getLogger(__name__)


@dataclass(frozen=True)
class WorkerType:
    module: str
    venv: str
    implemented: bool = True


WORKER_TYPES: dict[str, WorkerType] = {
    "mock": WorkerType("cosmos_workers.mock", "api"),
    "cosmos3": WorkerType("cosmos_workers.cosmos3", "cosmos3"),
    "transfer25": WorkerType("cosmos_workers.transfer25", "transfer25"),
    "wsm_renderer": WorkerType("cosmos_workers.wsm_renderer", "transfer25", implemented=False),  # Phase 5
    "transfer25_av": WorkerType("cosmos_workers.transfer25_av", "transfer25", implemented=False),  # Phase 6
}


@dataclass
class GpuInfo:
    index: int
    name: str
    memory_mib: int


@dataclass
class WorkerSpec:
    name: str
    type: str
    backends: list[str]
    gpus: list[int] = field(default_factory=list)
    args: list[str] = field(default_factory=list)
    venv: str | None = None

    @property
    def worker_type(self) -> WorkerType:
        try:
            return WORKER_TYPES[self.type]
        except KeyError:
            raise ValueError(f"worker '{self.name}': unknown type '{self.type}' (known: {sorted(WORKER_TYPES)})")


@dataclass
class Profile:
    name: str
    description: str
    workers: list[WorkerSpec]
    priority: int = 50
    match: dict | None = None
    path: Path | None = None

    def accepts(self, gpus: list[GpuInfo]) -> bool:
        if self.match is None:
            return False
        n = len(gpus)
        if n < int(self.match.get("min_gpus", 0)):
            return False
        if "max_gpus" in self.match and n > int(self.match["max_gpus"]):
            return False
        min_mem = float(self.match.get("min_memory_gib", 0))
        if gpus and min(g.memory_mib for g in gpus) / 1024 < min_mem:
            return False
        return True

    def validate(self) -> list[str]:
        errors = []
        names = [w.name for w in self.workers]
        if len(set(names)) != len(names):
            errors.append("duplicate worker names")
        for w in self.workers:
            try:
                w.worker_type
            except ValueError as exc:
                errors.append(str(exc))
            if not w.backends:
                errors.append(f"worker '{w.name}' serves no backends")
        return errors


def load_profile(path: Path) -> Profile:
    data = yaml.safe_load(path.read_text()) or {}
    workers = [WorkerSpec(name=w["name"], type=w["type"], backends=list(w.get("backends", [])),
                          gpus=[int(g) for g in w.get("gpus", [])], args=[str(a) for a in w.get("args", [])],
                          venv=w.get("venv")) for w in data.get("workers", [])]
    return Profile(name=data.get("name", path.stem), description=data.get("description", ""), workers=workers,
                   priority=int(data.get("priority", 50)), match=data.get("match"), path=path)


def load_profiles(directory: Path) -> dict[str, Profile]:
    profiles: dict[str, Profile] = {}
    for p in sorted(directory.glob("*.yaml")):
        prof = load_profile(p)
        profiles[prof.name] = prof
    return profiles


def detect_gpus() -> list[GpuInfo]:
    """GPU inventory via ``nvidia-smi``; empty list when there is none."""
    if shutil.which("nvidia-smi") is None:
        return []
    try:
        out = subprocess.run(["nvidia-smi", "--query-gpu=index,name,memory.total", "--format=csv,noheader,nounits"],
                             capture_output=True, text=True, timeout=20, check=True).stdout
    except (subprocess.SubprocessError, OSError) as exc:
        log.warning("nvidia-smi failed (%s); assuming no GPUs", exc)
        return []
    gpus = []
    for line in out.strip().splitlines():
        parts = [p.strip() for p in line.split(",")]
        if len(parts) >= 3:
            try:
                gpus.append(GpuInfo(int(parts[0]), parts[1], int(float(parts[2]))))
            except ValueError:
                continue
    return gpus


def select_profile(profiles: dict[str, Profile], gpus: list[GpuInfo], requested: str = "auto") -> Profile:
    if requested != "auto":
        if requested not in profiles:
            raise ValueError(f"unknown profile '{requested}' (available: {sorted(profiles)})")
        return profiles[requested]
    candidates = sorted((p for p in profiles.values() if p.accepts(gpus)), key=lambda p: p.priority)
    if not candidates:
        raise ValueError(f"no profile matches {len(gpus)} GPU(s) "
                         f"{[g.name for g in gpus]}; pass COSMOS_PROFILE=<name> (available: {sorted(profiles)})")
    return candidates[0]
