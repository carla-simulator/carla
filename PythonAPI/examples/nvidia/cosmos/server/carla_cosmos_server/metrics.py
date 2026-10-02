"""``GET /v1/metrics`` — Prometheus text exposition, hand-rolled (no extra dependency)."""

from __future__ import annotations

import time

from .scheduler import Scheduler
from .store import Store

_START = time.time()


def render(store: Store, scheduler: Scheduler) -> str:
    lines: list[str] = []

    def metric(name: str, help_: str, type_: str, samples: list[tuple[dict[str, str], float]]) -> None:
        lines.append(f"# HELP {name} {help_}")
        lines.append(f"# TYPE {name} {type_}")
        for labels, value in samples:
            lab = ",".join(f'{k}="{v}"' for k, v in labels.items())
            lines.append(f"{name}{{{lab}}} {value}" if lab else f"{name} {value}")

    counts = store.status_counts()
    metric("cosmos_jobs", "Jobs by status", "gauge",
           [({"status": s}, counts.get(s, 0)) for s in ("queued", "preparing", "running", "done", "failed", "cancelled")])
    metric("cosmos_queue_depth", "Queued jobs per backend", "gauge",
           [({"backend": b}, n) for b, n in sorted(store.queued_counts().items())] or [({"backend": "none"}, 0)])
    metric("cosmos_worker_state", "Worker state (1 = in this state)", "gauge",
           [({"worker": w.name, "state": w.state}, 1) for w in scheduler.workers])
    metric("cosmos_worker_busy", "Worker has a running job", "gauge",
           [({"worker": w.name}, 1 if w.current_job else 0) for w in scheduler.workers])
    n_blobs, n_bytes = store.blob_stats()
    metric("cosmos_blobs", "Stored blobs", "gauge", [({}, n_blobs)])
    metric("cosmos_blob_bytes", "Bytes in the blob store", "gauge", [({}, n_bytes)])
    metric("cosmos_ready", "All workers loaded and smoke-tested", "gauge", [({}, 1 if scheduler.is_ready() else 0)])
    metric("cosmos_uptime_seconds", "Seconds since the API started", "gauge", [({}, round(time.time() - _START, 1))])
    return "\n".join(lines) + "\n"
