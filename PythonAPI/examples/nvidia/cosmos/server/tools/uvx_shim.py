#!/usr/bin/env python3
"""``uvx`` stand-in installed as ``/usr/local/bin/uvx`` in the image.

cosmos-transfer2.5 resolves every checkpoint with
``uvx 'hf>=1.3.5' download <repo> [<file>] --revision <sha> [--include ..] [--exclude ..] [--quiet]``
(``cosmos_transfer2/_src/imaginaire/utils/checkpoint_db.py``).  Inside the image
there is no network and no uv tool cache, so this shim answers those calls
straight from the baked HF cache with ``huggingface_hub`` (``local_files_only``)
and prints the local path like the real CLI does.  Anything else is forwarded
to the real ``uvx`` when one is installed (``UVX_REAL``, default
``/usr/local/bin/uvx.real``).
"""

from __future__ import annotations

import os
import sys


def _hf_download(argv: list[str]) -> int:
    from huggingface_hub import hf_hub_download, snapshot_download

    repo, filename, revision, repo_type = None, None, None, "model"
    include, exclude = [], []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--revision":
            revision = argv[i + 1]
            i += 2
        elif a == "--repo-type":
            repo_type = argv[i + 1]
            i += 2
        elif a in ("--include", "--exclude"):
            lst = include if a == "--include" else exclude
            i += 1
            while i < len(argv) and not argv[i].startswith("--"):
                lst.append(argv[i])
                i += 1
        elif a in ("--quiet", "-q"):
            i += 1
        elif a.startswith("--"):
            i += 2 if i + 1 < len(argv) and not argv[i + 1].startswith("--") else 1
        elif repo is None:
            repo = a
            i += 1
        else:
            filename = a
            i += 1
    if repo is None:
        print("uvx shim: hf download needs a repo id", file=sys.stderr)
        return 2
    offline = os.environ.get("HF_HUB_OFFLINE", "1") not in ("0", "false", "")
    kw = {"repo_id": repo, "revision": revision, "repo_type": repo_type, "local_files_only": offline}
    try:
        if filename:
            path = hf_hub_download(filename=filename, **kw)
        else:
            path = snapshot_download(allow_patterns=include or None, ignore_patterns=exclude or None, **kw)
    except Exception as exc:  # noqa: BLE001
        print(f"uvx shim: cannot resolve {repo}@{revision} {filename or ''} from the baked cache "
              f"(HF_HOME={os.environ.get('HF_HOME')}): {exc}", file=sys.stderr)
        return 1
    print(path)
    return 0


def main() -> int:
    argv = sys.argv[1:]
    if len(argv) >= 2 and (argv[0] == "hf" or argv[0].startswith("hf>") or argv[0].startswith("hf=")) \
            and argv[1] == "download":
        return _hf_download(argv[2:])
    real = os.environ.get("UVX_REAL", "/usr/local/bin/uvx.real")
    if os.path.exists(real):
        os.execv(real, [real, *argv])
    print(f"uvx shim: only 'hf download' is supported offline (got: {' '.join(argv[:3])} ...)", file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
