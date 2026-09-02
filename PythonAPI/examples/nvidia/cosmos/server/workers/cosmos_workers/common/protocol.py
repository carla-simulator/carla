"""Worker <-> server protocol: newline-delimited JSON over a unix socket.

The worker **binds** the socket; the server connects (one connection per
operation, so a ``cancel`` can arrive while a ``run`` is streaming).  Every
request carries ``op`` and ``id``; every reply carries the same ``id`` and an
``event``:

==========  ======================================  ==========================================
op          request fields                          reply events (terminal in bold)
==========  ======================================  ==========================================
hello       -                                       **result** {name, backends, state, version, pid}
status      -                                       **result** {state, job_id}
smoke       -                                       **result** {ok, seconds, error?}
run         job (see :class:`JobSpec`)              progress {fraction, message}...,
                                                    **done** {files, manifest} | **failed** {error}
cancel      job_id                                  **result** {cancelled}
shutdown    -                                       **result** {}
==========  ======================================  ==========================================

Worker states: ``loading`` -> ``ready`` <-> ``busy``; ``error`` is terminal.

The job handed to ``run`` (all paths are local to the worker's filesystem):

.. code-block:: python

    {
      "job_id": "j_...",
      "request": {...JobRequest...},
      "manifest": {...ClipManifest...},
      "views": ["camera:front:wide:120fov"],
      "inputs": {
        "rgb": {"camera:front:wide:120fov": "/state/jobs/j_.../inputs/rgb_camera_front_wide_120fov.mp4"},
        "controls": {
          "depth": {"path": "/state/jobs/.../inputs/depth_....mp4", "weight": 1.0},
          "wsm":   {"scene_dir": "/state/jobs/.../inputs/scene", "weight": null},
          "edge":  {"derive": true, "weight": 0.5}
        },
        "masks": {"camera:front:wide:120fov": "/state/jobs/.../inputs/mask_camera_front_wide_120fov.mp4"}
      },
      "out_dir": "/state/jobs/j_.../result"
    }

``done.files`` lists paths relative to ``out_dir`` with an optional ``view``
and ``kind``; ``done.manifest`` is free-form worker metadata (checkpoint
sha256s, resolved native spec, timings) that the server embeds in the result
manifest.
"""

from __future__ import annotations

import asyncio
import json
import os
from typing import Any, AsyncIterator

VERSION = 1
MAX_LINE = 64 * 1024 * 1024


class ProtocolError(RuntimeError):
    """Malformed or unexpected message."""


def encode(msg: dict[str, Any]) -> bytes:
    """One JSON object per line."""
    return (json.dumps(msg, separators=(",", ":")) + "\n").encode()


def decode(line: bytes) -> dict[str, Any]:
    """Parse one line; raises :class:`ProtocolError` on garbage."""
    try:
        msg = json.loads(line)
    except json.JSONDecodeError as exc:
        raise ProtocolError(f"bad JSON line: {exc}") from exc
    if not isinstance(msg, dict):
        raise ProtocolError("message is not an object")
    return msg


async def read_message(reader: asyncio.StreamReader) -> dict[str, Any] | None:
    """Next message, or ``None`` at EOF."""
    try:
        line = await reader.readline()
    except (asyncio.LimitOverrunError, ValueError) as exc:
        raise ProtocolError(f"line too long: {exc}") from exc
    if not line:
        return None
    return decode(line)


async def open_connection(path: str) -> tuple[asyncio.StreamReader, asyncio.StreamWriter]:
    """Connect to a worker socket."""
    return await asyncio.open_unix_connection(path, limit=MAX_LINE)


async def call(path: str, op: str, **fields: Any) -> AsyncIterator[dict[str, Any]]:
    """Send one request and yield replies until the terminal event.

    Closes the connection afterwards.  Raises :class:`ProtocolError` if the
    worker closes the connection before a terminal event.
    """
    reader, writer = await open_connection(path)
    try:
        writer.write(encode({"op": op, "id": 1, **fields}))
        await writer.drain()
        while True:
            msg = await read_message(reader)
            if msg is None:
                raise ProtocolError(f"worker closed the connection during '{op}'")
            yield msg
            if msg.get("event") in TERMINAL_EVENTS:
                return
    finally:
        writer.close()
        try:
            await writer.wait_closed()
        except (ConnectionError, OSError):
            pass


async def call_result(path: str, op: str, **fields: Any) -> dict[str, Any]:
    """Convenience for single-reply ops: returns the ``result`` payload."""
    stream = call(path, op, **fields)
    try:
        async for msg in stream:
            event = msg.get("event")
            if event == "result":
                return msg
            if event == "failed":
                raise ProtocolError(msg.get("error", "worker reported failure"))
    finally:
        await stream.aclose()
    raise ProtocolError(f"no result for '{op}'")


TERMINAL_EVENTS = frozenset({"result", "done", "failed"})


def socket_ready(path: str) -> bool:
    """Whether a worker socket file exists (not necessarily accepting yet)."""
    return os.path.exists(path)
