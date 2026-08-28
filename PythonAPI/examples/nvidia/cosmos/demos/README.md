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
| `viewer.py` | input \| control \| result side by side, per camera, scrubbing; `--export FILE` serialises the same layout to mp4 with no display; `--clip` alone shows the local GT preview | — | interactive; ~2 s (single) / ~45 s (7-camera grid) to export |
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
    viewer_grid.mp4                the viewer, as a video (multi-camera results)
    viewer_single.mp4              the viewer, as a video (single-camera results)
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

### The viewer video (on by default)

Every stored job also gets `viewer_<layout>.mp4`: the side-by-side viewer
(`input | control | result`, or the per-camera grid with the control the model
saw over each result) rendered to a file, recorded in `job.json` with
`"kind": "viewer"`.  It is the *same compositor* the interactive window uses —
the same tiles, labels, aspect fitting and footer with the prompt — so the file
is exactly what `viewer.py` draws, only serialised.

It needs no display: rendering only uses `pygame`'s offscreen surfaces and the
font module, never SDL's video subsystem, so it works over ssh, in a container
and under `systemd-run` with no `DISPLAY`.  Encoding is ffmpeg (`libx264`,
`crf 18`, `yuv420p`, even dimensions, no audio track), so the mp4 plays in a
browser.  Roughly 2 s for a 93-frame single-camera row, 45 s for a 171-frame
7-camera grid.

Turn it off with `--no-viewer-video` (`showcase.py`, every `api_*` demo,
`carla-cosmos submit`/`watch`/`result`), `viewer_video=False`
(`job.download(...)`, `wait_and_store(...)`), or `COSMOS_VIEWER_VIDEO=0` for a
whole session.  The viewer needs the clip the job was generated from — the demos
pass it, and otherwise the store looks for it in `clips/` and `_clips/` beside
the results root (`$COSMOS_CLIPS` or `--clip` says where else to look).  When it
cannot be found the download still succeeds, with a note and no viewer video.

Export one by hand, or re-export with different options:

```bash
python demos/viewer.py --result $COSMOS_RESULTS/showcase_av7/j_xxxx --export av7.mp4
python demos/viewer.py --result <dir> --export out.mp4 --export-layout both \
    --export-fps 30 --export-overlay on --height 480
```

`--export` implies headless.  `--export-layout` is `auto` (grid for a
multi-camera result, the strip otherwise), `grid`, `single` or `both` (which
writes `out_grid.mp4` and `out_single.mp4`).  `--export-overlay on` bakes in the
`o` key: the control drawn over the input RGB.  In the interactive window, `v`
exports the layout on screen to `<result>/viewer_<layout>.mp4`.

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

A row may also carry `skip="reason"`: it stays in the matrix as the record of an
experiment that was run once, or reasoned about, and is not worth GPU time again.
The driver writes it to the ledger as `status: "skipped"` with the reason and
does not submit it (`--force` runs it anyway).

### The "just the RGB" rows, and what they showed (2026-08-28)

| row | backend | conditioning | what it shows |
| --- | --- | --- | --- |
| `t25-vis-golden` | transfer2.5 | `vis=derive` (RGB is the only upload) | 208 s. Photoreal pass over the capture — the layout, colours and lighting all survive |
| `t25-vis-night` | transfer2.5 | `vis=derive`, night prompt | 207 s. **The prompt is ignored**: the output is still broad daylight |
| `c3-blur` | cosmos3-nano | `blur=derive` | 516 s. Same story on the Nano |
| `c3-blur-mask-vehicles` | cosmos3-nano | `blur=derive` + `--mask-classes vehicle` | 513 s. The masked vehicles come back as **black holes** — see the mask note below |
| `c3-edge-night` | cosmos3-nano | `edge=derive`, night+rain prompt | the same question asked of a control that carries no colour |
| `c3-rgb-depth-seg-night` | cosmos3-nano | `blur=derive` + depth + seg | the RGB complemented by geometry rather than replaced by it |

`vis` (Transfer 2.5) and `blur` (Cosmos 3) are the same idea: a bilateral-filtered,
down-up-sampled copy of the capture.  It keeps every colour and every luminance, so a
job whose only control is one of them can re-texture the input but never restyle it.
`carla_cosmos.controls.blur_control` is the vendor kernel ported, so the control can be
looked at without a GPU; on `showcase_wsm` frame 50 it is 20.5/255 away from the RGB at
the vendor's default preset `medium` and 22.7/255 at the strongest, `very_high`.  The
client therefore asks for **`very_high`** whenever it leaves `blur`/`vis` to the server
(`carla_cosmos.client.DEFAULT_BLUR_PRESET`); pass
`extra={"preset_blur_strength": "medium"}` for the vendor default.  To actually restyle,
condition on something that throws the colours away — `edge`, `depth`, `seg`, `wsm`.

### Sampling keys that reach vLLM-Omni through `extra`

`control_guidance`, `control_guidance_interval`, `flow_shift`, `num_conditional_frames`,
`num_video_frames_per_chunk`, `sigma_max`, `preset_edge_threshold`, `preset_blur_strength`,
`show_control_condition`, `guardrails` — all passed verbatim by the cosmos3 worker
(`guidance` and `num_steps` are their own fields of `JobRequest`).

**Cosmos 3 multi-hint guidance.** vLLM-Omni's per-hint sampling preset
(`TRANSFER_DEFAULTS`: `wsm` 1.0/3.0/10, `depth`/`edge`/`blur` 3.0/1.5/10, `seg` 3.0/2.0/10
for guidance / control_guidance / flow_shift) is applied **only when the request carries
exactly one hint**.  With two or more it falls back to guidance 6.0 and control_guidance
1.0 — which is what made `c3-wsm-depth-seg` hazy and low-contrast next to the sharp
single-hint `c3-wsm`.  Our cosmos3 worker now fills a documented regime for multi-hint
requests instead (guidance 2.0, control_guidance 2.0, flow_shift 10;
`cosmos_workers.cosmos3.worker.MULTI_HINT_DEFAULTS`), anything the caller names still
wins, and the effective numbers with their provenance land in the result manifest under
`sampling`.

**Masking is refused where the backend has no mask input.**  `--mask-classes` /
`mask_classes=` on `cosmos3-nano` / `cosmos3-super` is now a validation error
(`carla_cosmos.contracts.mask_support_errors`), raised before anything is re-encoded.
Cosmos 3 exposes no equivalent of Transfer 2.5's `ControlConfig.mask_path`, so the mask
could only be baked into the pixels — and `c3-blur-mask-vehicles` showed the model then
reproduces the black hole frame for frame instead of re-imagining it.  Remove the objects
from the ClipGT scene package (the `wsm` control is rendered from it) instead.
