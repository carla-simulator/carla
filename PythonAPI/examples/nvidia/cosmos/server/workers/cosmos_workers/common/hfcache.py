"""Helpers for the baked Hugging Face cache (``$HF_HOME/hub``), stdlib only."""

from __future__ import annotations

import logging
import os
import shutil
from pathlib import Path

log = logging.getLogger("cosmos_worker.hfcache")


def snapshot_dir(hf_home: str | os.PathLike, repo_id: str, revision: str | None = None) -> Path | None:
    """``hub/models--<org>--<name>/snapshots/<rev>`` for a cached repo (the only snapshot when ``revision`` is None)."""
    snaps = Path(hf_home) / "hub" / ("models--" + repo_id.replace("/", "--")) / "snapshots"
    if revision:
        d = snaps / revision
        return d if d.is_dir() else None
    cands = sorted(p for p in snaps.glob("*") if p.is_dir()) if snaps.is_dir() else []
    return cands[-1] if cands else None


def materialize_snapshot(hf_home: str | os.PathLike, repo_id: str, revision: str | None = None) -> int:
    """Replace every symlink under a cached snapshot with a real copy of its blob; returns how many.

    An HF cache stores ``snapshots/<rev>/<path>`` as symlinks into ``blobs/``.  Loaders that open
    files with ``O_NOFOLLOW`` refuse those — NLTK's hardened ``pathsec`` (used by cosmos-guardrail
    for the blocklist's ``nltk_data``) is the case that matters here.  Meant for small artifacts
    (the guardrail repos are ~0.1 GiB); a no-op once done, and safe to call at every start.
    """
    snap = snapshot_dir(hf_home, repo_id, revision)
    if snap is None:
        return 0
    n = 0
    for link in sorted(p for p in snap.rglob("*") if p.is_symlink()):
        target = link.resolve()
        if not target.is_file():
            continue
        tmp = link.with_name(link.name + ".materialize")
        shutil.copyfile(target, tmp)
        os.replace(tmp, link)
        n += 1
    if n:
        log.info("materialized %d symlinked file(s) under %s", n, snap)
    return n
