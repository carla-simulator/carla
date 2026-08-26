"""Tiny stdlib HTTP helpers (JSON + multipart) so worker adapters stay dependency-free."""

from __future__ import annotations

import json
import mimetypes
import os
import secrets
import tempfile
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any, Iterable


class HttpError(RuntimeError):
    def __init__(self, status: int, body: str, url: str) -> None:
        self.status = status
        self.body = body
        self.url = url
        detail = body
        try:
            j = json.loads(body)
            detail = j.get("detail") or j.get("error") or j.get("message") or body
            if isinstance(detail, dict):
                detail = detail.get("message") or json.dumps(detail)
        except (ValueError, AttributeError):
            pass
        super().__init__(f"HTTP {status} from {url}: {detail}")


def request(method: str, url: str, data: bytes | Any | None = None, headers: dict[str, str] | None = None,
            timeout: float = 60.0) -> tuple[int, dict[str, str], bytes]:
    req = urllib.request.Request(url, data=data, method=method, headers=headers or {})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return resp.status, {k.lower(): v for k, v in resp.headers.items()}, resp.read()
    except urllib.error.HTTPError as exc:
        body = exc.read().decode(errors="replace")
        raise HttpError(exc.code, body, url) from None


def get_json(url: str, timeout: float = 60.0) -> Any:
    _, _, body = request("GET", url, timeout=timeout)
    return json.loads(body) if body else None


def delete(url: str, timeout: float = 60.0) -> int:
    status, _, _ = request("DELETE", url, timeout=timeout)
    return status


def download(url: str, dest: Path, timeout: float = 600.0) -> Path:
    req = urllib.request.Request(url, method="GET")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp, dest.open("wb") as f:
            while True:
                chunk = resp.read(1 << 20)
                if not chunk:
                    break
                f.write(chunk)
    except urllib.error.HTTPError as exc:
        raise HttpError(exc.code, exc.read().decode(errors="replace"), url) from None
    return dest


def post_multipart(url: str, fields: dict[str, str], files: Iterable[tuple[str, Path, str | None]] = (),
                   timeout: float = 600.0, headers: dict[str, str] | None = None) -> tuple[int, dict[str, str], bytes]:
    """POST ``multipart/form-data`` spooled to a temp file (uploads can be hundreds of MB)."""
    boundary = "----carla-cosmos-" + secrets.token_hex(12)
    with tempfile.TemporaryFile() as body:
        for name, value in fields.items():
            body.write(f"--{boundary}\r\nContent-Disposition: form-data; name=\"{name}\"\r\n\r\n".encode())
            body.write(str(value).encode())
            body.write(b"\r\n")
        for name, path, ctype in files:
            ctype = ctype or mimetypes.guess_type(str(path))[0] or "application/octet-stream"
            body.write((f"--{boundary}\r\nContent-Disposition: form-data; name=\"{name}\"; "
                        f"filename=\"{path.name}\"\r\nContent-Type: {ctype}\r\n\r\n").encode())
            with path.open("rb") as f:
                while True:
                    chunk = f.read(1 << 20)
                    if not chunk:
                        break
                    body.write(chunk)
            body.write(b"\r\n")
        body.write(f"--{boundary}--\r\n".encode())
        size = body.tell()
        body.seek(0)
        hdrs = {"Content-Type": f"multipart/form-data; boundary={boundary}", "Content-Length": str(size),
                **(headers or {})}
        return request("POST", url, data=body, headers=hdrs, timeout=timeout)


def free_port() -> int:
    import socket

    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def env_flag(name: str, default: bool) -> bool:
    v = os.environ.get(name)
    if v is None:
        return default
    return v.strip().lower() not in ("0", "false", "no", "off", "")
