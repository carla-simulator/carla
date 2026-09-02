# carla-cosmos server

HTTP/JSON job API in front of Cosmos worker processes.  Runs today with the
CPU `mock` worker; the model workers (Phases 3–6) and the all-in-one image
(Phase 7) plug into the same protocol and profiles.

```
launcher ── layout (nvidia-smi × image weights × --mode → planner, or profiles/*.yaml by name) ── spawns workers in their venvs
   │
 uvicorn ── FastAPI app
             auth middleware (bearer, before body read)
             /v1/blobs   content-addressed files            → <state>/blobs/aa/<sha256>
             /v1/jobs    SQLite job table + job dirs         → <state>/cosmos.sqlite, <state>/jobs/<id>/
             scheduler   per-backend FIFO, interactive>batch, one job per worker, one job per GPU
             worker RPC  newline-JSON over unix socket       → <state>/run/<worker>.sock
             gc          TTL for blobs (72 h unused) and finished jobs (168 h)
```

## The image

One Dockerfile, two variants, every model artifact inside — nothing is
downloaded at run time (``HF_HUB_OFFLINE=1``).

| tag | contents | size |
|---|---|---|
| `carla-cosmos:nano` | Cosmos 3 Nano, Transfer 2.5 general (4 branches), Transfer 2.5 AV multiview, renderer, guardrails | 104 GB weights + 66 GB stacks ≈ 170 GB |
| `carla-cosmos:full` | `:nano` + Cosmos 3 Super (133 GB) | ≈ 303 GB |
| `…-nomodels` | same code and venvs, empty `/models` (CI, API work) | 66 GB |

Layout inside: `vllm/vllm-openai` base (CUDA 13, Python 3.12) → vLLM-Omni pinned
(`VLLM_OMNI_REF`) in the base Python (= `/opt/venvs/cosmos3`); NVIDIA's
cosmos-transfer2.5 `v1.5.4` checked out to `/opt/cosmos-transfer2.5` with its own
`uv sync --extra=cu130` venv (= `/opt/venvs/transfer25`, also hosts the renderer);
a small `/opt/venvs/api` for the server; `/models/hf` is a normal Hugging Face
cache populated by `prefetch.py` from `artifacts.lock`; `/usr/local/bin/uvx` is
`tools/uvx_shim.py`, which answers cosmos-transfer2.5's `uvx hf download
… --revision <sha>` checkpoint lookups from the baked cache.

```sh
# one-shot BuildKit build — needs roughly 3× the weights free in the Docker root while it runs
# (≈ 350 GB for :nano, ≈ 750 GB for :full): build context + models stage + runtime layer
./build_image.sh --nano --hf-cache ~/.cache/huggingface --hf-token-file ~/.hf_token
./build_image.sh --full --hf-token-file ~/.hf_token --tag registry.example.com/carla-cosmos:full --push
./build_image.sh --nano --no-models            # code-only image (66 GB) for CI

# two-step build for hosts with less room: code image, then weights committed layer by layer
# (≤ 40 GB per layer, needs ≈ 2× one layer of headroom; nothing is downloaded — the caches must
# hold every file of artifacts.lock, see `python prefetch.py --image full --plan`)
./build_image.sh --nano --no-models
tools/bake_weights.sh --variant nano --base carla-cosmos:nano-nomodels --tag carla-cosmos:nano \
                      --hf-cache ~/.cache/huggingface

# registry build — the recommended way to publish: weights become independent layer images that
# are pushed and dropped locally one at a time (local peak ≈ 2× one layer), then the final images
# are assembled inside the registry (only a config + 1 KB marker layer is uploaded)
docker push  registry.example.com/carla-cosmos:nano-nomodels
tools/bake_weights.sh --variant nano --layers-to registry.example.com/carla-cosmos --hf-cache ~/.cache/huggingface
tools/bake_weights.sh --variant full --skip-variant nano --layers-to registry.example.com/carla-cosmos --hf-cache ...
python tools/compose_image.py --registry registry.example.com --repo carla-cosmos --code nano-nomodels \
       --layers nano-layer1,nano-layer2,nano-layer3 --variant nano
python tools/compose_image.py --registry registry.example.com --repo carla-cosmos --code nano-nomodels \
       --layers nano-layer1,nano-layer2,nano-layer3,full-layer1,full-layer2,full-layer3,full-layer4 --variant full

python tools/lock_artifacts.py --check          # is artifacts.lock still what HF serves?
```

With the containerd image store (`docker info` → `driver-type: io.containerd.snapshotter.v1`)
layers live under `/var/lib/containerd`, not the Docker data-root — check free space there.
All routes give the same `/models/hf` layout; the baked variant is recorded in
`/models/hf/ARTIFACTS_IMAGE` and the `com.carla.cosmos.variant` label. Layer
blobs are content-addressed and independent of what sits below them, so with the
registry route a code change is: rebuild `-nomodels`, push it, re-run
`compose_image.py` — the weight layers are reused untouched. Every file is
sha256-checked against `artifacts.lock` while it is baked (this caught a cached
xet download that had the right size but zero-filled gaps). ≤ 40 GB layers keep
`docker pull` of a 300 GB image resumable. `compose_image.py` speaks the plain
registry v2 API (basic or bearer auth via `--user`/`$REGISTRY_AUTH`).

`artifacts.lock` pins every repo to a commit and lists the files with sizes and
sha256; Transfer 2.5 checkpoints use the revisions hard-coded in NVIDIA's
checkpoint registry so the pipeline's offline resolution hits the baked
snapshot.  Gated repos (`nvidia/Cosmos-Transfer2.5-2B`, `-Predict2.5-2B`,
`-Guardrail1`, `-1.0-Guardrail`) need a Hugging Face token whose account
accepted the licences.  Two entries exist only because the Transfer 2.5 code
asks for them at load time and nothing else pulls them in:
`google/siglip-so400m-patch14-384` (hard-coded vision encoder of the guardrail
video content-safety filter) and the `nvidia/Cosmos-Predict2.5-2B`
`auto/multiview/524af350-…` checkpoint (the base the AV multiview control model
is loaded on top of).  `prefetch.py` also drops a plain copy of the
Video-Depth-Anything weights at `$HF_HOME/cosmos_depth_models/…`, the only place
cosmos-transfer2.5's depth auxiliary looks (it never reads the hub cache, and it
swallows the resulting error, silently dropping a `derive`d depth control).

Run: `../scripts/run_server.sh` (docker run with the right flags and the token
printed), or `compose/docker-compose.yaml` (`compose.caddy.yaml` adds TLS).

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
`MODE` (`auto`: `latency` | `throughput`, see Profiles), `PROFILES_DIR`, `MODELS_DIR` (`/models`), `IMAGE_VARIANT` (read from `/models/hf/ARTIFACTS_IMAGE`), `GUARDRAILS` (`1`), `TOKEN`
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
| `GET /v1/health/ready` | – | 200 / 503 + `{ready, profile, mode, backends, workers[{gpus, parallel, …}]}` |
| `GET /v1/status` | – | version, uptime, workers, queue and job counts, blob stats |
| `GET /v1/models` · `/v1/models/{id}` | – | `{id: {contract, available, workers, queued, placement: {mode, gpus, parallel}}}` |
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
  manifest, `worker_manifest` (checkpoint hashes, resolved spec, `gpus` the
  worker ran on and `parallel` = how the query was spread over them…), `timings`
  (`queued`, `preparing`, `running` seconds) and per-file `sha256`.
* Jobs interrupted by a server restart are re-queued.

## Profiles

`COSMOS_PROFILE=auto` (default) **plans the layout** from the detected GPUs
(count and memory, `nvidia-smi`), the weights baked into the image
(`/models/hf/ARTIFACTS_IMAGE`: `nano` | `full` | `none`) and the server
**mode** (`--mode` / `COSMOS_MODE`), so one big GPU, eight H100s or four RTX
6000 Pros each use every GPU and a `:nano` image never tries to start Cosmos 3
Super.  `carla-cosmos-server --list-profiles` prints the plan (per-worker GPUs,
parallelism and which GPUs are shared); `/v1/health/ready` reports `mode` and
the same per worker.

### Mode: latency vs throughput

| | `throughput` | `latency` |
|---|---|---|
| layout | one worker per model on **disjoint** GPUs | every worker that can scale gets **all** the GPUs it fits on; workers **time-share** them |
| one query uses | one GPU (Super: its TP ranks; AV: its ranks) | the whole node |
| concurrency | one job per worker, several models at once | one job at a time on shared GPUs (scheduler rule below) |
| planned name | `auto-4x96g-nano` | `auto-4x96g-nano-latency` |
| picked by `auto` | hosts with > 4 GPUs | hosts with ≤ 4 GPUs |

What each backend parallelises in latency mode (`parallel` in the health/models/manifest output):

| backend | worker args | mechanism |
|---|---|---|
| Cosmos 3 Nano | `--cfg-parallel 2 --ulysses N/2` | vLLM-Omni CFG-parallel (positive/negative branches on separate GPUs) × Ulysses sequence parallel; GPU counts are powers of two (`cfg × ulysses = GPUs`: 2 → cfg 2, 4 → cfg 2 × ulysses 2, 8 → cfg 2 × ulysses 4) |
| Cosmos 3 Super | `--tp T --cfg-parallel 2` | tensor parallel (T from the weight shard that fits a card) × CFG-parallel when 2·T GPUs fit |
| Transfer 2.5 general | `--context-parallel-size N` | NVIDIA `Control2WorldInference(context_parallel_size=N)`: one clip's latent sequence split over N torchrun ranks |
| Transfer 2.5 AV | `--nproc N` | `MultiviewInference(context_parallel_size=N)`, N torchrun ranks; any view count on any rank count (7 views on 8 ranks in NVIDIA's docs, on 4 or 1 here) |
| wsm renderer | – | unchanged: one GPU (the last one), beside the others |

Both Transfer 2.5 workers use the same rank model (`cosmos_workers/common/ranks.py`):
the socket worker starts `torchrun --nproc_per_node N --master_port <unique> -m
cosmos_workers.<backend>.ranks`, rank 0 serves a spool directory and broadcasts each
job to the other ranks, every rank runs the collective `generate`.  `torchrun` is
the venv's own (`/opt/venvs/transfer25/bin/torchrun`), never the base image's.

Memory rule for sharing: on every GPU, the **resident** sets of all workers placed
there (weights + idle engine) plus the **largest working set** of one running query
must stay within 90 % of the card (`*_RESIDENT_GIB` / `*_WORK_GIB` at the top of
`profiles.py`; residents measured on the 4 × 96 GB node: Nano ≈ 37 GiB, Transfer 2.5
≈ 28 GiB per rank; the rest are estimates).  A worker that fits nowhere is skipped
with a note that names the numbers.  Single-GPU hosts and 30–40 GiB cards degrade to
the throughput layout (`--default-resolution 480` below 40 GiB).

Allocation order in `throughput` mode, largest GPUs first (`carla_cosmos_server/profiles.py`):

1. `:full` only — Cosmos 3 Super first, on the smallest TP whose weight shard
   fits one GPU: TP=1 on B200, TP=2 on 96 GB RTX 6000 Pro, TP=4 on 80 GB H100.
2. Transfer 2.5 general + wsm renderer — 1 GPU ≥ 30 GiB (`--default-resolution 480`
   below 40 GiB).  On `:nano` images Cosmos 3 Nano (1 GPU ≥ 40 GiB) comes before it.
3. Cosmos 3 Nano — 1 GPU ≥ 40 GiB (`:full`: after Super and Transfer 2.5).
4. Leftover GPUs — Transfer 2.5 AV multiview, one rank per GPU (≥ 2, ≤ 8, ≥ 40 GiB
   each); single leftovers become extra Transfer 2.5 workers.  With no GPU left
   for Transfer 2.5 the renderer sits beside the first Cosmos 3 worker.

In `latency` mode the order is Super (`:full`), Nano, Transfer 2.5 general, AV, renderer,
each taking every GPU it fits on (least-loaded, largest first).

| GPUs | image | throughput layout | latency layout |
|---|---|---|---|
| 1 × 96 GB | nano | Nano + renderer | same |
| 2 × 96 GB | nano | Nano · Transfer 2.5 + renderer | Nano cfg 2 ∥ Transfer 2.5 CP=2 (both on 0-1) |
| 2 × 96 GB | full | Super TP=2 + renderer | Super TP=2 |
| 4 × 96 GB | nano | Nano · Transfer 2.5 · AV ×2 | Nano cfg 2 × Ulysses 2 ∥ Transfer 2.5 CP=4 (all on 0-3); AV skipped (does not fit beside them — use `latency-4gpu-av`) |
| 4 × 96 GB | full | Super TP=2 · Transfer 2.5 · Nano | Super TP=2 × cfg 2 on 0-3 |
| 8 × 80 GB | full | Super TP=4 · Transfer 2.5 · Nano · AV ×2 | Super TP=4 × cfg 2 ∥ Transfer 2.5 @480p CP=8 (explicit `--mode latency`) |
| 8 × 96 GB | nano | Nano · Transfer 2.5 · AV ×6 | Nano cfg 2 × Ulysses 4 ∥ Transfer 2.5 CP=8 (explicit) |
| 1 × 32 GB | nano | Transfer 2.5 @480p + renderer | same |
| 3 × 32 GB | nano | Transfer 2.5 ×3 | Transfer 2.5 @480p CP=3 |

Scheduler rule for shared GPUs: a worker only claims a job while **no job runs on a
model worker sharing a GPU with it** (`Scheduler.gpu_free`); the renderer is exempt
(it renders scene controls during a job's prepare phase).  Interactive jobs simply wait
for the wide worker.  `priority="batch"` keeps its ordering meaning only; running batch
jobs concurrently on narrow workers in throughput mode is future work.

A YAML in `profiles/` is a manual layout picked by name (`--profile <name>`); the
mode does not touch it (`mode: manual` in the health output):

```yaml
name: my-node
description: ...
workers:
  - name: cosmos3-nano
    type: cosmos3                  # mock | cosmos3 | transfer25 | transfer25_av | wsm_renderer
    backends: [cosmos3-nano]       # backend ids this worker serves
    gpus: [0, 1]                   # becomes CUDA_VISIBLE_DEVICES; workers may share GPUs (time-shared)
    args: ["--tp", "1", "--cfg-parallel", "2"]   # extra worker CLI args
```

Add `priority` + `match: {min_gpus, max_gpus, min_memory_gib}` to have a YAML
claim hosts before the planner runs (that is how `mock` takes 0-GPU hosts).
Shipped: `mock` (0 GPUs, auto), `nano-1gpu`, `nano-2gpu`, `full-8gpu` (manual
templates), `av-8gpu` (manual; Transfer 2.5 AV owns all 8 GPUs), `latency-4gpu-av`
(manual; Nano cfg 2 × Ulysses 2 and AV ×4 time-share a 4 × 96 GB node — the 7-camera
world scenario on one query).  Torchrun workers in one profile need distinct
`--master-port`s (`validate()` checks).  Worker types not yet implemented report
state `error` and keep the server `not ready` rather than silently serving fewer
backends.

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

## Validating on the GPU node (first run)

1. `./build_image.sh --nano --hf-token-file ~/.hf_token` (or `--hf-cache` with a mirror), or pull the pushed image.
2. `../scripts/run_server.sh` → note the token; `docker logs -f carla-cosmos` shows the workers loading
   (`/state/logs/worker-*.log` inside the volume has each worker's own log, incl. the `vllm serve` output).
3. `carla-cosmos health` until every worker is `ready` with `smoke=True` — each worker generates a tiny sample
   (5 frames / 2 steps for Cosmos 3, 93 frames / 1 step for Transfer 2.5, 2 views / 1 step for AV) before the
   server reports ready, so a broken model stack shows up here, not on the first real job.
4. `COSMOS_URL=… COSMOS_TOKEN=… ../scripts/smoke_test.sh --external` — one synthetic job per available backend.
5. Real clips: `demos/single_view_live.py` (Cosmos 3 / Transfer 2.5) and `demos/av7_world_scenario.py`
   (AV + rendering) against a CARLA server.

Things this machine could not verify (no GPU inference here) and where they would surface:

| unverified | expected symptom if wrong | knob |
|---|---|---|
| vLLM-Omni flags (`--vae-use-tiling`, TP) on the target GPUs | worker `error` at start; `worker-cosmos3-*.log` | profile `args`, `--vllm-arg` |
| latency mode: `--cfg-parallel-size 2 --ulysses-degree 2` on Cosmos 3 Nano (flags per vllm-omni `d3c990d`; Ulysses needs the GEN sequence length divisible by the degree) | vLLM-Omni refuses to start, or a 400 "must be divisible by ulysses_degree" on a job | `--mode throughput`, profile `args` (drop `--ulysses`), `--vllm-arg "--ulysses-mode advanced_uaa"` |
| latency mode: Transfer 2.5 general with `--context-parallel-size 4` (torchrun ranks; the AV path used the same call) | `transfer25` never `ready`; rank log in `worker-transfer25.log` | profile `args` (`--context-parallel-size 1`), `--mode throughput` |
| latency mode: resident + working-set budgets (`*_RESIDENT_GIB`, `*_WORK_GIB`) | CUDA OOM on a job when two workers share a card (the rank reports "GPU memory exhausted on rank …") | constants in `profiles.py`, `nvidia-smi` while a job runs |
| Transfer 2.5 offline checkpoint resolution through the `uvx` shim | `transfer25` worker `error: cannot resolve …` | `uvx_shim.py`, `artifacts.lock` revisions |
| `torchrun` rank loop for AV and Transfer 2.5 (`broadcast_object_list`, barriers) | `transfer25-av` / `transfer25` never `ready` / hangs | `common/ranks.py`; run with 1 view first |
| EGL in the container (`libegl1` + `NVIDIA_DRIVER_CAPABILITIES=graphics`) | renderer smoke fails `moderngl` | host driver, `--gpus` capabilities |
| Latency / VRAM per backend | slow or OOM | `resolution`, `num_steps`, profile GPU split |
