# Agent skills

CARLA ships a library of 62 vetted procedures for coding agents, under [`Skills/`](https://github.com/carla-simulator/carla/tree/HEAD/Skills) in the repository. An agent working inside a CARLA checkout finds them automatically and uses them instead of improvising from the `Makefile`.

Each skill is a directory holding a `SKILL.md` — YAML frontmatter (`name`, `description`, `compatibility`) followed by the step-by-step procedure — plus the `scripts/` and `references/` it needs. The procedures encode the failure modes that make the obvious approach fail, which is the point of them: they are the source of truth for these tasks, not a summary of the docs.

In practice this means an agent asked to "package CARLA with only Town10" reads `package-carla-ue58` and follows it, instead of guessing at CMake targets and discovering the Zen server requirement the hard way.

- [__What is covered__](#what-is-covered)
- [__Using them__](#using-them)
- [__Writing a skill__](#writing-a-skill)

---
## What is covered

| Group | Skills | Covers |
|---|---|---|
| `setup` | 6 | Downloading CARLA, installing the Python API, Scenario Runner, Leaderboard and Scenic |
| `python-api` | 23 | Driving a running server: spawning, sensors, traffic, weather, maps, recording, debug drawing |
| `ue58` | 8 | Building, packaging and running UE 5.8, importing maps, props and walkers, authoring vehicles, and running Autoware |
| `ue4` | 7 | The same for the UE 4.26 branch |
| `ue5` | 1 | UE 5.5 feature-support checks |
| `scenario-runner` | 5 | Writing and running scenarios, including OpenSCENARIO, and reading the results |
| `leaderboard` | 5 | Writing, packaging and evaluating a Leaderboard agent |
| `scenic` | 2 | Authoring and running Scenic scenarios |
| `ros2` | 3 | The native ROS 2 interface: publishers, message types, RViz |
| `docs` | 2 | Auditing the documentation for gaps and writing the pages that fill them |

---
## Using them

Agents that read [`AGENTS.md`](https://github.com/carla-simulator/carla/blob/HEAD/AGENTS.md) pick the library up on their own — that file carries the full catalogue and the repository conventions. No setup is needed beyond working inside the checkout.

The same library is also served over MCP by [`Util/AgenticTools/`](https://github.com/carla-simulator/carla/tree/HEAD/Util/AgenticTools), registered for the project by `.mcp.json`, which runs it with Node. The MCP tools add what plain file reading cannot do:

- `list_skills` — the catalogue, with each skill marked available or not depending on whether the checkout or install it needs is configured
- `read_skill` — a skill's `SKILL.md`, prefixed with its absolute directory so the relative `scripts/...` paths in it resolve
- `check_prerequisites` — runs that skill's read-only environment check and reports what is missing
- `get_config` / `set_config` — the configured paths (`CARLA_ROOT` and the engine-specific variables derived from it), persisted between sessions, with the CARLA installs found on this machine offered as candidates

Paths are asked for when a task first needs one, never guessed: several CARLA checkouts on one machine is normal, and the wrong one fails slowly.

---
## Writing a skill

Follow the conventions of the existing ones: a single `SKILL.md` with frontmatter, a `scripts/check_env.sh` named by `metadata.prerequisites` when the skill needs an environment, and a `references/` file for anything too long for the procedure itself. Keep the gotchas in the document — a skill that omits why the naive approach fails is not worth more than the `Makefile`.
