# CARLA Studio — Third-Party Notices

CARLA Studio is licensed under the **GNU Affero General Public License,
version 3 or later (AGPL-3.0-or-later)** — see `LICENSE` at the project root.
AGPL is a strong copyleft license with a network-use clause: anyone who
modifies CARLA Studio and offers the modified version to users — including
over a network (web UIs, SaaS frontends, remote-driving consoles) — must
release the modified source code to those users. SPDX identifier:
`AGPL-3.0-or-later`.

- **Author**: Abdul, Hashim — [github.com/im-hashim](https://github.com/im-hashim) · [linkedin.com/in/HashimAbdul](https://linkedin.com/in/HashimAbdul)
- **Maintainer**: Open — community-driven. Anyone willing to step in is welcome; see [CONTRIBUTING.md](../CONTRIBUTING.md) for how to volunteer.
- **Reviewer**: Per-PR convention — the contributor with the most lines contributed in the file modified by a PR acts as the default reviewer (see [CONTRIBUTING.md](../CONTRIBUTING.md)). Shifts review load to whoever has the most context, with no permanent reviewer roster.

For citation, see `CITATION.cff` at the project root — GitHub renders a
"Cite this repository" button from it that emits BibTeX, APA, and Chicago
forms automatically. Copyright notices embedded in source files satisfy
AGPL §4–5 (notice preservation); CITATION.cff is the social/academic layer
on top of that.

**License compatibility note.** AGPL-3.0 is compatible with permissive
licenses (MIT, BSD, Boost-1.0, ISC), with Apache-2.0 (one-way: combined work
becomes AGPL), and with the LGPL family (also one-way). It is **not**
compatible with GPL-incompatible source-available licenses such as CDDL,
EPL-1.0, or MS-PL. Every third-party dependency listed below has been
audited against this constraint. When you add a new dependency, verify its
license against the FSF compatibility matrix at
<https://www.gnu.org/licenses/license-list.html> before merging.

Studio bundles, links to, or coordinates a number of third-party components —
each retains its own license. Nothing in this file changes the licensing of
those components; the entries below exist so distributors can discharge their
attribution obligations and end-users can audit what runs.

The list is grouped by how each component reaches the user:

- **Bundled** — shipped inside the CARLA Studio binary or installer (Qt
  resources, vendored sources).
- **Linked** — required at build/run time but expected to be present on the
  user's system (e.g. Qt, LibCarla, system libraries).
- **Runtime-fetched** — downloaded by Studio on demand (CARLA binary,
  Hugging Face artifacts, NVIDIA NGC images, third-party tool repos).
  Studio never bundles these; it only orchestrates the fetch.

If you spot a missing entry or a license-version drift, file an issue against
this file before shipping a release.

---

## Bundled

| Component | License | Source |
|---|---|---|
| Sample Qt example plugin (`qt-carla-studio-example`) | AGPL-3.0-or-later | This repository, `Examples/QtClient/` |
| Lens-distortion CSV presets | AGPL-3.0-or-later (this project) | `resources/cameras/Lens_*.csv` |
| Mcity Quick-Start launch scripts (`start.sh`, `stop.sh`, `wait_functions.sh`) | Mixed: AGPL-3.0-or-later (Studio overlays) over upstream notices | `resources/integrations/scripts/` |
| Mcity terminal launch map (JSON) | AGPL-3.0-or-later (this project) | `resources/integrations/launch/terminal_launch_map.json` |
| Mcity RViz config (`carla_mcity_ros2.rviz`) | Apache-2.0 (Mcity / Michigan Traffic Lab origin) | `resources/integrations/rviz/` |
| Default vehicle blueprint thumbnails / metadata | per-asset; manifest in `resources/vehicles/MANIFEST.md` (PR 3) | `resources/vehicles/` |

## Linked (build / runtime)

| Component | License | Notes |
|---|---|---|
| Qt 5/6 (Widgets, Core, optionally Gamepad, Test) | LGPLv3 / GPLv3 / commercial | Dynamic linkage; ensure your distribution complies with LGPL relinking rules. |
| LibCarla (`carla-client`) | MIT | Distributed by the CARLA project. Optional — Studio degrades when absent. |
| Boost (Container, Filesystem, Random, Context, Atomic, Chrono, Coroutine, DateTime, Thread, Serialization, Exception) | Boost Software License 1.0 | Pulled in via LibCarla. |
| rpc / RPCLib | MIT | Pulled in via LibCarla. |
| zlib / png | zlib / libpng | Pulled in via LibCarla. |
| Recast / Detour / DetourCrowd | zlib | Pulled in via LibCarla. |

## Runtime-fetched (Studio orchestrates the download; not bundled)

| Component | License | Source |
|---|---|---|
| CARLA simulator binary releases (`CARLA_X.Y.Z.tar.gz`, `AdditionalMaps_*.tar.gz`) | MIT | `github.com/carla-simulator/carla` |
| TeraSim-Mcity-2.0 | per upstream LICENSE | `github.com/michigan-traffic-lab/TeraSim-Mcity-2.0` |
| Autoware (Mcity fork) | Apache-2.0 | `github.com/michigan-traffic-lab/autoware` |
| NVIDIA NuRec container images | NVIDIA AI Enterprise EULA / per NGC terms | `nvcr.io` and `docker.io/carlasimulator/nvidia-nurec-grpc` |
| NVIDIA Alpamayo / Alpamayo-R1-10B (model weights) | NVIDIA Open Model License (per HF model card) | `huggingface.co/nvidia/Alpamayo-R1-10B` |
| NVIDIA AlpaSim | per upstream LICENSE | `github.com/NVlabs/alpasim` |
| NVIDIA NCore | Apache-2.0 (per upstream LICENSE) | `github.com/NVIDIA/ncore` |
| NVIDIA Lyra 2.0 | per upstream LICENSE | `github.com/nv-tlabs/lyra` and `huggingface.co/nvidia/Lyra-2.0` |
| NVIDIA Asset-Harvester | per upstream LICENSE | `github.com/NVIDIA/asset-harvester` |
| Physical AI Autonomous Vehicles dataset (and NCore / NuRec variants) | NVIDIA Open Data License (per HF dataset card; gated repo) | `huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles*` |
| `huggingface_hub` Python package | Apache-2.0 | `pip install huggingface_hub` (only when Studio chooses the CLI download path) |
| `git-lfs` | MIT | Used as a fallback for HF downloads. |
| `uv` (Python env manager) | Apache-2.0 / MIT (dual-licensed) | `astral.sh/uv` |
| NVIDIA Brev CLI | per upstream LICENSE | `github.com/brevdev/brev-cli` |
| OpenSSH client | OpenSSH license / BSD | Used for Remote (LAN) compute backend. |
| Docker / `docker` CLI | Apache-2.0 | Used for Docker mode and NuRec containers. |
| Redis server | RSALv2 / SSPLv1 (recent releases) | Required by TeraSim co-simulation. |

## Combined-work licensing under AGPL-3.0-or-later

The Studio code in `Apps/CarlaStudio/` is offered under AGPL-3.0-or-later.
The combined work — Studio plus its linked dependencies — is governed by
AGPL-3.0 because AGPL is the most restrictive of the licenses involved.
Permissive deps (MIT, BSD, Boost, Apache-2.0) are absorbed compatibly;
LGPL'd Qt is dynamically linked and follows LGPL's relinking allowance,
which is preserved when the host application is under a stronger copyleft.

If you redistribute Studio (binary or source), you must:

1. Pass on `LICENSE` and `LICENSES/THIRD_PARTY.md` verbatim with the
   distribution.
2. Make the Corresponding Source available — the AGPL §6 "written offer"
   on a network server suffices. Studio's "Help → License" dialog
   surfaces a "Source" link to satisfy AGPL §13 for network use.
3. Preserve the SPDX identifier `AGPL-3.0-or-later` in any source-file
   headers you copy into a derivative work.

This file is not legal advice — it is a manifest of what runs. When in
doubt, consult an attorney familiar with copyleft licensing.

## How this manifest is maintained

Studio's "Help → License" dialog renders this file alongside the AGPL-3.0
text and a runtime-detected list of installed third-party tools. When PR 3
lands the Tools / Datasets / Models dialogs, each install action will
append a row to a runtime manifest at `~/.cache/carla_studio/installed.json`
recording repo, version, install path, and license tag — visible from the
same dialog. That runtime manifest is what powers the AGPL §13 "Source"
link's "what's running right now" disclosure.
