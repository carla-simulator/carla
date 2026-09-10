# Bhutan Atlas roadmap: features and open-source AV tool integrations

This is the working to-do list for the `dashboard/` Worker ("Atlas") and the
`toolkit` toolkit. It ranks the next dashboard features and the
popular GitHub autonomous-vehicle tools worth integrating, and records what
has already landed. The longer survey of tools by category lives in
[Tooling outline for a driverless fleet](bhutan_fleet_tools.md); this page is
the actionable subset.

Legend: `[x]` shipped, `[ ]` open. Items are ordered by value to the pilot.

- [Integrations with GitHub AV tools](#integrations-with-github-av-tools)
- [Packages to research next](#packages-to-research-next)
- [Dashboard features](#dashboard-features)
- [Toolkit and pipeline features](#toolkit-and-pipeline-features)
- [Platform and engineering](#platform-and-engineering)
- [Iteration log](#iteration-log)

---

## Integrations with GitHub AV tools

| Done | Tool (GitHub) | What the integration does | Where |
|---|---|---|---|
| [x] | **MCAP / Foxglove** ([foxglove/mcap](https://github.com/foxglove/mcap), [foxglove/schemas](https://github.com/foxglove/schemas)) | Export any run as an MCAP file with `foxglove.LocationFix` and `foxglove.Log` channels; one-click "Open in Foxglove"; Python reader/writer so MCAP captures from ROS 2 / rosbag2 recorders can be ingested | `dashboard/src/exports.ts`, `toolkit/bhutan_sim/adapters/mcap_io.py` |
| [x] | **CARLA ScenarioRunner / esmini** ([carla-simulator/scenario_runner](https://github.com/carla-simulator/scenario_runner), [esmini/esmini](https://github.com/esmini/esmini)) | Export scenario templates as ASAM OpenSCENARIO 1.2 `.xosc` files so they run in ScenarioRunner, esmini or partner simulators | `dashboard/src/openscenario.ts`, `bhutan_sim/openscenario.py` |
| [x] | **CARLA Leaderboard metrics** ([carla-simulator/leaderboard](https://github.com/carla-simulator/leaderboard)) | Route completion, infraction penalty and driving score computed from run events with the Leaderboard penalty table; stored per run and shown in the runs table | `dashboard/src/driving_score.ts`, `bhutan_sim/driving_score.py` |
| [x] | **Traccar** ([traccar/traccar](https://github.com/traccar/traccar)) | Accept Traccar position forwarding (JSON) and generic position posts; live fleet map and per-device tracks | `dashboard/src/routes/fleet.ts`, Fleet tab |
| [x] | **Prometheus / Grafana** ([prometheus/prometheus](https://github.com/prometheus/prometheus), [grafana/grafana](https://github.com/grafana/grafana)) | `/api/metrics` exposes every KPI and edge-case rate in Prometheus text format for Grafana alerting | `dashboard/src/metrics.ts` |
| [x] | **kepler.gl / QGIS** ([keplergl/kepler.gl](https://github.com/keplergl/kepler.gl), [qgis/QGIS](https://github.com/qgis/QGIS)) | GeoJSON and CSV export of routes and events for GIS risk overlays | `dashboard/src/exports.ts` |
| [x] | **GPX dashcam / phone logs** (gpxpy-compatible) | Convert GPX tracks from dashcams and phone loggers into the unified timeline | `bhutan_sim/adapters/gpx.py`, `scripts/convert_run.py` |
| [x] | **OpenAPI / Swagger UI** ([swagger-api/swagger-ui](https://github.com/swagger-api/swagger-ui)) | `/api/openapi.json` describes every endpoint for client generation and partner docs | `dashboard/src/openapi.ts` |
| [ ] | **Scenic** ([BerkeleyLearnVerify/Scenic](https://github.com/BerkeleyLearnVerify/Scenic)) | Generate a Scenic program per family for probabilistic coverage sampling; import sampled scenes as templates | `bhutan_sim/scenic_export.py` |
| [ ] | **openpilot / comma logs** ([commaai/openpilot](https://github.com/commaai/openpilot)) | Read-only rlog ingestion (GPS, IMU, CAN speed/brake) into the timeline via `openpilot-tools` | `bhutan_sim/adapters/openpilot.py` |
| [ ] | **cantools / can-utils** ([cantools/cantools](https://github.com/cantools/cantools)) | Decode J1939 candump logs with a DBC into `throttle`, `brake`, `speed_mps`, payload signals | `bhutan_sim/adapters/can_dbc.py` |
| [ ] | **CVAT / Label Studio** ([cvat-ai/cvat](https://github.com/cvat-ai/cvat), [HumanSignal/label-studio](https://github.com/HumanSignal/label-studio)) | Push released clips as labeling tasks; import exported labels as ground truth | `POST /api/clips/:id/labeling-task`, `scripts/import_labels.py` |
| [ ] | **FiftyOne** ([voxel51/fiftyone](https://github.com/voxel51/fiftyone)) | Dataset export in FiftyOne format for coverage and duplicate analysis by weather, lighting and route class | `scripts/export_fiftyone.py` |
| [ ] | **nuScenes devkit** ([nutonomy/nuscenes-devkit](https://github.com/nutonomy/nuscenes-devkit)) | Export ground truth and detections as nuScenes-style JSON so model teams reuse their tooling | `bhutan_sim/adapters/nuscenes.py` |
| [ ] | **DVC** ([iterative/dvc](https://github.com/iterative/dvc)) | Versioned dataset snapshots tied to the D1 catalog manifest | `scripts/snapshot_dataset.py` |
| [ ] | **Autoware** ([autowarefoundation/autoware](https://github.com/autowarefoundation/autoware)) | Adapter that converts Autoware perception topics (MCAP) into the model-adapter detections contract | `bhutan_sim/adapters/autoware.py` |
| [ ] | **Lanelet2 / OSM** ([fzi-forschungszentrum-informatik/Lanelet2](https://github.com/fzi-forschungszentrum-informatik/Lanelet2)) | Corridor lane maps for route archetypes; OSM basemap self-hosting | `bhutan_sim/route.py` |
| [ ] | **Great Expectations / pandera** ([great-expectations/great_expectations](https://github.com/great-expectations/great_expectations)) | Declarative quality-suite export of the Q1–Q5 gates | `bhutan_sim/quality.py` |
| [ ] | **OpenTelemetry** ([open-telemetry/opentelemetry-js](https://github.com/open-telemetry/opentelemetry-js)) | Traces for API calls and cron jobs | `dashboard/src/index.ts` |
| [ ] | **Cloudflare Queues + Durable Objects** | Streaming ingestion and live vehicle sessions once the fleet grows beyond batch uploads | `dashboard/wrangler.toml` |

## Packages to research next

Candidates that are not yet integrations — each needs a read of the project and a
short note on whether it earns a place in the pilot. Grouped by the question they
answer. `[x]` means the survey is written up and the verdict is recorded here.

**Synthetic scene generation at volume** (the 10k–100k+ scene bands the planner prices)

- [ ] [nvidia-cosmos/cosmos-predict](https://github.com/nvidia-cosmos/cosmos-predict) — world foundation models for AV synthetic data. Question: cost per generated clip versus CARLA rendering at the same diversity.
- [ ] [georghess/neurad-studio](https://github.com/georghess/neurad-studio) and [nerfstudio-project/nerfstudio](https://github.com/nerfstudio-project/nerfstudio) — neural reconstruction of real drives, then re-render under new weather/lighting. The cheapest route from 50 h of Bhutan footage to a 100k-scene corpus.
- [ ] [metadriverse/metadrive](https://github.com/metadriverse/metadrive) — lightweight procedural scenarios; useful for behaviour-cloning volume where photorealism is not the point.
- [ ] [Farama-Foundation/HighwayEnv](https://github.com/Farama-Foundation/HighwayEnv) — fast behaviour scenarios for planner-side evaluation.
- [ ] [tier4/AWSIM](https://github.com/tier4/AWSIM) — Unity simulator paired with Autoware; the alternative to CARLA if partners standardise on Autoware.
- [ ] [OpenDriveLab/Vista](https://github.com/OpenDriveLab/Vista) — a driving world model that continues a real clip under new actions. Question: whether generated continuations are consistent enough to train on, or only to stress-test.
- [ ] [nvidia-cosmos/cosmos-transfer](https://github.com/nvidia-cosmos/cosmos-transfer) — style transfer from a CARLA render to photoreal. The cheap half of the sim-to-real problem if it holds geometry; pairs with the cosmos-predict item above.
- [ ] CARLA's own **Digital Twin Tool** (procedural maps from OpenStreetMap, `carla-simulator/carla` 0.9.15+) — the fastest route from Bhutan's OSM road network to drivable maps, before any hand-built corridor.

**Turning GNSS traces into routes and maps**

- [ ] [valhalla/valhalla](https://github.com/valhalla/valhalla) — Meili map-matching would snap raw device tracks to the road network, which is what the route-archetype work and per-segment quality colouring both need first.
- [ ] [Project-OSRM/osrm-backend](https://github.com/Project-OSRM/osrm-backend) — the lighter alternative if only matching, not routing, is wanted.
- [ ] [OvertureMaps/data](https://github.com/OvertureMaps/data) — a second basemap opinion next to OSM for Bhutan's road coverage; worth a coverage comparison before committing the corridor maps.

**Safety argument and closed-loop checks**

- [ ] [intel/ad-rss-lib](https://github.com/intel/ad-rss-lib) — Responsibility-Sensitive Safety, already integrated with CARLA. An RSS check per run would give the safety case a formal, quotable rule set next to the Leaderboard penalties.
- [ ] [microsoft/presidio](https://github.com/microsoft/presidio) — PII detection in the free-text fields (notes, consent refs, reviewer comments), the part of redaction that EgoBlur does not touch.

**Pipeline plumbing the planner's arithmetic assumes**

- [ ] [NVIDIA/DALI](https://github.com/NVIDIA/DALI) — GPU decode and augmentation. Directly attacks the "preprocessing eats GPU-hours before training starts" line in the planner's assumptions; worth measuring before trusting the sub-linear scaling exponent.
- [ ] [lancedb/lance](https://github.com/lancedb/lance) and [activeloopai/deeplake](https://github.com/activeloopai/deeplake) — columnar/multimodal dataset formats with versioning built in; the alternative to webdataset shards plus DVC rather than a complement to them.
- [ ] [Lightning-AI/litdata](https://github.com/Lightning-AI/litdata) — a third streaming-shard option to benchmark against webdataset and MosaicML streaming on R2 specifically.
- [ ] [dagster-io/dagster](https://github.com/dagster-io/dagster) or [PrefectHQ/prefect](https://github.com/PrefectHQ/prefect) — orchestration for ingest → quality gates → redaction → release, once that pipeline outgrows `scripts/`.
- [ ] [runpod/runpodctl](https://github.com/runpod/runpodctl) — the second marketplace API next to `vast-python`, so the live-price feature quotes more than one supplier.

**Scenario authoring and benchmarks**

- [ ] [pyoscx/scenariogeneration](https://github.com/pyoscx/scenariogeneration) — generates OpenSCENARIO/OpenDRIVE from Python. Would replace the hand-rolled XML writer in `openscenario.py`/`openscenario.ts` if the schema coverage is worth the dependency.
- [ ] [motional/nuplan-devkit](https://github.com/motional/nuplan-devkit) — closed-loop planning metrics; a second scoring opinion next to the Leaderboard driving score.
- [ ] [Thinklab-SJTU/Bench2Drive](https://github.com/Thinklab-SJTU/Bench2Drive) and [autonomousvision/carla_garage](https://github.com/autonomousvision/carla_garage) — closed-loop CARLA benchmarks and baselines to compare a Bhutan-tuned model against.

**Dataset packaging and training pipeline**

- [ ] [webdataset/webdataset](https://github.com/webdataset/webdataset) and [mosaicml/streaming](https://github.com/mosaicml/streaming) — shard formats that stream from R2 to a rented GPU without a full local copy; directly cuts the disk line in the planner's budget.
- [ ] [skypilot-org/skypilot](https://github.com/skypilot-org/skypilot) — launches checkpointed jobs on the cheapest available (including interruptible) GPUs across clouds and marketplaces. The natural executor for a plan the dashboard produces.
- [ ] [vast-ai/vast-python](https://github.com/vast-ai/vast-python) — the marketplace's own CLI/API; the source for live $/hour instead of the planner's static reference rates.
- [ ] [ray-project/ray](https://github.com/ray-project/ray), [mlflow/mlflow](https://github.com/mlflow/mlflow), [aimhubio/aim](https://github.com/aimhubio/aim) — job orchestration and run tracking, so planned GPU-hours can be compared against actual ones.

**Data and visualisation**

- [ ] [rerun-io/rerun](https://github.com/rerun-io/rerun) — an embeddable viewer next to the Foxglove deep link; worth it if `.rrd` export is cheap from the same sample stream.
- [ ] [argoverse/av2-api](https://github.com/argoverse/av2-api), [waymo-research/waymo-open-dataset](https://github.com/waymo-research/waymo-open-dataset), [zenseact/zod](https://github.com/zenseact/zod) — public corpora to pretrain on before Bhutan fine-tuning, and label schemas worth matching.
- [ ] [facebookresearch/EgoBlur](https://github.com/facebookresearch/EgoBlur) — face/plate redaction; the concrete candidate for the pipeline's redaction step.

## Dashboard features

- [x] KPI trend chart from nightly snapshots on the Overview tab.
- [x] Run export menu: GeoJSON, CSV, MCAP and "Open in Foxglove".
- [x] Driving score, route completion and infraction penalty per run.
- [x] Fleet tab: live device positions, last-seen table and per-device track playback.
- [x] Scenario library: download any template as OpenSCENARIO.
- [x] Deep links: `#runs/<run_id>`, `#scenarios/<family>`, `#evaluations/<id>` restore the view.
- [x] Planner tab: dataset size, storage and GPU-cost bands for a target corpus, measured against what the catalog already holds (`/api/planner`).
- [x] Saved plans: name a plan, store it, and diff two plans (target size, GPU, cost) so a budget change is reviewable.
- [x] Budget export: the planner's numbers as CSV and as a printable Markdown section of the partner report.
- [ ] Run comparison: overlay two runs' timelines and event markers.
- [ ] Live GPU prices in the planner: poll the Vast.ai (and RunPod) listing API on the cron trigger, cache in D1, and replace the static reference rates with a real quote plus a price sparkline.
- [ ] Training-run registry: register an actual training job (dataset snapshot hash, GPU, hours, spend) and show planned versus actual next to the model's evaluation rows.
- [ ] Inverse planner: enter a budget ceiling and get the corpus it buys — the question a partner actually asks, and the mirror of the plan the tab produces today.
- [ ] Plan revisions: keep every save of a plan instead of overwriting, so a budget's history is auditable and any two revisions diff with the machinery `/api/plans/diff` already has.
- [ ] Plan sensitivity (tornado chart): which single input — scenes, cameras, resolution, program, rate — moves the total most, so the negotiation starts with the variable that matters.
- [ ] Programme view: sum several saved plans into one budget with a total band and a month-by-month spend line.
- [ ] Drift alerts: when a saved plan's repriced total moves more than N % (the `drift` field is already computed on every read), raise it on the Overview tab and through the notification webhook below.
- [ ] Egress and transfer line in the plan: R2 egress plus marketplace ingress for the shards, which the current model ignores entirely.
- [ ] Energy and carbon estimate per plan: kWh from GPU-hours × board power × PUE, and gCO2e at the grid intensity of the region the instance runs in — Bhutan's hydro grid is a genuine argument to make in the partner report.
- [ ] Cost attribution by scenario family: which families consumed the accepted hours, so the next generation batch is aimed at the expensive gaps.
- [ ] Cost KPIs: dollars per accepted hour, per released clip and per discovered edge case, on the Overview tab.
- [ ] Collection plan from the coverage gap: turn "still to collect" into a per-cell (weather × lighting × route class) target and a scenario-generation batch.
- [ ] Map overlays: event heatmap by class, per-segment quality colouring on the route.
- [ ] Pagination and free-text search on runs, scenarios and evaluations.
- [ ] Reviewer notes on scenario and event review dialogs (the API already stores them).
- [ ] Partner report page: printable Month-3 summary from a KPI snapshot.
- [ ] Notifications: webhook (Slack, e-mail via Workers) when a critical event is uploaded unreviewed.
- [ ] Clip player with event markers for released clips.
- [ ] Route archetype view from `route_profile.py` output (curvature, grade histograms).
- [ ] Coverage matrix: scenarios and runs by weather × lighting × route class with gaps highlighted.
- [ ] Per-tenant branding and read-only share links with expiring tokens.

## Toolkit and pipeline features

- [x] `scripts/convert_run.py`: GPX, MCAP and Traccar exports into a run directory.
- [x] `scripts/export_scenario.py`: OpenSCENARIO export of one template, a family or the whole library.
- [x] Driving score in `quality.json` and in the dashboard's `finish` step.
- [ ] Redaction step (EgoBlur or understand.ai anonymizer) that flips `redaction_status`.
- [ ] Batch runner: run every template of a family in CARLA and upload with one command.
- [ ] Route-to-scenario generator: sample templates from a real GNSS trace's archetypes.
- [ ] Scenario diff tool: what changed between two library versions and which reviews it invalidated.
- [ ] ROS 2 native bridge recipe: record CARLA UE5 ROS 2 topics into MCAP and ingest.
- [ ] `scripts/plan_budget.py`: the planner's arithmetic in Python so a proposal can be costed without the Worker, sharing the reference tables.
- [ ] `scripts/save_plan.py`: push a costed proposal straight into `/api/plans` from a notebook, so the budget in a document and the one in the dashboard are the same record.
- [ ] WebDataset/MosaicML shard export from R2 so a rented GPU streams the corpus instead of downloading it.
- [ ] SkyPilot job template that launches a checkpointed run on an interruptible instance sized by a saved plan.

## Platform and engineering

- [x] Worker unit tests (`npm test`) for router, auth, exports, OpenSCENARIO, driving score and metrics.
- [x] D1 migration `0002_fleet_and_scores.sql`, `0003_plans.sql`.
- [ ] Vitest with `@cloudflare/vitest-pool-workers` for end-to-end route tests against Miniflare.
- [ ] Cloudflare Access / SSO in front of the dashboard; keep bearer tokens for machine clients.
- [ ] Key rotation for `MANIFEST_SIGNING_KEY` with multiple `key_id`s.
- [ ] Rate limiting per token on write endpoints.
- [ ] Retention policy job: expire raw chunks of rejected runs after N days.

## Iteration log

This roadmap is worked one feature per iteration: each pass picks the highest-value
open item above, ships it with tests, ticks the box and adds a line here. Newest first.

### Iteration 3 — saved plans, plan diff and budget export

A plan you cannot save is a plan you cannot argue about. This ships named budget
plans, a field-by-field diff between any two, and the export that puts one in
front of a partner.

The design decision worth recording: **a saved plan stores only its planner
input**, never its numbers. Every read re-runs `planProgram` over that input, so
a plan saved last quarter is quoted at today's reference rates; the totals
captured at save time are kept beside it purely so the movement shows up as a
`drift` field instead of a silent restatement. That is also what makes the
future live-price feature safe to land — it will move every saved plan at once,
visibly.

* `dashboard/migrations/0003_plans.sql` — `plans(plan_id, tenant_id, name, notes,
  input, saved_totals, created_by, …)`, ids being slugs of the name so links read.
* `dashboard/src/plans.ts` — pure model: `slugPlanId`, `summarize` (the eleven
  headline numbers a budget line quotes), `diffTotals`, `comparePlanRecords`
  (input diffs carry labels, not enum ids), `planToCsv` (flat
  `section,item,value,unit` line items) and `planToMarkdown` (corpus table, a
  low/expected/high cost table and the assumptions, ready to paste into the
  Month-3 report).
* `dashboard/src/routes/plans.ts` — `GET|POST /api/plans`,
  `GET /api/plans/diff?a=&b=`, `GET|DELETE /api/plans/:id`,
  `GET /api/plans/:id/export/{csv,md}`. Totals are computed server-side from the
  submitted input, so a client cannot store numbers the model would not produce;
  saving is tenant-guarded with an explicit 409 rather than a silent no-op.
* Front end: a save form, a saved-plans table showing each plan repriced with its
  drift, per-row Load / CSV / MD / Delete, and an A-vs-B diff table. Only money
  is coloured in the diff — cheaper is good, but fewer scenes is not.
* `dashboard/test/plans.test.ts` — 9 tests (slugs, repricing from input, drift,
  input and metric diffs, CSV quoting, Markdown completeness) plus a router test
  pinning `/api/plans/diff` ahead of `/api/plans/:id`. Verified end to end against
  a local D1 and in the browser.

Open follow-ups it creates: plan revisions instead of overwrite, drift alerts on
the Overview tab, an inverse planner (budget in, corpus out) and a sensitivity
chart — all listed above.

### Iteration 2 — dataset and compute planner

Ships the **Planner** tab and `/api/planner`. It answers the question a pilot budget
starts from: how big does the corpus need to be, what does it cost to store, and
what does training on it cost on marketplace GPUs.

* `dashboard/src/planner.ts` — pure model: dataset tiers (1k–10k proof of concept,
  10k–100k domain adaptation, 100k+ robust), storage from resolution × clip length ×
  cameras, aggregate GPU-hour bands per training program scaled by
  `(frames/baseline)^0.85`, A100/H100/L40S/RTX 4090 throughput and list rates, an
  interruptible discount, and disk rent beyond the default 10 GB instance disk.
* `dashboard/src/routes/planner.ts` — `GET`/`POST /api/planner` add the catalog's
  own coverage (accepted collection time cut into clips of the planned length) and a
  side-by-side comparison across GPUs and programs; `GET /api/planner/options` serves
  the reference tables so the form cannot drift from the API.
* Front end: form, tier banner, cost tiles, two cost charts, a coverage table and the
  assumption list behind every number; `barChart` gained a `format` hook for currency.
* `dashboard/test/planner.test.ts` — 8 tests covering tiers, the baseline hour band,
  storage arithmetic, GPU trade-offs, spot pricing, sub-linear scaling, storage rent
  and query clamping.

Open follow-ups it creates: live marketplace prices, saved plans, budget export and
a training-run registry for planned-versus-actual (all listed above).

### Iteration 1 — exports, scoring and live fleet

1. MCAP, GeoJSON and CSV export endpoints plus the Foxglove deep link.
2. OpenSCENARIO export in both the Worker and the Python toolkit.
3. Leaderboard-style driving score stored on every finished run.
4. Traccar and generic position ingestion with a live Fleet tab.
5. Prometheus metrics and an OpenAPI description of the API.
6. Front end: KPI trends, export menu, XOSC download, hash routing.
7. Python adapters (GPX, MCAP, Traccar) and `convert_run.py`.
8. Worker unit-test suite and CI updates.
