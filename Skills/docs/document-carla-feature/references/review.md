# Reviewing a page

A review answers one question: can a reader of the target release follow this page and get the right result? On an existing page it is step 1 of the pipeline, before anything is edited; on the text that was written, the check in step 5 repeats the parts that apply to the diff.

## What it checks

| Check | Passes when | Done by |
|---|---|---|
| Correct | Every claim has evidence that supports it exactly as the page states it ([evidence.md](evidence.md)) | verifier |
| Runs as written | The page's commands and snippets were run on a real server or editor and did what the page says | run agent |
| Up to date | None of the stale facts below; every content path was listed before it was written | verifier |
| Style | The rules in [style.md](style.md) for the page's type | verifier, then the check on the diff |
| Structure | Nothing stated twice, no contradictions, sections in the order a reader needs them, the section list under the intro matches the `##` headings | whole-page pass |
| Complete | A reader can finish the page's task with this page and the links it gives | whole-page pass |
| Reachable | `mkdocs build --strict` passes, no dead anchors, the page is in the nav or linked from a page that is, API links go to an anchor that exists | main session |
| Images | `scripts/images/check_images.py` passes and the images follow [images.md](images.md) | main session |

## Running the page

Execute the page as written, with nothing saved. `git status` of the code repository and of `Unreal/CarlaUnreal/Content/Carla` must be identical before and after, and no temporary asset folder may be left on disk. Report it as *OK* only when the page itself ran, and *Launched* when only part of it could run; say which part.

## Stale facts

Pages written for UE4 / 0.9.x repeat the same stale facts. Grep for each and check it against the tree:

| Stale | Now |
|---|---|
| `make launch`, `make import`, `make PythonAPI`, `make package`, `make rebuild` | `cmake --build Build/Release --target ...`, `Util/Tools/Import.py` |
| `UE4_ROOT` | `CARLA_UNREAL_ENGINE_PATH` |
| Unreal 4.26 / 4.27, Visual Studio 2019 | Unreal 5.8, Visual Studio 2022 |
| `.egg` files | wheels |
| `PythonAPI/carla/source` | `PythonAPI/carla/src` |
| `boost::shared_ptr` in bindings | `std::shared_ptr` |
| sensors overriding `Tick` | `ASensor::Tick` is `final`: override `PostPhysTick` |
| `Stream.Send` | `SerializeAndSend` |
| `CarlaUE4` paths, `../img/` links | `CarlaUnreal`, `img/` relative to `Docs/` |
| the CARLA forum | [GitHub Discussions](https://github.com/carla-simulator/carla/discussions) |
| `<div style="text-align: right"><i>` captions | `<div class="img-caption">` |

Content paths move too: list the folder before keeping one.

## A page about something that no longer exists

Rewrite it, do not delete it: describe the current state, with diagrams and real captures, and mark it for an expert of that area.

## The report

The user sees the findings, and the plan for any new content, before anything is edited. Write them to `{W}/review/<page>.md` (see [agents.md](agents.md) for `{W}`):

```markdown
# <page>.md — Minor

**Agent check:** Launched (the server and the first two snippets ran; the ROS 2 section needs a ROS 2 install)

| # | Kind | Finding | Evidence |
|---|---|---|---|
| BC-1 | error | `make launch` no longer exists; the command is `cmake --build Build/Release --target launch` | `Unreal/CMakeLists.txt:682` |
| BC-2 | missing | No step to create the virtual environment before the first `pip3` | |
| BC-3 | format | Windows block comes before Linux | |
| BC-4 | unverified | 130 GB disk figure | needs a clean build on a fresh disk |
```

- The verdict in the title is one of *Good*, *Minor*, *Needs revision*, *Rewrite* or *Obsolete*.
- `Kind` is `error`, `missing`, `format` or `unverified`. An unverified finding says what would settle it.
- Findings are numbered with a short prefix for the page, so they can be referred to later.
- Contradictions with other pages, which the whole-page pass notes, go in a list under the table, each naming both pages; they are fixed on whichever page is wrong, which may need its own review.
- New content gets a short plan under the table: where it goes and which ledger rows it uses.
- Mark the findings the user agrees to; the writer applies only those. What stays open after the fix goes in the hand-off, one sentence each.
