"""SQLite-backed job and blob records under ``<state>/cosmos.sqlite``.

Files live next to it: blobs in ``blobs/<aa>/<sha256>``, job inputs and
results in ``jobs/<job_id>/{inputs,result}``.  The store is synchronous and
guarded by one lock; every call is a handful of row operations, which is fine
for the request rates of a generation service.
"""

from __future__ import annotations

import json
import secrets
import sqlite3
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

from carla_cosmos.contracts import JobInfo, JobRequest, TERMINAL_STATUSES

PRIORITY_RANK = {"interactive": 0, "batch": 1}

SCHEMA = """
CREATE TABLE IF NOT EXISTS blobs (
    id TEXT PRIMARY KEY,
    size INTEGER NOT NULL,
    content_type TEXT,
    filename TEXT,
    created REAL NOT NULL,
    last_used REAL NOT NULL
);
CREATE TABLE IF NOT EXISTS jobs (
    id TEXT PRIMARY KEY,
    token_id TEXT NOT NULL,
    backend TEXT NOT NULL,
    priority TEXT NOT NULL,
    priority_rank INTEGER NOT NULL,
    status TEXT NOT NULL,
    request TEXT NOT NULL,
    manifest TEXT NOT NULL,
    views TEXT NOT NULL,
    blobs TEXT NOT NULL,
    created REAL NOT NULL,
    started REAL,
    finished REAL,
    progress REAL NOT NULL DEFAULT 0,
    message TEXT NOT NULL DEFAULT '',
    error TEXT,
    worker TEXT,
    timings TEXT
);
CREATE INDEX IF NOT EXISTS jobs_queue ON jobs(status, backend, priority_rank, created);
"""


@dataclass
class BlobRecord:
    id: str
    size: int
    content_type: str | None
    filename: str | None
    created: float
    last_used: float


@dataclass
class JobRecord:
    id: str
    token_id: str
    backend: str
    priority: str
    status: str
    request: dict[str, Any]
    manifest: dict[str, Any]
    views: list[str]
    blobs: list[str]
    created: float
    started: float | None
    finished: float | None
    progress: float
    message: str
    error: str | None
    worker: str | None
    timings: dict[str, float]

    def info(self, queue_position: int | None = None) -> JobInfo:
        return JobInfo(
            id=self.id, backend=self.backend, status=self.status, priority=self.priority,
            token_id=self.token_id, created=_iso(self.created), started=_iso(self.started),
            finished=_iso(self.finished), progress=self.progress, message=self.message,
            error=self.error, views=self.views, worker=self.worker, queue_position=queue_position,
        )


def _iso(t: float | None) -> str | None:
    if t is None:
        return None
    return time.strftime("%Y-%m-%dT%H:%M:%S", time.gmtime(t)) + f".{int((t % 1) * 1000):03d}Z"


def new_job_id() -> str:
    return "j_" + secrets.token_hex(8)


class Store:
    def __init__(self, db_file: Path, blobs_dir: Path, jobs_dir: Path) -> None:
        self.db_file = db_file
        self.blobs_dir = blobs_dir
        self.jobs_dir = jobs_dir
        self._lock = threading.RLock()
        self._conn = sqlite3.connect(str(db_file), check_same_thread=False, isolation_level=None)
        self._conn.row_factory = sqlite3.Row
        self._conn.execute("PRAGMA journal_mode=WAL")
        self._conn.execute("PRAGMA synchronous=NORMAL")
        self._conn.executescript(SCHEMA)

    def close(self) -> None:
        with self._lock:
            self._conn.close()

    # blobs ---------------------------------------------------------------------------
    def blob_path(self, blob_id: str) -> Path:
        return self.blobs_dir / blob_id[:2] / blob_id

    def get_blob(self, blob_id: str) -> BlobRecord | None:
        with self._lock:
            row = self._conn.execute("SELECT * FROM blobs WHERE id=?", (blob_id,)).fetchone()
        return BlobRecord(**dict(row)) if row else None

    def add_blob(self, blob_id: str, size: int, content_type: str | None, filename: str | None) -> BlobRecord:
        now = time.time()
        with self._lock:
            self._conn.execute(
                "INSERT OR REPLACE INTO blobs(id,size,content_type,filename,created,last_used) VALUES(?,?,?,?,?,?)",
                (blob_id, size, content_type, filename, now, now))
        return BlobRecord(blob_id, size, content_type, filename, now, now)

    def touch_blobs(self, ids: Iterable[str]) -> None:
        now = time.time()
        with self._lock:
            self._conn.executemany("UPDATE blobs SET last_used=? WHERE id=?", [(now, i) for i in ids])

    def missing_blobs(self, ids: Iterable[str]) -> list[str]:
        ids = list(ids)
        if not ids:
            return []
        with self._lock:
            rows = self._conn.execute(
                f"SELECT id FROM blobs WHERE id IN ({','.join('?' * len(ids))})", ids).fetchall()
        present = {r["id"] for r in rows}
        return [i for i in ids if i not in present or not self.blob_path(i).exists()]

    def delete_blob(self, blob_id: str) -> None:
        with self._lock:
            self._conn.execute("DELETE FROM blobs WHERE id=?", (blob_id,))
        p = self.blob_path(blob_id)
        if p.exists():
            p.unlink()

    def blob_stats(self) -> tuple[int, int]:
        with self._lock:
            row = self._conn.execute("SELECT COUNT(*) AS n, COALESCE(SUM(size),0) AS b FROM blobs").fetchone()
        return int(row["n"]), int(row["b"])

    def expired_blobs(self, older_than: float) -> list[str]:
        """Blobs unused since ``older_than`` and not referenced by a live job."""
        with self._lock:
            rows = self._conn.execute("SELECT id FROM blobs WHERE last_used < ?", (older_than,)).fetchall()
            live = self._conn.execute(
                "SELECT blobs FROM jobs WHERE status NOT IN ('done','failed','cancelled')").fetchall()
        referenced = {b for r in live for b in json.loads(r["blobs"])}
        return [r["id"] for r in rows if r["id"] not in referenced]

    # jobs ---------------------------------------------------------------------------------
    def job_dir(self, job_id: str) -> Path:
        return self.jobs_dir / job_id

    def create_job(self, token_id: str, request: JobRequest, manifest: dict[str, Any],
                   views: list[str], blobs: list[str]) -> JobRecord:
        jid = new_job_id()
        now = time.time()
        with self._lock:
            self._conn.execute(
                "INSERT INTO jobs(id,token_id,backend,priority,priority_rank,status,request,manifest,views,blobs,"
                "created,progress,message,timings) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                (jid, token_id, request.backend, request.priority, PRIORITY_RANK[request.priority], "queued",
                 request.model_dump_json(), json.dumps(manifest), json.dumps(views), json.dumps(blobs),
                 now, 0.0, "", "{}"))
        self.touch_blobs(blobs)
        return self.get_job(jid)  # type: ignore[return-value]

    def get_job(self, job_id: str) -> JobRecord | None:
        with self._lock:
            row = self._conn.execute("SELECT * FROM jobs WHERE id=?", (job_id,)).fetchone()
        return _job(row) if row else None

    def list_jobs(self, status: str | None = None, backend: str | None = None, token_id: str | None = None,
                  limit: int = 100) -> list[JobRecord]:
        clauses, params = [], []
        if status:
            clauses.append("status=?")
            params.append(status)
        if backend:
            clauses.append("backend=?")
            params.append(backend)
        if token_id:
            clauses.append("token_id=?")
            params.append(token_id)
        where = ("WHERE " + " AND ".join(clauses)) if clauses else ""
        with self._lock:
            rows = self._conn.execute(
                f"SELECT * FROM jobs {where} ORDER BY created DESC LIMIT ?", (*params, limit)).fetchall()
        return [_job(r) for r in rows]

    def queue_position(self, job: JobRecord) -> int | None:
        if job.status != "queued":
            return None
        with self._lock:
            row = self._conn.execute(
                "SELECT COUNT(*) AS n FROM jobs WHERE status='queued' AND backend=? AND "
                "(priority_rank < ? OR (priority_rank = ? AND created < ?))",
                (job.backend, PRIORITY_RANK[job.priority], PRIORITY_RANK[job.priority], job.created)).fetchone()
        return int(row["n"])

    def queued_counts(self) -> dict[str, int]:
        with self._lock:
            rows = self._conn.execute(
                "SELECT backend, COUNT(*) AS n FROM jobs WHERE status='queued' GROUP BY backend").fetchall()
        return {r["backend"]: int(r["n"]) for r in rows}

    def status_counts(self) -> dict[str, int]:
        with self._lock:
            rows = self._conn.execute("SELECT status, COUNT(*) AS n FROM jobs GROUP BY status").fetchall()
        return {r["status"]: int(r["n"]) for r in rows}

    def claim_next(self, backends: Iterable[str], worker: str) -> JobRecord | None:
        """Atomically move the best queued job for ``backends`` to ``preparing``."""
        backends = list(backends)
        if not backends:
            return None
        with self._lock:
            row = self._conn.execute(
                f"SELECT id FROM jobs WHERE status='queued' AND backend IN ({','.join('?' * len(backends))}) "
                "ORDER BY priority_rank, created LIMIT 1", backends).fetchone()
            if row is None:
                return None
            self._conn.execute(
                "UPDATE jobs SET status='preparing', started=?, worker=? WHERE id=? AND status='queued'",
                (time.time(), worker, row["id"]))
        return self.get_job(row["id"])

    def set_status(self, job_id: str, status: str, *, error: str | None = None,
                   timings: dict[str, float] | None = None, message: str | None = None) -> None:
        fields, params = ["status=?"], [status]
        if status in TERMINAL_STATUSES:
            fields.append("finished=?")
            params.append(time.time())
        if error is not None:
            fields.append("error=?")
            params.append(error)
        if timings is not None:
            fields.append("timings=?")
            params.append(json.dumps(timings))
        if message is not None:
            fields.append("message=?")
            params.append(message)
        if status == "done":
            fields.append("progress=1.0")
        params.append(job_id)
        with self._lock:
            self._conn.execute(f"UPDATE jobs SET {', '.join(fields)} WHERE id=?", params)

    def set_progress(self, job_id: str, fraction: float, message: str) -> None:
        with self._lock:
            self._conn.execute("UPDATE jobs SET progress=?, message=? WHERE id=?", (fraction, message, job_id))

    def cancel_if_queued(self, job_id: str) -> bool:
        with self._lock:
            cur = self._conn.execute(
                "UPDATE jobs SET status='cancelled', finished=? WHERE id=? AND status='queued'",
                (time.time(), job_id))
            return cur.rowcount > 0

    def delete_job(self, job_id: str) -> None:
        with self._lock:
            self._conn.execute("DELETE FROM jobs WHERE id=?", (job_id,))

    def expired_jobs(self, older_than: float) -> list[str]:
        with self._lock:
            rows = self._conn.execute(
                "SELECT id FROM jobs WHERE status IN ('done','failed','cancelled') AND finished < ?",
                (older_than,)).fetchall()
        return [r["id"] for r in rows]

    def requeue_interrupted(self) -> int:
        """On startup, jobs left in preparing/running by a crash go back to the queue."""
        with self._lock:
            cur = self._conn.execute(
                "UPDATE jobs SET status='queued', started=NULL, worker=NULL, progress=0, message='requeued after restart' "
                "WHERE status IN ('preparing','running')")
            return cur.rowcount


def _job(row: sqlite3.Row) -> JobRecord:
    d = dict(row)
    d.pop("priority_rank", None)
    d["request"] = json.loads(d["request"])
    d["manifest"] = json.loads(d["manifest"])
    d["views"] = json.loads(d["views"])
    d["blobs"] = json.loads(d["blobs"])
    d["timings"] = json.loads(d["timings"] or "{}")
    return JobRecord(**d)
