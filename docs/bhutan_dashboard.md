# Bhutan dashboard on Cloudflare Workers

The `dashboard/` directory contains "Atlas", the operations and evidence
application for the Bhutan pilot. One Worker serves both the JSON API under
`/api/*` and a static single-page dashboard (deck.gl route playback, unified
timeline, scenario library, evaluations, safety review and governance).

- [Architecture](#architecture)
- [Deploy](#deploy)
- [Local development](#local-development)
- [Authentication and tenants](#authentication-and-tenants)
- [API](#api)
- [KPI definitions](#kpi-definitions)
- [Evidence packs](#evidence-packs)
- [Data governance](#data-governance)

---

## Architecture

| Component | Cloudflare product | Role |
|---|---|---|
| API and static assets | Workers with static assets | Same-origin API and dashboard, no separate frontend host |
| Metadata catalog | D1 | Runs, telemetry chunk index, events, quality segments, scenarios, evaluations, clips, KPI snapshots, audit log |
| Raw data | R2 | Immutable telemetry chunks (JSONL, SHA-256 in object metadata) and clips |
| Scheduled work | Cron trigger | Nightly KPI snapshot per tenant for trend lines and partner reports |
| Signing | Worker secret + WebCrypto | HMAC-SHA256 signature on evidence packs |

The Worker is dependency-free TypeScript (no framework) so it stays small and
auditable. The front end is plain HTML, CSS and JavaScript with deck.gl loaded
from a CDN; there is no build step.

## Deploy

```sh
cd dashboard
npm install
npx wrangler d1 create carla-bhutan-atlas            # copy database_id into wrangler.toml
npx wrangler r2 bucket create carla-bhutan-atlas-data
npm run db:migrate:remote
npx wrangler secret put API_TOKENS                   # token=tenant:role;token2=tenant2:reader
npx wrangler secret put MANIFEST_SIGNING_KEY         # 32+ random bytes
npm run deploy
```

The GitHub Actions workflow `.github/workflows/bhutan_dashboard.yml`
typechecks on every pull request touching `dashboard/` and deploys on pushes
to the main branches when the `CLOUDFLARE_API_TOKEN` and
`CLOUDFLARE_ACCOUNT_ID` secrets are present.

To seed a fresh deployment with the scenario library:

```sh
cd toolkit
python scripts/generate_library.py --dashboard https://carla-bhutan-atlas.<account>.workers.dev --token <writer token>
```

## Local development

```sh
cd dashboard
npm install
cp .dev.vars.example .dev.vars           # local tokens and signing key
npm run db:migrate:local
npm run dev                              # http://127.0.0.1:8787
```

Then, from `toolkit`, `python scripts/seed_demo.py` fills the local
instance with the scenario library, three synthetic switchback runs (clearly
labelled as synthetic), a baseline evaluation and two clips. Open the
dashboard, paste `dev-writer-token` into the token box and connect.

## Authentication and tenants

`API_TOKENS` is a secret of the form `token=tenant:role;...`. Roles are
`reader` (dashboards, evidence export, released clips), `writer` (uploads,
reviews, governance changes) and `admin` (may switch tenant with the
`X-Tenant` header). Every row in D1 carries `tenant_id`, R2 keys are prefixed
with the tenant, and every write is recorded in `audit_log` with a short
non-reversible token fingerprint as the actor. Tokens are compared in
constant time.

## API

All endpoints expect `Authorization: Bearer <token>` except `GET /api/health`,
`GET /api/openapi.json` and the ingestion webhooks noted below (those still
need a writer token, just not necessarily in an `Authorization` header).

| Method and path | Role | Purpose |
|---|---|---|
| `GET /api/health` | none | Liveness check |
| `GET /api/openapi.json` | none | OpenAPI 3.1 description of every endpoint, for client generation and Swagger UI |
| `GET /api/metrics` | reader | Prometheus text exposition of KPIs, edge-case rates and perception metrics, for Grafana |
| `GET /api/kpis` | reader | Live KPI report, edge-case rates and perception benchmark |
| `GET /api/kpis/history` | reader | Nightly KPI snapshots |
| `POST /api/kpis/snapshot` | writer | Force a snapshot |
| `GET /api/planner/options` | none | Reference tables behind the planner: dataset tiers, training programs, GPU rates, video bitrates |
| `GET`/`POST /api/planner` | reader | Dataset size, storage and GPU-cost plan for a target corpus, plus the catalog's coverage against it |
| `GET /api/runs`, `GET /api/runs/:id` | reader | Run catalog and detail (segments, chunks, event summary, evaluations, driving score) |
| `POST /api/runs` | writer | Upsert a run manifest |
| `POST /api/runs/:id/telemetry?seq=N` | writer | Upload a chunk of samples (stored in R2, indexed in D1) |
| `GET /api/runs/:id/telemetry?max=4000` | reader | Merged, downsampled samples for playback |
| `POST /api/runs/:id/events`, `GET /api/runs/:id/events` | writer / reader | Bulk event upload and listing |
| `POST /api/runs/:id/finish` | writer | Record quality report, segments, streams, privacy status; computes the Leaderboard-style driving score |
| `GET /api/runs/:id/evidence` | reader | Signed evidence pack |
| `GET /api/runs/:id/export/:format` | reader | Export a run as `geojson` (kepler.gl, QGIS), `csv` (pandas, PlotJuggler), `mcap` (Foxglove Studio, ROS 2) or `json` |
| `GET /api/events/critical`, `POST /api/events/:id/review` | reader / writer | Safety review queue |
| `GET /api/scenarios`, `GET /api/scenarios/:id` | reader | Library with per-family review counts |
| `POST /api/scenarios/import` | writer | Import a library manifest; a changed content hash resets the review |
| `POST /api/scenarios/:id/review` | writer | Expert alignment review |
| `GET /api/scenarios/:id/export/xosc` | reader | ASAM OpenSCENARIO 1.2 export for CARLA ScenarioRunner or esmini |
| `GET /api/evaluations`, `GET /api/evaluations/:id`, `POST /api/evaluations` | reader / writer | Benchmark reports |
| `POST /api/evaluations/:id/replay-verified` | writer | Mark reproducibility (compares a replay's inputs and counts) |
| `GET /api/clips`, `POST /api/clips`, `PUT /api/clips/:id/object`, `GET /api/clips/:id/object`, `POST /api/clips/:id/governance` | mixed | Clip registration, upload, gated download, redaction and release |
| `GET /api/fleet/live` | reader | Latest position and status of every fleet device |
| `POST /api/fleet/positions` | writer | Generic position upload from edge loggers |
| `GET /api/fleet/devices/:id/track` | reader | A device's positions in a time window |
| `POST /api/fleet/devices/:id/materialize` | writer | Turn a device's track into a run so it enters the catalog and KPIs |
| `POST /api/ingest/traccar` | writer | [Traccar](https://github.com/traccar/traccar) forward webhook (`forward.url` + a writer token in `forward.header`) |
| `GET`/`POST /api/ingest/osmand` | writer | OsmAnd protocol used by Traccar Client and GPSLogger (`?token=<writer token>`) |
| `GET /api/audit` | reader | Audit trail |

The Python client in `bhutan_sim/telemetry.py` wraps the run, scenario, evaluation and clip endpoints.

### Fleet ingestion

Three producers feed the same `fleet_positions`/`fleet_devices` tables so a
partner's existing GPS tracking keeps working while the fleet grows into
dedicated edge loggers:

* **Traccar** — an existing [Traccar](https://github.com/traccar/traccar)
  server (hundreds of supported GPS protocols) forwards every position to
  `POST /api/ingest/traccar` via `forward.url` / `forward.header` in
  `traccar.xml`.
* **OsmAnd protocol** — Traccar Client, OsmAnd and GPSLogger apps on a
  driver's phone post directly to `GET/POST /api/ingest/osmand?token=<writer token>`.
* **Generic** — an edge logger posts batches of `{device_id, t, lat, lon, ...}`
  to `POST /api/fleet/positions`.

The Fleet tab shows live positions and per-device tracks on the same deck.gl
map as run playback. `POST /api/fleet/devices/:id/materialize` turns a time
window of a device's track into a run (derived speed, heading and grade)
so it can be finished with a quality report and counted toward the
real-world route-coverage KPI.

### Exports

`GET /api/runs/:id/export/:format` and `GET /api/scenarios/:id/export/xosc`
turn Atlas data into formats other open-source AV tools already read:

* **MCAP** (`foxglove.LocationFix` + `foxglove.Log` channels, plus the full
  Atlas sample) opens directly in [Foxglove Studio](https://github.com/foxglove/studio)
  or any ROS 2 tool that reads MCAP.
* **GeoJSON** loads into [kepler.gl](https://github.com/keplergl/kepler.gl),
  QGIS or any web map for risk overlays.
* **CSV** is for spreadsheets, pandas and PlotJuggler.
* **OpenSCENARIO 1.2** (`.xosc`) runs a scenario template in
  [CARLA ScenarioRunner](https://github.com/carla-simulator/scenario_runner)
  or [esmini](https://github.com/esmini/esmini):
  `python scenario_runner.py --openscenario bt-x.xosc`.

### Driving score

`POST /api/runs/:id/finish` computes a
[CARLA Leaderboard](https://github.com/carla-simulator/leaderboard)-style
`driving_score = route_completion * infraction_penalty` from the run's
events, using the Leaderboard 2.0 coefficients for collisions and
documented Atlas-specific coefficients for the safety rules that have no
Leaderboard equivalent (see `dashboard/src/driving_score.ts` and
`toolkit/bhutan_sim/driving_score.py`, which are kept identical).

### Dataset and compute planner

The Planner tab and `/api/planner` size a target corpus and price the training
program behind it, so a proposal can quote storage and GPU spend instead of
guessing. Everything is computed in `dashboard/src/planner.ts`; the reference
points are returned with every response in `assumptions`, and can be overridden
per request.

```sh
curl -H "authorization: Bearer $TOKEN" \
  "$BASE/api/planner?scenes=100000&clip_seconds=10&fps=10&cameras=3&resolution=1080p&program=medium_train&gpu=h100_80gb&gpus=4&interruptible=true"
```

What the model assumes, and why:

* **Dataset tiers** — 1k–10k scenes is a proof of concept, 10k–100k is useful
  domain adaptation, 100k+ is where day/night, rain/fog and urban/highway/rural
  can all be covered. Coverage across those conditions matters more than the raw
  count, and for behaviour cloning so does clip length, which is why the planner
  takes seconds-per-scene and frames-per-second rather than a frame total.
* **Storage** — encoded video per camera (4/8/14/28 Mbit/s for 720p/1080p/1440p/4k),
  plus 8 % for labels and manifests, plus one working copy for decoded shards and
  checkpoints. A toy demo lands in the single-GB range, a serious fine-tune in the
  tens of GB, and a broad synthetic programme in the TB range. Instances default to
  a 10 GB disk, so the extra is billed per GB-month.
* **Compute** — each training program carries an aggregate A100-class GPU-hour band
  at a 10M-frame baseline (fine-tune 150–1000, medium 1000–5000, from scratch
  5000–20000), scaled by `(frames/baseline)^0.85` because preprocessing, validation
  and evaluation do not shrink linearly. Other GPUs divide those hours by a
  throughput factor (H100 ≈ 2.2× an A100). Aggregate hours do not change with the
  number of GPUs; wall-clock time does, at a conservative 0.92 per doubling.
* **Price** — list rates are a starting point (A100 80GB ≈ $1.15/h, H100 ≈ $1.80/h
  on marketplaces, with H100s advertised as low as $0.90/h); marketplace pricing
  moves constantly with supply, so pass `usd_per_gpu_hour` for a real quote.
  Interruptible instances are priced at 60 % of on-demand and are only safe with
  checkpointed training.

The response also carries `coverage`: how much accepted collection time the catalog
already holds, expressed as scenes of the planned clip length, and how many hours of
driving remain to hit the target.

## KPI definitions

| KPI | Computation | Month-3 target |
|---|---|---|
| Real-world route coverage | Sum of `duration_s` of accepted runs with `source = vehicle` | 50–100 h |
| Replay completeness | Finished runs with `replay_complete` (GNSS, IMU and events present) | ≥ 95 % |
| Scenario-library coverage | Count of scenarios in the tenant | 100+ |
| Synthetic-to-real alignment review | Distinct families with at least one reviewed template | 20+ |
| Data-quality acceptance rate | Passed segments over all segments | ≥ 90 % |
| Edge-case discovery rate | Events per 100 km by class over total distance | tracked |
| Perception benchmark | Precision and recall by class, weather, lighting and route class from stored evaluations | baseline |
| Evaluation reproducibility | Evaluations whose input hashes are all recorded | 100 % |
| Critical safety-rule violations | Critical events that are logged, classified (rule id) and reviewed | 100 % |
| Data-governance compliance | Released clips that are redacted (or exempt) and consent-traceable | 100 % |

## Evidence packs

`GET /api/runs/:id/evidence` returns a self-describing manifest with the run
row, scenario reference and whether its content hash still matches, telemetry
chunk hashes, quality segments, all events with review state, evaluations with
input hashes, clip governance state and a set of attestations (replay
complete, quality status, privacy status, consent traceability, unreviewed
critical events). The canonical JSON is hashed and HMAC-signed with
`MANIFEST_SIGNING_KEY`; the `key_id` lets partners know which key to verify
against. Exports are audited.

## Data governance

* A clip can only be released when `redaction_status` is `redacted` or `not_required` and a `consent_ref` is recorded; the API refuses otherwise.
* Readers can download only released clips; writers and admins see their tenant's clips for review.
* Runs record `privacy_status`; the quality gates flag video that has not been redacted.
* All raw objects are immutable in R2 and carry their SHA-256 in object metadata.
