# Tooling outline for a driverless fleet

This outline lists popular, mostly open-source tools worth adding around the
Bhutan pilot as it grows from a data and evaluation platform into a
driverless-fleet programme. Each row says what the tool is for, where it plugs
into the platform, and when in the roadmap it makes sense. Phase 1 needs only
the first two sections; the rest are for Phase 2 and beyond.

- [Simulation and scenario tooling](#simulation-and-scenario-tooling)
- [Data collection, logging and ingestion](#data-collection-logging-and-ingestion)
- [Labeling, dataset management and perception QA](#labeling-dataset-management-and-perception-qa)
- [Autonomy stacks and middleware](#autonomy-stacks-and-middleware)
- [Mapping and localization](#mapping-and-localization)
- [Fleet operations, teleoperation and remote assistance](#fleet-operations-teleoperation-and-remote-assistance)
- [Safety, evaluation and compliance](#safety-evaluation-and-compliance)
- [Vehicle hardware and edge compute](#vehicle-hardware-and-edge-compute)
- [Platform, observability and MLOps](#platform-observability-and-mlops)
- [Suggested adoption order](#suggested-adoption-order)

---

## Simulation and scenario tooling

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **CARLA ScenarioRunner** and **CARLA Leaderboard** | OpenSCENARIO execution, standard route and scenario metrics, driving-score style benchmarks | Wrap `bhutan_sim` templates as ScenarioRunner scenarios once actors need scripted triggers beyond the runner | Weeks 5–8 |
| **Scenic** | Probabilistic scenario language; samples thousands of variants from one specification | Generate Scenic programs from taxonomy families for coverage-driven sampling; CARLA already ships a Scenic interface | Phase 2 |
| **ASAM OpenSCENARIO / OpenDRIVE** | Interchange formats regulators and OEMs expect | Export templates and route archetypes so partners can run them in their own simulators | Weeks 9–12 |
| **esmini** | Lightweight OpenSCENARIO player for fast, headless regression runs | Nightly checks that every template still parses and executes | Phase 2 |
| **RoadRunner** (MathWorks) | Build OpenDRIVE road networks with real elevation and switchbacks | Author a Bhutan corridor map from survey data; CARLA imports the result | Phase 2 |
| **CARLA OpenStreetMap import** | Turns OSM data into drivable CARLA maps | First rough digital route of the pilot corridor without a survey | Weeks 5–8 |
| **AWSIM / Autoware simulation**, **LGSVL (archived)** | Alternative simulators if a customer's stack targets them | Only through the evaluation API; the evidence format stays the same | On demand |
| **Inverted AI, SYNKROTRON** | Learned, human-like traffic behaviour and scenario generation | Optional plug-ins for background traffic realism (already in the CARLA ecosystem) | Phase 2 |

## Data collection, logging and ingestion

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **comma three / openpilot logging (read-only)** | Low-cost camera, GNSS, IMU and CAN capture with proven hardware | Optional ingestion adapter that converts comma logs to the `telemetry.py` schema; keep proprietary data out of comma Connect | Weeks 1–4 |
| **ROS 2 bag (rosbag2 / MCAP)** | Standard multi-sensor recording container | Record real vehicles and CARLA (native ROS 2 bridge) into MCAP; convert topics to the timeline schema | Weeks 5–8 |
| **Foxglove Studio** | Visualise MCAP and ROS 2 data, replay, annotate | Field debugging of captures before upload; can read the same MCAP files | Weeks 1–4 |
| **can-utils, SavvyCAN, cantools** | Capture, decode and reverse-engineer CAN/J1939 signals | Decode read-only truck signals (speed, brake, gear, load) into `Sample` fields | Weeks 1–4 |
| **PlotJuggler** | Time-series inspection of logs | Sensor-synchronisation checks during vehicle instrumentation | Weeks 1–4 |
| **rclone / R2 multipart upload** | Store-and-forward uploads from depots with poor connectivity | Edge logger syncs encrypted run directories to R2 when on Wi-Fi | Weeks 1–4 |
| **Apache Kafka / Cloudflare Queues** | Buffered streaming ingestion once many vehicles report continuously | Replace batch `POST /telemetry` with a queue consumer in the Worker | Phase 2 |

## Labeling, dataset management and perception QA

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **CVAT** | Open-source image and video annotation with tracking, suited to a local labeling workforce | Redacted clips are pulled into CVAT projects; exported labels become ground truth for `evaluate_model.py` | Weeks 9–12 |
| **Label Studio** | Multi-modal labeling incl. event and timeline tagging | Human event labels (interventions, near misses) on the unified timeline | Weeks 9–12 |
| **SAM 2 / Grounding DINO** | Prompt-based auto-labeling and mask proposals | Pre-label frames to cut labeling cost; humans verify | Phase 2 |
| **FiftyOne** | Dataset curation, embeddings, coverage and duplicate analysis | Coverage analysis by weather, lighting, route class; find under-sampled conditions | Weeks 5–8 |
| **DVC / LakeFS** | Versioned datasets and reproducible pipelines | Version released dataset snapshots alongside the D1 catalog manifest | Weeks 9–12 |
| **Anonymizer tools (e.g. understand.ai anonymizer, DeepPrivacy, EgoBlur)** | Face and licence-plate blurring | Redaction step that flips `redaction_status` to `redacted` before release | Weeks 1–4 |
| **nuScenes / Waymo Open / Argoverse devkits** | Public dataset formats and metrics that model teams already use | Export curated Bhutan data in a familiar format for licensing | Phase 2 |

## Autonomy stacks and middleware

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **Autoware (ROS 2)** | Full open-source AV stack with CARLA bridge | Reference model under evaluation; its perception outputs feed the model-adapter contract | Phase 2 |
| **Apollo (Baidu)** | Alternative full stack, strong on planning and HD maps | Evaluation target only | On demand |
| **openpilot** | Production-grade driver-assistance stack | Benchmark its perception and planning on Bhutan scenarios; never used for control in Phase 1 | Phase 2 |
| **ROS 2 (Humble / Jazzy) and DDS** | Middleware for vehicle software; CARLA UE5 ships native ROS 2 | Common message layer between simulation, vehicle and evaluation containers | Weeks 5–8 |
| **Nav2 / MoveIt** | Robotics navigation stacks for shuttles, delivery and industrial robots | Extends the evaluation platform to off-road and low-speed robotics customers | Phase 2 |

## Mapping and localization

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **OpenStreetMap + JOSM** | Base road network and editing | Route model and dashboard basemap tiles (respect tile-usage policy or self-host) | Weeks 1–4 |
| **Lanelet2** | HD lane-level map format used by Autoware | Digital route representation once lane geometry is surveyed | Phase 2 |
| **Vector Map Builder / Tier IV tools** | Lanelet2 authoring | Corridor HD maps | Phase 2 |
| **QGIS** | GIS analysis of risk hot spots, slopes, landslide zones | Overlay event rates on terrain and hazard layers for insurers and ministries | Weeks 9–12 |
| **LIO-SAM, FAST-LIO, KISS-ICP** | Lidar-inertial mapping and localization | Build point-cloud maps of the corridor for simulation calibration | Phase 2 |
| **RTKLIB / NTRIP** | Centimetre GNSS with corrections | Better route reconstruction on narrow mountain roads | Phase 2 |

## Fleet operations, teleoperation and remote assistance

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **Traccar** | Open-source GPS fleet tracking server with hundreds of device protocols | Live vehicle positions for the ops view before custom edge loggers exist | Weeks 1–4 |
| **Fleetbase / OpenFleet** | Open-source fleet and dispatch management | Trip scheduling, driver assignment, depot workflows | Phase 2 |
| **OpenRemote / ThingsBoard** | IoT platforms for device health and alerts | Logger health, storage, connectivity alarms | Phase 2 |
| **WebRTC teleoperation (e.g. Ottopia-style, open WebRTC stacks, FormantOSS ideas)** | Remote assistance and low-latency video for geo-fenced shuttles | Only for Phase 2 shuttles with a safety driver on board | Phase 2 |
| **Grafana** | Ops dashboards and alerting | Complement Atlas KPIs with device and pipeline health | Weeks 5–8 |
| **Valhalla / OSRM / GraphHopper** | Routing engines with truck profiles | Route planning that avoids hairpins beyond turning radius, freight corridor planning | Phase 2 |

## Safety, evaluation and compliance

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **UL 4600, ISO 21448 (SOTIF), ISO 26262 checklists** | Safety case structure for autonomy | Map the evidence-pack attestations to safety-case claims | Weeks 9–12 |
| **Goal Structuring Notation tools (e.g. Assurance Case Tooling)** | Structured safety arguments | Partner-facing safety case built from evidence exports | Phase 2 |
| **OpenSCENARIO-based coverage tools, Foretellix-style ODD coverage** | Operational-domain coverage metrics | Extend KPI "scenario coverage" to ODD-parameter coverage | Phase 2 |
| **CARLA Leaderboard metrics (route completion, infractions)** | Standard driving-quality metrics | Add to the evaluation API for planning-level models | Phase 2 |
| **Great Expectations / pandera** | Data-quality assertions as code | Formalise the quality gates as declarative suites | Weeks 5–8 |
| **OpenTelemetry + Cloudflare Workers observability** | Traces and logs across the API | Audit and incident review | Weeks 5–8 |

## Vehicle hardware and edge compute

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **NVIDIA Jetson Orin** | Edge inference and multi-camera capture | Edge logger with on-device redaction | Phase 2 |
| **Raspberry Pi 5 + CAN HAT, ESP32 CAN** | Cheap read-only CAN/J1939 taps | Passive telemetry on partner trucks | Weeks 1–4 |
| **Ouster / Hesai / Livox lidar, FLIR ADK thermal** | Additional sensors for fog and night | Sensor-fusion benchmarks in fog and night families | Phase 2 |
| **u-blox ZED-F9P / F9R** | RTK GNSS and dead reckoning | Better GNSS in valleys and tunnels | Phase 2 |
| **Dashcams with GNSS overlay (e.g. Viofo, BlackVue)** | Zero-integration camera capture | Fastest way to reach the 50–100 h coverage target | Weeks 1–4 |

## Platform, observability and MLOps

| Tool | What it adds | Integration point | When |
|---|---|---|---|
| **Cloudflare D1, R2, Queues, Durable Objects, Workers AI** | Serverless catalog, storage, streaming, live state and inference at the edge | Already used for the dashboard; Durable Objects for live vehicle sessions, Workers AI for lightweight redaction triage | Now / Phase 2 |
| **PostgreSQL + PostGIS (via Hyperdrive)** | Spatial queries at scale | Replace D1 when spatial joins are needed | Phase 2 |
| **MLflow / Weights & Biases** | Experiment tracking for customer models | Link evaluation ids to training runs | Phase 2 |
| **Docker + Kubernetes / Nomad** | Containerised model runners and CARLA workers | Run customer model containers against scenarios in isolation | Weeks 5–8 |
| **Argo Workflows / Prefect / Dagster** | Pipeline orchestration | Nightly regeneration, evaluation and snapshot pipelines | Phase 2 |
| **Keycloak / Cloudflare Access** | SSO and per-partner access control | Replace static tokens with identity-based access | Weeks 9–12 |
| **HashiCorp Vault / Cloudflare Secrets** | Key management for manifest signing | Rotate `MANIFEST_SIGNING_KEY` with versioned key ids | Weeks 9–12 |

## Suggested adoption order

1. **Weeks 1–4**: dashcams or comma hardware for capture, CAN tools for read-only signals, Foxglove for field checks, an anonymizer for redaction, Traccar for live positions, OSM for the route model.
2. **Weeks 5–8**: ScenarioRunner and OpenSCENARIO export, ROS 2 and MCAP recording, FiftyOne for coverage analysis, Great Expectations for quality gates, containerised model runners, Grafana.
3. **Weeks 9–12**: CVAT and Label Studio with a local labeling team, DVC for dataset snapshots, QGIS risk overlays, SSO and key rotation, safety-case mapping to UL 4600 and SOTIF.
4. **Phase 2**: Autoware or openpilot as evaluation targets, Scenic for coverage sampling, Lanelet2 HD maps, lidar and RTK, teleoperation for geo-fenced shuttles, PostGIS and orchestration at scale.
