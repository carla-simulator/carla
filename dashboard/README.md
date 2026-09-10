# Bhutan Mobility Atlas (Cloudflare Workers)

Operations dashboard and evaluation API for the Bhutan mobility-data pilot.
One Worker serves the JSON API (`/api/*`) and the static dashboard; D1 holds
the metadata catalog, R2 holds raw telemetry and clips, a cron trigger writes
nightly KPI snapshots. See `../docs/bhutan_dashboard.md` for the full guide and
`../docs/bhutan_roadmap.md` for the feature/integration to-do list.

```sh
npm install
cp .dev.vars.example .dev.vars
npm run db:migrate:local
npm run dev                                  # http://127.0.0.1:8787
(cd ../toolkit && python scripts/seed_demo.py)   # demo data
npm run typecheck
npm test                                     # node:test unit tests
```

Beyond the run/scenario/evaluation/clip API, the Worker also exposes:

* `GET /api/runs/:id/export/{geojson,csv,mcap}` and `GET /api/scenarios/:id/export/xosc`
  — export to [kepler.gl](https://github.com/keplergl/kepler.gl)/QGIS,
  pandas, [Foxglove Studio](https://github.com/foxglove/studio) and
  [CARLA ScenarioRunner](https://github.com/carla-simulator/scenario_runner)/[esmini](https://github.com/esmini/esmini).
* `GET /api/fleet/live`, `/api/ingest/traccar`, `/api/ingest/osmand` — live
  fleet positions from [Traccar](https://github.com/traccar/traccar) or the
  OsmAnd phone protocol, shown on the Fleet tab.
* `GET /api/planner` — dataset size, storage and GPU-cost bands for a target
  corpus (scenes, clip length, cameras, resolution) on marketplace GPUs, with the
  catalog's current coverage against that target. Powers the Planner tab.
* `GET /api/coverage` — ODD coverage matrix: scenario variants and the runs
  recorded against them, crossed by visibility and lighting class. A cell is
  covered at `min_runs` accepted runs (default 3), thin when variants exist but
  too few runs do, and a gap when nothing is there; gaps come back worst first.
  Runs with no scenario behind them land in an `unlabelled` row and column that
  is shown but excluded from the coverage percentage.
* `GET /api/scenes`, `GET /api/scenes/:id` — synthetic demo scenes, generated
  from a fixed seed. No tenant data and no authentication, so the Demo scenes
  tab works on a fresh deployment before a single run is ingested. Each scene
  carries road geometry, ego and actor tracks at 10 Hz, a sensor model and
  events, and sits in an ODD cell the coverage matrix reports as a gap. The
  lidar is not shipped — the viewer simulates the returns in the browser by
  casting rays against the same geometry, so a scene stays around 70 kB.
* `GET /api/metrics` — Prometheus exposition for Grafana.
* `GET /api/openapi.json` — OpenAPI 3.1 description of the whole API.
* A [CARLA Leaderboard](https://github.com/carla-simulator/leaderboard)-style
  driving score computed on `POST /api/runs/:id/finish`.

Deploy:

```sh
npx wrangler d1 create carla-bhutan-atlas    # put database_id in wrangler.toml
npx wrangler r2 bucket create carla-bhutan-atlas-data
npm run db:migrate:remote
npx wrangler secret put API_TOKENS           # token=tenant:role;...
npx wrangler secret put MANIFEST_SIGNING_KEY
npm run deploy
```
