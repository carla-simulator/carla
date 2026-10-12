# Where CARLA actually comes from

Detail layer for `download-carla`. Every URL, size and naming quirk below was
resolved live (2026-09) against the GitHub API and the mirrors; `list`/`resolve` re-query
it, so treat the numbers as illustrative and the *mechanisms* as the contract.

## GitHub releases carry no assets

`GET /repos/carla-simulator/carla/releases` returns **zero attached assets** for
every release. The download links live in the release **body**, as markdown. Two
different shapes, depending on the vintage:

```
0.9.16  - `[Ubuntu]`  [CARLA_0.9.16.tar.gz](https://downloads.carlasim.com/Linux/CARLA_0.9.16.tar.gz)
0.9.15  - `[Ubuntu]`  [CARLA_0.9.15.tar.gz](https://tiny.carla.org/carla-0-9-15-linux)
0.10.0  - `[Ubuntu]`  [CARLA_0.10.0.tar.gz](https://tiny.carla.org/carla-0-10-0-linux-tar)
```

0.9.16 links a mirror directly. Everything older (and 0.10.0) links
`tiny.carla.org`, which 308-redirects to `carla-releases.b-cdn.net`.

## The b-cdn zone is dead — remap by path

`carla-releases.b-cdn.net` is a BunnyCDN pull zone, and since ~2026-09 it answers
**HTTP 403** (`server: BunnyCDN-...`, html body) for *every* object, with and
without a browser UA/Referer. Every pre-0.9.16 shortlink therefore lands on a 403.
The release bodies have not been rewritten, so the GitHub API alone cannot get you
a working URL.

The files are unchanged on the live mirrors, under the **identical path**, so
`resolve_url` strips the host and retries in order:

| Mirror | 0.9.16 | nightly `Dev/` | 0.9.15 and older | 0.10.0 |
|---|---|---|---|---|
| `downloads.carlasim.com` | yes | yes | **404** | **404** |
| `carla-releases.s3.us-east-005.backblazeb2.com` | yes | yes | yes | yes |
| ~~`carla-releases.b-cdn.net`~~ | 403 | 403 | 403 | 403 |

The Backblaze bucket is the origin the CDN fronted; the docs' own download pages
now link it (the UE5 docs use the equivalent `s3.us-east-005.backblazeb2.com/carla-releases/…`
form). It alone is sufficient — `downloads.carlasim.com` is preferred first only
because it is the host CARLA itself advertises today.

Practical consequences for the code:

- Probing must read the **status**, not just "did we get headers": a dead mirror
  answers HEAD with 403 and no `Content-Length`, which an older size-only probe
  read as "size unknown" and then tried to download.
- `urllib`'s HEAD does **not** follow the 308 (it raises `HTTPError 308`), which is
  why probing uses `curl -sIL`, falling back to a 1-byte ranged GET.
- A remap prints a `note:` line. If the notes ever disappear, the bodies were
  fixed upstream; if a *new* host starts 403ing, the note points straight at it.

## Naming is not stable across the line

| Release | Resolved file | Size | Served by |
|---|---|---|---|
| 0.9.16 | `Linux/CARLA_0.9.16.tar.gz` | 8.3 GB | carlasim.com |
| 0.9.16 maps | `Linux/AdditionalMaps_0.9.16.tar.gz` | 14.8 GB | carlasim.com |
| 0.9.15 | `Linux/CARLA_0.9.15.tar.gz` | 8.4 GB | Backblaze |
| 0.9.15 maps | `Linux/AdditionalMaps_0.9.15.tar.gz` | 7.4 GB | Backblaze |
| 0.9.14 | `Linux/CARLA_0.9.14.tar.gz` | 6.7 GB | Backblaze |
| 0.9.16 (Windows) | `Windows/CARLA_0.9.16.zip` | 7.8 GB | carlasim.com |
| **0.10.0** | `Linux/Carla-0.10.0-Linux-Shipping.tar.gz` | 10.4 GB | Backblaze |
| nightly (UE4) | `Linux/Dev/CARLA_Latest.tar.gz` | 8.4 GB | both |
| nightly maps | `Linux/Dev/AdditionalMaps_Latest.tar.gz` | 14.8 GB | both |
| nightly (UE5) | `Linux/Dev/CARLA_UE5_Latest.tar.gz` | 15.7 GB | Backblaze |

`Linux/CARLA_0.10.0.tar.gz` returns **404** on every mirror — the UE5 line renamed
the artifact. Anything that builds URLs from a template breaks there, which is the
whole reason for reading the release body.

The 0.10 line also renames the launcher (`CarlaUnreal.sh`, not `CarlaUE4.sh`), so
`find_launcher` looks for both. Note that a 0.10 download is a **UE5** build: the
`ue4` skills do not apply to it, and its client wheel is likewise 0.10.x.

`nightly` means the UE4 `Dev` build; `nightly --ue5` fetches `CARLA_UE5_Latest.tar.gz`
instead. The UE5 nightly is **Linux only** — no `Windows/Dev/CARLA_UE5_Latest.zip`
is published (404 on both mirrors) — and there is no separate
`AdditionalMaps_UE5_Latest`, because the UE5 package bundles its content. So
`--ue5` refuses on Windows and takes no `--with-maps`.

## What each mode gives you

### `release` / `nightly`

A cooked, runnable simulator. Contains:

- `CarlaUE4.sh` (or `CarlaUnreal.sh`) — the launcher `run-carla-server` detects;
- `PythonAPI/carla/dist/*.whl` — the **matching** client, which is why
  [[install-python-api]] prefers a bundled wheel over PyPI;
- `Import/` + `ImportAssets.sh` — where AdditionalMaps is staged and imported;
- cooked content only: **no source**, so nothing here can be built or edited.

`nightly` is the same shape with no version identity — it is a moving target, fine
for "does master fix my bug", wrong for anything reproducible. `nightly --ue5` is
the UE5 equivalent: same shape, `CarlaUnreal.sh` as the launcher, a 0.10.x client
wheel inside, and the `ue4` skills do not apply to it.

### `git`

A source checkout, and *only* the source: **content is not included**. The maps
and assets are a separate ~31 GB fetch ([[build-carla-ue4]] step 05), and the
engine is another ~80 GB build. A checkout is the right choice only when the goal
is building, importing assets, or changing C++ — otherwise a release is two orders
of magnitude cheaper.

`--depth 1` is used deliberately: full CARLA history is large and no skill needs it.
Pass a tag (`--ref 0.9.16`) for a released source tree, or a branch (`ue4-dev`,
`ue5-dev`) for development.

### `docker`

`carlasim/carla:<tag>` ships the simulator, headless, and expects the NVIDIA
container runtime plus `--net=host` for the RPC/streaming ports. The client is not
in the image: install it on the host with [[install-python-api]]. Useful for CI and
for keeping a machine clean; not useful for the `ue4` build/import skills, which
need a real checkout on the filesystem.

## Disk arithmetic

The check in `check_env.sh` uses these numbers:

| Step | Cost |
|---|---|
| release archive | ~8.3 GB (0.9.x) / ~10.4 GB (0.10) |
| extraction | roughly the same again |
| **so a release needs** | **~19 GB free**, transiently |
| AdditionalMaps archive | ~14.8 GB, plus the import |
| a checkout + content + engine | ~1 GB + ~31 GB + ~80 GB |

The archive is deleted after a verified extraction (unless `--keep-archive`), so the
steady-state cost is roughly the extracted size.

## Integrity

CARLA publishes no checksums for these artifacts, so the strongest available check
is the mirror's `Content-Length` compared against the file on disk — which the skill
does before extracting, and which also makes a truncated resume detectable. There is
no signature to verify; if that matters for your environment, mirror the artifacts
yourself and check them out-of-band.
