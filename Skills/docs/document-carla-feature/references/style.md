# House style for CARLA documentation

Read it before writing or reviewing. First decide which kind of page you are editing; each has its own rules, and applying one kind's rules to another is the most common reason an edit gets rejected.

| Page type | Examples | Rules |
|---|---|---|
| Reference entry | `PythonAPI/docs/*.yml`, `ref_sensors.md` attribute tables, `bp_library` | [Reference entries](#reference-entries) |
| Narrative | `start_introduction.md`, `core_foundations.md`, overviews, tutorials | [Narrative pages](#narrative-pages) |
| Task | `start_quickstart.md`, `build_*.md`, `build_docker.md`, install and run instructions | [Task pages](#task-pages) |

The rules in [Every page](#every-page) apply to all three.

## Every page

- **Keep the wording, update the facts, add what is new.** Start from the page's last reviewed text: the current page if the team has already reviewed or rewritten it for the target release (`git log -- Docs/<page>.md`), otherwise the previous release's (`git show 0.10.0:Docs/<page>.md`, or older). Change a sentence only when it is wrong for the target release; add new material in the voice of the text around it. A shorter rewrite of a correct paragraph is a regression, however tidy. A rewrite the team already made is the page's wording now, not a finding.
- **Write for the target release** (1.0.0), even before it exists. No caveats about the previous release or about what has not shipped yet. Where something cannot exist yet (a release link, an image tag), write it for the target and put a visible `__(TODO: ...)__` next to it, see [evidence.md](evidence.md).
- **Linux first, Windows second**, always: subsections, code blocks, table rows, inline mentions. Per-platform content goes under `### Linux` and `### Windows`, each with that platform's own command. Do not split by platform when the command is the same on both. Do not explain in prose why the platforms differ; just show each command.
- **Code blocks are compact**: no blank lines between commands, a short `#` comment only where two commands go to different terminals.
- **Never hard-wrap prose.** A paragraph or a bullet is one line.
- **Do not add what nobody asked for.** No justification ("with no bridge process", "the same API from C++"), no marketing. A new item enters a list only if it is the same kind of thing as the items already there; items the team has already put in a list (NuRec in the introduction's key modules) stay.
- **Page layout.** One `#` heading per page, the title; sections are `##` and below. Under the intro paragraph, the page lists its `##` sections as `- [__Heading__](#anchor)` lines, and each `##` heading has a `---` line right above it. Keep the list in step with the headings when you add, rename or remove a section. **Do not add "this is useful for" bullet lists under the intro**; keep one only if it is in the page's original text.
- **Do not merge a walkthrough away.** A quick start keeps the commands its reader needs to finish, even if a deeper page repeats them; link out for variants, not for the main path.
- **Do not coin labels.** Don't name a thing the page hasn't introduced with an invented noun phrase ("the consuming project", "the base release" without saying what it is); say what it is ("your project"). Original wording stays as it is.
- **Avoid generated-sounding prose**: one sentence packed with facts, colon-led fragments in place of sentences, stock lines ("contributions are welcome"), em-dash asides.
- **Document what is current.** When the code or content holds an old and a new generation of the same thing, document only the current one, even if old assets still use the old one. Ask the user when usage and age disagree.
- **When another page already explains a subject in depth**, write two or three lines and link to it.
- **Only the table columns the reader acts on.** A column that repeats the same folder for every row is noise; say the folder once in the text.
- **Names exactly as they are.** Use an asset or parameter name as the code spells it (`Emissive atenuance`, `Difuse texture`) and say nothing about the spelling: fixing it is a separate task.
- **Admonitions take no fenced code.** The site has no `superfences`: a fenced block inside `!!! Note` renders as one inline code line. Put the code after the admonition, or inline.

## New sections and pages

Copy the shape of a neighbouring section on the same page first; a new layout stands out. When there is none:

- **A feature** (on a narrative or reference page): what it is and what the reader can do with it, in one paragraph; the code to use it; a table of its parameters or values; then the caveat, in one admonition at most.
- **A task**: the prerequisites; the steps, each with its command per platform (Linux first); what the reader sees when it worked.
- **A new page**: one of the above, under an intro paragraph that says what the page covers. Add it to the `mkdocs.yml` nav next to its siblings and link it from the page a reader comes from.

### Content-authoring pages

For an artist-facing page (materials, blueprints, tools in the editor):

- **Say what a parameter is for, from the user's side.** The `EmissiveIntensity` and `On/Off` parameters of the *API Values* groups simulate what the CARLA API sends at runtime, so an artist can preview the lit material without launching PIE.
- **Name the channel**: "the `R` channel of the ORME texture", "the BaseColor alpha channel", in the text and in image titles, never a bare "ORME R".
- **Examples must not mislead.** If an example was made for the picture (textures of one asset in another master, a demo placed in a town), the caption says so.
- **Render the effect before describing it.** `Brightness`, `Contrast` and similar corrections were described wrong from their names until a render showed what they do.

A master material page takes this order: what the master is for (one paragraph); a panel with the textures it reads, each channel separated and titled; a crop of each parameter group of the instance editor with its explanation right below it, with every switch ticked in an unsaved copy so all parameters show; examples in a CARLA town, shot with the CARLA camera, where they help (variants of one asset, emissive on and off by day and night, dry and wet road). How to produce those images is in [images.md](images.md#editor-recipes).

## Reference entries

A doc string says **what the thing is or does**, and stops.

- One or two sentences. Third person, present tense.
- A getter starts with "Returns".
- No reason, no history, no "this is useful when", no "note that".
- No caveats, failure modes or known issues. If a caveat genuinely has to be recorded, it goes in a `note:` or `warning:` key, never in `doc:`.
- No cross-reference unless the type is only usable through another call.

```yaml
# good
doc: >
  Returns the actor's transform.

# bad - explains itself, warns, and speculates
doc: >
  Returns the actor's transform. This is useful when you need to know where an
  actor is, for example to place a sensor. Note that in asynchronous mode the
  value may be one tick old, so you should call world.tick() first.
```

### Values and ranges

The only detail worth adding is **the range a value can take**, and what its ends mean. Put units in `param_units` / `return_units` / `var_units`, not in the sentence.

```yaml
# good
- def_name: ignore_lights_percentage
  params:
  - param_name: actor
    type: carla.Actor
  - param_name: percentage
    type: float
  doc: >
    Chance for a vehicle to ignore a traffic light, being 0 always detecting it
    and 100 a full ignore.
```

An enum-valued parameter lists its values the same way. A parameter whose range is the obvious one for its type gets nothing.

### Names are API

Parameter names are **keyword arguments**: `actor.set_collisions(enabled=False)` works because the binding declares `arg("enabled")`. A name invented for the docs is a documented call that raises `TypeError`.

- Copy `param_name` from the binding's `arg("…")`, in order, spelling included.
- Copy `default:` from the binding's `arg("…") = value`, written Python-side (`True`, not `true`).
- `def_name` and `var_name` are the string the binding registers, not the C++ method name.

### Types

`type:` is what a user writes or receives in Python: `int`, `float`, `str`, `bool`, `list(int)`, `dict`, or a qualified CARLA class (`carla.Vector3D`). Not the C++ type.

## Narrative pages

Introductions, overviews and tutorials are prose with a voice: they may say why CARLA does something and what it stands for. Do not compress them into reference sentences, and do not apply "lead with the result" to them.

- **What counts as wrong:** stale facts (engine version, standard names, requirements, paths), broken grammar, a word that does not mean what was intended, and repetition across paragraphs.
- **New features join in the same voice.** A new module in the key modules list gets a bullet that explains what it is for, like its neighbours, not a label and a fragment.
- **Order for the reader.** What a reader needs before anything else, such as a version note, goes directly under the title.

```markdown
<!-- good: the original, with the engine version and standard name updated -->
CARLA is an open-source driving simulator for use in autonomous driving applications. It was built from scratch with a modular and flexible API to address a range of tasks involved in the problem of driving. [...] CARLA is built on Unreal Engine 5.8 to run the simulation and uses the ASAM OpenDRIVE standard to define roads and urban settings.

<!-- bad: the same facts, voice and purpose gone -->
CARLA is an open-source driving simulator for autonomous driving research. It runs on Unreal Engine 5.8, describes its roads with ASAM OpenDRIVE, and is driven through a Python and C++ API.
```

```markdown
<!-- good: explains what the module is for, in the list's voice -->
* __Recorder__: The recorder can replay a simulation exactly to enable retrospective evaluation of the outcome of a simulation and to rerun the simulation with alternative configurations of sensors, for example.

<!-- bad: a label and a fragment -->
* __Recorder__: replays a simulation exactly, to evaluate a run afterwards or re-run it with a different sensor rig.
```

## Task pages

Install, build and run instructions. The reader follows them step by step and copies every command.

- **Every command is one the reader can paste**, for the target release, on the platform named above it.
- **Keep the original explanation around commands.** "This repository contains different versions of CARLA. You will find options to download the current release [...]" stays; it is not filler.
- **Prerequisites before the first command that needs them.** If the page recommends a virtual environment, it says how to create one before the first `pip3`.
- **Do not pin a version the original left generic.** `pip3 install carla-*.whl` is right; a file name with a specific version goes stale. A generic name still has to match the real one: check the package or the packaging code for its pattern.

```markdown
<!-- good -->
### Linux

    cd CARLA_ROOT/PythonAPI/carla/dist/
    pip3 install carla-*.whl

### Windows

    cd CARLA_ROOT\PythonAPI\carla\dist\
    pip3 install carla-<version>-cp310-cp310-win_amd64.whl

<!-- bad: one command, then prose about the other platform -->
    pip3 install carla-*.whl

On Windows, the command prompt does not expand `*`, so type the full name of the wheel file.
```

## Images

Captions, formats, folders and diagrams follow [images.md](images.md).
