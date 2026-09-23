"""Runtime profiles: which workers to start, on which GPUs, in which venv.

``COSMOS_PROFILE=auto`` (the default) **plans** a layout from the detected GPUs
(count and memory of each, ``nvidia-smi``), the weights baked into the image
(``nano`` / ``full`` / ``none``) and the server **mode** (see :func:`plan_layout`):

* ``throughput`` — one worker per model on disjoint GPUs (several queries run at once);
* ``latency`` — every worker that can scale gets *all* suitable GPUs and the workers
  time-share them (one query at a time per GPU, but that query uses every GPU:
  Cosmos 3 CFG-parallel + Ulysses, Transfer 2.5 context-parallel);
* ``auto`` (default) — ``latency`` on hosts with <= :data:`AUTO_LATENCY_MAX_GPUS` GPUs.

With no GPU the ``mock`` profile is picked (the only YAML with a ``match`` block).

A YAML file in ``profiles/`` is a manual layout selected by name
(``COSMOS_PROFILE=<name>`` / ``--profile <name>``)::

    name: my-node
    description: Cosmos 3 Nano on GPU 0, Transfer 2.5 + renderer on GPU 1
    workers:
      - name: cosmos3-nano
        type: cosmos3          # see WORKER_TYPES
        backends: [cosmos3-nano]
        gpus: [0]              # becomes CUDA_VISIBLE_DEVICES
        args: ["--tp", "1"]

A YAML may also carry ``match: {min_gpus, max_gpus, min_memory_gib}`` and a
``priority``; such profiles are tried before the planner and the lowest
priority that accepts the GPUs wins (that is how ``mock`` claims 0-GPU hosts).
Manual YAML layouts are what they say (``mode`` ``manual``): the server mode does
not touch them.
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
    "wsm_renderer": WorkerType("cosmos_workers.wsm_renderer", "transfer25"),
    "transfer25_av": WorkerType("cosmos_workers.transfer25_av", "transfer25"),
}


@dataclass
class GpuInfo:
    index: int
    name: str
    memory_mib: int


PARALLEL_ARGS = {"--tp": "tp", "--cfg-parallel": "cfg", "--ulysses": "ulysses",
                 "--context-parallel-size": "context", "--nproc": "nproc"}
"""Worker CLI flags that describe how one query is spread over the worker's GPUs -> manifest key."""


def parallel_of(args: list[str]) -> dict[str, int]:
    """``{"cfg": 2, "ulysses": 2}`` / ``{"context": 4}`` / ``{"nproc": 4}`` from worker CLI args (>1 only)."""
    out: dict[str, int] = {}
    for flag, key in PARALLEL_ARGS.items():
        if flag in args:
            try:
                value = int(args[args.index(flag) + 1])
            except (IndexError, ValueError):
                continue
            if value > 1:
                out[key] = value
    return out


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

    @property
    def parallel(self) -> dict[str, int]:
        return parallel_of(self.args)


@dataclass
class Profile:
    name: str
    description: str
    workers: list[WorkerSpec]
    priority: int = 50
    match: dict | None = None
    path: Path | None = None
    mode: str = "manual"
    """``latency`` | ``throughput`` for planned layouts, ``manual`` for YAML profiles."""

    def shared_gpus(self) -> dict[int, list[str]]:
        """GPU index -> names of the model workers (renderer excluded) that share it; empty when disjoint."""
        by_gpu: dict[int, list[str]] = {}
        for w in self.workers:
            if w.type == "wsm_renderer":
                continue
            for g in w.gpus:
                by_gpu.setdefault(g, []).append(w.name)
        return {g: names for g, names in by_gpu.items() if len(names) > 1}

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
        ports: dict[str, str] = {}
        for w in self.workers:
            if "--master-port" in w.args:
                port = w.args[w.args.index("--master-port") + 1]
                if port in ports:
                    errors.append(f"workers '{ports[port]}' and '{w.name}' share torchrun --master-port {port}")
                ports[port] = w.name
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


# ----------------------------------------------------------------------------- automatic layout
MODES = ("latency", "throughput", "auto")
AUTO_LATENCY_MAX_GPUS = 4
"""``--mode auto`` picks ``latency`` on hosts with at most this many GPUs, ``throughput`` above."""

# Backends each image variant can serve (``none`` = a -nomodels / dev image: nothing but mock).
VARIANT_BACKENDS: dict[str, frozenset[str]] = {
    "none": frozenset(),
    "nano": frozenset({"cosmos3-nano", "transfer2.5", "transfer2.5-av", "wsm-renderer"}),
    "full": frozenset({"cosmos3-nano", "cosmos3-super", "transfer2.5", "transfer2.5-av", "wsm-renderer"}),
}

# Throughput-mode budgets: the card size a worker needs for itself, GiB (weights are bf16 sizes from
# artifacts.lock; the rest is what the engines add on top: KV cache / activations / VAE / guardrails).
NANO_GIB = 40.0            # Cosmos3-Nano 32.5 GiB weights + vLLM-Omni working set (nano-1gpu threshold)
SUPER_WEIGHTS_GIB = 123.5  # Cosmos3-Super, split across the TP ranks
SUPER_EXTRA_GIB = 16.0     # per rank on top of its weight shard
SUPER_TP_CHOICES = (1, 2, 4, 8)
VLLM_GPU_FRACTION = 0.9    # vLLM's default --gpu-memory-utilization; also the share of a GPU latency mode may fill
TRANSFER_720_GIB = 40.0    # Transfer 2.5 general at 720p (+ the wsm renderer beside it)
TRANSFER_480_GIB = 30.0    # ... at 480p: ~21 GiB static weights + activations (fits a 32 GB card)
AV_RANK_GIB = 40.0         # Transfer 2.5 auto/multiview, per torchrun rank
AV_MAX_RANKS = 8

# Latency-mode budgets, GiB per GPU.  Workers time-share GPUs, so what must fit is the *resident* set of every
# worker on the GPU (weights + engine baseline while idle) plus the *working set* of the one query running
# (activations; the largest wins).  Resident numbers measured on the 4x RTX PRO 6000 (96 GB) node, 2026-08-27;
# working sets are estimates — the job manifest's ``peak_memory_mb`` (Cosmos 3) is the place to read them back.
NANO_RESIDENT_GIB = 37.0
NANO_WORK_GIB = 8.0
TRANSFER_RESIDENT_GIB = 28.0       # general, four control branches, 720p
TRANSFER_WORK_GIB = 12.0
TRANSFER_480_RESIDENT_GIB = 21.0   # (docstring of the worker: ~21 GiB static weights at 480p)
TRANSFER_480_WORK_GIB = 7.0
AV_RESIDENT_GIB = 22.0             # one control branch instead of four (estimate: not measured yet)
AV_WORK_GIB = 12.0
NANO_PARALLEL_SIZES = (1, 2, 4, 8)  # cfg-parallel 2 x Ulysses N/2: powers of two only
TORCHRUN_PORTS = {"transfer25-av": 12341, "transfer25": 12342}


def _gib(g: GpuInfo) -> float:
    return g.memory_mib / 1024


def resolve_mode(mode: str, n_gpus: int) -> str:
    """``auto`` -> ``latency`` on small hosts (<= :data:`AUTO_LATENCY_MAX_GPUS` GPUs), else ``throughput``."""
    if mode not in MODES:
        raise ValueError(f"unknown mode '{mode}' (choose from {MODES})")
    if mode == "auto":
        return "latency" if n_gpus <= AUTO_LATENCY_MAX_GPUS else "throughput"
    return mode


def super_tp(memory_gib: float) -> int | None:
    """Smallest tensor-parallel size whose per-rank share of Cosmos3-Super fits ``memory_gib``."""
    usable = memory_gib * VLLM_GPU_FRACTION
    for tp in SUPER_TP_CHOICES:
        if SUPER_WEIGHTS_GIB / tp + SUPER_EXTRA_GIB <= usable:
            return tp
    return None


def nano_parallel_args(n: int) -> list[str]:
    """``--cfg-parallel 2 --ulysses N/2`` for ``n`` GPUs (power of two); nothing on one GPU."""
    args = ["--tp", "1"]
    if n >= 2:
        args += ["--cfg-parallel", "2"]
        if n // 2 > 1:
            args += ["--ulysses", str(n // 2)]
    return args


def _check_variant(gpus: list[GpuInfo], variant: str) -> frozenset[str]:
    backends = VARIANT_BACKENDS.get(variant, VARIANT_BACKENDS["none"])
    if not gpus:
        raise ValueError("no GPU detected: only the 'mock' profile can run here")
    if not backends:
        raise ValueError(f"this image carries no model weights (variant '{variant}'): only "
                         "COSMOS_PROFILE=mock or a custom profile can run here")
    return backends


def _profile_name(gpus: list[GpuInfo], variant: str, mode: str) -> str:
    mem = sorted({round(_gib(g)) for g in gpus})
    return f"auto-{len(gpus)}x{'/'.join(str(m) for m in mem)}g-{variant}" + ("-latency" if mode == "latency" else "")


def plan_layout(gpus: list[GpuInfo], variant: str, mode: str = "throughput") -> Profile:
    """Plan a worker layout for these GPUs, this image's weights and this mode.

    ``throughput`` (:func:`plan_throughput`): one worker per model on disjoint GPUs, largest GPUs
    first.  ``latency`` (:func:`plan_latency`): every worker that can scale spans all GPUs it fits
    on and the workers time-share them.  ``auto`` resolves with :func:`resolve_mode`.  A single
    GPU always gets the throughput layout (nothing to spread a query over).
    """
    mode = resolve_mode(mode, len(gpus))
    if mode == "latency" and len(gpus) > 1:
        return plan_latency(gpus, variant)
    prof = plan_throughput(gpus, variant)
    prof.mode = mode
    prof.name = _profile_name(gpus, variant, mode)
    return prof


def plan_throughput(gpus: list[GpuInfo], variant: str) -> Profile:
    """One worker per model on disjoint GPUs (several queries run at once).

    Largest GPUs first.  ``nano`` images allocate Cosmos 3 Nano (1 GPU ≥ NANO_GIB) → Transfer 2.5
    general with the wsm renderer beside it (1 GPU; 480p-only when the card is between
    TRANSFER_480_GIB and TRANSFER_720_GIB).  ``full`` images allocate Cosmos 3 Super first (the
    smallest TP whose weight shard fits one GPU — that is what the image was pulled for), then
    Transfer 2.5 + renderer, then Nano.  Whatever is left becomes Transfer 2.5 AV multiview over
    ≥ 2 GPUs (one rank per GPU, up to AV_MAX_RANKS); single leftovers become extra Transfer 2.5
    workers.  When no GPU is left for Transfer 2.5 the renderer shares the first Cosmos 3 GPU so
    ``wsm`` scene controls still work.

    Examples: 1×96 GB nano → Nano + renderer; 2×96 GB nano → Nano | Transfer 2.5 + renderer;
    4×96 GB full → Super TP=2 | Transfer 2.5 | Nano; 8×80 GB full → Super TP=4 | Transfer 2.5 |
    Nano | AV ×2; 8×96 GB nano → Nano | Transfer 2.5 | AV ×6; 1×32 GB → Transfer 2.5 @480p.
    """
    backends = _check_variant(gpus, variant)
    free = sorted(gpus, key=lambda g: (-g.memory_mib, g.index))
    workers: list[WorkerSpec] = []
    notes: list[str] = []

    def take(min_gib: float, count: int = 1) -> list[GpuInfo]:
        picked = [g for g in free if _gib(g) >= min_gib][:count]
        if len(picked) < count:
            return []
        for g in picked:
            free.remove(g)
        return picked

    def transfer_worker(name: str, g: GpuInfo) -> WorkerSpec:
        res = ["--default-resolution", "480"] if _gib(g) < TRANSFER_720_GIB else []
        return WorkerSpec(name=name, type="transfer25", backends=["transfer2.5"], gpus=[g.index], args=res)

    def add_super() -> None:
        if "cosmos3-super" not in backends or not free:
            return
        tp = super_tp(_gib(free[0]))
        if tp is None:
            notes.append("Super skipped: no GPU large enough for a TP shard")
            return
        ranks = take(SUPER_WEIGHTS_GIB / tp + SUPER_EXTRA_GIB, tp)
        if not ranks:
            notes.append(f"Super skipped: needs {tp} free GPU(s), {len(free)} left")
            return
        workers.append(WorkerSpec(name="cosmos3-super", type="cosmos3", backends=["cosmos3-super"],
                                  gpus=[g.index for g in ranks], args=["--tp", str(tp)]))
        notes.append(f"Super TP={tp} on GPUs {[g.index for g in ranks]}")

    def add_nano() -> None:
        nano = take(NANO_GIB) if "cosmos3-nano" in backends else []
        if nano:
            workers.append(WorkerSpec(name="cosmos3-nano", type="cosmos3", backends=["cosmos3-nano"],
                                      gpus=[nano[0].index], args=["--tp", "1"]))
            notes.append(f"Nano on GPU {nano[0].index}")

    def add_transfer() -> None:
        picked = take(TRANSFER_480_GIB) if "transfer2.5" in backends else []
        if picked:
            w = transfer_worker("transfer25", picked[0])
            workers.append(w)
            workers.append(WorkerSpec(name="wsm-renderer", type="wsm_renderer", backends=["wsm-renderer"], gpus=w.gpus))
            notes.append(f"Transfer 2.5{' @480p' if w.args else ''} + renderer on GPU {picked[0].index}")

    for step in ((add_super, add_transfer, add_nano) if variant == "full" else (add_nano, add_transfer)):
        step()

    if not any(w.type == "wsm_renderer" for w in workers):
        host = next((w for w in workers if w.type == "cosmos3"), None)
        if host is not None:
            workers.append(WorkerSpec(name="wsm-renderer", type="wsm_renderer", backends=["wsm-renderer"],
                                      gpus=[host.gpus[0]]))
            notes.append(f"renderer beside {host.name}")

    if "transfer2.5-av" in backends:
        av = [g for g in free if _gib(g) >= AV_RANK_GIB][:AV_MAX_RANKS]
        if len(av) >= 2:
            for g in av:
                free.remove(g)
            workers.append(WorkerSpec(name="transfer25-av", type="transfer25_av", backends=["transfer2.5-av"],
                                      gpus=[g.index for g in av], args=["--nproc", str(len(av))]))
            notes.append(f"AV multiview ×{len(av)} on GPUs {[g.index for g in av]}")

    extra = 1
    while "transfer2.5" in backends and any(w.type == "transfer25" for w in workers) \
            and free and _gib(free[0]) >= TRANSFER_480_GIB:
        g = take(TRANSFER_480_GIB)[0]
        extra += 1
        workers.append(transfer_worker(f"transfer25-{extra}", g))
        notes.append(f"extra Transfer 2.5 on GPU {g.index}")
    if free:
        notes.append(f"idle: GPUs {[g.index for g in free]} ({[round(_gib(g)) for g in free]} GiB)")

    if not workers:
        raise ValueError(f"no worker fits {len(gpus)} GPU(s) {[(g.name, round(_gib(g))) for g in gpus]}: "
                         f"smallest useful GPU is {TRANSFER_480_GIB:.0f} GiB; pass COSMOS_PROFILE=<name>")
    return Profile(name=_profile_name(gpus, variant, "throughput"), description="; ".join(notes), workers=workers,
                   priority=1000, match=None, mode="throughput")


def plan_latency(gpus: list[GpuInfo], variant: str) -> Profile:
    """Every worker spans all the GPUs it fits on; workers time-share GPUs (one query at a time, see the
    scheduler's GPU-overlap rule) so a single query gets the whole node.

    Allocation order — Super (``:full``, TP from :func:`super_tp` × CFG-parallel 2 when twice the ranks
    fit), Nano (CFG-parallel 2 × Ulysses N/2 on a power-of-two GPU count), Transfer 2.5 general
    (context parallel over every GPU it fits on; 480p budget when 720p does not fit), Transfer 2.5 AV
    (context parallel over every GPU it fits on, ≤ AV_MAX_RANKS), renderer on the last GPU.  A worker
    fits a GPU when the resident sets already there + its own + the largest working set stay within
    ``VLLM_GPU_FRACTION`` of the card (constants above); a worker that fits nowhere is skipped with a
    note.  GPUs are picked least-loaded first, largest first, so unequal cards degrade gracefully.
    """
    backends = _check_variant(gpus, variant)
    resident: dict[int, float] = {g.index: 0.0 for g in gpus}
    work: dict[int, float] = {g.index: 0.0 for g in gpus}
    workers: list[WorkerSpec] = []
    notes: list[str] = []

    def fits(g: GpuInfo, res: float, wrk: float) -> bool:
        return resident[g.index] + res + max(work[g.index], wrk) <= _gib(g) * VLLM_GPU_FRACTION

    def place(res: float, wrk: float, max_count: int, sizes: tuple[int, ...] | None = None,
              min_count: int = 1) -> list[int]:
        cand = sorted((g for g in gpus if fits(g, res, wrk)),
                      key=lambda g: (resident[g.index], -g.memory_mib, g.index))
        n = min(len(cand), max_count)
        if sizes is not None:
            n = max((s for s in sizes if s <= n), default=0)
        if n < min_count:
            return []
        chosen = sorted(cand[:n], key=lambda g: g.index)
        for g in chosen:
            resident[g.index] += res
            work[g.index] = max(work[g.index], wrk)
        return [g.index for g in chosen]

    def skipped(what: str, res: float, wrk: float) -> None:
        worst = max(gpus, key=lambda g: _gib(g) * VLLM_GPU_FRACTION - resident[g.index])
        notes.append(f"{what} skipped: {res:.0f}+{wrk:.0f} GiB do not fit beside "
                     f"{resident[worst.index]:.0f} GiB resident on GPU {worst.index} "
                     f"({_gib(worst) * VLLM_GPU_FRACTION:.0f} GiB usable)")

    if "cosmos3-super" in backends:
        tp = super_tp(max(_gib(g) for g in gpus))
        if tp is None:
            notes.append("Super skipped: no GPU large enough for a TP shard")
        else:
            shard, wrk = SUPER_WEIGHTS_GIB / tp, SUPER_EXTRA_GIB
            picked = place(shard, wrk, 2 * tp, sizes=(tp, 2 * tp), min_count=tp)
            if picked:
                args = ["--tp", str(tp)] + (["--cfg-parallel", "2"] if len(picked) == 2 * tp else [])
                workers.append(WorkerSpec(name="cosmos3-super", type="cosmos3", backends=["cosmos3-super"],
                                          gpus=picked, args=args))
                notes.append(f"Super TP={tp}{' x cfg-parallel 2' if len(picked) == 2 * tp else ''} on GPUs {picked}")
            else:
                skipped("Super", shard, wrk)

    if "cosmos3-nano" in backends:
        picked = place(NANO_RESIDENT_GIB, NANO_WORK_GIB, len(gpus), sizes=NANO_PARALLEL_SIZES)
        if picked:
            workers.append(WorkerSpec(name="cosmos3-nano", type="cosmos3", backends=["cosmos3-nano"],
                                      gpus=picked, args=nano_parallel_args(len(picked))))
            par = parallel_of(nano_parallel_args(len(picked)))
            notes.append(f"Nano{' ' + ' x '.join(f'{k} {v}' for k, v in par.items()) if par else ''} on GPUs {picked}")
        else:
            skipped("Nano", NANO_RESIDENT_GIB, NANO_WORK_GIB)

    if "transfer2.5" in backends:
        picked = place(TRANSFER_RESIDENT_GIB, TRANSFER_WORK_GIB, len(gpus))
        res_args: list[str] = []
        if not picked:
            picked = place(TRANSFER_480_RESIDENT_GIB, TRANSFER_480_WORK_GIB, len(gpus))
            res_args = ["--default-resolution", "480"]
        if picked:
            args = list(res_args)
            if len(picked) > 1:
                args += ["--context-parallel-size", str(len(picked)), "--master-port", str(TORCHRUN_PORTS["transfer25"])]
            workers.append(WorkerSpec(name="transfer25", type="transfer25", backends=["transfer2.5"], gpus=picked,
                                      args=args))
            notes.append(f"Transfer 2.5{' @480p' if res_args else ''} context-parallel x{len(picked)} on GPUs {picked}")
        else:
            skipped("Transfer 2.5", TRANSFER_480_RESIDENT_GIB, TRANSFER_480_WORK_GIB)

    if "transfer2.5-av" in backends:
        picked = place(AV_RESIDENT_GIB, AV_WORK_GIB, AV_MAX_RANKS)
        if picked:
            args = ["--nproc", str(len(picked))]
            if len(picked) > 1:
                args += ["--master-port", str(TORCHRUN_PORTS["transfer25-av"])]
            workers.append(WorkerSpec(name="transfer25-av", type="transfer25_av", backends=["transfer2.5-av"],
                                      gpus=picked, args=args))
            notes.append(f"AV multiview context-parallel x{len(picked)} on GPUs {picked}")
        else:
            skipped("AV multiview", AV_RESIDENT_GIB, AV_WORK_GIB)

    if not workers:
        raise ValueError(f"no worker fits {len(gpus)} GPU(s) {[(g.name, round(_gib(g))) for g in gpus]} in latency "
                         f"mode: {'; '.join(notes)}; try --mode throughput or COSMOS_PROFILE=<name>")
    last = max(i for w in workers for i in w.gpus)
    workers.append(WorkerSpec(name="wsm-renderer", type="wsm_renderer", backends=["wsm-renderer"], gpus=[last]))
    notes.append(f"renderer on GPU {last}")
    return Profile(name=_profile_name(gpus, variant, "latency"), description="; ".join(notes), workers=workers,
                   priority=1000, match=None, mode="latency")


def select_profile(profiles: dict[str, Profile], gpus: list[GpuInfo], requested: str = "auto",
                   variant: str = "nano", mode: str = "auto") -> Profile:
    """A named YAML profile, or for ``auto`` the first matching YAML (``mock`` on 0 GPUs) else a planned layout."""
    if requested != "auto":
        if requested not in profiles:
            raise ValueError(f"unknown profile '{requested}' (available: {sorted(profiles)})")
        return profiles[requested]
    candidates = sorted((p for p in profiles.values() if p.accepts(gpus)), key=lambda p: p.priority)
    if candidates:
        return candidates[0]
    try:
        return plan_layout(gpus, variant, mode)
    except ValueError as exc:
        raise ValueError(f"{exc} (available profiles: {sorted(profiles)})") from exc
