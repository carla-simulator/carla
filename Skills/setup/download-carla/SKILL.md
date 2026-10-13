---
name: download-carla
description: Downloads CARLA so nothing has to be fetched by hand — an official release package for any published version (resolved live from the GitHub releases, with AdditionalMaps), the rolling nightly Dev build, a shallow source checkout of a branch or tag, or the carlasim/carla Docker image. Reports exactly what was obtained and where, plus the environment variables the other skills read. Use when the user asks to "download CARLA", "get CARLA 0.9.16", "install CARLA", "clone the carla repo", "fetch the latest CARLA", or has no CARLA at all yet.
license: MIT
compatibility: Linux or Windows with curl (recommended, for resumable downloads) and tar/unzip; git only for the checkout mode, Docker only for the image mode. Needs network access to api.github.com and the release mirrors (downloads.carlasim.com, carla-releases.s3.us-east-005.backblazeb2.com). A release is ~8-10 GB compressed and needs roughly double that with extraction; AdditionalMaps adds ~15 GB. Downloads only — builds nothing, launches nothing.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/sources.md
---

# Download CARLA

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The first link in the chain: get CARLA onto the machine and **say precisely what
landed where**, so the skills that follow — [[install-python-api]],
[[run-carla-server]], [[build-carla-ue4]] — need no guesswork from the user.

Four ways to obtain CARLA, and the choice is the only real decision:

| Mode | Gives you | Ready to run? | Size |
|---|---|---|---|
| `release` | an official package, extracted | **yes** | ~8-10 GB (+15 GB maps) |
| `nightly` | the rolling `Dev` build, extracted | **yes** | ~8.4 GB (UE4) / ~15.7 GB (`--ue5`) |
| `git` | a source checkout of a branch/tag | no — must be built | ~1 GB (content is separate, ~31 GB) |
| `docker` | the `carlasim/carla` image | yes, in a container | image-sized |

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: list / resolve — see what exists and what it will cost
- [ ] Step 3: fetch it (release | nightly | git | docker)
- [ ] Step 4: record the path with `set_config`, then hand over to the next skill
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

It fails early on the things that waste a multi-GB download: no network to the API
or the CDN, an unwritable destination, or too little disk (a release needs ~19 GB
with extraction; `--with-maps` another ~15 GB).

### Steps 2-3: look, then fetch

```bash
source scripts/env.sh

python3 scripts/download_carla.py list                       # versions + what each offers
python3 scripts/download_carla.py resolve --version 0.9.16   # real URLs + sizes, no download

python3 scripts/download_carla.py release                    # newest stable, extracted
python3 scripts/download_carla.py release --version 0.9.15 --with-maps
python3 scripts/download_carla.py nightly
python3 scripts/download_carla.py nightly --ue5              # the UE5 line, Linux only
python3 scripts/download_carla.py git --ref ue4-dev
python3 scripts/download_carla.py docker --version 0.9.16
```

`--dry-run` on any fetching command prints the exact `curl`/`git`/`docker` command
and changes nothing. `--dest DIR` (or `CARLA_DOWNLOAD_DIR`) chooses where things
land; the default is `~/carla-downloads`.

Downloads are **resumable and idempotent**: an interrupted run continues where it
stopped, and a complete archive is never fetched twice. The size is checked against
the server's `Content-Length` before extraction, and the archive is deleted
afterwards unless you pass `--keep-archive`.

### Step 4: the handover

Every command ends with a block naming what you now have:

```
=== obtained ===
  kind    : release
  version : 0.9.16
  path    : /home/me/carla-downloads/CARLA_0.9.16
  launcher: /home/me/carla-downloads/CARLA_0.9.16/CarlaUE4.sh

  export these for the other skills:
    export CARLA_TARGET=/home/me/carla-downloads/CARLA_0.9.16
    # run it:             run-carla-server
    # install the client: install-python-api   (uses the wheel inside it)
```

That is the point of the skill: `CARLA_TARGET` makes [[run-carla-server]] detect
and launch it with no further configuration, and [[install-python-api]] finds the
matching client wheel *inside* the download — so the client can never mismatch the
simulator.

A checkout instead prints `CARLA_UE4_ROOT` and points at [[build-carla-ue4]],
because a checkout cannot run until it is built.

## Why URLs are resolved, never constructed — and never trusted

The filename scheme is **not** stable across the CARLA line. Verified live:

```
0.9.16  ->  Linux/CARLA_0.9.16.tar.gz                8.3 GB
0.9.16  ->  Linux/AdditionalMaps_0.9.16.tar.gz      14.8 GB
0.10.0  ->  Linux/Carla-0.10.0-Linux-Shipping.tar.gz 10.4 GB   <- different scheme
```

So the skill reads the **GitHub release body** (the authority for the *filename*,
always current), which is also why a newly published version — 1.0 included —
needs no change here: `list` and `resolve` see it as soon as GitHub does.
Constructing `CARLA_<version>.tar.gz` would 404 on the 0.10 (UE5)
line. GitHub releases themselves carry **no attached assets** — the links in the
body are the only source.

The **host** in those links, however, is stale for everything before 0.9.16: they
point at `tiny.carla.org`, which still 308-redirects to the BunnyCDN zone
`carla-releases.b-cdn.net` — and that zone answers **403 for every object** (as of
2026-09). So each resolved URL is probed for *status*, not merely a response, and
remapped by identical path onto the first live mirror:

| Mirror | Holds |
|---|---|
| `downloads.carlasim.com` | 0.9.16 and the nightly `Dev/` builds |
| `carla-releases.s3.us-east-005.backblazeb2.com` | everything, incl. 0.9.15 and older, and 0.10.0 |

When that happens, the command prints a `note:` line naming the dead URL and the
mirror used, so a future breakage is visible rather than silent.

## AdditionalMaps

`--with-maps` fetches the extra towns, and they are *not* extracted: they are
staged into the release's `Import/` directory, because a package imports them with
its own script:

```bash
cd <release> && ./ImportAssets.sh
```

## Examples

**Example 1: a user with nothing**

User says: "I have no CARLA, set me up"

`release` → `export CARLA_TARGET=<printed path>` → [[install-python-api]] `install`
(finds the bundled wheel) → [[run-carla-server]] (detects the release, launches it).
Three skills, no manual downloads, no version decisions.

**Example 2: a specific old version**

User says: "I need 0.9.14 to reproduce a bug"

`list` shows it, `resolve --version 0.9.14` shows the size, `release --version
0.9.14`. The client then matches automatically, since it comes from that package.

**Example 3: they want to build from source**

User says: "clone the ue4-dev branch"

`git --ref ue4-dev` → `CARLA_UE4_ROOT` → [[build-carla-ue4]]. The skill warns that
content is a separate ~31 GB fetch (build step 05) and that a checkout cannot be
run until built.

### Recording the path

An `export` lasts until the shell exits. Persist `CARLA_ROOT` instead, so the
next session — and `list_skills` — still knows where this went:

```
set_config({"CARLA_ROOT": "<the path printed above>"})
```

Without it the group this just enabled keeps reporting `available: false`,
and the next skill re-detects from scratch. `CARLA_ROOT` is the only CARLA
path to record: `set_config` derives the engine-specific variable itself.

## Verify

The printed `path` exists and, for a release/nightly, holds `CarlaUE4.sh`:

```bash
ls "$CARLA_TARGET"/CarlaUE4.sh
DETECT=1 bash ../../ue4/run-carla-server/scripts/run_server.sh   # should report mode=package
```

`DETECT=1` is the real end-to-end check that the download is usable: it is the same
detection [[run-carla-server]] uses to launch.

## Troubleshooting

**Problem: the download stops partway**
Cause: a dropped connection on a multi-GB transfer.
Solution: re-run the same command — `curl -C -` resumes. Nothing is re-fetched once
the size matches.

**Problem: `not enough disk: need ~19 GB`**
Cause: the check counts the archive *plus* extraction.
Solution: free space, or `--dest` / `CARLA_DOWNLOAD_DIR` to a bigger filesystem.

**Problem: `no release tagged X`**
Cause: that tag does not exist (or is older than the 20 releases queried).
Solution: `list` shows the tags. For unreleased code use `git --ref <branch>`.

**Problem: extraction succeeded but no `CarlaUE4.sh` was found**
Cause: an interrupted extraction, or a release whose launcher has another name
(the UE5 line — 0.10 and 1.0 — uses `CarlaUnreal.sh`).
Solution: the skill searches for both and warns when neither appears; check the
printed path, and re-run with `--keep-archive` to retry extraction without
re-downloading.

**Problem: `the UE5 nightly is published for Linux only`**
Cause: no `Windows/Dev/CARLA_UE5_Latest.zip` is published — the UE5 nightly is a
Linux artifact only.
Solution: `release --version 0.10.0` gives a UE5 build on Windows.

**Problem: Docker image runs but no client works**
Cause: the image ships the simulator only.
Solution: [[install-python-api]] `--source pypi --version <image tag>`.

## Outputs

An extracted CARLA (or a checkout, or a pulled image) under `CARLA_DOWNLOAD_DIR`,
plus the printed `kind`/`version`/`path` and the exports the next skill needs. The
archive is removed after a successful extraction unless `--keep-archive` is given.

Per-mode detail, the CDN layout, version/naming history and what each artifact
contains: [`references/sources.md`](references/sources.md).
