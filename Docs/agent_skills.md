# Agentic CARLA skills

CARLA ships a library of 63 vetted procedures for AI coding assistants, under [`Skills/`](https://github.com/carla-simulator/carla/tree/ue58-dev/Skills) in the repository. An assistant working inside a CARLA checkout finds them automatically and follows them instead of improvising a procedure of its own.

Each skill is a directory holding a `SKILL.md` — YAML frontmatter (`name`, `description`, `compatibility` and `metadata`) followed by the step-by-step procedure — plus the `scripts/` and `references/` it needs. The procedures encode the failure modes that make the obvious approach fail, which is the point of them: they are the source of truth for these tasks, not a summary of the docs.

In practice this means an assistant asked to "package CARLA with only Town10" reads `package-carla-ue58` and follows it, instead of guessing at CMake targets and cook options.

- [__Set up Agentic CARLA__](#set-up-agentic-carla)
- [__What is covered__](#what-is-covered)
- [__Using them__](#using-them)
- [__Unreal MCP in the editor__](#unreal-mcp-in-the-editor)
- [__Writing a skill__](#writing-a-skill)

---
## Set up Agentic CARLA

Most procedures in this documentation can be handed to an AI coding assistant: installing CARLA, building it, running a server, spawning traffic and sensors, importing maps and vehicles, writing and running scenarios. Throughout the pages, a box like this one marks those procedures, with a prompt to give your agent and the skills it will follow:

!!! agent "Agentic CARLA"
    *"Download CARLA 1.0, install its Python client and start a server on Town10."*

    Skills: `download-carla`, `install-python-api`, `run-carla-ue58-server`.

The manual steps stay below each box, for those who prefer to run them by hand or want to know what the assistant does.

To give your assistant the skills, use one of these:

* __Inside a CARLA checkout__, nothing to do. Assistants that read [`AGENTS.md`](https://github.com/carla-simulator/carla/blob/ue58-dev/AGENTS.md) find the library, and the root `.mcp.json` starts the CARLA MCP server for MCP clients.
* __As a plugin__, with no CARLA checkout at all. In Claude Code:

```
/plugin marketplace add carla-simulator/carla-agentic-tools
/plugin install carla@carla-agentic-tools
```

In Codex, run `codex plugin marketplace add carla-simulator/carla-agentic-tools`; in Cursor, add the same marketplace from __Customize__ in the sidebar.

* __As an MCP server registered by path__, from a CARLA checkout, so that assistants working in other projects can use it (needs Node 12 or later):

```sh
claude mcp add carla -s user -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
codex mcp add carla -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
```

Starting from nothing, ask for the download first: the `download-carla`, `install-python-api` and `run-carla-ue58-server` skills fetch a release, install the matching client and start the server, and the assistant records where CARLA is for the next tasks.

---
## What is covered

| Group | Skills | Covers |
|---|---|---|
| `setup` | 6 | Downloading CARLA, installing the Python API, Scenario Runner, Leaderboard and Scenic, and checking what a given install supports |
| `python-api` | 23 | Driving a running server: spawning, sensors, traffic, weather, maps, recording, debug drawing |
| `ue58` | 8 | Building, packaging and running UE 5.8, importing maps, props and walkers, authoring vehicles, and running Autoware |
| `ue4` | 7 | The same for the UE 4.26 branch |
| `ue5` | 1 | UE 5.5 feature-support checks |
| `scenario-runner` | 5 | Writing and running scenarios, including OpenSCENARIO, and reading the results |
| `leaderboard` | 5 | Writing, packaging and evaluating a Leaderboard agent |
| `scenic` | 2 | Authoring and running Scenic scenarios |
| `ros2` | 3 | The native ROS 2 interface: publishers, message types, RViz |
| `docs` | 2 | Auditing the documentation for gaps and writing the pages that fill them |
| `contributing` | 1 | Reporting bugs and requesting features as issues, opening discussions to agree CI or large changes, and checking a branch against the contribution rules before opening its pull request against `ue58-dev` |

---
## Using them

Assistants that read [`AGENTS.md`](https://github.com/carla-simulator/carla/blob/ue58-dev/AGENTS.md) pick the library up on their own — that file carries the full catalogue and the repository conventions. No setup is needed beyond working inside the checkout.

The same library is also served over MCP by [`Util/AgenticTools/`](https://github.com/carla-simulator/carla/tree/ue58-dev/Util/AgenticTools), registered for the project by `.mcp.json`, which runs it with Node. The MCP tools add what plain file reading cannot do:

- `list_skills` — the catalogue, with each skill marked available or not depending on whether the checkout or install it needs is configured
- `read_skill` — a skill's `SKILL.md`, prefixed with its absolute directory so the relative `scripts/...` paths in it resolve
- `check_prerequisites` — runs that skill's read-only environment check and reports what is missing
- `get_config` / `set_config` — the configured paths (`CARLA_ROOT` and the engine-specific variables derived from it), persisted between sessions, with the CARLA installs found on this machine offered as candidates

Paths are asked for when a task first needs one, never guessed: several CARLA checkouts on one machine is normal, and the wrong one fails slowly.

---
## Unreal MCP in the editor

The skills tell an assistant how to do a task; [Unreal MCP](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-mcp-in-unreal-editor) lets it act on the open Unreal Editor itself. It is Epic's experimental MCP server plugin for Unreal Engine 5.8 (`ModelContextProtocol` in the engine source), and the CarlaUnreal project enables it: open the project in the editor and an assistant can place and inspect actors, move the viewport camera, read the output log, edit assets and materials, and start Play-In-Editor sessions.

The three servers an assistant may see do different jobs:

| Server | Talks to | Use it for |
|---|---|---|
| `carla` | The skill library on disk | Knowing how to build, package, import and run things |
| `unreal-mcp` | The running Unreal Editor | Editing the level and assets the editor has open |
| *(the Python API)* | A running CARLA server, port 2000 | Driving the simulation: spawning, sensors, traffic, weather |

!!! agent "Agentic CARLA"
    *"Put the viewport camera 10 m above the centre of the level looking straight down, and place a cube on the ground below it."*

    Server: `unreal-mcp`, editor open on `CarlaUnreal.uproject`.

### Connecting

Inside a CARLA checkout there is nothing to configure:

1. `CarlaUnreal.uproject` enables the `ModelContextProtocol` and `EditorToolset` plugins, for the Editor target only.
2. `Config/DefaultEditorPerProjectUserSettings.ini` turns on __Auto Start Server__, so the server starts when the editor finishes loading, on `http://localhost:6256/mcp`. The editor log confirms it with `LogModelContextProtocol: Starting MCP server on port 6256`.
3. The root `.mcp.json` registers that URL as `unreal-mcp`, next to `carla`. Claude Code picks it up when it is opened in the checkout and asks once to approve the project server.

Open the editor __before__ the assistant. Clients connect to MCP servers when the session starts: if the editor was not running then, `unreal-mcp` shows as failed (`ECONNREFUSED`). Once the editor is up, reconnect it with `/mcp` → `unreal-mcp` → __Reconnect__ in Claude Code, or restart the session.

To check the server by hand:

```sh
ss -ltnp | grep 6256            # UnrealEditor should be listening on 127.0.0.1:6256
curl -s -X POST http://localhost:6256/mcp \
  -H 'Content-Type: application/json' -H 'Accept: application/json, text/event-stream' \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-06-18","capabilities":{},"clientInfo":{"name":"check","version":"0"}}}'
```

For other clients, or for a project outside this checkout, run `ModelContextProtocol.GenerateClientConfig <ClaudeCode|Cursor|VSCode|Gemini|Codex|All>` in the editor console. It writes `.mcp.json`, `.cursor/mcp.json`, `.vscode/mcp.json`, `.gemini/settings.json` or `.codex/config.toml` with the current port and path. To register it by hand in Claude Code for every project:

```sh
claude mcp add --transport http unreal-mcp -s user http://localhost:6256/mcp
```

### How it works

The server runs inside the editor process and serves MCP over HTTP, bound to `127.0.0.1` only. Its tools are not built into the plugin: they come from __toolset__ plugins registered with the engine's `ToolsetRegistry`. CarlaUnreal enables `EditorToolset`, which provides:

| Toolset | What it does |
|---|---|
| `SceneTools` | Loads levels, finds, places and removes actors, traces the world, organises the outliner |
| `ActorTools` | Transforms, bounds, labels, tags, components and parenting of actors |
| `EditorAppToolset` | Viewport camera, viewport and editor screenshots, selection, Content Browser, console variables, Play-In-Editor |
| `LogsToolset` | Reads the output log and changes log category verbosity |
| `ObjectTools` | Reads and writes properties of any object or class, Blueprints included |
| `AssetTools`, `BlueprintTools`, `MaterialTools`, `MaterialInstanceTools`, `StaticMeshTools`, `SkeletalMeshTools`, `TextureTools`, `DataAssetTools`, `DataTableTools`, `CurveTableTools`, `StringTableTools` | Create and edit those asset types |
| `PrimitiveTools` | Adds primitive geometry components to actors |
| `ProgrammaticToolset` | Runs a small sandboxed Python script that chains many tool calls in one request, for example to aggregate the bounds of every actor in a level |
| `AgentSkillToolset` | Lists, reads and writes skills stored in the editor |

By default the server lists only three tools, `list_toolsets`, `describe_toolset` and `call_tool`, so the client's context does not fill with hundreds of tool schemas. The assistant discovers a toolset's tools on demand and calls them through `call_tool`, using the tool's short name and the toolset's full name.

Tool calls run on the editor's game thread one at a time: a long call blocks the editor while it runs, and a client should not issue overlapping calls. Screenshots come back as base64 PNG and can be several megabytes.

### Settings and limits

* __Editor only.__ The server starts from the editor module once the editor UI is up, so it is not available in `-game` mode, in the server started by `run-carla-ue58-server`, or in packaged builds. The plugins are restricted to the Editor target, so cooking and packaging are unaffected.
* __Settings__ are in __Editor Preferences → General → Model Context Protocol__: Auto Start Server, port (`6256` here, over the plugin default of `8000`, which collides with the Traffic Manager), URL path (default `/mcp`) and tool search (the three-tool listing above). They are per-user settings; the project file only sets the defaults.
* __Command line:__ `-ModelContextProtocolStartServer` starts the server even with Auto Start Server off, and `-ModelContextProtocolPort=N` moves it to another port. If you change the port, update `.mcp.json` to match.
* __More toolsets__ (PCG, Niagara, StateTree, Gameplay Tags and others) can be enabled from __Edit → Plugins__, or all at once with the `AllToolsets` plugin. CarlaUnreal does not enable them by default because they load many extra engine plugins.
* __Edits are real edits.__ Actors the assistant places or changes stay in the level, unsaved, like any manual change. Save or discard them yourself, and keep levels under source control.
* __Keep the editor running independently of the assistant.__ An editor launched from an IDE terminal, or by an assistant from one, exits when that terminal closes. Launch it on its own, for example with `setsid`.
* The plugin is __experimental__ in UE 5.8: tool names and settings may change between engine versions. Its log warns that data sent through it to an LLM service is subject to Epic's licensing terms.

---
## Writing a skill

Follow the conventions of the existing ones: a single `SKILL.md` with frontmatter, a `scripts/check_env.sh` named by `metadata.prerequisites` when the skill needs an environment, and a `references/` file for anything too long for the procedure itself. Keep the gotchas in the document — a skill that omits why the naive approach fails saves nobody the day that mistake costs.
