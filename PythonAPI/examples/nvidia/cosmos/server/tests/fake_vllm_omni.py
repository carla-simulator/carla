"""A stand-in for `vllm serve --omni` exposing the videos API the Cosmos 3 worker uses.

Behaviour: ``POST /v1/videos`` (multipart) -> queued job that becomes ``in_progress``
then ``completed`` after ``--delay`` seconds; the "generated" mp4 is the uploaded
``input_reference`` if any, else the first ``control_path`` file, else a stub.
Prompts containing ``BLOCKME`` fail with the guardrail 400 error.  Every request
is appended to ``<storage>/requests.jsonl`` for assertions.  Stdlib only.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
import threading
import time
import uuid
from email.parser import BytesParser
from email.policy import HTTP
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

JOBS: dict[str, dict] = {}
LOCK = threading.Lock()
ARGS: argparse.Namespace


def parse_multipart(headers, body: bytes) -> tuple[dict[str, str], dict[str, bytes]]:
    msg = BytesParser(policy=HTTP).parsebytes(
        f"Content-Type: {headers['Content-Type']}\r\nMIME-Version: 1.0\r\n\r\n".encode() + body)
    fields, files = {}, {}
    for part in msg.iter_parts():
        name = part.get_param("name", header="content-disposition")
        fname = part.get_param("filename", header="content-disposition")
        payload = part.get_payload(decode=True)
        if fname:
            files[name] = payload
        else:
            fields[name] = payload.decode()
    return fields, files


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a):  # quiet
        pass

    def _json(self, code: int, obj) -> None:
        data = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/health":
            return self._json(200, {})
        if self.path == "/version":
            return self._json(200, {"version": "fake-vllm-omni-0.0"})
        m = re.fullmatch(r"/v1/videos/([^/]+)(/content)?", self.path)
        if not m:
            return self._json(404, {"detail": "not found"})
        vid, content = m.group(1), m.group(2)
        with LOCK:
            job = JOBS.get(vid)
        if job is None:
            return self._json(404, {"detail": "unknown video"})
        _advance(job)
        if content:
            if job["status"] != "completed":
                return self._json(404, {"detail": "Generation is still in-progress"})
            data = Path(job["file"]).read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "video/mp4")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return
        code = 200
        if job["status"] == "failed" and job["error"]["code"] >= 400:
            code = job["error"]["code"]
        return self._json(code, _public(job))

    def do_DELETE(self):
        m = re.fullmatch(r"/v1/videos/([^/]+)", self.path)
        with LOCK:
            job = JOBS.pop(m.group(1), None) if m else None
        if job is None:
            return self._json(404, {"detail": "unknown video"})
        job["deleted"] = True
        _log({"event": "delete", "id": job["id"], "status_at_delete": job["status"]})
        return self._json(200, {"id": job["id"], "deleted": True})

    def do_POST(self):
        if self.path not in ("/v1/videos", "/v1/videos/sync"):
            return self._json(404, {"detail": "not found"})
        length = int(self.headers.get("Content-Length", "0"))
        fields, files = parse_multipart(self.headers, self.rfile.read(length))
        extra = json.loads(fields.get("extra_params", "{}"))
        vid = "video_gen_" + uuid.uuid4().hex[:12]
        storage = Path(ARGS.storage)
        storage.mkdir(parents=True, exist_ok=True)
        out = storage / f"{vid}.mp4"
        src = None
        if "input_reference" in files:
            out.write_bytes(files["input_reference"])
            src = "input_reference"
        else:
            for hint in ("edge", "blur", "depth", "seg", "wsm"):
                cp = (extra.get(hint) or {}).get("control_path") if isinstance(extra.get(hint), dict) else None
                if cp and Path(cp).exists():
                    shutil.copyfile(cp, out)
                    src = cp
                    break
            if src is None:
                out.write_bytes(b"\x00" * 64)
                src = "stub"
        for hint in ("depth", "seg", "wsm"):
            v = extra.get(hint)
            if v is True or (isinstance(v, dict) and "control_path" not in v):
                err = {"code": 500, "message": f"{hint} requires a precomputed control_path"}
                break
        else:
            err = None
        if "BLOCKME" in fields.get("prompt", ""):
            err = {"code": 400, "message": "Input was blocked by Cosmos3 guardrails."}
        job = {"id": vid, "status": "queued", "created": time.time(), "file": str(out), "error": err,
               "fields": fields, "extra": extra, "files": sorted(files), "source": src, "deleted": False,
               "progress": 0}
        with LOCK:
            JOBS[vid] = job
        _log({"event": "submit", "id": vid, "fields": {k: v for k, v in fields.items()}, "extra": extra,
              "files": sorted(files), "source": src})
        if self.path.endswith("/sync"):
            time.sleep(ARGS.delay)
            _advance(job)
            data = out.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "video/mp4")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return
        return self._json(200, _public(job))


def _advance(job: dict) -> None:
    age = time.time() - job["created"]
    if job["status"] in ("completed", "failed"):
        return
    if job["error"] and age > ARGS.delay / 2:
        job["status"] = "failed"
    elif age > ARGS.delay:
        job["status"] = "completed"
        job["progress"] = 100
        job["inference_time_s"] = ARGS.delay
    elif age > 0.05:
        job["status"] = "in_progress"


def _public(job: dict) -> dict:
    return {"id": job["id"], "object": "video", "status": job["status"], "progress": job["progress"],
            "model": job["fields"].get("model"), "prompt": job["fields"].get("prompt"),
            "error": job["error"] if job["status"] == "failed" else None,
            "inference_time_s": job.get("inference_time_s"), "stage_durations": {"diffusion": ARGS.delay},
            "peak_memory_mb": 12345, "file_name": Path(job["file"]).name if job["status"] == "completed" else None}


def _log(rec: dict) -> None:
    with LOCK, (Path(ARGS.storage) / "requests.jsonl").open("a") as f:
        f.write(json.dumps(rec) + "\n")


def main() -> int:
    global ARGS
    p = argparse.ArgumentParser()
    p.add_argument("--port", type=int, required=True)
    p.add_argument("--storage", required=True)
    p.add_argument("--delay", type=float, default=1.0)
    p.add_argument("--startup-delay", type=float, default=0.0)
    ARGS = p.parse_args()
    time.sleep(ARGS.startup_delay)
    Path(ARGS.storage).mkdir(parents=True, exist_ok=True)
    srv = ThreadingHTTPServer(("127.0.0.1", ARGS.port), Handler)
    print(f"fake vllm-omni on {ARGS.port}", file=sys.stderr, flush=True)
    srv.serve_forever()
    return 0


if __name__ == "__main__":
    sys.exit(main())
