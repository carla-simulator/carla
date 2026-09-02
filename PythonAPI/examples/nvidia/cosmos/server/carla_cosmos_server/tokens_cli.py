"""``carla-cosmos-tokens``: manage API tokens on the host (never over HTTP).

    carla-cosmos-tokens list
    carla-cosmos-tokens add --label "ci"
    carla-cosmos-tokens revoke <id>

Inside the container: ``docker exec <name> carla-cosmos-tokens list``.
"""

from __future__ import annotations

import argparse
import sys
import time

from .auth import TokenStore
from .config import Settings


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="carla-cosmos-tokens", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--state", default=None, help="state directory (default: $COSMOS_STATE or /state)")
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list", help="list token ids and labels")
    a = sub.add_parser("add", help="mint a new token (printed once)")
    a.add_argument("--label", default="")
    r = sub.add_parser("revoke", help="delete a token by id")
    r.add_argument("id")
    args = p.parse_args(argv)

    settings = Settings()
    if args.state:
        from pathlib import Path

        settings.state_dir = Path(args.state)
    store = TokenStore(settings.tokens_file)

    if args.cmd == "list":
        if not store.list():
            print("no tokens")
        for rec in store.list():
            used = time.strftime("%Y-%m-%d %H:%M", time.localtime(rec.last_used)) if rec.last_used else "never"
            print(f"{rec.id}  created {time.strftime('%Y-%m-%d', time.localtime(rec.created))}  "
                  f"last used {used}  {rec.label}")
    elif args.cmd == "add":
        print(store.create(label=args.label))
    elif args.cmd == "revoke":
        if not store.revoke(args.id):
            print(f"no token with id {args.id}", file=sys.stderr)
            return 1
        print(f"revoked {args.id}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
