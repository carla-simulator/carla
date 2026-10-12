# Workspace-side patches for the CARLA e2e (VAD) stack

These patches capture the workspace edits that were required to get
`run_carla_autoware.sh --mode e2e` driving end-to-end (validated 2026-08-24,
Autoware 1.9.0 source workspace, autoware_universe/autoware_launch 0.52.0,
autoware_launch PR #1685 VAD glue). They apply to the *Autoware workspace*
checkouts (default `~/autoware/src/...`), not to this repository.

Recommended setup sequence (after `install_autoware.sh --source --with-vad`):

```bash
AW=~/autoware/src
P=$(dirname "$0")   # this patches/ directory

# 1. autoware_launch: put the PR #1685 feature commit on top of the 0.52.0 tag
#    (the PR branch predates the tier4_*_launch restructure; see the installer's
#    checkout_vad_launch_branch notes for the conflict-resolution guidance).
git -C $AW/launcher/autoware_launch checkout -B vad-on-0.52 0.52.0
git -C $AW/launcher/autoware_launch cherry-pick 90d1465d   # resolve conflicts, skip the style commit

# 2. Apply the launch/config patches.
git -C $AW/launcher/autoware_launch apply "$P/autoware_launch-e2e-gating-fix.patch"
git -C $AW/launcher/autoware_launch apply "$P/autoware_launch-e2e-departure-fix.patch"
git -C $AW/universe/autoware_universe apply "$P/autoware_universe-vad-tf-clock-segfault.patch"
git -C $AW/universe/autoware_universe apply "$P/autoware_universe-vad-input-800x450.patch"

# 3. Rebuild the touched packages (launch/config are symlink-installed; only
#    the universe lib change needs a compile).
cd ~/autoware && colcon build --symlink-install --packages-select autoware_tensorrt_vad
```

## What each patch fixes

- **autoware_launch-e2e-gating-fix.patch** (`e2e_simulator.launch.xml`):
  PR #1685 gates the e2e branch with a python `$(eval ...)` that yields the
  python literal `True`, which never string-compares equal to `'true'`, so the
  full classical perception/planning stack launches anyway (and its traffic
  light TensorRT nodes crash-loop). Normalizes the flag to a `'true'/'false'`
  string and forwards it through the deprecated `use_e2e_planner` alias so the
  e2e planning branch actually activates.

- **autoware_launch-e2e-departure-fix.patch** (`pid.param.yaml`):
  `enable_keep_stopped_until_steer_convergence: false`. An e2e planner
  re-infers the trajectory every frame, so the trajectory endpoint jitters by
  more than the MPC's `new_traj_end_dist` (0.3 m) and
  `isTrajectoryShapeChanged()` never settles; steer convergence is therefore
  never reported, and with this flag `true` the longitudinal controller holds
  `stopped_acc` (-3.4 m/s²) forever — engage succeeds but the vehicle never
  departs, with no log output above DEBUG level.

- **autoware_universe-vad-tf-clock-segfault.patch** (`coordinate_transformer.cpp`):
  the TF-miss error path passes a temporary `rclcpp::Clock::make_shared()`
  into `RCLCPP_ERROR_THROTTLE`; the throttle macro keeps a reference across
  calls, so the first missed base_link->camera lookup dereferences a dangling
  clock and the vad_node dies with SIGSEGV instead of logging. Upstream-report
  candidate.

- **autoware_universe-vad-input-800x450.patch** (`vad_carla_tiny.param.yaml`):
  matches `interface_params.input_image_width/height` to the 800x450 camera
  rig spawned by `run/spawn_vad_rig.py` (six raw 1600x900 RGBA streams
  saturate DDS on a single host; 800x450 keeps 16:9 so the model-input resize
  stretch matches training; VAD's own input is 640x384 per param.json).

Two further e2e requirements are handled inside this repository (no workspace
patch needed): the sim publishes `camera_info` under the sensor's topic
namespace and stamps `frame_id` with the bare sensor name, so
`run/e2e_state_publishers.launch.py` relays
`<base>/image_raw/camera_info -> /sensing/camera/CAM_*/camera_info` and
publishes identity static transforms `CAM_*/camera_optical_link -> CAM_*`.
Without these the VAD input frame is never complete (silently — no log) and,
respectively, the first TF lookup segfaults an unpatched vad_node.
