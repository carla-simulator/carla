# System tests

End-to-end checks of a CARLA UE 5.8 source build. The suite covers offline checks, the CI smoke suite, the agentic skill scripts, the example scripts and stress tests. Each test ends with a verdict that does not depend on reading logs: PASS, FAIL, SKIP, XFAIL (a known bug) or XPASS (a known bug that no longer reproduces).

```sh
python3 PythonAPI/test/system/run_system_tests.py --list          # what would run
python3 PythonAPI/test/system/run_system_tests.py                 # T0,T2,T3,T4,T5 (~5 h)
python3 PythonAPI/test/system/run_system_tests.py --tiers T0      # offline only
python3 PythonAPI/test/system/run_system_tests.py --only T3.set-weather,T5.churn
python3 PythonAPI/test/system/run_system_tests.py --resume Build/test-results/system/<run>
```

The runner starts and stops the server itself. It uses the editor binary in `-game` mode, headless (`-RenderOffScreen`) with `-ros2`, started through `Skills/ue58/run-carla-ue58-server/scripts/run_server.sh`. No other server may be running on the same ports.

Each run writes `Build/test-results/system/<timestamp>/`:

- `results.jsonl`: one line per test, with status, message, metrics and the server's memory use before and after.
- `report.md`: summary per tier, a table of all tests, stress metrics, and failures with log tails.
- `logs/<id>.log`: every command a test ran, with its output.
- `artifacts/<id>/`: images, point clouds, JUnit XML and other outputs.
- `server/NN-portP/`: the log of each server start.

## Tiers

| Tier | Server | What it covers |
|---|---|---|
| T0 | none | Unit tests, skill frontmatter and syntax, `--help` of every python-api skill script, examples compile, `doc_gen.py` reproduces `Docs/python_api.md`, AgenticTools Node and pytest suites, MCP `list_skills`/`read_skill` over stdio |
| T2 | :3654 | `Util/Tools/RunSmokeTests.sh`, exactly as CI runs it. Each smoke test case becomes its own result line |
| T3 | :2000 | Every python-api skill script, judged by querying the world afterwards. For example, the vehicles counted with role `autopilot` must match the number requested, speed must drop after `speed-diff`, saved frames must not be blank, and a replay must re-create the recorded actors |
| T4 | :2000 | `PythonAPI/examples/*.py` run headless (`SDL_VIDEODRIVER=dummy`) with a time limit, plus per-script checks |
| T5 | :2000, fresh server each | Stress: spawn/destroy churn, maximum traffic, many sensors, many clients, map reload loop, determinism, a sync client killed mid-run, long soak |

T3 to T5 use port 2000 because several examples hardcode `localhost:2000`. The Traffic Manager uses its default port 8000, so that port must be free.

## How a verdict is reached

Each test has its own check. After every test that uses the server, the runner also checks:

- the server still answers RPC and the world still ticks;
- the server log has no new crash markers;
- no vehicles, walkers, sensors or controllers were leaked;
- the world is not left in synchronous or no-rendering mode.

Any of these turns a PASS into a FAIL. The runner then cleans up so the next test starts from a known state. If the server died, it is restarted.

Exit codes are not trusted on their own. On this branch the server segfaults on SIGTERM and the Python client can segfault at interpreter exit.

## Adding a test

Put a function in the matching `tests/tN_*.py` file:

```python
@test("T3.my-skill")
def my_skill(ctx):
    out = skill(ctx, "weather", "preset", "ClearNoon")   # runs the script, fails on rc/traceback
    check(ctx.world().get_weather().cloudiness < 10, "preset not applied")
    return {"some_metric": 1.0}                          # lands in results.jsonl
```

- `ctx.sh` and `ctx.bg` run commands and capture their output to the test log.
- `raise Skip(...)` marks a test whose prerequisite is missing.
- `@test(..., xfail="reason")` documents a known bug.
- Stress scripts live in `stress/`. They print one `RESULT {json}` line, and the thresholds are applied in `tests/t5_stress.py`.
