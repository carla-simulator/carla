# carla-agentic-tools

> Part of the CARLA repository. The skill library lives at `Skills/` in the repo
> root, not beside this file; `.mcp.json` at the root registers this server for
> the project, so an MCP client opened in the checkout picks it up with no setup.

A library of **vetted CARLA procedures** ("skills") for any agent. Each skill is
a `SKILL.md` plus executable scripts with its failure modes encoded, so an agent
discovers the right procedure and checks its prerequisites instead of improvising
from the Makefile.

## Quick start

Inside a CARLA checkout there is nothing to do: the root `.mcp.json` starts the
server. To use the skills from another project, register the server by path:

```bash
claude mcp add carla -s user -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
codex mcp add carla -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
```

Or install it as a plugin, which writes no config at all. In Claude Code:

```
/plugin marketplace add carla-simulator/carla-agentic-tools
/plugin install carla@carla-agentic-tools
```

In Codex, from a shell — and in Cursor, from **Customize** in the sidebar:

```bash
codex plugin marketplace add carla-simulator/carla-agentic-tools
```

Nothing in a skill is client-specific — plain Markdown and POSIX shell. A
standalone [MCP](https://modelcontextprotocol.io) server serves them to any MCP
client, and comes in **two implementations**: a self-contained Node one and a
Python one. Same skills, same answers, no wrapper between them —
`tests/test_node_parity.py` runs both and diffs what they return.

The server targets a **specific CARLA instance at runtime**, recorded on first
use, so one checkout's server can drive any build.

## Layout

```
carla/
├── .mcp.json                     # registers the Node server for this checkout
├── Skills/
│   ├── _common/env_common.sh     # every env.sh loads the recorded paths through this
│   ├── setup/                    # get the pieces at all: download-carla, install-python-api,
│   │                             #   install-scenario-runner, install-leaderboard
│   ├── python-api/               # drives any running server (world-data, create-sensor, …)
│   ├── ue4/                      # needs a UE4 checkout (build, package, run, import)
│   ├── ue5/                      # what UE 5.5 cannot do that 5.8 can
│   ├── ue58/                     # needs a UE 5.8 checkout (build, package, run, import, Autoware)
│   ├── ros2/                     # native ROS 2 interface (publishers, msg types, RViz)
│   ├── scenario-runner/          # CARLA's scenario engine (scenarios, OpenSCENARIO, routes)
│   ├── leaderboard/              # the AD Leaderboard on top of it (agents, evaluation, scoring)
│   └── scenic/                   # probabilistic scenarios (write and run .scenic)
└── Util/AgenticTools/
    ├── plugin.json, mcp.json     # Agent Plugin manifests, for Cursor and Codex
    ├── .claude-plugin/           # plugin + marketplace manifests, for Claude Code
    ├── bin/carla-agentic-tools.js # the Node entry point
    ├── lib/                      # the Node server: server.js, skills.js, config.js
    │                             #   zero dependencies, Node >= 12
    ├── src/carla_agentic_tools/  # the Python server: server.py, config.py
    ├── test/node_smoke.js        # Node smoke checks
    └── tests/                    # pytest, including the Node/Python parity checks
```

**Starting from nothing?** Three skills, in order: `download-carla` (fetches a
release and prints the path), `install-python-api` (installs the client wheel from
inside that download, so versions cannot mismatch), `run-carla-server` (detects the
download and launches it). The `setup` group is never gated on an environment
variable, so it stays visible to a user who has no CARLA yet.

Skills are **auto-discovered**: any `Skills/<group>/<name>/SKILL.md` is picked up,
no registration step. The **group** says what a skill binds to — `python-api`
works against any CARLA server regardless of engine version, while `ue4`/`ue5`
need that engine's checkout. `list_skills` reports the group and whether its
environment is present (`available: false` plus a reason when e.g.
`CARLA_UE4_ROOT` is unset); unavailable skills are still listed, because creating
that environment is often the task at hand.

### Which CARLA each group is about

The group names are engine branches, not release numbers, because one release
number spans two of them:

| Group | Branch | CARLA |
|---|---|---|
| `ue4` | `ue4-dev` | 0.9.x, through 0.9.16 |
| `ue5` | `ue5-dev` | the UE5 line at UE 5.5 — an earlier revision, reports `0.10.0` |
| `ue58` | `ue58-dev` | the same line at UE 5.8 — **CARLA 1.0** |

`ue5-dev` and `ue58-dev` are one line, not parallel products, so the `ue58`
skills are the procedures for 5.5 too, minus five gaps that
[`check-ue5-limitations`](../../Skills/ue5/check-ue5-limitations/SKILL.md) enumerates.
Pre-1.0 builds of `ue58-dev` report `0.10.0`, and that is the version string most
measurements in this repo were taken against — where a skill says `0.10.0`, read
it as naming the UE5 line unless it is quoting a specific build.

`python-api`, `scenario-runner`, `leaderboard`, `scenic` and `ros2` are not tied
to an engine: they bind to a running server, a checkout of the companion repo, or
CARLA's native ROS 2 sources.

## Two servers, one library

The skills, and everything that decides which are usable, exist in both
languages. Which one you pick decides only what has to be on the machine:

| | needs | command |
|---|---|---|
| **Node** | Node >= 12, nothing else | `node Util/AgenticTools/bin/carla-agentic-tools.js` |
| **Python** | Python >= 3.10 with the `mcp` SDK (`pip install "mcp>=1.2.0,<3"`) | `python3 -m carla_agentic_tools.server`, with `Util/AgenticTools/src` on `PYTHONPATH` |

The Node implementation has **no runtime dependencies**, which is why it is the
one `.mcp.json` and the commands below use. The Python one is the same server
for people whose tooling is already Python. Both are the same version, so pick
whichever runtime you already have.

The skills themselves still shell out to `bash`, and the ones that drive the
CARLA client need an interpreter with the `carla` wheel — that is the `PYTHON`
key, part of the user's environment rather than the server's runtime. Building
CARLA needs a CARLA checkout either way.

Both read and write the same config file, so a path recorded through one is
visible to the other.

## Checking the server

Run it in a terminal to check it before wiring it into a client. A healthy start
is **silent** — it says nothing on either stream and blocks waiting for MCP
traffic on stdin, so a process that just sits there is the success case, and
Ctrl-C ends it. To see it actually answer, hand it a handshake:

```bash
echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"c","version":"1"}}}' \
  | node Util/AgenticTools/bin/carla-agentic-tools.js
```

which replies with the server name, its version, and the instructions the client
will load.

Diagnostics go to stderr and stdio is inherited, never piped, so stdout stays a
clean MCP stream and the client talks to the server directly.

## Registering with an MCP client

The server is ordinary stdio MCP with nothing client-specific in it, so any MCP
client can run it. Most take the same block — Claude Code (`.mcp.json` beside
your project, or `~/.claude.json`), Claude Desktop, Cursor (`~/.cursor/mcp.json`),
Windsurf (`~/.codeium/windsurf/mcp_config.json`), Gemini CLI
(`~/.gemini/settings.json`):

```json
{
  "mcpServers": {
    "carla": {
      "command": "node",
      "args": ["/path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js"]
    }
  }
}
```

For the Python implementation, use `"command": "python3"`,
`"args": ["-m", "carla_agentic_tools.server"]` and
`"env": {"PYTHONPATH": "/path/to/carla/Util/AgenticTools/src"}`. **No CARLA
paths go here** — you would have to know them before the skills that create
them have run.

Two clients want a different shape for the same server:

- **VS Code** (`.vscode/mcp.json`, or the user `mcp.json`) uses `servers`, not
  `mcpServers`, and wants an explicit `"type": "stdio"`. A block copied from
  above is ignored with no error, which is the most common setup mistake.
- **Codex** uses TOML in `~/.codex/config.toml`:

  ```toml
  [mcp_servers.carla]
  command = "node"
  args = ["/path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js"]
  ```

Clients with a CLI will write that entry for you:

```bash
claude mcp add carla -s user -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
codex mcp add carla -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
gemini mcp add -s user carla node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
code --add-mcp '{"name":"carla","command":"node","args":["/path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js"]}'
```

Each of these only records the entry; the client starts the server on first use.
For `claude` and `codex` the `--` separates the server command from the client's
own flags.

### Claude Code: one line, as a plugin

This repository is also its own plugin marketplace, which is the shortest path
on Claude Code — no registration, no paths:

```
/plugin marketplace add carla-simulator/carla-agentic-tools
/plugin install carla@carla-agentic-tools
```

The plugin ships the **same MCP server** and starts it from its own checkout
(`node ${CLAUDE_PLUGIN_ROOT}/bin/carla-agentic-tools.js`), so the tools, the
skills and the answers are identical to every other client's — only the delivery
differs. It needs Node >= 12 and nothing else, fetches nothing at launch, and
updates with `claude plugin update carla`. The tools appear under the plugin's
prefix (`mcp__plugin_carla_carla__list_skills`), which matters only if you write
permission rules for them.

To try a working tree without installing anything:

```bash
claude --plugin-dir /path/to/carla-agentic-tools
```

### Cursor and Codex: the same thing, as an Agent Plugin

`plugin.json` and `mcp.json` at the repo root are the
[Agent Plugin](https://agent-plugins.org) format, which Cursor and Codex both
read. They declare the same stdio server, resolved from `${PLUGIN_ROOT}`, so a
marketplace install gets the identical five tools:

```bash
codex plugin marketplace add carla-simulator/carla-agentic-tools
```

Cursor installs from **Customize** in the sidebar, at project or user scope, and
also accepts a one-click MCP deeplink if you would rather skip the plugin layer.
Claude Code needs its own `.claude-plugin/` manifests because it does not read a
root `plugin.json` — same server, two manifest dialects, kept in agreement by
`tests/test_version.py`.

Either client can also just register the server directly, per **Registering with
an MCP client** above; the plugin route only saves you writing the config.

Five tools, whatever the client: `list_skills` (optionally `group`-filtered),
`read_skill(name)`, `check_prerequisites(name)`, `get_config()`,
`set_config(paths)`.

## Paths, and when you are asked for them

Nothing is configured up front. `list_skills` on a bare machine already returns
the `setup` group (download CARLA, install the Python API) and `python-api`
(drives any running server) as available; everything else is listed with
`available: false` and a reason, because obtaining the missing piece is usually
the task at hand.

A path is asked for the first time a skill needs one it does not have.
`check_prerequisites` reports it as a `needs` block naming the key, the skill
that would obtain it, and every candidate found on the machine **with its flavor
and branch** — several CARLA checkouts side by side is normal, and picking the
wrong one fails slowly. The agent asks; `set_config` records the answer.

`CARLA_ROOT` is the only CARLA path anyone is asked for. `set_config` inspects
the directory and writes the engine-specific variable itself:

```
set_config({"CARLA_ROOT": "/home/me/carla"})
  CARLA_ROOT is source, ue58, branch ue58-dev; also set CARLA_UE58_ROOT
```

That is what gates the `ue58` group — the flavor comes from structural markers in
the tree, not from which of the five variable names you happened to set.

Resolution order for every key, highest first:

| # | source | for |
|---|---|---|
| 1 | an exported environment variable | a one-off override, CI |
| 2 | `./.carla-tools.env` | a repo carrying its own CARLA |
| 3 | `${XDG_CONFIG_HOME:-~/.config}/carla-agentic-tools/config.env` | the normal case |
| 4 | each `env.sh`'s own search list | last resort |

The config outranks the search lists deliberately: once the user has confirmed
which checkout to use, detection must not silently pick the other one. Override
the file's location with `CARLA_TOOLS_CONFIG`. It is `KEY=value` lines, parsed
rather than sourced, so nothing in it can execute.

The install skills record what they created, so a group flips to `available:
true` right after the install that enabled it, and stays that way next session.

## Developing on the skills

From `Util/AgenticTools/`:

```bash
pytest -q tests/                       # structural + MCP + Node/Python parity
node test/node_smoke.js                # the Node server
node bin/carla-agentic-tools.js        # the Node server straight from the tree
claude --plugin-dir $PWD               # Claude Code on this tree, nothing installed
```

The servers read `Skills/` on every start, so editing a `SKILL.md` needs only a
reconnect. `CARLA_SKILLS_DIR` points either server at a different skills tree.

Change how a skill is *selected* — the gating, the config keys, the detection
markers — and you are editing two implementations. `tests/test_node_parity.py`
runs both servers over stdio and diffs every answer, so a change made on one
side only fails there rather than reaching a user.

## Versioning

The version lives in three files — `src/carla_agentic_tools/__init__.py`,
`plugin.json` and `.claude-plugin/plugin.json` — and `tests/test_version.py`
fails on drift between any of them. Claude Code derives a git-installed plugin's
version from a `{name}--v{version}` tag, which `claude plugin tag . --push`
creates after checking that `plugin.json` and the marketplace entry agree; run
`claude plugin validate . --strict` first.

## Targeting a CARLA instance

One install drives any CARLA. These are the keys the skills read; see **Paths,
and when you are asked for them** above for how they get set — in normal use you
answer a prompt and never type a variable name.

Asked for, when a skill needs one:

| Key | Meaning |
|---|---|
| `CARLA_ROOT` | **the only CARLA path you are asked for**: a release or a source checkout. Its flavor is detected and the engine variable below is written for you |
| `PYTHON` | the interpreter that has the `carla` wheel, when it is not the `python3` first on PATH |
| `SCENARIO_RUNNER_ROOT` | a scenario_runner checkout — gates the `scenario-runner` group |
| `LEADERBOARD_ROOT` | a leaderboard checkout — gates the `leaderboard` group |
| `SCENIC_ROOT` | a Scenic checkout or installed package — gates the `scenic` group |
| `CARLA_UNREAL_ENGINE_PATH` | the Unreal Engine fork CARLA builds against |

Derived from `CARLA_ROOT`, or set by hand to override:

| Key | Written when `CARLA_ROOT` is |
|---|---|
| `CARLA_UE4_ROOT` | a `ue4-dev` checkout (`Unreal/CarlaUE4/CarlaUE4.uproject`) |
| `CARLA_UE5_ROOT` | a `ue5-dev` checkout (`CMakePresets.json` + `Unreal/CarlaUnreal`) |
| `CARLA_UE58_ROOT` | as above, plus the Autoware plugin and `CMake/DLSS.cmake` |
| `CARLA_PACKAGE_ROOT`, `CARLA_TARGET` | an extracted release (a `CarlaUE4.sh` at the top) |

Defaults, rarely touched: `CARLA_HOST` / `CARLA_PORT` (`127.0.0.1:2000`),
`CARLA_TM_PORT` (`8000`), `CARLA_TIMEOUT`, `CARLA_PRESET`, `ROS_DOMAIN_ID`.

No `carla` wheel yet? The `install-python-api` skill installs it from your
release's bundled wheel or from PyPI, checks it matches the simulator, and
records `PYTHON`.

One install can still drive several checkouts: give a repo its own
`./.carla-tools.env`, or export a variable for a single run — an export always
wins over the config.

**Version pairing matters** for the scenario-runner and leaderboard groups: a
scenario_runner branch belongs to a CARLA version, and a leaderboard version
belongs to a scenario_runner branch. The two installer skills derive the pairing
and every `check_env.sh` in those groups fails loudly on a mismatch, because the
symptom otherwise is scenarios that silently never trigger.

Paths also auto-resolve to `$PWD` when a skill runs from inside the relevant
checkout, and `check_prerequisites` fails loudly, naming what it checked, when
something is missing or wrong. That search is the last resort, below the config:
a recorded answer is never silently overridden by a guess.

## Running a skill directly (no server)

The scripts are runnable with plain bash — the server is only the discovery
layer:

```bash
cd Skills/ue4/package-carla-ue4
export CARLA_UE4_ROOT=/path/to/your/carla
export UE4_ROOT=/path/to/your/UnrealEngine_4.26
# activate the python env whose python3 has `carla` + `build` first

bash scripts/check_env.sh                 # check prerequisites
PACKAGES=Town15 bash scripts/package.sh   # cook + package (see SKILL.md for knobs)
```

See `Skills/ue4/package-carla-ue4/SKILL.md` and `references/packaging.md` for the
full procedure, knobs (`PACKAGE_DEST`, `CLEAN_INTERMEDIATE`, …), and gotchas.

## Adding a skill

Drop a new `Skills/<group>/<name>/` directory containing at minimum a `SKILL.md`
(with `description:` and `metadata.group:` matching the directory) and, for
prerequisite checks, a `scripts/check_env.sh`. The MCP server discovers it on the
next start.

`pytest -q tests/` checks the invariants (frontmatter, group, links, script
syntax) the release gates on.

Body prose must reach `scripts/` and `references/` by **absolute** path: the
agent's working directory is the user's project, not the skill directory. Each
`SKILL.md` opens with a `> **Paths.**` note saying so, and `read_skill` prefixes
its output with the skill's absolute directory. A new skill also needs its
`scripts/env.sh` to source `Skills/_common/env_common.sh`, or it cannot see the
paths the user recorded — `tests/test_config.py` fails when one does not.

Groups are directories: add `Skills/ue5/` or `Skills/scenic/` and register the
variable that gates it in `GROUP_REQUIREMENTS` — in **both** `src/carla_agentic_tools/server.py`
and `lib/skills.js`, or the two servers disagree about what is usable.
`tests/test_node_parity.py` fails when they do.
`Skills/ue4/package-carla-ue4` is the worked example to model a new skill on.

## License

MIT — see [LICENSE](LICENSE).
