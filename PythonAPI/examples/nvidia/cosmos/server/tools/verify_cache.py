"""verify_cache.py <artifacts.lock> <HF_HOME> <artifact_id>... — sha256-check every lock file present in a local HF cache.

Resolution follows prefetch.py: the pinned revision's snapshot first, then the sha256-named blob, then the same
path under any other revision (NVIDIA re-tags the same weight file across commits, so a file downloaded before a
lock refresh sits under the commit that was `main` then).  Only files that are nowhere in the cache count as
missing.
"""
import hashlib, json, sys
from pathlib import Path


def sha256_of(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as fh:
        for c in iter(lambda: fh.read(1 << 24), b""):
            h.update(c)
    return h.hexdigest()


def locate(rd: Path, revision: str, path: str, size: int, sha: str | None) -> tuple[Path | None, bool]:
    """(file, exact_revision) — None when the file is not in this cache at all."""
    snap = rd / "snapshots" / revision / path
    if snap.exists():
        return snap, True
    if sha and (rd / "blobs" / sha).exists():
        return rd / "blobs" / sha, False
    for other in (rd / "snapshots").glob(f"*/{path}") if (rd / "snapshots").exists() else []:
        if other.stat().st_size == size:
            return other, False
    return None, False


def main() -> int:
    lock = json.load(open(sys.argv[1]))
    cache = Path(sys.argv[2])
    ids = set(sys.argv[3:])
    bad = missing = ok = elsewhere = 0
    for a in lock["artifacts"]:
        if a["id"] not in ids:
            continue
        rd = cache / "hub" / ("models--" + a["repo"].replace("/", "--"))
        for f in a["files"]:
            p, exact = locate(rd, a["revision"], f["path"], f["size"], f["sha256"])
            if p is None:
                missing += 1
                print("MISSING", a["id"], f["path"])
                continue
            if p.stat().st_size != f["size"]:
                bad += 1
                print("BAD size", a["id"], f["path"])
                continue
            if f["sha256"] and sha256_of(p) != f["sha256"]:
                bad += 1
                print("BAD sha256", a["id"], f["path"], "->", p.resolve().name)
                continue
            ok += 1
            if not exact:
                elsewhere += 1
                print("ok (other revision)", a["id"], f["path"], "->", p.parent.name if p.parent.name != "blobs" else "blobs")
    print(f"ok {ok} ({elsewhere} under another revision), bad {bad}, missing {missing}")
    return 1 if (bad or missing) else 0


if __name__ == "__main__":
    raise SystemExit(main())
