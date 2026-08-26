"""Bearer-token authentication.

* Tokens look like ``cc_<id>_<secret>``; only ``sha256(secret)`` is stored in
  ``<state>/tokens.json`` together with the public ``id`` and a label.
* Verification is a dictionary lookup on ``id`` plus a constant-time compare of
  the digest, so timing does not leak which ids exist.
* The first boot with an empty store creates a token, prints it once to the
  log and writes it to ``<state>/initial_token.txt`` (mode 0600) so
  ``docker exec ... cat /state/initial_token.txt`` recovers it.
* The middleware is plain ASGI and rejects before the request body is read,
  so an unauthenticated multi-gigabyte upload costs nothing.
* There are deliberately **no** rate limits, quotas or per-token caps: a valid
  token is the only gate.  Management happens on the host through
  :mod:`carla_cosmos_server.tokens_cli`, never over HTTP.
"""

from __future__ import annotations

import hashlib
import hmac
import json
import logging
import os
import secrets
import time
from dataclasses import asdict, dataclass
from pathlib import Path

from starlette.types import ASGIApp, Receive, Scope, Send

log = logging.getLogger(__name__)

PREFIX = "cc"
PUBLIC_PATHS = frozenset({"/", "/ui", "/favicon.ico", "/v1/health/live", "/v1/health/ready"})
PUBLIC_PREFIXES = ("/ui/",)


@dataclass
class TokenRecord:
    id: str
    sha256: str
    label: str
    created: float
    last_used: float | None = None


class TokenStore:
    """``tokens.json`` on disk, in-memory index by id."""

    def __init__(self, path: Path) -> None:
        self.path = path
        self._by_id: dict[str, TokenRecord] = {}
        self._load()

    # persistence ----------------------------------------------------------------
    def _load(self) -> None:
        if not self.path.exists():
            return
        data = json.loads(self.path.read_text() or "{}")
        for rec in data.get("tokens", []):
            r = TokenRecord(**rec)
            self._by_id[r.id] = r

    def _save(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        tmp = self.path.with_suffix(".tmp")
        tmp.write_text(json.dumps({"tokens": [asdict(r) for r in self._by_id.values()]}, indent=2))
        os.chmod(tmp, 0o600)
        os.replace(tmp, self.path)

    # operations -------------------------------------------------------------------
    def __len__(self) -> int:
        return len(self._by_id)

    def list(self) -> list[TokenRecord]:
        return sorted(self._by_id.values(), key=lambda r: r.created)

    def create(self, label: str = "") -> str:
        """Mint a token; the raw value is returned once and never stored."""
        tid = secrets.token_hex(4)
        while tid in self._by_id:
            tid = secrets.token_hex(4)
        secret = secrets.token_urlsafe(32)
        self._by_id[tid] = TokenRecord(id=tid, sha256=_digest(secret), label=label, created=time.time())
        self._save()
        return f"{PREFIX}_{tid}_{secret}"

    def add_raw(self, token: str, label: str = "") -> str:
        """Register an externally chosen token (bootstrap via ``COSMOS_TOKEN``)."""
        tid, secret = _split(token)
        if tid is None:
            # Not in our format: derive a stable id from the whole value so that
            # any opaque string can be used as a bootstrap token.
            tid = hashlib.sha256(token.encode()).hexdigest()[:8]
            secret = token
        if tid in self._by_id and hmac.compare_digest(self._by_id[tid].sha256, _digest(secret)):
            return tid
        self._by_id[tid] = TokenRecord(id=tid, sha256=_digest(secret), label=label, created=time.time())
        self._save()
        return tid

    def revoke(self, tid: str) -> bool:
        if tid in self._by_id:
            del self._by_id[tid]
            self._save()
            return True
        return False

    def verify(self, token: str) -> str | None:
        """Token id if valid, else ``None``.  Constant-time on the secret."""
        tid, secret = _split(token)
        if tid is None:
            tid, secret = hashlib.sha256(token.encode()).hexdigest()[:8], token
        rec = self._by_id.get(tid)
        # Always hash so a missing id costs the same as a wrong secret.
        digest = _digest(secret)
        if rec is None or not hmac.compare_digest(rec.sha256, digest):
            return None
        rec.last_used = time.time()
        return tid

    def flush_usage(self) -> None:
        """Persist ``last_used`` (called by the GC loop, not on every request)."""
        if self._by_id:
            self._save()


def _digest(secret: str) -> str:
    return hashlib.sha256(secret.encode()).hexdigest()


def _split(token: str) -> tuple[str | None, str]:
    parts = token.split("_", 2)
    if len(parts) == 3 and parts[0] == PREFIX and parts[1] and parts[2]:
        return parts[1], parts[2]
    return None, token


def bootstrap(store: TokenStore, initial_token_file: Path, env_token: str | None) -> None:
    """First-boot behaviour: honour ``COSMOS_TOKEN``, else mint and announce one."""
    if env_token:
        tid = store.add_raw(env_token, label="COSMOS_TOKEN")
        log.info("bootstrap token from COSMOS_TOKEN registered (id %s)", tid)
    if len(store) == 0:
        token = store.create(label="initial")
        initial_token_file.write_text(token + "\n")
        os.chmod(initial_token_file, 0o600)
        banner = "\n".join([
            "=" * 78,
            "  carla-cosmos: no tokens found, generated the initial API token:",
            f"      {token}",
            f"  It is also stored in {initial_token_file} (mode 0600).",
            "  Manage tokens with: carla-cosmos-tokens {list,add,revoke}",
            "=" * 78,
        ])
        log.warning("%s", banner)
        print(banner, flush=True)


class BearerAuthMiddleware:
    """Reject unauthenticated requests before the body is read."""

    def __init__(self, app: ASGIApp, store: TokenStore) -> None:
        self.app = app
        self.store = store

    async def __call__(self, scope: Scope, receive: Receive, send: Send) -> None:
        if scope["type"] != "http":
            await self.app(scope, receive, send)
            return
        path = scope.get("path", "")
        if path in PUBLIC_PATHS or path.startswith(PUBLIC_PREFIXES):
            await self.app(scope, receive, send)
            return
        token = _bearer(scope)
        tid = self.store.verify(token) if token else None
        if tid is None:
            await _reject(send, "missing or invalid bearer token" if token else "missing bearer token")
            return
        scope.setdefault("state", {})["token_id"] = tid
        await self.app(scope, receive, send)


def _bearer(scope: Scope) -> str | None:
    for name, value in scope.get("headers", []):
        if name == b"authorization":
            text = value.decode("latin-1")
            scheme, _, rest = text.partition(" ")
            if scheme.lower() == "bearer" and rest.strip():
                return rest.strip()
            return None
    return None


async def _reject(send: Send, detail: str) -> None:
    body = json.dumps({"detail": detail}).encode()
    await send({
        "type": "http.response.start",
        "status": 401,
        "headers": [
            (b"content-type", b"application/json"),
            (b"content-length", str(len(body)).encode()),
            (b"www-authenticate", b"Bearer"),
        ],
    })
    await send({"type": "http.response.body", "body": body})
