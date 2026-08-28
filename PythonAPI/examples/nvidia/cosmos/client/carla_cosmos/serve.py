"""Launch or adopt a carla-cosmos server.

Two flavours:

* :func:`ensure_server` — the production path: find a container started by us
  (Docker labels), or ``docker run`` the image with the state directory
  mounted, then wait for readiness and read the initial token from
  ``<state>/initial_token.txt``.
* :class:`MockServer` — a plain-Python server with the mock worker (no Docker,
  no GPU) for tests and ``scripts/smoke_test.sh``; needs the
  ``carla-cosmos-server`` package installed in the same interpreter.
"""

from __future__ import annotations

import json
import logging
import os
import secrets
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

from .client import CosmosClient, CosmosError

log = logging.getLogger(__name__)

DEFAULT_IMAGE = os.environ.get("COSMOS_IMAGE", "carla-cosmos:nano")
LABEL_ROLE = "com.carla.cosmos.role"
LABEL_PORT = "com.carla.cosmos.port"
LABEL_IMAGE = "com.carla.cosmos.image"
LABEL_STATE = "com.carla.cosmos.state"
CONTAINER_PORT = 8000


@dataclass
class ContainerInfo:
    name: str
    image: str
    port: int
    state_dir: str
    running: bool

    @property
    def url(self) -> str:
        return f"http://localhost:{self.port}"


def _docker(*args: str, check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(["docker", *args], capture_output=True, text=True, check=check)


def find_containers() -> list[ContainerInfo]:
    """Containers we started (by label), running or not."""
    r = _docker("ps", "-a", "--filter", f"label={LABEL_ROLE}=server", "--format", "{{json .}}", check=False)
    if r.returncode != 0:
        return []
    out = []
    for line in r.stdout.splitlines():
        row = json.loads(line)
        insp = _docker("inspect", row["Names"], "--format", "{{json .Config.Labels}}")
        labels = json.loads(insp.stdout or "{}") or {}
        out.append(ContainerInfo(name=row["Names"], image=labels.get(LABEL_IMAGE, row["Image"]),
                                 port=int(labels.get(LABEL_PORT, CONTAINER_PORT)),
                                 state_dir=labels.get(LABEL_STATE, ""),
                                 running=row.get("State", "").lower() == "running"))
    return out


def start_container(image: str = DEFAULT_IMAGE, port: int = CONTAINER_PORT, state_dir: str | Path = "~/.carla-cosmos/state",
                    gpus: str = "all", profile: str | None = None, env: dict[str, str] | None = None,
                    name: str | None = None, extra_args: list[str] | None = None) -> ContainerInfo:
    """``docker run -d`` the server image with our labels."""
    state = Path(state_dir).expanduser().resolve()
    state.mkdir(parents=True, exist_ok=True)
    name = name or f"carla-cosmos-{secrets.token_hex(3)}"
    cmd = ["run", "-d", "--name", name, "--restart", "unless-stopped",
           "-p", f"{port}:{CONTAINER_PORT}", "-v", f"{state}:/state", "--shm-size", "16g",
           "--label", f"{LABEL_ROLE}=server", "--label", f"{LABEL_PORT}={port}",
           "--label", f"{LABEL_IMAGE}={image}", "--label", f"{LABEL_STATE}={state}"]
    if gpus:
        cmd += ["--gpus", gpus]
    if profile:
        cmd += ["-e", f"COSMOS_PROFILE={profile}"]
    for k, v in (env or {}).items():
        cmd += ["-e", f"{k}={v}"]
    cmd += extra_args or []
    cmd.append(image)
    log.info("docker %s", " ".join(cmd))
    _docker(*cmd)
    return ContainerInfo(name=name, image=image, port=port, state_dir=str(state), running=True)


def stop_container(name: str, remove: bool = False) -> None:
    _docker("stop", name, check=False)
    if remove:
        _docker("rm", name, check=False)


def read_initial_token(state_dir: str | Path) -> str | None:
    p = Path(state_dir).expanduser() / "initial_token.txt"
    try:
        return p.read_text().strip() or None
    except OSError:
        return None


def ensure_server(image: str = DEFAULT_IMAGE, port: int = CONTAINER_PORT, state_dir: str | Path = "~/.carla-cosmos/state",
                  gpus: str = "all", profile: str | None = None, token: str | None = None,
                  timeout: float = 3600.0, verbose: bool = True) -> tuple[ContainerInfo, CosmosClient]:
    """Reuse a matching container or start one; return it with a ready client."""
    state = str(Path(state_dir).expanduser().resolve())
    existing = [c for c in find_containers() if c.port == port]
    if existing:
        c = existing[0]
        if c.image != image or (c.state_dir and c.state_dir != state):
            raise CosmosError(0, f"container {c.name} on port {port} runs {c.image} with state {c.state_dir}; "
                                 f"stop it (carla-cosmos serve --stop) or pick another --port")
        if not c.running:
            log.info("starting stopped container %s", c.name)
            _docker("start", c.name)
            c.running = True
    else:
        c = start_container(image, port, state, gpus, profile)
    tok = token or os.environ.get("COSMOS_TOKEN")
    if tok is None:
        for _ in range(60):
            tok = read_initial_token(state)
            if tok:
                break
            time.sleep(1.0)
    if tok is None:
        raise CosmosError(0, f"no token: {state}/initial_token.txt not written yet and COSMOS_TOKEN unset "
                             f"(docker logs {c.name})")
    client = CosmosClient(c.url, token=tok)
    last = [0.0]

    def on_wait(body):
        if verbose and time.monotonic() - last[0] > 15:
            last[0] = time.monotonic()
            states = ", ".join(f"{w['name']}={w['state']}" for w in body.get("workers", [])) or "starting"
            print(f"[carla-cosmos] waiting for workers: {states}", flush=True)

    client.wait_ready(timeout=timeout, on_wait=on_wait)
    return c, client


class MockServer:
    """In-process (subprocess) server with the mock worker; no Docker, no GPU."""

    def __init__(self, state_dir: str | Path, port: int = 0, token: str | None = None,
                 delay: float = 1.0, python: str = sys.executable, log_file: str | Path | None = None) -> None:
        self.state_dir = Path(state_dir)
        self.port = port or _free_port()
        self.token = token or f"cc_{secrets.token_hex(4)}_{secrets.token_urlsafe(16)}"
        self.delay = delay
        self.python = python
        self.log_file = Path(log_file) if log_file else None
        self.proc: subprocess.Popen | None = None

    @property
    def url(self) -> str:
        return f"http://127.0.0.1:{self.port}"

    def start(self, timeout: float = 120.0) -> CosmosClient:
        self.state_dir.mkdir(parents=True, exist_ok=True)
        profiles = self.state_dir / "profiles"
        profiles.mkdir(exist_ok=True)
        (profiles / "mock.yaml").write_text(
            "name: mock\ndescription: mock worker\npriority: 1\nmatch: {min_gpus: 0}\n"
            "workers:\n  - name: mock\n    type: mock\n"
            "    backends: [cosmos3-nano, cosmos3-super, transfer2.5, transfer2.5-av]\n"
            f"    args: ['--delay', '{self.delay}', '--steps', '4']\n"
            "  - name: wsm-renderer\n    type: wsm_renderer\n    backends: [wsm-renderer]\n"
            "    args: ['--engine', 'fake', '--fake-delay', '0.2']\n")
        env = {**os.environ, "COSMOS_TOKEN": self.token, "COSMOS_GC_INTERVAL_S": "5"}
        out = self.log_file.open("ab") if self.log_file else None
        self.proc = subprocess.Popen(
            [self.python, "-m", "carla_cosmos_server", "--state", str(self.state_dir), "--host", "127.0.0.1",
             "--port", str(self.port), "--profile", "mock", "--profiles-dir", str(profiles)],
            env=env, stdout=out, stderr=subprocess.STDOUT if out else None)
        client = CosmosClient(self.url, token=self.token)
        t0 = time.monotonic()
        while True:
            if self.proc.poll() is not None:
                raise RuntimeError(f"mock server exited with {self.proc.returncode}"
                                   + (f"; see {self.log_file}" if self.log_file else ""))
            ok, _ = client.ready()
            if ok:
                return client
            if time.monotonic() - t0 > timeout:
                self.stop()
                raise RuntimeError("mock server did not become ready")
            time.sleep(0.2)

    def stop(self) -> None:
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(15)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        self.proc = None

    def __enter__(self) -> CosmosClient:
        return self.start()

    def __exit__(self, *exc) -> None:
        self.stop()


def _free_port() -> int:
    import socket

    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]
