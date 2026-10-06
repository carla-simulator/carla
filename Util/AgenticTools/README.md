# carla-agentic-tools

A library of **vetted CARLA procedures** ("skills") for any agent. Each skill is
a `SKILL.md` plus executable scripts with its failure modes encoded, so an agent
discovers the right procedure and checks its prerequisites instead of improvising
from the Makefile.

## Quick start

```bash
claude mcp add carla -s user -- npx -y github:carla-simulator/carla-agentic-tools
codex mcp add carla -- npx -y github:carla-simulator/carla-agentic-tools
```

Cursor, VS Code, Windsurf and the rest take the same two fields in their MCP
config (`~/.cursor/mcp.json`, and note VS Code wants `servers` + `"type": "stdio"`):

```json
{"mcpServers": {"carla": {"command": "npx", "args": ["-y", "github:carla-simulator/carla-agentic-tools"]}}}
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

Needs Node >= 12 and `git`, and fetches on first launch. **[Install](#install)**
below covers pinning to a tag, machines without `git`, the Python
implementation, and how to check a server before wiring it into a client.

Nothing in a skill is client-specific — plain Markdown and POSIX shell. A
standalone [MCP](https://modelcontextprotocol.io) server serves them to any MCP
client, and comes in **two implementations**: a self-contained Node one and a
Python one. Same skills, same answers, no wrapper between them —
`tests/test_node_parity.py` runs both and diffs what they return.

**No package registry is in the loop.** Both install straight from this
repository — by git ref or release tarball — so a git tag *is* the release. It
also ships as a plugin for the clients that have a marketplace, which registers
the server for you.

This repo is independent of any CARLA checkout: it targets a **specific CARLA
instance at runtime**, recorded on first use, so one install can drive any build.

## Layout

```
carla-agentic-tools/
├── pyproject.toml            # hatchling; maps skills/ into the wheel
├── package.json              # the Node package; ships bin/ lib/ skills/ (never published)
├── plugin.json, mcp.json     # Agent Plugin manifests, for Cursor and Codex
├── .claude-plugin/           # plugin + marketplace manifests, for Claude Code
├── bin/carla-agentic-tools.js # npx and plugin entry point
├── lib/                      # the Node server: server.js, skills.js, config.js
│                             #   zero dependencies, Node >= 12
├── src/carla_agentic_tools/  # the Python server: server.py, config.py
├── test/node_smoke.js        # `npm test`
├── tests/                    # pytest, including the Node/Python parity checks
└── skills/
    ├── _common/env_common.sh # every env.sh loads the recorded paths through this
    ├── setup/                # get the pieces at all: download-carla, install-python-api,
    │                         #   install-scenario-runner, install-leaderboard
    ├── python-api/           # drives any running server (world-data, create-sensor, …)
    ├── ue4/                  # needs a UE4 checkout (build, package, run, import)
    ├── ue5/                  # what UE 5.5 cannot do that 5.8 can
    ├── ue58/                 # needs a UE 5.8 checkout (build, package, run, import, Autoware)
    ├── ros2/                 # native ROS 2 interface (publishers, msg types, RViz)
    ├── scenario-runner/      # CARLA's scenario engine (scenarios, OpenSCENARIO, routes)
    ├── leaderboard/          # the AD Leaderboard on top of it (agents, evaluation, scoring)
    └── scenic/               # probabilistic scenarios (write and run .scenic)
```

**Starting from nothing?** Three skills, in order: `download-carla` (fetches a
release and prints the path), `install-python-api` (installs the client wheel from
inside that download, so versions cannot mismatch), `run-carla-server` (detects the
download and launches it). The `setup` group is never gated on an environment
variable, so it stays visible to a user who has no CARLA yet.

Skills are **auto-discovered**: any `skills/<group>/<name>/SKILL.md` is picked up,
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
[`check-ue5-limitations`](skills/ue5/check-ue5-limitations/SKILL.md) enumerates.
Pre-1.0 builds of `ue58-dev` report `0.10.0`, and that is the version string most
measurements in this repo were taken against — where a skill says `0.10.0`, read
it as naming the UE5 line unless it is quoting a specific build.

`python-api`, `scenario-runner`, `leaderboard`, `scenic` and `ros2` are not tied
to an engine: they bind to a running server, a checkout of the companion repo, or
CARLA's native ROS 2 sources.

## Two servers, one library

The skills, and everything that decides which are usable, exist in both
languages. Which one you pick decides only what has to be on the machine:

| | needs | runner |
|---|---|---|
| **Node** | Node >= 12, nothing else | `npx` |
| **Python** | Python >= 3.10, and it builds the MCP SDK on first run | `uvx` |

The Node implementation carries the skills beside it and has **no runtime
dependencies** — no Python, no `uv`, no build step at first run, which is why it
is the one the commands below use. The Python one is the same server for people
whose tooling is already Python; it resolves ~29 packages the first time. Both
are the same version, so pick whichever runtime you already have.

The skills themselves still shell out to `bash`, and the ones that drive the
CARLA client need an interpreter with the `carla` wheel — that is the `PYTHON`
key, part of the user's environment rather than the server's runtime. Building
CARLA needs a CARLA checkout either way.

Both read and write the same config file, so a path recorded through one is
visible to the other.

## Install

**There is usually nothing to install.** An MCP client starts a server by
running a command, and both runners fetch on demand, so the command you register
*is* the install:

```bash
npx -y github:carla-simulator/carla-agentic-tools                                    # Node
uvx --from git+https://github.com/carla-simulator/carla-agentic-tools \
    carla-agentic-tools                                                              # Python
```

Both clone this repository. If `git` is not on the machine, name the tarball
GitHub generates for any branch or tag instead — same result, no git:

```bash
npx -y https://github.com/carla-simulator/carla-agentic-tools/archive/refs/heads/main.tar.gz
```

Run one of them in a terminal to check it before wiring it into a client. A healthy
start is **silent** — it says nothing on either stream and blocks waiting for
MCP traffic on stdin, so a process that just sits there is the success case, and
Ctrl-C ends it. To see it actually answer, hand it a handshake:

```bash
echo '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"c","version":"1"}}}' \
  | npx -y github:carla-simulator/carla-agentic-tools
```

which replies with the server name, the version you resolved, and the
instructions the client will load.

The first launch fetches the repository and starts it; later launches reuse the
runner's cache. That caching is the runner's behaviour, not the client's: no
agent is tracking versions on your behalf, and the client only ever re-runs the
string it was given.

Diagnostics go to stderr and stdio is inherited, never piped, so stdout stays a
clean MCP stream and the client talks to the server directly.

### Following latest, or pinning

Tracking latest is usually right for a skill library that is still growing, but
the ref is part of the command, so changing that behaviour is an args edit and
needs no reinstall:

| spec | effect |
|---|---|
| `github:carla-simulator/carla-agentic-tools` | `main`, as it is now |
| `github:carla-simulator/carla-agentic-tools#v0.6.0` | frozen at a tag — **the reproducible form** |
| `github:carla-simulator/carla-agentic-tools#<sha>` | frozen at a commit |
| `.../archive/refs/tags/v0.6.0.tar.gz` | the same tag, without needing `git` |

For `uvx`, append `@v0.6.0` to the `git+https://…` URL, and `uvx --refresh` to
bypass its cache.

**The caching is worth understanding**, because it is the one place a
registry-free install behaves differently. `npx` caches by spec *string*, so a
spec naming a moving branch can serve a clone made before your last push, and a
user stuck on old skills is almost always looking at that. A tag is immutable,
so a new tag is always a cache miss and always correct — which is why tags, not
branches, are the supported form. To force a refetch of a branch spec, clear the
runner's cache (`~/.npm/_npx`, or `uvx --refresh`).

### A durable install instead

Installing the package outright puts a `carla-agentic-tools` executable on
`PATH`, and the client then launches that with no network access and no
resolution step:

```bash
npm install -g github:carla-simulator/carla-agentic-tools#v0.6.0
uv tool install git+https://github.com/carla-simulator/carla-agentic-tools@v0.6.0
```

Register it as `command: "carla-agentic-tools"` with empty `args`. Upgrades
become explicit — re-run the same command with a newer tag — which is the point:
this is the shape for offline and air-gapped machines, for CI, and for anywhere
a pinned version matters more than getting new skills as they land. A machine
with neither runner can clone the repo and register
`node /path/to/carla-agentic-tools/bin/carla-agentic-tools.js`, which fetches
nothing at all.

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
      "command": "npx",
      "args": ["-y", "github:carla-simulator/carla-agentic-tools"]
    }
  }
}
```

Swap `command`/`args` for `uvx` + `["--from", "git+https://github.com/carla-simulator/carla-agentic-tools", "carla-agentic-tools"]`
to run the Python implementation instead. **No paths go here** — you would have
to know them before the skills that create them have run.

Two clients want a different shape for the same server:

- **VS Code** (`.vscode/mcp.json`, or the user `mcp.json`) uses `servers`, not
  `mcpServers`, and wants an explicit `"type": "stdio"`. A block copied from
  above is ignored with no error, which is the most common setup mistake.
- **Codex** uses TOML in `~/.codex/config.toml`:

  ```toml
  [mcp_servers.carla]
  command = "npx"
  args = ["-y", "github:carla-simulator/carla-agentic-tools"]
  ```

Clients with a CLI will write that entry for you:

```bash
claude mcp add carla -s user -- npx -y github:carla-simulator/carla-agentic-tools
codex mcp add carla -- npx -y github:carla-simulator/carla-agentic-tools
gemini mcp add -s user carla npx -- -y github:carla-simulator/carla-agentic-tools
code --add-mcp '{"name":"carla","command":"npx","args":["-y","github:carla-simulator/carla-agentic-tools"]}'
```

Each of these only records the entry — nothing is fetched until the client first
starts the server. Note where the `--` falls: for `claude` and `codex` it
separates the whole server command from the client's own flags, while `gemini`
takes the command as a positional and needs the `--` after it so npx's `-y` is
not read as a flag for `gemini` itself.

### Claude Code: one line, as a plugin

This repository is also its own plugin marketplace, which is the shortest path
on Claude Code — no runner to choose, no registration, no paths:

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
marketplace install gets the identical five tools with no runner and no `npx`:

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

Work from a checkout when you are *writing* skills:

```bash
pip install -e .                       # the Python server from source
pytest -q tests/                       # structural + MCP + Node/Python parity
node test/node_smoke.js                # the Node server (also `npm test`)
node bin/carla-agentic-tools.js        # the Node server straight from the tree
claude --plugin-dir $PWD               # Claude Code on this tree, nothing installed
```

`CARLA_SKILLS_DIR` points either server at a working tree, so you can edit a
`SKILL.md` and re-run without reinstalling — useful when the server itself was
fetched (`CARLA_SKILLS_DIR=$PWD/skills npx -y github:carla-simulator/carla-agentic-tools`)
rather than run from the checkout.

Change how a skill is *selected* — the gating, the config keys, the detection
markers — and you are editing two implementations. `tests/test_node_parity.py`
runs both servers over stdio and diffs every answer, so a change made on one
side only fails there rather than reaching a user.

## Releasing

A release is a **git tag**. Nothing is published, and there is no registry
account to hold:

```bash
pytest -q tests/ && node test/node_smoke.js    # both suites
claude plugin validate . --strict              # the plugin manifests
git tag v0.6.0 && git push origin v0.6.0       # what the install specs pin
claude plugin tag . --push                     # carla--v0.6.0, for the plugin
```

**Two tag names, one release.** `v<version>` is what the `npx`/`uvx` specs above
name. Claude Code derives a plugin's version from a `{name}--v{version}` tag
instead — it records the commit SHA and a tag-derived semver for a git-installed
plugin — so `claude plugin tag` creates `carla--v0.6.0`, after checking that
`plugin.json` and the marketplace entry agree. It refuses on a dirty tree, which
is the behaviour you want from a release step.

The version lives in five files — `pyproject.toml`,
`src/carla_agentic_tools/__init__.py`, `package.json`, `plugin.json` and
`.claude-plugin/plugin.json` — and `tests/test_version.py` fails on drift
between any of them.

`package.json` carries `"private": true`, so a stray `npm publish` is refused
instead of claiming the name. Nothing here forecloses adding npm or PyPI later;
every spec above keeps working if you do.

Two things to know about tagging as a release mechanism: the plugin manifests
must be on the **default branch** for `/plugin marketplace add` to find them, and
the runners cache branch specs but never tag specs — so push the branch and the
tag together, and point users at tags.

## Targeting a CARLA instance

One install drives any CARLA. These are the keys the skills read; see **Paths,
and when you are asked for them** above for how they get set — in normal use you
answer a prompt and never type a variable name.

Asked for, when a skill needs one:

| Key | Meaning |
|---|---|
| `CARLA_ROOT` | **the only CARLA path you are asked for**: a release or a source checkout. Its flavor is detected and the engine variable below is written for you |
| `PYTHON` | the interpreter that has the `carla` wheel. Needed whenever the server runs under `uvx`/`npx`, whose own python is first on PATH |
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
cd skills/ue4/package-carla-ue4
export CARLA_UE4_ROOT=/path/to/your/carla
export UE4_ROOT=/path/to/your/UnrealEngine_4.26
# activate the python env whose python3 has `carla` + `build` first

bash scripts/check_env.sh                 # check prerequisites
PACKAGES=Town15 bash scripts/package.sh   # cook + package (see SKILL.md for knobs)
```

See `skills/ue4/package-carla-ue4/SKILL.md` and `references/packaging.md` for the
full procedure, knobs (`PACKAGE_DEST`, `CLEAN_INTERMEDIATE`, …), and gotchas.

## Adding a skill

Drop a new `skills/<group>/<name>/` directory containing at minimum a `SKILL.md`
(with `description:` and `metadata.group:` matching the directory) and, for
prerequisite checks, a `scripts/check_env.sh`. The MCP server discovers it on the
next start.

`pytest -q tests/` checks the invariants (frontmatter, group, links, script
syntax) the release gates on.

Body prose must reach `scripts/` and `references/` by **absolute** path: the
agent's working directory is the user's project, not the skill directory. Each
`SKILL.md` opens with a `> **Paths.**` note saying so, and `read_skill` prefixes
its output with the skill's absolute directory. A new skill also needs its
`scripts/env.sh` to source `skills/_common/env_common.sh`, or it cannot see the
paths the user recorded — `tests/test_config.py` fails when one does not.

Groups are directories: add `skills/ue5/` or `skills/scenic/` and register the
variable that gates it in `GROUP_REQUIREMENTS` — in **both** `src/carla_agentic_tools/server.py`
and `lib/skills.js`, or the two servers disagree about what is usable.
`tests/test_node_parity.py` fails when they do.
`skills/ue4/package-carla-ue4` is the worked example to model a new skill on.

## License

MIT — see [LICENSE](LICENSE).
