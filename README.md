# OA Driverless Vision

**A synthetic-first mobility-intelligence platform for roads the AV industry has never seen.**

OA Driverless Vision is a data dashboard and evaluation platform for collecting real-world road
data, generating locally relevant synthetic driving scenarios, evaluating autonomous-driving and
ADAS models against them, and producing auditable safety evidence. Bhutan is the reference
environment; the platform is designed to be portable to Nepal, India, Southeast Asia, Africa,
Latin America and any region with complex, underrepresented road conditions.

This is **not** a public-road autonomous-driving service. It is the validation and
operational-data layer — a "Waymo-grade evidence stack" — for countries and fleets that lack the
local maps, driving datasets, simulation coverage and safety-evaluation tooling needed to
responsibly deploy ADAS, shuttles, freight automation or robotics.

```
Local real-world routes and fleet logs
                ↓
Privacy processing, labeling, and quality controls
                ↓
Digital routes + synthetic scenario variations
                ↓
Replay and evaluation against ADAS/AV models
                ↓
Risk metrics, failure clusters, safety evidence
                ↓
Improved data collection and targeted synthetic generation
```

## Repository map

The main app is the **data dashboard** at the root of this monorepo; it links every other
component together.

| Directory | What it is |
|---|---|
| [`dashboard/`](dashboard) | **The main app.** "Atlas" operations dashboard and evaluation API on Cloudflare Workers: D1 metadata catalog, R2 raw storage, route playback, KPI dashboard, fleet map, safety review, clip governance, signed evidence exports, Prometheus metrics, OpenAPI. |
| [`toolkit/`](toolkit) | Python package `bhutan_sim`: scenario taxonomy and 100+ parameterized templates, CARLA scenario runner, unified telemetry schema, safety rules, quality gates, perception benchmark, driving score, OpenSCENARIO export, GPX/MCAP/Traccar adapters, upload/seed scripts. |
| [`carla/`](carla) | The [CARLA](http://carla.org) open-source driving simulator (Unreal Engine 5.5, `ue5-dev`), vendored as the synthetic-world engine. Build and use it exactly as upstream — see [`carla/README.md`](carla/README.md). |
| [`docs/`](docs) | Platform documentation: [pilot overview](docs/bhutan_pilot.md), [dashboard guide](docs/bhutan_dashboard.md), [fleet tooling survey](docs/bhutan_fleet_tools.md), [detailed roadmap](docs/bhutan_roadmap.md). |
| [`.github/`](.github) | CI: dashboard/toolkit tests and Cloudflare deploy; CARLA UE5 build pipelines (scoped to `carla/**`). |

## Platform layers

| Layer | Phase-1 function | Where it lives today | Long-term product |
|---|---|---|---|
| **Meridian01 · Field Data** | Camera, GNSS/IMU, read-only CAN/J1939, route and event capture | `toolkit/bhutan_sim/telemetry.py`, `route.py`, adapters | Managed data-collection kit and fleet telemetry product |
| **Compass02 · Evaluation** | Scenario scoring, perception metrics, intervention/event analysis, risk rules | `toolkit/bhutan_sim/evaluation.py`, `safety_rules.py`, `driving_score.py` | Model-evaluation and safety-evidence SaaS/API |
| **Tidewater03 · Synthetic World** | CARLA scenarios, route reconstruction, weather and traffic variation, counterfactual replay | `carla/`, `toolkit/bhutan_sim/scenario.py`, `library.py`, `runner.py`, `openscenario.py` | Synthetic-data generation and simulation platform |
| **Horizon Sense04 · Perception QA** | Annotation, sensor-fusion benchmark, coverage analysis, edge-case review | `toolkit/bhutan_sim/quality.py`, `evaluation.py`, clip review in the dashboard | Dataset QA, labeling, perception-validation service |
| **Atlas05 · Operations** | Route dashboard, playback, approvals, safety workflows, evidence exports | `dashboard/` | Fleet, city, insurer and regulator portal |

## Quick start

**Dashboard (the main app):**

```sh
cd dashboard
npm install
cp .dev.vars.example .dev.vars
npm run db:migrate:local
npm run dev                                   # http://127.0.0.1:8787
(cd ../toolkit && python scripts/seed_demo.py)  # seed demo data, no CARLA needed
npm run check                                 # typecheck + unit tests
```

**Toolkit (scenarios, evaluation, ingestion — no CARLA server required except `run_scenario.py`):**

```sh
cd toolkit
python -m unittest discover -s tests
python scripts/generate_library.py            # build the 100+ scenario templates
python scripts/export_scenario.py --scenario bt-landslide-debris-01 --out-dir _out/xosc
```

**Simulator (optional, needs a GPU workstation):** build CARLA from [`carla/`](carla/README.md),
start a server, then `python toolkit/scripts/run_scenario.py --scenario bt-landslide-debris-01`.

## Feature map

The development map for the 12-week pilot and beyond, by layer. `[x]` shipped, `[ ]` to develop.
The fine-grained, actionable list (with target files and upstream tools) is maintained in
[`docs/bhutan_roadmap.md`](docs/bhutan_roadmap.md).

### Meridian01 · Field Data — collect

- [x] Unified timeline schema for simulated and real capture (video refs, GNSS, IMU, CAN-derived speed/brake, weather, events)
- [x] Passive-capture run directories with JSONL telemetry, manifests and store-and-forward upload (`upload_run.py`)
- [x] Ingestion adapters: GPX dashcam/phone tracks, MCAP (Foxglove/ROS 2), Traccar position forwarding
- [ ] Read-only openpilot/comma rlog ingestion (GPS, IMU, CAN speed/brake) — research component only, own storage, no vehicle control
- [ ] J1939/DBC decoding of truck candump logs (cantools) into throttle/brake/speed signals
- [ ] Android/embedded edge-logger app: encrypted local storage, consent capture, offline-first upload
- [ ] Automated redaction pipeline (faces, plates, depot locations) that flips `redaction_status` before release
- [ ] Consent registry and retention controls wired into the catalog (governance pack exists as policy)
- [ ] Multi-vehicle fleet rollout: per-device provisioning, health monitoring, collection-window scheduling

### Compass02 · Evaluation — measure

- [x] Perception benchmark: precision/recall by object class, weather, lighting and route class, with failure clustering
- [x] Explainable safety rules SR-01..SR-08 per vehicle class; 100% of critical violations logged and reviewable
- [x] CARLA-Leaderboard-style driving score, route completion and infraction penalty per run
- [x] Model adapters: submit detections/trajectories as JSONL against ground truth (`evaluate_model.py`, baseline detector)
- [ ] Containerized model runners: submit a whole model for server-side evaluation
- [ ] Counterfactual replay: same route, same model, varied weather/light/actors — side-by-side deltas
- [ ] Run comparison view: overlay two runs' timelines and event markers
- [ ] nuScenes-format export of ground truth and detections so model teams reuse their tooling
- [ ] Intervention/disengagement analytics once fleet capture includes driver events

### Tidewater03 · Synthetic World — generate

- [x] Bhutan scenario taxonomy: 6 groups, 24 families (blind curves, narrow lanes, monsoon fog, livestock, landslide debris, freight grade/brake-fade events)
- [x] 100+ deterministic, versioned, content-hashed scenario templates (`library.json`)
- [x] CARLA runner with route-archetype selection, weather presets and sensor degradation
- [x] ASAM OpenSCENARIO 1.2 export — templates run in ScenarioRunner, esmini and partner simulators
- [x] Real GNSS trace → route archetypes → recommended scenario families (`route.py`, `route_profile.py`)
- [ ] Scenic program generation per family for probabilistic coverage sampling
- [ ] Route-to-scenario generator: sample new templates directly from a real trace's archetypes
- [ ] Batch runner: execute a whole family in CARLA and upload results with one command
- [ ] Bhutan-specific CARLA assets: road furniture, signage, vehicle liveries, terrain materials
- [ ] Digital-route reconstruction of the pilot corridor (OpenDRIVE from survey + Lanelet2/OSM lane maps)
- [ ] Synthetic-to-real alignment reviews: 20+ expert-reviewed scenario families scored for local fidelity

### Horizon Sense04 · Perception QA — trust the data

- [x] Quality gates Q1–Q5: missing data, timestamp drift, sensor dropouts, route coverage, privacy-processing status
- [x] Replay-completeness metric (target: ≥95% of runs fully synchronized)
- [x] Clip governance: review, release and audit workflow in the dashboard
- [ ] CVAT / Label Studio round-trip: push released clips as labeling tasks, import labels as ground truth
- [ ] FiftyOne dataset export for coverage and duplicate analysis
- [ ] Coverage matrix: scenarios and runs by weather × lighting × route class with gaps highlighted
- [ ] Edge-case discovery dashboard: events per 100 km by class, trending
- [ ] Declarative quality suites (Great Expectations / pandera) exported from the Q-gates
- [ ] Local labeling-workforce workflow (Phase 2)

### Atlas05 · Operations — show and prove

- [x] Cloudflare Workers app: runs, scenarios, evaluations, clips, KPI trends, fleet map, audit log
- [x] Signed (HMAC) evidence-pack exports; GeoJSON/CSV/MCAP export; "Open in Foxglove"
- [x] Prometheus `/api/metrics` for Grafana alerting; OpenAPI spec for partner clients
- [x] Multi-tenant token auth with roles; nightly KPI snapshots via cron
- [ ] deck.gl route playback with synchronized video/telemetry/event timeline
- [ ] Clip player with event markers
- [ ] Partner report page: printable Month-3 evidence summary for fleets, insurers, regulators
- [ ] Notifications: Slack/e-mail webhook when a critical event is uploaded unreviewed
- [ ] Cloudflare Access / SSO in front of the dashboard; expiring read-only share links
- [ ] Per-tenant branding; approvals workflow for dataset releases
- [ ] Streaming ingestion (Queues + Durable Objects) and live vehicle sessions as the fleet grows
- [ ] Versioned dataset catalog snapshots (DVC) tied to signed manifests

## 12-week program

| Weeks | Focus | Key deliverables |
|---|---|---|
| **1–4** | Data foundation and safety boundary | Operational **Data** Domain definition; data schema + governance pack; one vehicle instrumented for **passive capture only**; initial route model and scenario taxonomy |
| **5–8** | Synthetic world and evaluation engine | 100+ parameterized scenario templates; simulation/replay environment; annotation and event-review workflow; evaluation API and KPI dashboard |
| **9–12** | Evidence pack and commercial demo | Controlled, geo-fenced simulation-backed demo (trained safety driver, no autonomous public-road operation); partner-facing web app; benchmark report and safety-evidence export; Phase-2 go/no-go |

### Month-3 KPIs

| KPI | Target |
|---|---|
| Real-world route coverage | 50–100 h of quality-controlled passive capture |
| Replay completeness | ≥95% of runs fully synchronized (video, location, telemetry, events) |
| Scenario-library coverage | 100+ parameterized Bhutan-relevant scenarios |
| Synthetic-to-real alignment | 20+ expert-reviewed scenario families |
| Data-quality acceptance | ≥90% of segments pass quality gates |
| Evaluation reproducibility | 100% of published benchmarks replay from versioned inputs |
| Critical safety-rule violations | 100% logged, classified, reviewed |
| Data-governance compliance | 100% of released clips redacted and consent-traceable |

## Commercial products (long-term)

| Product | Buyer | Model |
|---|---|---|
| Data-collection kit + dashboard | Fleets, cities, research partners | Setup fee + subscription |
| Curated regional datasets | Model developers, OEMs, universities | License / annual access |
| Synthetic scenario packs | ADAS/AV/robotics companies | Per-region / per-ODD license |
| Model-evaluation API | AI and autonomy teams | Usage-based / enterprise |
| Safety-evidence reporting | Fleets, insurers, regulators | Service + recurring reporting |
| Custom digital-route deployment | Cities, industrial parks, freight corridors | Implementation + hosting |

## Safety and data-governance boundaries

- Phase 1 is **read-only**: no steering, acceleration, braking or vehicle-control messages. Ever.
- No autonomous public-road operation; demos are simulation-backed and geo-fenced with a trained safety driver.
- Proprietary data stays in our own encrypted storage with consent workflow, redaction pipeline, signed manifests, role-based access and audit logs — third-party services (e.g. comma Connect) are never the primary dataset store.
- Every released clip is redacted and consent-traceable before it leaves the platform.

## License

CARLA (in [`carla/`](carla)) is MIT-licensed by the CARLA team (see [LICENSE](LICENSE) and its
[Docs](carla/Docs)); CARLA assets are CC-BY. Platform code in `dashboard/`, `toolkit/` and `docs/`
is part of the OA Driverless Vision pilot.
