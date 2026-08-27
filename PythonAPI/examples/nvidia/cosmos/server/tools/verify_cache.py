"""verify_cache.py <artifacts.lock> <HF_HOME> <artifact_id>... — sha256-check every lock file present in a local HF cache."""
import hashlib, json, sys
from pathlib import Path
lock = json.load(open(sys.argv[1])); cache = Path(sys.argv[2]); ids = set(sys.argv[3:])
bad = missing = ok = 0
for a in lock["artifacts"]:
    if a["id"] not in ids: continue
    rd = cache / "hub" / ("models--" + a["repo"].replace("/", "--"))
    for f in a["files"]:
        p = rd / "snapshots" / a["revision"] / f["path"]
        if not p.exists(): missing += 1; continue
        if p.stat().st_size != f["size"]:
            bad += 1; print("BAD size", a["id"], f["path"]); continue
        if f["sha256"]:
            h = hashlib.sha256()
            with p.open("rb") as fh:
                for c in iter(lambda: fh.read(1 << 24), b""): h.update(c)
            if h.hexdigest() != f["sha256"]:
                bad += 1; print("BAD sha256", a["id"], f["path"], "->", p.resolve().name); continue
        ok += 1
print(f"ok {ok}, bad {bad}, missing {missing}")
