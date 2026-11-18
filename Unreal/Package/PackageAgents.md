# AGENTS.md

Guidance for any coding agent working in this CARLA package.

## This directory

This is a packaged CARLA simulator, not a source checkout: the launcher
(`CarlaUnreal.sh`, `CarlaUnreal.exe` on Windows), the cooked content under
`CarlaUnreal/`, the Python client wheels under `PythonAPI/carla/dist/`, the
examples under `PythonAPI/examples/` and the API reference in
`PythonAPI/python_api.md`. `README` has the first steps and `VERSION` the
commits the package was built from.

Only the maps cooked into the package, and the maps of installed content packs
(`Tools/carla_pack.py`), can be loaded. Nothing here can be rebuilt: building,
cooking, packaging and importing assets need a source checkout, which the
`download-carla` skill can fetch.

## Skills

This package ships a library of **vetted CARLA procedures** under
[`Skills/`](Skills/). Each is a directory containing a `SKILL.md` with YAML
frontmatter (`name`, `description`, `compatibility`, `metadata`), the step-by-step
procedure, and often `scripts/` and `references/` beside it.

**These skills are the source of truth** for running the server, installing the
client, driving the simulation through the Python API, scenarios and ROS 2, and
they cover most of the commands those tasks need. It is the library of the
commit in `VERSION`, so it matches this build.

### How to use them

1. Match the user's request against the catalogue below.
2. Read that skill's `SKILL.md` in full before acting — each encodes the
   failure modes that make the naive approach fail.
3. Run its `scripts/check_env.sh` (when `metadata.prerequisites` names one)
   before starting.
4. Follow the procedure rather than improvising; the gotchas sections exist
   because those mistakes have already cost someone a day.

Paths (`CARLA_ROOT` and friends) are configured on first need, not up front.
Never guess a path — several CARLA installs on one machine is normal and the
wrong one fails slowly. This directory is the obvious candidate: offer it to the
user first, then record the answer.

### In a package

- Start the server with `run-carla-ue58-server` in its `package` mode, and
  install the matching client with `install-python-api`, which takes the wheel
  bundled here.
- The `python-api` skills drive that running server. The `scenario-runner`,
  `leaderboard` and `scenic` skills do too, once the matching `setup` skill has
  installed the tool they wrap.
- The rest need a source checkout and cannot run here: building, packaging and
  importing assets (the other `ue58` skills, `ue4`, `ue5`), `docs`,
  `contributing`, and adding ROS 2 publishers or message types. They ship so the
  library is the same everywhere.

### Optional: MCP server

[`Util/AgenticTools/`](Util/AgenticTools/) serves the same skills over MCP
(`list_skills`, `read_skill`, `check_prerequisites`, `get_config`,
`set_config`), which adds environment detection and persistent path config.
[`.mcp.json`](.mcp.json) registers it for this directory; clients that read it
get the tools automatically (needs Node 12 or later). Agents without MCP should
read `Skills/` directly — the files are the source of truth either way.

