"""Populate the image's Hugging Face cache from ``artifacts.lock`` (build stage).

    python prefetch.py --image nano --dest /models/hf [--from-cache /mnt/hf-cache]... [--no-hash]
    python prefetch.py --image nano --plan --max-layer-gb 40      # shard plan for tools/bake_weights.sh

For every file of every artifact in the requested image: reuse it from a local
HF cache mirror when given (same snapshot layout; matched by sha256/blob id,
hard-linked or copied), otherwise ``hf_hub_download`` at the pinned revision
(``HF_TOKEN`` needed for gated repos).  Sizes are always verified, sha256 by
default.  The result is a normal HF cache (``hub/models--*/snapshots/<rev>/…``)
that every runtime resolves offline: vLLM-Omni via ``HF_HUB_OFFLINE``,
cosmos-transfer2.5 via its ``hf download --revision <sha>`` calls (see
``tools/uvx_shim.py``).

``--plan`` prints a JSON list of shards (``[[artifact_id, [paths...]], ...]``) that
partitions the image's files into groups of at most ``--max-layer-gb``; ``--select``
places only the files of one such shard.  ``tools/bake_weights.sh`` commits one
image layer per shard on hosts without room for the BuildKit path.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import sys
import time
from pathlib import Path

LOCK = Path(__file__).resolve().parent / "artifacts.lock"


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(8 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def repo_dir(cache: Path, repo: str) -> Path:
    return cache / "hub" / ("models--" + repo.replace("/", "--"))


def find_in_cache(cache: Path, repo: str, revision: str, path: str, sha256: str | None, size: int) -> Path | None:
    rd = repo_dir(cache, repo)
    snap = rd / "snapshots" / revision / path
    if snap.exists() and snap.stat().st_size == size:
        return snap
    if sha256:
        blob = rd / "blobs" / sha256
        if blob.exists() and blob.stat().st_size == size:
            return blob
    # same file under another revision (identical content, different commit)
    for other in (rd / "snapshots").glob(f"*/{path}") if (rd / "snapshots").exists() else []:
        if other.stat().st_size == size and (sha256 is None or sha256_of(other) == sha256):
            return other
    return None


def place(dest: Path, repo: str, revision: str, path: str, src: Path, sha256: str | None, blob_id: str | None) -> Path:
    """Copy/link ``src`` into ``dest`` as ``blobs/<etag>`` + ``snapshots/<rev>/<path>``."""
    rd = repo_dir(dest, repo)
    etag = sha256 or blob_id or sha256_of(src)
    blob = rd / "blobs" / etag
    blob.parent.mkdir(parents=True, exist_ok=True)
    if not blob.exists():
        src = src.resolve()
        try:
            os.link(src, blob)
        except OSError:
            shutil.copyfile(src, blob)
    target = rd / "snapshots" / revision / path
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.is_symlink() or target.exists():
        target.unlink()
    target.symlink_to(os.path.relpath(blob, target.parent))
    return target


def plan_shards(lock: dict, wanted: set[str], max_bytes: int) -> list[list]:
    """Greedy partition of (artifact, file) pairs into shards of at most ``max_bytes``.

    Artifacts stay in lock order and are never split across shards unless one
    artifact alone exceeds ``max_bytes``; a single file larger than the limit gets
    its own shard.
    """
    shards: list[list] = []
    cur: list[list] = []
    cur_bytes = 0

    def flush() -> None:
        nonlocal cur, cur_bytes
        if cur:
            shards.append(cur)
        cur, cur_bytes = [], 0

    for art in lock["artifacts"]:
        if art["id"] not in wanted:
            continue
        if cur_bytes and cur_bytes + art["bytes"] > max_bytes:
            flush()
        if art["bytes"] <= max_bytes:
            cur.append([art["id"], [f["path"] for f in art["files"]]])
            cur_bytes += art["bytes"]
            continue
        paths: list[str] = []
        for f in sorted(art["files"], key=lambda f: f["size"], reverse=True):
            if cur_bytes and cur_bytes + f["size"] > max_bytes:
                if paths:
                    cur.append([art["id"], paths])
                    paths = []
                flush()
            paths.append(f["path"])
            cur_bytes += f["size"]
        if paths:
            cur.append([art["id"], paths])
    flush()
    return shards


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--image", choices=["nano", "full"], required=True)
    p.add_argument("--dest", default=os.environ.get("HF_HOME", "/models/hf"))
    p.add_argument("--from-cache", action="append", default=[], metavar="HF_HOME",
                   help="local HF cache to reuse files from (repeatable; searched in order)")
    p.add_argument("--lock", default=str(LOCK))
    p.add_argument("--no-hash", action="store_true", help="verify sizes only")
    p.add_argument("--only", default=None, help="comma-separated artifact ids (debugging)")
    p.add_argument("--plan", action="store_true", help="print the shard plan as JSON and exit")
    p.add_argument("--max-layer-gb", type=float, default=40.0, help="shard size for --plan")
    p.add_argument("--select", default=None, metavar="SHARD_JSON",
                   help="place only this shard ([[artifact_id, [paths]], ...]); no ARTIFACTS_IMAGE marker")
    p.add_argument("--finalize", action="store_true", help="with --select: also write the ARTIFACTS_IMAGE marker")
    p.add_argument("--no-download", action="store_true", help="fail on files missing from --dest and the caches")
    args = p.parse_args()

    lock = json.loads(Path(args.lock).read_text())
    dest = Path(args.dest)
    src_caches = [Path(c) for c in args.from_cache]
    wanted = set(lock["images"][args.image]["artifacts"])
    if args.only:
        wanted &= set(args.only.split(","))
    if args.plan:
        print(json.dumps(plan_shards(lock, wanted, int(args.max_layer_gb * 1e9))))
        return 0
    selected: dict[str, set[str]] | None = None
    if args.select:
        shard = json.loads(Path(args.select).read_text()) if os.path.exists(args.select) else json.loads(args.select)
        selected = {aid: set(paths) for aid, paths in shard}
        wanted &= set(selected)
    token = os.environ.get("HF_TOKEN")
    total = downloaded = reused = 0
    t0 = time.time()
    for art in lock["artifacts"]:
        if art["id"] not in wanted:
            continue
        repo, rev = art["repo"], art["revision"]
        print(f"== {art['id']} ({repo} @ {rev[:10]}, {art['bytes'] / 1e9:.2f} GB, {len(art['files'])} files)", flush=True)
        for f in art["files"]:
            path, size, sha = f["path"], f["size"], f["sha256"]
            if selected is not None and path not in selected[art["id"]]:
                continue
            total += size
            existing = repo_dir(dest, repo) / "snapshots" / rev / path
            if existing.exists() and existing.stat().st_size == size:
                if args.no_hash or sha is None or sha256_of(existing) == sha:
                    reused += size
                    continue
            src = next((c for c in (find_in_cache(sc, repo, rev, path, sha, size) for sc in src_caches) if c), None)
            if src is not None:
                if sha and not args.no_hash and src.name != sha and sha256_of(src) != sha:
                    print(f"   cache copy of {path} has a different sha256; downloading", flush=True)
                    src = None
            if src is not None:
                place(dest, repo, rev, path, src, sha, f.get("blob_id"))
                reused += size
                print(f"   reused     {path} ({size / 1e6:.1f} MB)", flush=True)
                continue
            if args.no_download:
                raise SystemExit(f"{repo}/{path} @ {rev[:10]} missing from {dest} and the given caches")
            from huggingface_hub import hf_hub_download

            got = Path(hf_hub_download(repo, path, revision=rev, cache_dir=dest / "hub", token=token))
            if got.stat().st_size != size:
                raise SystemExit(f"{repo}/{path}: size {got.stat().st_size} != lock {size}")
            if sha and not args.no_hash and sha256_of(got) != sha:
                raise SystemExit(f"{repo}/{path}: sha256 mismatch against artifacts.lock")
            downloaded += size
            print(f"   downloaded {path} ({size / 1e6:.1f} MB)", flush=True)
        # `main` ref so plain repo ids resolve offline too
        refs = repo_dir(dest, repo) / "refs"
        refs.mkdir(parents=True, exist_ok=True)
        if art.get("requested_revision", "main") == "main" or not (refs / "main").exists():
            (refs / "main").write_text(rev)
    print(f"done: {total / 1e9:.1f} GB total, {reused / 1e9:.1f} GB reused, {downloaded / 1e9:.1f} GB downloaded "
          f"in {time.time() - t0:.0f}s -> {dest}")
    if selected is None or args.finalize:
        (dest / "ARTIFACTS_IMAGE").write_text(args.image + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
