# Bhutan Mobility Atlas (Cloudflare Workers)

Operations dashboard and evaluation API for the Bhutan mobility-data pilot.
One Worker serves the JSON API (`/api/*`) and the static dashboard; D1 holds
the metadata catalog, R2 holds raw telemetry and clips, a cron trigger writes
nightly KPI snapshots. See `Docs/bhutan_dashboard.md` for the full guide.

```sh
npm install
cp .dev.vars.example .dev.vars
npm run db:migrate:local
npm run dev                                  # http://127.0.0.1:8787
(cd ../PythonAPI/bhutan && python scripts/seed_demo.py)   # demo data
npm run typecheck
```

Deploy:

```sh
npx wrangler d1 create carla-bhutan-atlas    # put database_id in wrangler.toml
npx wrangler r2 bucket create carla-bhutan-atlas-data
npm run db:migrate:remote
npx wrangler secret put API_TOKENS           # token=tenant:role;...
npx wrangler secret put MANIFEST_SIGNING_KEY
npm run deploy
```
