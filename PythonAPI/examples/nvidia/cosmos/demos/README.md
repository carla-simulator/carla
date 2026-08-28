# carla-cosmos demos

Three families:

* `showcase.py` + `showcase_sheets.py` — the **mode matrix**: one job per
  conditioning setup, stored and turned into comparison videos (see *Showcase*
  below).
* `api_*.py` — **Python API** demos.  They import `carla_cosmos` directly (no
  subprocess to the CLI), are short enough to read in one go and each stores its
  results through `ResultStore` (see *Results storage* below).
* the older workflow demos (`single_view_live.py`, `single_view_replay.py`,
  `av7_world_scenario.py`) — full pipelines with capture options, batches and
  the viewer.

All of them read the node from `--endpoint`/`--token`, else `COSMOS_URL` /
`COSMOS_TOKEN` (`COSMOS_TOKEN_FILE` also works).  The `api_*` demos check the
node **before** doing anything expensive and exit 1 with an actionable message
when it is unreachable, not ready, or does not serve the backend.

| demo | what it shows | backend | runtime |
| --- | --- | --- | --- |
| `api_transfer25_basic.py` | load a clip from disk, submit `depth`+`seg` from the clip and a server-derived `edge`, wait, store, print where the videos are | `transfer2.5` | ~141 s generation for 93 f @ 16 fps, 480p (measured on 4x RTX PRO 6000); plus upload of the clip on first use |
| `api_cosmos3_wsm.py` | single view, world-scenario control rendered server-side from the clip's ClipGT scene package; shows the returned `control_wsm.mp4` | `cosmos3-nano` (or `-super`) | ~86 s for 101 f @ 10 fps, 480p (measured, Nano) |
| `api_av_multiview.py` | 7-camera AV job with the `hdmap_bbox` scene control, keeps the rendered controls and `grid.mp4`, then opens `viewer.py` as a per-camera grid | `transfer2.5-av` | not yet measured end to end on 7 views (worker smoke was 42 s on 2 GPUs) |
| `api_masking_and_weights.py` | remove semantic classes (`--mask-classes vehicle`) from the pixel-derived inputs and set per-control weights, then submit, wait and store | `transfer2.5` | same as `api_transfer25_basic.py`, plus ~1 min to re-encode the masked videos for a 93 f 720p clip |
| `api_capture_to_job.py` | capture a fresh clip from a running CARLA server (traffic + rig code paths reused from `single_view_live.py`), then submit and store | any (default `transfer2.5`) | capture ~1-2 min for 93 frames, plus the backend's own time |
| `single_view_live.py` | TM-driven hero, single 720p camera, capture → submit (`--backend`, `--control`) → download → viewer | any | capture + backend |
| `single_view_replay.py` | deterministic clip from a recorder log, then `batch.yaml` prompts x seeds as `batch` jobs | any | one backend run per (prompt, seed) |
| `av7_world_scenario.py` | NVIDIA 7-camera rig, ClipGT scene export, Transfer 2.5 AV; `--also-cosmos3` adds a Cosmos 3 `wsm` job | `transfer2.5-av` (+ `cosmos3-*`) | capture + backend |
| `viewer.py` | input \| control \| result side by side, per camera, scrubbing; `--clip` alone shows the local GT preview | — | interactive |
| `showcase.py` | the whole mode matrix (11 rows: weather x controls x weights x mask-out classes x world scenario x 7-camera AV) against one node, resumable, timings per row | all | 183-244 s per Transfer 2.5 row, 543-999 s per Cosmos 3 row, 1390 s per 7-camera AV row (720p, measured 2026-08-28 on 4 x RTX PRO 6000) |
| `showcase_sheets.py` | `input \| control(s) \| output` sheets, the 7-camera grid and a `modes_reel.mp4` from a showcase run | — | ~1 min per row |

Runtimes marked *measured* come from the first GPU run on the 4x RTX PRO 6000
node (see `.omc/plans/cosmos-handoff.md`); they exclude queueing and upload.

## Running them

```bash
export COSMOS_URL=http://<node>:8000
export COSMOS_TOKEN=cc_...
export COSMOS_RESULTS=$PWD/cosmos-results          # optional, see below

python demos/api_capture_to_job.py --port 2000 --frames 93 --fps 16   # CARLA -> clip -> job
python demos/api_transfer25_basic.py --clip ./clips/api_town10_1756...
python demos/api_cosmos3_wsm.py     --clip ./clips/<10|15|30 fps clip with scene/>
python demos/api_av_multiview.py    --clip ./clips/av7_...            # add --no-view for headless
```

No CARLA server?  `carla-cosmos synthetic-clip --out ./clips --frames 93 --fps 16 --scene`
(or `--av7`) writes a test-pattern clip the first three demos accept.

## Results storage

Nothing has to be downloaded by hand.  `job.download(out_dir)` (and every
`api_*` demo, and `carla-cosmos submit --wait` / `watch` / `result`) writes:

```
<results root>/
  index.json                       every job this machine knows about
  <clip_id>/<job_id>/
    <camera>.mp4                   generated video(s)
    control_<hint>[_<camera>].mp4  the control the model actually saw
    grid.mp4                       multi-view contact sheet (AV)
    manifest.json                  the server's result listing, verbatim
    job.json                       request as submitted (prompt, seed, backend,
                                   controls incl. weights, views, resolution),
                                   timings, server status/error, every file with
                                   size + sha256, and when the server's copy expires
```

The results root is `--results` (`--out` for the CLI), else `$COSMOS_RESULTS`,
else `./cosmos-results`.  Files are verified against the server's listing (size,
then sha256) and downloads are idempotent — re-running a demo re-checks what is
on disk and fetches only what is missing or damaged.

`carla-cosmos jobs` prints, for every job the server lists, whether its result
is stored here and where, warns before the server garbage-collects results that
are not stored (server default: 168 h after the job finishes,
`COSMOS_JOB_TTL_HOURS`), and lists results that are only on this machine.

## Showcase

`showcase.py` runs one job per conditioning setup and keeps everything:

```bash
export COSMOS_URL=http://<node>:8000
export COSMOS_TOKEN=cc_...
export COSMOS_RESULTS=/home/german/Projects/CARLA_SOURCE/cosmos-results

python demos/showcase.py --list                       # the matrix, nothing else
python demos/showcase.py --clips ./clips               # all 11 rows
python demos/showcase.py --only av7-day,c3-wsm         # a subset (row id, backend or clip role)
python demos/showcase.py --mock --clips ./clips        # the whole matrix against the mock server
python demos/showcase_sheets.py                        # -> $COSMOS_RESULTS/_showcase
```

| row | backend | conditioning | what it shows |
| --- | --- | --- | --- |
| `t25-golden` | transfer2.5 | depth + seg + edge | golden hour — the reference row |
| `t25-rain` | transfer2.5 | depth + seg + edge | heavy rain, wet road, same controls |
| `t25-night` | transfer2.5 | depth + seg + edge | night, same controls |
| `t25-w-depthseg` | transfer2.5 | depth 0.7 / seg 0.3 | geometry-led weights, no edge branch |
| `t25-w-edge` | transfer2.5 | edge only, w = 1.0 | the edge branch alone, same prompt as `t25-golden` |
| `t25-mask-vehicles` | transfer2.5 | + `--mask-classes vehicle` | the traffic removed from every pixel-derived input; the model re-imagines it |
| `t25-mask-vru` | transfer2.5 | + `--mask-classes vru` | pedestrians, riders and bicycles removed |

Measured on the node (720p, 2026-08-28): the Transfer 2.5 rows cost 235-244 s of generation with three
control branches, 210 s with two and 183 s with one — the branch count, not the prompt, sets the price.
Masking adds ~56 s of client-side re-encode and nothing on the GPU. The AV rows are fixed by shape
(7 views x 57 frames = 2 chunks x 35 steps): 1390 s, plus 83 s to render `hdmap_bbox` for seven cameras.
| `c3-wsm` | cosmos3-nano | `wsm=scene` | the world-scenario map alone: layout from the scene package |
| `c3-wsm-depth-seg` | cosmos3-nano | `wsm` + depth + seg | layout **and** the captured appearance |
| `av7-day` | transfer2.5-av | `hdmap_bbox=scene`, 7 views | the seven-camera rig, occlusion-filtered obstacles |
| `av7-rain` | transfer2.5-av | `hdmap_bbox=scene`, 7 views | the same scene, weather variation |

Three clips feed it (roles `t25`, `wsm`, `av7`): 93 frames @ 16 fps, 101 @ 10 and
171 @ 30 with the `nvidia_av7` rig — the frame-count rules of the three
backends.  `--clip t25=<dir>` overrides one, `--clips <dir>` looks for
`showcase_t25` / `showcase_wsm` / `showcase_av7` inside it.

A row whose stored directory already holds a video is skipped, so an
interrupted run resumes (`--force` reruns).  Every row is appended to
`<results>/showcase.json` with its prompt, controls, weights, mask classes and
timings (`queued` / `rendering` / `running` from the server, upload, download
and wall clock from the client); `showcase_sheets.py` reads that file.
