# bhutan_sim: Bhutan mobility-data and AV-evaluation toolkit

Python package and scripts implementing the real-to-synthetic-to-evaluation
loop for the Bhutan pilot on top of the CARLA client API. Everything except
`bhutan_sim/runner.py` runs without a CARLA server. Full documentation lives in
`Docs/bhutan_pilot.md`; the dashboard is documented in `Docs/bhutan_dashboard.md`.

```
bhutan_sim/
  taxonomy.py      scenario families with parameter ranges (scenarios/taxonomy.json)
  scenario.py      versioned, hashable ScenarioTemplate
  library.py       deterministic generation of 100+ templates (scenarios/library.json)
  weather.py       Bhutan weather/lighting presets -> carla.WeatherParameters
  runner.py        runs a template in CARLA, records telemetry, events, ground truth
  telemetry.py     unified timeline schema, JSONL logger, dashboard client
  safety_rules.py  explainable per-vehicle-class safety rules (SR-01..SR-08)
  quality.py       segment-level quality gates and replay completeness
  evaluation.py    perception benchmark by class/weather/lighting/route + failure clusters
  route.py         real GNSS trace -> route archetypes -> recommended families
scripts/
  generate_library.py   build the library, optionally upload it
  run_scenario.py       execute templates in CARLA (needs the carla package)
  baseline_detector.py  synthetic detector to validate the measurement pipeline
  evaluate_model.py     benchmark detections against ground truth
  check_quality.py      quality gates over a run directory
  upload_run.py         push a run directory to the dashboard
  route_profile.py      profile a real GNSS trace
  seed_demo.py          seed a dashboard with synthetic demo data (no CARLA)
tests/                  unit tests (python -m unittest discover -s tests)
```

Quick start:

```sh
cd PythonAPI/bhutan
python -m unittest discover -s tests
python scripts/generate_library.py
python scripts/run_scenario.py --scenario bt-landslide-debris-01 --map Town10HD_Opt   # with a CARLA server running
python scripts/baseline_detector.py --run-dir _out/bhutan_runs/<run>
python scripts/evaluate_model.py --run-dir _out/bhutan_runs/<run> --detections _out/bhutan_runs/<run>/detections-baseline.jsonl --model-id baseline-bev
```

Environment variables `BHUTAN_DASHBOARD_URL`, `BHUTAN_DASHBOARD_TOKEN` and
`BHUTAN_TENANT` set the dashboard for every `--upload` flag.
