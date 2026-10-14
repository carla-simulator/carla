# Category: changelog

Closes the audit's `changelog` findings: a user-visible change the release notes do not carry, and entries that carry it badly.

Edits `CHANGELOG.md` only, so it runs alongside any other category. It is also the category most likely to send you back into the code: an entry is a claim about behaviour, and most of the claims in a changelog written during development have stopped being true.

## 1. Know what the previous release contained

Every judgement below needs one fact: what shipped last time. Find the commit that closed the previous release — the one that added its heading to `CHANGELOG.md`:

```bash
git log --all --date=short --pretty="%h %ad %s" -S"## CARLA 0.10.0" -- CHANGELOG.md | tail -1
```

Call that `$REL`. Anything introduced after it is new in this cycle, and `git ls-tree -r --name-only $REL | grep -i <feature>` answers whether a feature existed then.

## 2. Put the entry in the right section

Three sections, in this order, each ordered by how much the reader is affected:

| Section | Holds | Test |
|---|---|---|
| `### New features` | Something that did not exist | A user can now do something they could not do before |
| `### Extensions` | Something that existed and changed | Behaviour, performance, API shape or defaults moved |
| `### Bug fixes` | Something that was broken **in the previous release** | A user of `$REL` could hit it |

**Breaking changes lead Extensions.** Renames, deprecations, changed defaults and changed return values go first, before the improvements, because they are what breaks a reader's script.

When a section passes about ten entries, group it with `####` headings rather than letting it run as one list. The groups that have worked: `Simulator`, `Sensors and cameras`, `Python API`, `Ecosystem and integrations`, `Build and tooling`, and in Extensions also `API changes` (first), `Performance`, `Traffic Manager and agents`, `Documentation`.

## 3. The bug-fix test, applied strictly

**A fix for a feature that never shipped is not a bug fix.** It is development on that feature, and the reader of a changelog has no way to have experienced it. Fold it into the feature's own entry if it changes what the feature does, otherwise drop it.

For each candidate, ask whether the broken thing existed at `$REL`:

```bash
git ls-tree -r --name-only $REL | grep -i fisheye     # nothing -> the feature is new this cycle
git show $REL:LibCarla/source/carla/geom/Rotation.h   # read the old code and see if the bug is in it
```

Three real outcomes from one audit:

- *"Fixed the fisheye cameras crashing on spawn"* — no fisheye files at `$REL`. Dropped.
- *"Fixed the `noise_seed` attribute being ignored"* — `$REL`'s `RayCastLidar.cpp` seeds in the constructor, before the attributes are read: the same bug. Kept.
- *"Fixed three latent bugs in the GPU readback path"* — the fix's own message says the path had no callers until the new sensors used it. Dropped.

Beware the inverse too: a commit that says "fix" may be the only entry a genuinely new API ever gets. When a fix is dropped, check that the feature it belongs to is itself in New features.

## 4. The sentence

The docs style applies, with three rules of its own.

**Lead with the result, not the mechanism.** The reader wants to know what is different for them; the reason earns at most a clause.

```
bad   Reworked the sensor render pipeline: GPU readbacks are pooled across all cameras,
      GBuffer capture is gated on having a listener, and ray tracing is togglable per camera.
good  Multi-camera rigs are much faster: all cameras now share one GPU synchronisation per
      tick instead of one each. A 5-camera 1080p rig gains about 30%. A single camera is
      unchanged, and so is the image.
```

**Name the action the reader takes.** "The old name still works and raises a `DeprecationWarning`." "Set it again after loading a map." "Scripts that polled `is_alive` must use `world.tick()` instead."

**Cut every clause that argues for the change.** "so a name tells you what it is", "none of these were documented", "which have been available for years" — all of it belongs in the pull request, not here.

Two more prohibitions:

- **No branch names.** The branch being worked on is production, like `main`. Say "on CARLA's own fork of the engine", not "on branch `ue58-dev-carla`"; say "missing since the move to Unreal Engine 5", not "missing since the UE4 branch".
- **No internal plumbing.** A subsystem with no API, no flag and no visible behaviour does not belong in a changelog however much work it was. One audit dropped a whole road-stencil parser on this test: nothing in the Python API or the Unreal plugin consumed it, only a unit test did.

## 5. Separate the engine from the simulator

An engine upgrade is one entry, and it names **only what the engine brings**. Features built during the same cycle get their own entries, even though they landed together.

The UE 5.8 entry originally carried five sub-bullets: World Partition, weather and night lighting, Traffic Manager, generated OpenDRIVE and "Rendering: Lumen and ray-traced shadows". Three of those are CARLA's own work, and Lumen had shipped in the previous release, on UE 5.5 — the engine upgrade only moved it onto hardware ray tracing. The entry now names World Partition, Large World Coordinates, hardware-ray-traced Lumen and the path tracer; weather, OpenDRIVE generation and the Traffic Manager are separate entries.

## 6. Find what has no entry at all

The audit finds documented-but-wrong. Missing-entirely needs a sweep of the cycle:

```bash
git log --oneline --date=short --pretty="%h %ad %s" $REL..HEAD | grep -viE "fix|docs|changelog|merge|typo|test|refactor"
```

Read the subjects, and for each ask whether `CHANGELOG.md` mentions it. What that found in one pass: `World.set_sky_light_map` (a whole new API, with no entry), wheels animating without Chaos (half of a PR whose other half was documented), the classic towns returning, per-camera rain, and two new wheel getters.

Content and config changes never appear in `git log` of this repository at all. Check the content repository's own history, `Config/*.json` and `Config/*.ini` for new blueprints, attributes, presets and maps.

## 7. Verify every claim before you keep it

An entry written mid-development describes the code of that week. Check each one against what is on disk now:

- **Names and signatures** against the bindings in `PythonAPI/carla/src/*.cpp`, never against the YAML or the generated reference.
- **Numbers and defaults** against the `.ini`, the `Constants.h` or the content JSON they come from.
- **Lists** against the data: which towns are World Partition, which blueprints carry an attribute, how many presets ship.

Three claims that failed this check in one pass: the Traffic Manager's "lateral PID retuned to its 0.9.12 behaviour" (two later commits replaced it with per-vehicle steering-authority scaling), "Town12, Town13 and Town15" as the World Partition maps (Town11 is one too), and "Ships GoPro, AutomotiveHDR and Dashcam presets" (nine ship, and the default was renamed).

## 8. Verify the shape

```bash
for s in "New features" "Extensions" "Bug fixes"; do
  printf "%s: " "$s"
  awk -v s="### $s" 'f&&/^### |^## CARLA/{exit} $0==s{f=1;next} f&&/^\* /{n++} END{print n}' CHANGELOG.md
done
```

Entry counts before and after are the cheapest evidence that a pass landed, and that nothing was lost while regrouping.
