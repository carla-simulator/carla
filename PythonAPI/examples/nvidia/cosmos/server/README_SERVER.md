# carla-cosmos server

HTTP/JSON job API in front of Cosmos worker processes.  Runs today with the
CPU `mock` worker; the model workers (Phases 3–6) and the all-in-one image
(Phase 7) plug into the same protocol and profiles.

```
launcher ── profile (nvidia-smi → profiles/*.yaml) ── spawns workers in their venvs
   │
 uvicorn ── FastAPI app
             auth middleware (bearer, before body read)
             /v1/blobs   content-addressed files            → <state>/blobs/aa/<sha256>
             /v1/jobs    SQLite job table + job dirs         → <state>/cosmos.sqlite, <state>/jobs/<id>/
             scheduler   per-backend FIFO, interactive>batch, one job per worker
             worker RPC  newline-JSON over unix socket       → <state>/run/<worker>.sock
             gc          TTL for blobs (72 h unused) and finished jobs (168 h)
```

## Run from source (mock)

```sh
pip install -e ../client -e workers -e .          # carla-cosmos (core), carla-cosmos-workers, carla-cosmos-server
COSMOS_TOKEN=dev carla-cosmos-server --state /tmp/cosmos-state --profile mock --port 8000
carla-cosmos-server --list-profiles               # detected GPUs and which profile auto matches
```

In the container the entry point is the same `carla-cosmos-server`, with
`/state` mounted from the host and `COSMOS_PROFILE=auto`.

## Configuration

All settings are `COSMOS_*` environment variables (CLI flags override):
`STATE` (`/state`), `HOST`/`PORT` (`0.0.0.0`/`8000`), `PROFILE` (`auto`),
`PROFILES_DIR`, `MODELS_DIR` (`/models`), `GUARDRAILS` (`1`), `TOKEN`
(bootstrap token), `BLOB_TTL_HOURS` (`72`), `JOB_TTL_HOURS` (`168`),
`GC_INTERVAL_S` (`600`), `LOG_LEVEL`, `RUN_DIR` (sockets), `VENV_<NAME>`
(`/opt/venvs/<name>`; falls back to the running interpreter when missing).
See `carla_cosmos_server/config.py`.

## Authentication

* Tokens: `cc_<id>_<secret>`; the store keeps `sha256(secret)` only
  (`<state>/tokens.json`, mode 0600).  Verification = id lookup + constant-time
  digest compare.
* First boot with no tokens mints one, prints it once and writes
  `<state>/initial_token.txt` (0600).  `COSMOS_TOKEN=<any string>` registers a
  bootstrap token (scripts, CI).
* Host-side management only: `carla-cosmos-tokens list|add --label X|revoke <id>`
  (`docker exec <container> carla-cosmos-tokens ...`).  No HTTP endpoint mints
  tokens, so a leaked token cannot create more.
* Public routes: `/`, `/ui`, `/v1/health/live`, `/v1/health/ready`.
  Everything else answers `401` **before** the request body is read.
* By design there are **no** rate limits, quotas or caps.  The token id is
  recorded on every job (`token_id`, `GET /v1/jobs?mine=1`).

## API

| method & path | body / params | returns |
|---|---|---|
| `GET /v1/health/live` | – | `{status, version}` |
| `GET /v1/health/ready` | – | 200 / 503 + `{ready, profile, backends, workers[]}` |
| `GET /v1/status` | – | version, uptime, workers, queue and job counts, blob stats |
| `GET /v1/models` · `/v1/models/{id}` | – | `{id: {contract, available, workers, queued}}` |
| `PUT /v1/blobs/{sha256}` | raw bytes, `Content-Type`, `X-Filename` | 201 `{id,size,existed:false}` / 200 existed / 400 hash mismatch |
| `POST /v1/blobs/check` | `{"ids": [...]}` | `{present, missing}` |
| `GET /v1/blobs/{sha256}` | – | the file |
| `POST /v1/jobs` | `JobSubmission {request, manifest}` | 202 `JobInfo`; 400 `{detail:{errors:[...]}}`; 409 no worker for backend |
| `GET /v1/jobs` | `status`, `backend`, `mine`, `limit` | `[JobInfo]` |
| `GET /v1/jobs/{id}` | – | `JobInfo` (with `queue_position` while queued) |
| `DELETE /v1/jobs/{id}` | – | cancel (queued/preparing/running) or delete (terminal, removes files) |
| `GET /v1/jobs/{id}/result` | – | `ResultManifest` (409 until done, 410 after TTL) |
| `GET /v1/jobs/{id}/result/{name}` | – | one result file |
| `GET /v1/metrics` | – | Prometheus text (`cosmos_jobs`, `cosmos_queue_depth`, `cosmos_worker_state`, …) |
| `GET /ui` | – | status page (token in localStorage) · `GET /v1/docs` OpenAPI |

Models are the pydantic classes in `carla_cosmos.contracts` (`JobRequest`,
`ClipManifest`, `JobInfo`, `ResultManifest`, `BackendContract`).  The server
re-runs `validate_request` and checks every referenced blob exists before a job
is queued, so a client cannot enqueue work that would fail at prepare time for
contract reasons.

### Job lifecycle

`queued → preparing → running → done | failed | cancelled`

* **preparing**: blobs are symlinked into `jobs/<id>/inputs/` (`rgb_<cam>.mp4`,
  `<control>_<cam>.mp4`), scene packages (zip) are unpacked to
  `inputs/scene_<control>/` with traversal checks.
* **running**: the worker streams `progress {fraction, message}`; stored on the
  job row (`progress`, `message`).
* **done**: `jobs/<id>/result/manifest.json` — backend, worker, request, clip
  manifest, `worker_manifest` (checkpoint hashes, resolved spec…), `timings`
  (`queued`, `preparing`, `running` seconds) and per-file `sha256`.
* Jobs interrupted by a server restart are re-queued.

## Profiles

`profiles/<name>.yaml`:

```yaml
name: nano-2gpu
description: ...
priority: 20                       # auto-selection order (lower first)
match: {min_gpus: 2, max_gpus: 7, min_memory_gib: 40}   # omit for manual-only
workers:
  - name: cosmos3-nano
    type: cosmos3                  # mock | cosmos3 | transfer25 | transfer25_av | wsm_renderer
    backends: [cosmos3-nano]       # backend ids this worker serves
    gpus: [0]                      # becomes CUDA_VISIBLE_DEVICES
    args: ["--tp", "1"]            # extra worker CLI args
```

Shipped: `mock` (0 GPUs), `nano-1gpu` (1×≥40 GB), `nano-2gpu` (2–7 GPUs),
`full-8gpu` (≥8×≥70 GiB, provisional layout), `av-8gpu` (manual; Transfer 2.5
AV owns all 8 GPUs).  Worker types not yet implemented report state `error`
and keep the server `not ready` rather than silently serving fewer backends.

## Worker protocol

`workers/cosmos_workers/common/protocol.py` — newline-delimited JSON over a
unix socket the worker binds; one connection per operation so `cancel` can
reach a worker mid-`run`.  Ops: `hello`, `status`, `smoke`, `run`, `cancel`,
`shutdown`.  Implement a worker by subclassing `cosmos_workers.common.base.Worker`
(`load`, `run(job, ctx)`, `smoke`) and calling `serve()`; `run` executes in a
thread and must call `ctx.check_cancelled()` between chunks.  The package is
dependency-free so it installs into the Transfer 2.5 and Cosmos 3 venvs
untouched.  `cosmos_workers.mock` is the reference implementation.

Readiness of the whole server = every worker `ready` **and** its smoke sample
passed.

## State directory

```
<state>/
  tokens.json  initial_token.txt  cosmos.sqlite(+ -wal/-shm)
  blobs/aa/<sha256>
  jobs/<job_id>/{request.json, clip_manifest.json, inputs/, result/manifest.json ...}
  logs/worker-<name>.log
  run/<worker>.sock                (or $TMPDIR/cosmos-run-<hash>/ when the state path is too long for AF_UNIX)
```

## Tests

```sh
cd server && python -m pytest tests      # auth, blobs, jobs, scheduler ordering, gc, profiles, protocol (mock worker subprocess)
../scripts/smoke_test.sh                 # black-box: mock server, one job per backend, auth rejection
```
