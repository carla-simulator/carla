"""Version-neutral TCP transport between CARLA Python 3.10 and Alpamayo Python 3.12."""

from __future__ import annotations

import json
import socket
import struct
from dataclasses import dataclass
from typing import Any, Mapping

import numpy as np


MAGIC = b"CA2S"
VERSION = 1
MAX_HEADER_BYTES = 1 << 20
MAX_ARRAY_BYTES = 1 << 30
_PREFIX = struct.Struct("!4sBI")


class ProtocolError(RuntimeError):
    pass


@dataclass(frozen=True)
class WireMessage:
    kind: str
    metadata: dict[str, Any]
    arrays: dict[str, np.ndarray]


def _recv_exact(sock: socket.socket, size: int) -> bytes:
    payload = bytearray(size)
    view = memoryview(payload)
    offset = 0
    while offset < size:
        received = sock.recv_into(view[offset:])
        if received == 0:
            raise EOFError("peer closed the connection")
        offset += received
    return bytes(payload)


def send_message(
    sock: socket.socket,
    kind: str,
    metadata: Mapping[str, Any] | None = None,
    arrays: Mapping[str, np.ndarray] | None = None,
) -> None:
    ordered_arrays: list[np.ndarray] = []
    array_records: list[dict[str, Any]] = []
    for name, value in (arrays or {}).items():
        if not isinstance(name, str) or not name:
            raise ValueError("array names must be non-empty strings")
        array = np.ascontiguousarray(value)
        if array.dtype.hasobject:
            raise ValueError(f"object dtype is not supported for {name}")
        if array.nbytes > MAX_ARRAY_BYTES:
            raise ValueError(f"array {name} exceeds the payload limit")
        ordered_arrays.append(array)
        array_records.append(
            {
                "name": name,
                "dtype": array.dtype.str,
                "shape": list(array.shape),
                "nbytes": array.nbytes,
            }
        )
    header = json.dumps(
        {"kind": kind, "metadata": dict(metadata or {}), "arrays": array_records},
        separators=(",", ":"),
    ).encode("utf-8")
    if len(header) > MAX_HEADER_BYTES:
        raise ValueError("message header exceeds the protocol limit")
    sock.sendall(_PREFIX.pack(MAGIC, VERSION, len(header)))
    sock.sendall(header)
    for array in ordered_arrays:
        sock.sendall(memoryview(array).cast("B"))


def recv_message(sock: socket.socket) -> WireMessage:
    magic, version, header_size = _PREFIX.unpack(_recv_exact(sock, _PREFIX.size))
    if magic != MAGIC:
        raise ProtocolError("invalid protocol magic")
    if version != VERSION:
        raise ProtocolError(f"unsupported protocol version {version}")
    if header_size > MAX_HEADER_BYTES:
        raise ProtocolError("header exceeds the protocol limit")
    try:
        header = json.loads(_recv_exact(sock, header_size))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ProtocolError("invalid JSON header") from exc
    if not isinstance(header, dict) or not isinstance(header.get("kind"), str):
        raise ProtocolError("header must contain a message kind")
    metadata = header.get("metadata", {})
    records = header.get("arrays", [])
    if not isinstance(metadata, dict) or not isinstance(records, list):
        raise ProtocolError("invalid metadata or array table")
    result: dict[str, np.ndarray] = {}
    for record in records:
        try:
            name = record["name"]
            dtype = np.dtype(record["dtype"])
            shape = tuple(int(size) for size in record["shape"])
            nbytes = int(record["nbytes"])
        except (KeyError, TypeError, ValueError) as exc:
            raise ProtocolError("malformed array record") from exc
        if not isinstance(name, str) or not name or name in result:
            raise ProtocolError("array names must be unique non-empty strings")
        expected_size = int(dtype.itemsize * np.prod(shape, dtype=np.int64))
        if nbytes != expected_size or nbytes < 0 or nbytes > MAX_ARRAY_BYTES:
            raise ProtocolError(f"invalid payload size for {name}")
        result[name] = np.frombuffer(_recv_exact(sock, nbytes), dtype=dtype).reshape(shape)
    return WireMessage(header["kind"], metadata, result)


def connect(host: str, port: int, timeout: float = 30.0) -> socket.socket:
    sock = socket.create_connection((host, port), timeout=timeout)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    sock.settimeout(None)
    return sock
