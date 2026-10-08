---
name: run-openscenario
description: Runs standard-format scenarios through ScenarioRunner — ASAM OpenSCENARIO 1.x (.xosc, via --openscenario, with catalogs and ParameterDeclaration overrides) and OpenSCENARIO 2.0 (.osc, via --openscenario2, the ANTLR/osc2 pipeline). Validates a file against the schema before running, lists the bundled examples, and reports which OSC features ScenarioRunner actually implements. Use when the user asks to "run an xosc", "run an OpenSCENARIO file", "use the OpenSCENARIO standard", "run a .osc / OSC2 scenario", or hands over a scenario file from another tool.
license: MIT
compatibility: Any OS with a scenario_runner checkout, importable CARLA PythonAPI and a running server. OSC 1.x needs xmlschema==1.0.18; OSC 2.0 needs antlr4-python3-runtime==4.10 and graphviz. Example locations differ by branch (srunner/examples on master, srunner/osc_examples on ue5-master).
metadata:
  group: scenario-runner
  prerequisites: scripts/check_env.sh
  reference: references/openscenario.md
---

# Run an OpenSCENARIO file

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

ScenarioRunner speaks two unrelated standards that happen to share a name:

| | OpenSCENARIO 1.x | OpenSCENARIO 2.0 |
|---|---|---|
| Extension | `.xosc` | `.osc` |
| Format | XML (`<Storyboard>`, `<Act>`, `<Maneuver>`) | a domain-specific *language* |
| Flag | `--openscenario` | `--openscenario2` |
| Implementation | `srunner/tools/openscenario_parser.py` | `srunner/osc2/` (ANTLR grammar) |
| Maturity | broad, partial | experimental |

They are not versions of each other and no file works with both flags.

Unlike Python scenarios, an OSC file carries its **own** map, entities and ego —
so there is no `--scenario` name, no town lookup, and `--reloadWorld` is implied by
whatever `<RoadNetwork>` says.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Find or validate the file (list / validate)
- [ ] Step 3: Run it with the right flag
- [ ] Step 4: Drive or observe the ego if the file expects an external controller
```

### Step 2: Find and validate

```bash
source scripts/env.sh

bash scripts/run_openscenario.sh list                  # bundled .xosc and .osc, per branch
bash scripts/run_openscenario.sh validate path/to.xosc # schema + map + entity check, no server
```

`validate` is worth running first: it resolves the `<RoadNetwork><LogicFile>` map
against the server's map list, checks catalog references, and reports the
`ParameterDeclarations` you can override — all of which otherwise fail several
seconds into a run, after the world has already been reloaded.

### Step 3: Run

```bash
source scripts/env.sh

# OpenSCENARIO 1.x
bash scripts/run_openscenario.sh run "$SCENARIO_RUNNER_ROOT/srunner/examples/FollowLeadingVehicle.xosc"

# with global parameter overrides — note the 'key: value' comma-separated form
PARAMS='leadingSpeed: 8.0, egoSpeed: 12.0' \
    bash scripts/run_openscenario.sh run .../FollowLeadingVehicle.xosc

# OpenSCENARIO 2.0
bash scripts/run_openscenario.sh run2 "$SCENARIO_RUNNER_ROOT/srunner/examples/cut_in_and_slow_right.osc"
```

Direct equivalents:

```bash
python3 "$SCENARIO_RUNNER_ROOT/scenario_runner.py" --openscenario  file.xosc --openscenarioparams 'k: v'
python3 "$SCENARIO_RUNNER_ROOT/scenario_runner.py" --openscenario2 file.osc
```

`--openscenarioparams` overrides only entries in the **global**
`ParameterDeclarations`; per-story parameters are not reachable. The format is
`'name: value, name2: value2'` — it is split on `,` then on `:` and both sides are
stripped, so a value containing either character breaks the parse.

The OSC timeout is hard-coded to `100000` seconds for both flags. A stuck OSC run
does not time out on its own; interrupt it.

### Step 4: Who drives the ego

An `.xosc` that assigns a controller (`<ObjectController>` with the CARLA
`NpcVehicleDriving` / `simple_vehicle_control` controllers) drives itself. One
that declares the ego with an *external* controller does not — same silent
"nothing happens" as with Python scenarios:

```bash
python3 "$SCENARIO_RUNNER_ROOT/manual_control.py" -a --rolename=hero
```

`OscControllerExample.xosc` is the file to read for how controllers are wired.

## Examples

**Example 1: "run this xosc I got from another tool"**

`validate` first. The two usual rejections are a map name the server does not have
(`<LogicFile filepath="Town04">` needs Town04 loaded/available) and OSC features
ScenarioRunner does not implement — `validate` names both. Then `run`.

**Example 2: "sweep a parameter"**

```bash
for v in 4 8 12; do
  PARAMS="leadingSpeed: $v" bash scripts/run_openscenario.sh run .../FollowLeadingVehicle.xosc
done
```

**Example 3: "try OpenSCENARIO 2.0"**

`list` shows the `.osc` corpus (`cut_in_and_slow_right.osc`, `overtake1.osc`,
`follow_trajectory.osc`, …). On `ue5-master` they live in
`srunner/osc_examples/`, not `srunner/examples/`. Expect gaps: the osc2 pipeline
parses a subset of the language and raises from `srunner/osc2/` on the rest.

## Troubleshooting

**Problem: `File does not exist`**
Cause: ScenarioRunner checks the path before anything else, and relative paths
resolve against your CWD, not `$SCENARIO_RUNNER_ROOT`.
Solution: use an absolute path.

**Problem: `xmlschema.validators.exceptions.XMLSchemaValidationError`**
Cause: the file targets a newer OpenSCENARIO revision than the bundled schema
(`srunner/openscenario/0.9.x/*.xsd` + `xmlschema==1.0.18`).
Solution: `validate` reports the offending element. Remove or downgrade it; there
is no schema switch.

**Problem: the scenario loads but an action never happens**
Cause: an unimplemented OSC feature. `openscenario_parser.py` silently skips some
actions/conditions rather than failing.
Solution: check the support matrix in
[references/openscenario.md](references/openscenario.md).

**Problem: `--openscenarioparams` ignored, with `WARN: Ignoring --openscenarioparams`**
Cause: it was passed without `--openscenario` (e.g. with `--openscenario2`, which
has no parameter override).
Solution: put the values in the file for OSC 2.0.

**Problem: `ModuleNotFoundError: No module named 'antlr4'`**
Cause: OSC 2.0 needs the ANTLR runtime.
Solution: `pip install antlr4-python3-runtime==4.10`. The version matters — the
generated parser in `srunner/osc2/osc2_parser/` is pinned to 4.10.

**Problem: catalog references fail to resolve**
Cause: `<CatalogLocation>` paths are relative to the `.xosc` file.
Solution: keep the file next to its `catalogs/` directory, as the bundled examples
do.

## Outputs

The scenario executed on the server, with the same criteria report as any other
ScenarioRunner run (`OUTPUT=1`, `JSON=1`, `OUTPUT_DIR` all work the same way).
`validate` prints a schema verdict, the map required, the entities and the
overridable parameters, and exits non-zero if the file cannot run.

Feature support and the OSC 2.0 pipeline are covered in
[references/openscenario.md](references/openscenario.md).
