"""Time-to-live garbage collection for blobs and finished jobs."""

from __future__ import annotations

import asyncio
import logging
import shutil
import time

from .auth import TokenStore
from .config import Settings
from .store import Store

log = logging.getLogger(__name__)


def collect(store: Store, settings: Settings) -> tuple[int, int]:
    """One pass.  Returns (blobs deleted, jobs deleted)."""
    now = time.time()
    n_blobs = 0
    for blob_id in store.expired_blobs(now - settings.blob_ttl_hours * 3600):
        store.delete_blob(blob_id)
        n_blobs += 1
    n_jobs = 0
    for job_id in store.expired_jobs(now - settings.job_ttl_hours * 3600):
        d = store.job_dir(job_id)
        if d.exists():
            shutil.rmtree(d, ignore_errors=True)
        store.delete_job(job_id)
        n_jobs += 1
    if n_blobs or n_jobs:
        log.info("gc: removed %d blob(s), %d job(s)", n_blobs, n_jobs)
    return n_blobs, n_jobs


async def gc_loop(store: Store, settings: Settings, tokens: TokenStore) -> None:
    while True:
        try:
            await asyncio.sleep(settings.gc_interval_s)
            await asyncio.to_thread(collect, store, settings)
            await asyncio.to_thread(tokens.flush_usage)
        except asyncio.CancelledError:
            raise
        except Exception:  # noqa: BLE001
            log.exception("gc pass failed")
