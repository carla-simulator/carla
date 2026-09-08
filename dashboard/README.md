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
