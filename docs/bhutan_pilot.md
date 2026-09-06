# Bhutan mobility-data and AV-evaluation pilot

This fork of CARLA adds a synthetic-first mobility intelligence layer for the
Bhutan pilot. Phase 1 is **not** a public-road driverless service. It is a
repeatable system for collecting real-world road data, generating
Bhutan-relevant synthetic scenarios, evaluating autonomous-driving and ADAS
models, and producing auditable safety evidence. Everything below is designed
to be portable to other regions with underrepresented road conditions.

- [What was added to CARLA](#what-was-added-to-carla)
- [The real-to-synthetic-to-evaluation loop](#the-real-to-synthetic-to-evaluation-loop)
- [Scenario taxonomy and library](#scenario-taxonomy-and-library)
- [Running a scenario in CARLA](#running-a-scenario-in-carla)
- [Quality gates, safety rules and evaluation](#quality-gates-safety-rules-and-evaluation)
- [Dashboard](#dashboard)
- [Mapping to the 12-week plan](#mapping-to-the-12-week-plan)
- [Safety boundary](#safety-boundary)

---

## What was added to CARLA

| Layer (proposal name) | Repository location | What it does |
|---|---|---|
| Meridian01 · Field Data | `toolkit/bhutan_sim/telemetry.py`, `route.py` | Unified timeline schema for simulated and passive real-world capture, JSONL logger, upload client, GNSS route profiling |
| Compass02 · Evaluation | `bhutan_sim/evaluation.py`, `safety_rules.py`, `quality.py` | Perception benchmark by class, weather, lighting and route class; explainable safety rules; segment-level quality gates |
| Tidewater03 · Synthetic World | `bhutan_sim/taxonomy.py`, `scenario.py`, `library.py`, `weather.py`, `runner.py` | 24 Bhutan scenario families, 127 parameterized templates, weather presets, CARLA runner with route-archetype selection and sensor degradation |
| Horizon Sense04 · Perception QA | `scripts/baseline_detector.py`, `evaluate_model.py` | Ground-truth export, model-adapter contract, baseline detector for validating the measurement pipeline, failure clustering |
| Atlas05 · Operations | `dashboard/` | Cloudflare Workers application: D1 catalog, R2 raw storage, deck.gl route playback, KPI dashboard, review workflows, signed evidence exports, audit log |

The CARLA engine itself is unchanged. The additions are a Python package that
uses the public CARLA client API and a separate Cloudflare Workers project, so
they track upstream CARLA releases without merge conflicts.

## The real-to-synthetic-to-evaluation loop

```
Local real-world routes and fleet logs          scripts/route_profile.py
                ↓
Privacy processing, labeling, quality controls  scripts/check_quality.py, clip governance in the dashboard
                ↓
Digital routes + synthetic scenario variations  scripts/generate_library.py, bhutan_sim.runner
                ↓
Replay and evaluation against ADAS/AV models    scripts/run_scenario.py, scripts/evaluate_model.py
                ↓
Risk metrics, failure clusters, safety evidence GET /api/kpis, GET /api/runs/:id/evidence
                ↓
Improved collection and targeted generation     recommended_families from route_profile.py
```

Every artefact carries a content hash: scenario templates, telemetry chunks,
ground truth, detections and evaluation inputs. That is what makes "100 % of
published benchmark runs replay from versioned inputs" a checkable claim
rather than a promise.

## Scenario taxonomy and library

`toolkit/scenarios/taxonomy.json` defines six groups and 24 families
that match the seed taxonomy in the proposal:

| Group | Families |
|---|---|
| sight_distance | blind_mountain_curve, crest_limited_sight |
| lane_geometry | narrow_lane, unmarked_lane, faded_markings |
| weather_light | monsoon_rain, valley_fog, night_glare, low_sun, mud_debris_surface |
| vru | pedestrian_roadside, school_zone, motorcycle_oncoming, livestock_on_road, roadside_market_activity |
| obstruction | landslide_debris, roadworks, stalled_vehicle, oncoming_encroachment |
| freight | steep_descent_braking, grade_climb, load_shift_curve, tight_turning_radius, stopping_distance_wet |

Each family declares parameter ranges (curvature, grade, traffic density, lane
quality, speed limit, payload), weather presets, times of day, relative actor
placements and expected events. `generate_library.py` expands them
deterministically into `scenarios/library.json` (127 templates). Regenerating
with the same seed produces byte-identical hashes, and the unit tests fail if
the library drops below 100 templates or 20 families.

Each template records:

* `params`: weather preset, time of day, traffic density, lane quality, road curvature, grade, speed limit, vehicle class, payload, and a `sensor_degradation` block (camera blur, GNSS noise, IMU noise, lidar noise, dropout probability).
* `actors`: roles such as `oncoming_truck`, `motorcycle`, `pedestrian_crossing`, `livestock`, `debris`, `cones`, placed by lane and distance ahead of the ego.
* Derived `route_class`, `lighting_class` and `visibility_class` used for benchmark breakdowns.
* `content_hash` and `review_status` for the synthetic-to-real alignment review.

Known asset gaps are stated in the taxonomy rather than hidden: CARLA has no
livestock mesh, so a slow walker stands in until a custom asset is imported,
and the UE5 catalogue has no motorcycle, so a small car is used with the
`motorcycle` ground-truth class until one is added.

## Running a scenario in CARLA

```sh
cd toolkit
python scripts/generate_library.py                      # writes scenarios/library.json
python scripts/run_scenario.py --scenario bt-valley-fog-01 --map Town10HD_Opt
python scripts/run_scenario.py --family landslide_debris --save-camera --upload \
    --dashboard https://carla-bhutan-atlas.<account>.workers.dev --token <writer token>
```

The runner:

1. Scores spawn points by the curvature and grade of the road ahead and picks the one closest to the template, so a `steep_descent_braking` template lands on a descent even on a stock town map. The measured geometry is recorded in the run notes.
2. Applies the weather preset and time of day, then sets GNSS, IMU and camera noise from the sensor-degradation block.
3. Spawns the ego (truck, shuttle or car), the template's actors and background traffic, all driven by the Traffic Manager.
4. Ticks synchronously at 20 Hz, writing `telemetry.jsonl`, `events.jsonl` and bird's-eye `ground_truth.jsonl` plus a `run.json` manifest with SHA-256 hashes.
5. Emits scenario events (oncoming encroachment, VRU close pass, obstacle ahead), sensor events (collision, lane departure), and the rule violations from the safety rules.
6. Runs the quality gates and, with `--upload`, pushes the run to the dashboard.

Real-world runs use the same file layout. A passive logger writes the same
JSONL records with `source: "vehicle"`, and `scripts/upload_run.py` sends
them to the same API, so simulated and captured data share one timeline.

## Quality gates, safety rules and evaluation

**Quality gates** (`quality.py`) split a run into 30-second segments and check
completeness against the expected rate, timestamp drift and monotonicity,
missing required fields and frozen GNSS while moving. A run passes when at
least 90 % of segments pass and no governance issue is open (for example video
present but redaction not run). The per-run stream flags feed the replay
completeness KPI.

**Safety rules** (`safety_rules.py`) are per-vehicle-class thresholds with
named rule ids so a safety manager can review each violation:

| Rule | Class | Severity |
|---|---|---|
| SR-01 hard braking | hard_brake | warning |
| SR-02 speeding | speeding | warning |
| SR-03 time-to-collision | ttc_low | critical |
| SR-04 lateral acceleration (load shift) | lateral_accel_high | warning |
| SR-05 collision | collision | critical |
| SR-06 downhill overspeed on steep grade | downhill_overspeed | critical |
| SR-07 following headway | lead_vehicle_close | warning |
| SR-08 lane departure | lane_departure | warning |

**Evaluation** (`evaluation.py`) matches bird's-eye detections to ground truth
by centre distance within a class, then reports precision, recall and F1
overall, by class, and by weather, lighting and route class. False negatives
are clustered by class, condition and range band so the report answers "where
does this model fail" rather than only "how well does it do". The report
embeds the hashes of its inputs; the dashboard marks an evaluation
replay-verified when a second run with the same inputs produces the same
counts.

```sh
python scripts/baseline_detector.py --run-dir _out/bhutan_runs/<run>     # pipeline check, not a real model
python scripts/evaluate_model.py --run-dir _out/bhutan_runs/<run> --detections <run>/detections-baseline.jsonl \
    --model-id baseline-bev --model-version 0.1 --upload
```

A customer model adapter only has to emit one JSONL line per frame:
`{"frame": 120, "detections": [{"cls": "pedestrian", "x": 18.2, "y": 2.1, "score": 0.87}]}`.

## Dashboard

See [Bhutan dashboard on Cloudflare Workers](bhutan_dashboard.md) for
deployment, the API and the KPI definitions.

## Mapping to the 12-week plan

| Weeks | Proposal deliverable | Where it lives now |
|---|---|---|
| 1–4 | Data schema for video, CAN/J1939, GNSS, IMU, weather, events | `telemetry.py` record types, `dashboard/migrations/0001_init.sql` |
| 1–4 | Consent, redaction, access-control and retention policy | Clip governance endpoints, role-scoped tokens, audit log, `privacy_status` on every run |
| 1–4 | Initial route model and scenario taxonomy | `scenarios/taxonomy.json`, `route.py` |
| 5–8 | 100+ parameterized scenario templates | `scenarios/library.json` (127) |
| 5–8 | Simulation/replay environment | `runner.py`, `run_scenario.py` |
| 5–8 | deck.gl route playback and unified timeline | Dashboard runs view |
| 5–8 | Model adapters and evaluation API | `evaluate_model.py`, `POST /api/evaluations` |
| 5–8 | Data-quality checks | `quality.py`, `check_quality.py` |
| 9–12 | Partner-facing web application | `dashboard/` |
| 9–12 | Versioned dataset catalog and evidence export | D1 catalog, `GET /api/runs/:id/evidence` (HMAC-signed) |
| 9–12 | Model benchmark report | KPI perception table, evaluation detail view |

## Safety boundary

* The CARLA runner drives simulated vehicles only. Nothing in this package sends steering, throttle, brake or any control message to a real vehicle.
* The real-world truck workflow is read-only capture: camera, GNSS/IMU and read-only CAN/J1939 where the vehicle, owner, insurer and authority have approved it.
* If comma hardware or openpilot-derived logging is used for capture, keep it as an optional ingestion component. Do not use comma Connect as the primary store: keep proprietary data in your own encrypted R2 bucket under your own consent and retention controls.
* Geo-fenced demonstrations use simulation plus telemetry playback with a trained safety driver and no autonomous public-road operation.
