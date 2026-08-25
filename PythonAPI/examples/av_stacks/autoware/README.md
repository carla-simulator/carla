# Autoware on CARLA (Native ROS2)

Reference integration of [Autoware](https://autoware.org/) against CARLA's
built-in ROS2 interface. The CARLA server publishes sensors and vehicle status
and subscribes to Autoware's control commands **directly over DDS** — there is
no bridge process. Two driving modes are provided:

- **`classical`** — the full Autoware stack: NDT localization against a
  point-cloud map, lidar/camera perception, behavior/motion planning, and the
  classic trajectory-follower controller.
- **`e2e`** — the camera-only **VAD** end-to-end model
  (`autoware_tensorrt_vad` from Autoware Universe): six surround cameras in,
  a planned trajectory out, executed by the same classic trajectory follower.
  Localization comes from simulator ground truth instead of NDT.

If you have never used CARLA: CARLA is a UE5-based driving simulator. You run
one **server** process (the simulator) and any number of **clients** that
connect over RPC (default port 2000). In *synchronous mode* — required here —
the simulation only advances when exactly **one** designated client calls
"tick"; our spawner script owns that role. The native ROS2 layer means that,
once a client has enabled it, ROS2 topics appear on your DDS network just as
if a robot were publishing them.

---

## Prerequisites

**Hardware / OS**

- Ubuntu 22.04 (ROS2 Humble) or Ubuntu 24.04 (ROS2 Jazzy). Both are supported
  by current Autoware (release 1.9.0 / universe 0.52.0); Jazzy is Autoware's
  Docker default. The TIER IV validation of this integration was done against
  Autoware release 0.45.1 on Humble (source build).
- NVIDIA GPU strongly recommended; **required** for `--mode e2e` (TensorRT).
  CUDA + cuDNN + TensorRT must already be installed system-wide — the installer
  deliberately does **not** install them; `install_autoware.sh --check` reports
  on what is present (see `install/README.md`).
- Disk: budget ~100 GB free for an Autoware source build (sources + build +
  install + dependencies); the VAD model and its cached TensorRT engines add a
  few GB under `~/autoware_data/`.

**CARLA**

- A packaged CARLA build from this UE5.8 branch with native ROS2 enabled.
  Verify your package starts with ROS2 support:

  ```bash
  ./CarlaUnreal.sh --ros2
  ```

  (Canonical single-dash flags also work and are what our scripts use:
  `-ros2 -rmw=fastdds|cyclonedds|zenoh -ros-domain-id=N -carla-rpc-port=N`.)

- **Migration branch prerequisite (`autoware-port`).** The Autoware vehicle
  interface inside the simulator — publication of
  `/vehicle/status/{velocity_status,steering_status,gear_status,control_mode,turn_indicators_status,hazard_lights_status}`,
  subscription of
  `/control/command/{control_cmd,gear_cmd,turn_indicators_cmd,hazard_lights_cmd,emergency_cmd}`
  and `/vehicle/engage`, and the `PythonAPI/examples/autoware_demo.py`
  ego/sensor spawner — lands with the concurrent `autoware-port` migration
  work. Until your build includes it, `run_carla_autoware.sh` will detect the
  missing pieces and exit with a clear message rather than half-start. Nothing
  in `install/` or `map_tools/` depends on it, so you can prepare Autoware and
  maps ahead of time.

**Kernel UDP buffers (required — the stack is unusable without this)**

- Raise the socket-buffer limits to 64 MB **and persist them** before starting
  anything:

  ```bash
  sudo sh -c 'sysctl -w net.core.rmem_max=67108864 net.core.wmem_max=67108864 \
    && printf "net.core.rmem_max=67108864\nnet.core.wmem_max=67108864\n" \
       > /etc/sysctl.d/99-carla-dds.conf'
  ```

  At the stock 4 MB cap, the 20 Hz lidar `PointCloud2` stream over reliable
  DDS writers overruns the receive sockets (`netstat -su` shows
  `RcvbufErrors` climbing by the hundreds of thousands), reliable-writer
  retransmits stall every DDS participant for seconds at a time, and Autoware
  MRM-stops the vehicle. Buffer size is fixed at socket creation, so if you
  change the sysctl you must **restart every DDS process — including the
  simulator** — for it to take effect. The run script checks this and prints
  the fix if the limits are too low.

**ROS2 middleware (RMW)**

- Nothing to configure by hand: the run script generates the DDS configs and
  picks the validated topology (simulator on **Fast DDS**, Autoware on
  **CycloneDDS**, both pinned to the docker bridge). See the
  [compatibility matrix / DDS topology](#compatibility-matrix--dds-topology)
  before changing `--rmw`.

---

## Three-step flow

### 1. Install Autoware

```bash
./install/install_autoware.sh --check       # read-only prerequisite report
./install/install_autoware.sh               # source build (classical mode)
./install/install_autoware.sh --with-vad    # source build + VAD extras (e2e mode)
```

This performs a pinned source install of Autoware **outside** the CARLA tree
(no CARLA rebuild is involved). It pins the TIER IV-validated baseline
(`0.45.1` on Humble; current `1.9.0` on Jazzy). With `--with-vad` it
additionally checks out the `autoware_launch` branch from the (unmerged) PR
[autowarefoundation/autoware_launch#1685](https://github.com/autowarefoundation/autoware_launch/pull/1685)
that adds the `use_e2e_planning` / `e2e_planning_type:=vad` glue — see
[Known limitations](#known-limitations) — and downloads the VAD model
(HuggingFace `AutowareFoundation/tensorrt_vad`, tag `v0.1`) into
`~/autoware_data/ml_models/vad/v0.1/`. A `--docker` mode (prebuilt GHCR image)
is also available and is what the validated classical run used; see
`install/README.md`.

Both source and docker modes also fix up the **lidar_centerpoint model
layout**: the perception bundle lands under
`~/autoware_data/ml_models/lidar_centerpoint/<subdir>/` (e.g. `tiny/`), but the
launch expects four files *flat* in `lidar_centerpoint/`
(`centerpoint_tiny_ml_package.param.yaml`, `detection_class_remapper.param.yaml`,
`pts_voxel_encoder.onnx`, `pts_backbone_neck_head.onnx`); the installer creates
relative symlinks if they are missing. Without them classical-mode lidar
perception dies at startup.

### 2. Get map artifacts

> **Town10HD ships in-tree — skip this step for the default town.** The
> repository already commits the complete, validated artifact set in
> `map_tools/maps/Town10HD/` (UE5.8-regenerated point cloud, pruned lanelet2
> map, projector info); `run_carla_autoware.sh` finds it there by default.
> You only need this step for other towns or to regenerate.

Autoware needs, per town, a directory containing:

```
map_tools/maps/<Town>/
├── pointcloud_map.pcd        # for NDT localization (>= 0.2 m resolution)
├── lanelet2_map.osm          # HD vector map (lanes, rules)
└── map_projector_info.yaml   # must contain: projector_type: Local
```

(`map_tools/maps/<Town>` is where `run_carla_autoware.sh` looks by default;
any directory works via its `--map-path` option.)

**Option A — prebuilt (fast):**

```bash
./map_tools/fetch_prebuilt_maps.sh Town10HD
```

Downloads the community-maintained artifacts from
`bitbucket.org/carla-simulator/autoware-contents` (available for Town01–Town07
and Town10HD) into `map_tools/maps/Town10HD/`. These are UE4-era maps, but
TIER IV verified the Town10 pair works against UE5 Town10. Known warts: the
lanelet2 files are y-axis-inverted relative to raw CARLA coordinates (this is
the convention Autoware expects here, not a bug), they carry **no
traffic-light regulatory elements**, and there are small origin offsets
between the pcd and osm.

The upstream `.osm` files also lack the
`<MetaInfo format_version="1.0.0" map_version="1"/>` element that Autoware's
`route_handler` requires — without it the map is silently rejected, planning
never starts, and `mission_planner` can even segfault. The fetch script
injects it automatically (idempotently); for an `.osm` you obtained some other
way, run `./map_tools/fetch_prebuilt_maps.sh --metainfo-only <file.osm>`
(the run script also checks and fixes this at preflight).

**Option B — generate from a running simulator (any town, adds traffic lights):**

```bash
python3 ./map_tools/generate_map_artifacts.py \
    --town Town10HD_Opt --out ./map_tools/maps/Town10HD --tick
```

Connects to a running CARLA server (it never launches one), exports the road
network from OpenDRIVE, builds the lanelet2 map and point-cloud map, and
injects traffic-light regulatory elements from the simulator's actual
traffic-light actors. Caveat: the injected elements are not yet consumable by
Autoware's traffic-light pipeline (stop-line anchoring and camera ROI
projection are unverified), which is why the run script disables the
traffic-light module by default — treat generated maps as
traffic-light-free for now. `--tick`
makes the tool the **single** ticking client of a synchronous world; omit it
only if another client is already ticking. The lanelet2 converter needs the
pinned deps from `map_tools/requirements.txt` in a venv — see
`map_tools/README.md`, which also covers fully offline conversion
(`generate_lanelet2_map.py --xodr <file>`) and resolution options (`--help`).

### 3. Run

```bash
# Full classical stack, drive to a goal (CARLA coordinates: x,y,yaw°):
./run/run_carla_autoware.sh --mode classical --goal "80.0,-16.5,90"

# End-to-end VAD:
./run/run_carla_autoware.sh --mode e2e
```

The default town is `Town10HD_Opt` (the UE5 packaged name; its map dir is
`map_tools/maps/Town10HD`, matching step 2). Pick another town with
`--town <Name>`.

Classical mode runs Autoware either from your **source workspace**
(`--stack source`) or from the **official docker image** (`--stack docker`,
the validated path); the default `--stack auto` picks whichever is installed.
Useful common options: `--goal "x,y,yaw"` (drive there automatically; CARLA
coordinates, converted for you), `--spawn-index N` (which spawn point the ego
starts at), `--no-auto` (skip all post-launch automation), `--no-gates` (skip
the pre-engage safety gates — see below), `--with-rviz` (RViz in a sibling
container / local `rviz2`),
`--rmw fastdds|cyclonedds|zenoh` (simulator side; default **fastdds** — see
the matrix below), `--domain-id N` (default **42**; alias `--ros-domain-id`),
`--carla-rpc-port N`, `--map-path <dir>`, `--bridge-if <name>` /
`--docker-network <name>` to override the auto-detected DDS bridge, and
`--dry-run` to print every command without executing anything. Run `--help`
for the full list. Stop a running session from another terminal with
`./run/stop_all.sh` (kills only the exact process groups it started via the
pidfile in the log dir, and `docker rm -f`s only containers it created —
read its header before "improving" it with `pkill`).

Under the hood the script sequences, in order:

1. **DDS config generation**: auto-detects the docker bridge interface and
   writes `<log-dir>/dds/fastdds_profile.xml` (simulator side) and
   `<log-dir>/dds/cyclonedds.xml` (Autoware side). See
   [DDS topology](#compatibility-matrix--dds-topology).
2. **CARLA server** with native ROS2:
   `./CarlaUnreal.sh -ros2 -rmw=<rmw> -ros-domain-id=<N> -carla-rpc-port=<N>`
   with `FASTRTPS_DEFAULT_PROFILES_FILE` pointing at the generated profile
   (for `-rmw=zenoh` it first starts the required Zenoh router:
   `ros2 run rmw_zenoh_cpp rmw_zenohd`).
3. A **one-shot town loader** (non-ticking client) that switches the server to
   `--town` if needed.
4. **`PythonAPI/examples/autoware_demo.py`** — spawns the ego vehicle and the
   sensor rig, switches the world to synchronous mode, and becomes the **only
   ticking client**. Do not run other ticking clients (e.g. a second demo or a
   Traffic Manager script in sync mode) against the same server.
5. *(e2e only)* **`run/spawn_vad_rig.py`** (attaches the six VAD cameras to
   the ego; never ticks) and **`run/e2e_state_publishers.launch.py`**
   (ground-truth localization + image republish nodes).
6. *(classical only)* **CARLA-specific Autoware overrides** (applied via
   `docker exec`/`sed` in the container, or in the workspace install tree):
   - `ndt_scan_matcher.param.yaml`:
     `converged_param_nearest_voxel_transformation_likelihood: 2.3 → 1.0`.
     The UE4-era prebuilt pcd maps score low against UE5.8 geometry; without
     this NDT never reports convergence. Regenerating the pcd with
     `map_tools` (step 2, option B) removes the need for this override.
   - `tier4_localization_launch` `pose_twist_estimator.launch.xml`:
     `stop_check_enabled → false`. The sim runs below real time, so the
     stopped-vehicle check never passes and initialization hangs.
   - `diagnostics/autoware-carla.yaml`: the `/autoware/localization/state`
     and `/adapi/mrm_request/delegate` diag units get `timeout: 30.0`. These
     ADAPI topics publish at a low rate; at sub-realtime sim speed the stock
     3 s staleness window flaps ERROR and the MRM pulses EMERGENCY_STOP —
     the car freezes mid-drive with nothing actually wrong.
   - `planning/preset/default_preset.yaml`:
     `launch_traffic_light_module → false`. The generated lanelet2 maps do
     not yet carry usable traffic-light regulatory elements; with the module
     on, the car can wait forever at a light it cannot see.
7. **Autoware**, per mode (below).
8. *(classical, unless `--no-auto`)* **post-launch automation**: wait for the
   ADAPI, then `ros2 service call /api/localization/initialize` (an empty
   request auto-initializes from GNSS; retried — right after startup the
   initializer may not have a GNSS fix yet). Then, unless `--no-gates`, the
   **pre-engage safety gates** run:
   - **Gate 1 (localization truth)**: compares
     `/localization/kinematic_state` against CARLA ground truth (accounting
     for `base_link` being the **rear axle**, half a wheelbase behind CARLA's
     center transform). Must agree within 0.5 m / 5°. NDT can report
     convergence with the believed pose meters from the vehicle; engaging on
     a diverged pose drives the car into things — the gate refuses.
   - **Gate 2 (distortion corrector health)**: watches `/diagnostics` for
     12 s; any ERROR from the pointcloud distortion corrector fails the gate
     (a broken corrector silently degrades NDT once the car is moving).
   If the gates pass and `--goal` was given: publish the goal pose on
   `/planning/mission_planning/goal` and call
   `/api/operation_mode/change_to_autonomous` (retried until planning accepts
   it). Goal conversion CARLA → map frame: `x_map = x`, `y_map = -y`,
   `yaw_map = -yaw` (the quaternion is computed for you). The executed
   trajectory appears on **`/planning/trajectory`** (current Autoware moved it
   from the old `/planning/scenario_planning/trajectory`).

### Driving from RViz

With `--with-rviz` (and no `--goal`), drive interactively: **2D Goal Pose**,
then click on a lane and **drag along the driving direction** before
releasing — the arrow's heading matters. Mission planner accepts a goal only
if it lands inside a lanelet **and** points within ~45° of that lane's travel
direction; a click against traffic, in a junction interior, or on the
curb/median is rejected with `Goal is not valid! Please check position and
angle of goal_pose` in the Autoware log. A click that produces *no* log line
at all never reached mission planner (DDS hiccup) — just click again. Engage
via the OperationMode panel or:

```bash
ros2 service call /api/operation_mode/change_to_autonomous \
  autoware_adapi_v1_msgs/srv/ChangeOperationMode {}
```

RViz **must** render on the GPU: on llvmpipe (software GL) it burns 4+ cores
rendering the lidar cloud and destabilizes the whole pipeline. The script
passes the NVIDIA offload environment and runs `xhost +local:` for you, and
warns if it detects llvmpipe in the rviz log; if you see that warning, fix
`nvidia-container-toolkit` before trusting the session.

### What each mode runs

**`classical`** launches the validated entry point (in the container or the
source workspace):

```bash
ros2 launch autoware_launch e2e_simulator.launch.xml \
  vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit \
  perception_mode:=lidar rviz:=false \
  simulator_type:=carla launch_simulator_interface:=false \
  map_path:=/maps/Town10HD        # docker; source ws uses the host map dir
```

`simulator_type:=carla` is **required**: it selects the CARLA diagnostic
profile (`autoware-carla.yaml`), which drops the `routing/state` staleness
check that the default awsim profile flaps on whenever the sim runs below
real time (each flap pulses an MRM `EMERGENCY_STOP`).
`launch_simulator_interface:=false` because this branch's simulator publishes
the vehicle and sensor topics natively — there is no external bridge node.
(Both arguments need a current `autoware_launch`; an older workspace that
predates `simulator_type` should drop them and expect MRM flapping below
real time.)

RViz deliberately runs **outside** the stack launch (`rviz:=false`): with
`--with-rviz` the script starts it as a separate container (same DDS env,
`DISPLAY` passthrough, config
`/opt/autoware/autoware_launch/share/autoware_launch/rviz/autoware.rviz`) or
a local `rviz2` in source mode.

Autoware localizes with **NDT matching against `pointcloud_map.pcd`** and
therefore needs lidar, IMU and GNSS — all provided by `autoware_demo.py`:
an XYZIRCAEDT lidar on `/sensing/lidar/top/pointcloud_raw_ex` (frame
`velodyne_top`), IMU on `/sensing/imu/tamagawa/imu_raw`, GNSS pose on
`/sensing/gnss`, plus a traffic-light camera. The automation initializes
localization from GNSS for you; without `--goal`, set a goal in RViz and
engage via
`ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}`
(or the RViz AutowareStatePanel).

**`e2e`** runs camera-only driving with VAD:

- `run/spawn_vad_rig.py` (started by the run script) attaches **six 800×450
  cameras** to the ego and publishes them natively on
  `/sensing/camera/CAM_*/image_raw` (+ `/camera_info`): `CAM_FRONT` (FOV 70),
  `CAM_BACK` (FOV 110), `CAM_FRONT_LEFT`, `CAM_FRONT_RIGHT`, `CAM_BACK_LEFT`,
  `CAM_BACK_RIGHT` (FOV 70). (nuScenes-native would be 1600×900, but six raw
  RGBA streams at that size saturate DDS on one host; 800×450 keeps the 16:9
  aspect so the model-input resize stretch matches training.) The input
  **order is load-bearing** for the model: image0=FRONT, 1=BACK,
  2=FRONT_LEFT, 3=BACK_LEFT, 4=FRONT_RIGHT, 5=BACK_RIGHT. VAD consumes them
  compressed via `image_transport` republish nodes, which the run script
  starts — together with `camera_info` relays and `CAM_*` TF aliases (see
  `run/e2e_state_publishers.launch.py`; without them VAD silently never
  assembles a complete input frame, or segfaults on the first TF lookup with
  an unpatched `autoware_tensorrt_vad`).
- The e2e mode needs four small **workspace patches** on top of the PR #1685
  cherry-pick (launch gating, STOPPED-state departure, a vad_node segfault
  fix, and the 800×450 input size) — shipped in `install/patches/` with a
  README describing the validated sequence.
- **The e2e demo is hands-free**: the ego spawns on the Town10 outer ring
  (spawn 52 by default — VAD's navigation command is a *fixed* LANE_FOLLOW,
  there is no route input, so only looping roads sustain a demo; dead-end
  roads wedge the car at the road end), and the run script auto-engages
  autonomous mode once VAD reports initialized (`--no-auto` to disable).
- `run/e2e_drive_keeper.py` (started by the run script) attaches a
  **collision sensor** to the ego and logs every contact, a per-minute
  heartbeat (position, speed, odometer, collision count), wedge detection
  (<0.5 m displacement over 20 s), and — unless `--no-recover` — teleports
  the ego to a nearby aligned lane spawn so the loop continues. Watch
  `run/logs/drive_keeper.log`: it is the ground truth for driving quality;
  position/velocity alone cannot see contacts (the wheel-speed VelocityReport
  keeps reading 4–7 m/s while the car is pinned against an obstacle).
- The stack's RViz normally points its image panel at a traffic-light debug
  topic that does not exist in e2e mode (black panel); the run script
  generates `logs/vad_e2e.rviz` with the panel repointed to the front VAD
  camera and passes it via `rviz_config:=`.
- Localization is **ground truth**, not NDT: three plain ROS nodes from
  `autoware_carla_interface` (`carla_state_publisher`,
  `autoware_vehicle_velocity_converter`, `autoware_twist2accel`) turn
  `/sensing/gnss/pose_with_covariance` and `/vehicle/status/velocity_status`
  into `/localization/kinematic_state` and `/localization/acceleration`.
- VAD publishes `/planning/trajectory`, which the **classic trajectory
  follower** executes — so the control path is identical to classical mode.
- If the pinned PR #1685 branch is unavailable, the run script falls back to
  launching `e2e/autoware_tensorrt_vad/launch/vad_carla_tiny.launch.xml`
  directly with sensing/localization/perception disabled.
- The **first** e2e run builds TensorRT engines from the ONNX model. This is
  slow (tens of minutes on some GPUs) and looks like a hang — it isn't. The
  engines are cached; subsequent runs start fast.

---

## Compatibility matrix / DDS topology

The **validated topology** (Town10, full classical stack, 2026-08) is
deliberately **mixed-RMW** — DDS vendors interoperate over UDPv4:

- **Simulator: Fast DDS** (`-rmw=fastdds`, the script default) with a
  generated profile whose UDPv4 transport **whitelists only the docker bridge
  IP** (`interfaceWhiteList` + `useBuiltinTransports=false`).
- **Autoware (container or source ws): CycloneDDS** with a generated
  `cyclonedds.xml` **pinned to the docker bridge interface**
  (`NetworkInterface name="br-…"`), `MaxAutoParticipantIndex=300` (the stack
  is 63+ nodes; the CycloneDDS default index range is far too small),
  `MaxMessageSize=65500B`, and socket receive buffers of **10–64 MB** (the
  kernel `rmem_max`/`wmem_max` must be raised to 64 MB to allow this — see
  Prerequisites; at 4 MB the lidar stream overruns the sockets and
  reliable-writer retransmit stalls freeze the whole stack).

Why the bridge: DDS locators embed concrete IPs. A WiFi/DHCP interface whose
IP rotates **poisons the locators mid-session** — never bind DDS to one. The
docker bridge has a stable IP on both ends. `run_carla_autoware.sh`
auto-detects it (`docker network inspect`, falling back to `ip -br addr`) and
generates both XML files into `<log-dir>/dds/` each run; override with
`--bridge-if`/`--docker-network`.

Two hard-won warnings:

- The **official Autoware docker image ships its own `cyclonedds.xml`** that
  pins the `lo` interface, which breaks discovery with the simulator. It
  **must** be overridden via `CYCLONEDDS_URI`; the run script does this. (Its
  10 MB buffer demand, on the other hand, was right all along — see the kernel
  UDP buffer prerequisite.)
- **Do not run the simulator with `-rmw=cyclonedds` for now**: the CARLA
  CycloneDDS receive path has a known fragmented-receive bug (large samples
  can be dropped; a fix is in progress separately). The script defaults to
  `fastdds` on the sim side and warns if you override it.

| Simulator RMW | Status |
|---|---|
| **Fast DDS** (`-rmw=fastdds`, default) | **Validated.** UDPv4-only with the generated bridge-whitelist profile; containerized Autoware (CycloneDDS) discovers the simulator out of the box. |
| CycloneDDS (`-rmw=cyclonedds`) | **Not recommended (sim side)** until the fragmented-receive fix lands. |
| Zenoh (`-rmw=zenoh`) | Works with `--stack source`; the script starts the required router first (`ros2 run rmw_zenoh_cpp rmw_zenohd`) and runs zenoh end-to-end. |

Both Humble/22.04 and Jazzy/24.04 hosts work; Jazzy is the Autoware Docker
default OS. All processes must agree on `ROS_DOMAIN_ID` (server flag
`-ros-domain-id=N`; script default **42**, override with `--domain-id`).

---

## Known limitations

Read this before filing bugs — most of it is inherited and known.

- **`autoware_launch` PR #1685 is unmerged.** The launch-level glue for e2e
  planning (`use_e2e_planning` / `e2e_planning_type:=vad`) exists only as an
  open PR against `autowarefoundation/autoware_launch`. The installer pins
  that PR branch; if the pin ever breaks, e2e mode falls back to launching
  `vad_carla_tiny.launch.xml` manually. Expect this section to simplify once
  the PR merges.
- **Prebuilt maps lack traffic-light regulatory elements**, so classical-mode
  Autoware will not stop for red lights on them. Regenerate against a running
  simulator with `generate_map_artifacts.py` (step 2, option B) to inject
  traffic lights from ground truth. The prebuilt lanelet2 files are also
  y-axis-inverted by convention and have small pcd/osm origin offsets.
- **Prebuilt (UE4-era) pcd maps score low against UE5.8 lidar returns**, which
  is why the run script relaxes the NDT convergence threshold
  (`converged_param_nearest_voxel_transformation_likelihood` 2.3 → 1.0).
  Regenerating `pointcloud_map.pcd` with `map_tools` against the UE5.8 server
  removes the need for that override.
- **Cleanup discipline.** Never `pkill -f <pattern>` when the pattern also
  appears in a wrapping `docker exec bash -c` command line — it kills its own
  wrapper shell (exit 143) and leaves the container processes running. Use
  `stop_all.sh` (exact PIDs + `docker rm -f` of script-created containers);
  for ad-hoc host-side cleanup use bracketed patterns like
  `pgrep -f 'carla-rpc-port=200[0]'`.
- **VAD domain gap.** The published VAD weights were trained on
  Bench2Drive / CARLA 0.9.15-era imagery. Behavior on UE5.8 visuals is
  **unvalidated** — expect degraded performance in some scenes; treat e2e mode
  as a pipeline reference, not a driving-quality benchmark.
- **Control calibration.** Autoware's `sample_vehicle` gains and the original
  interface calibration were tuned for a Prius-class vehicle. Other ego
  blueprints will track more loosely until you re-tune the vehicle model /
  controller parameters.
- **`fixed_delta_seconds` coupling.** Sensor rates, the sync-mode tick, and
  Autoware's expectations are coupled through the simulator step
  (`autoware_demo.py` sets it). Changing it changes effective sensor
  frequencies; don't tune it independently of the sensor configuration.
- **First e2e run builds TensorRT engines** — slow but one-time (cached under
  `~/autoware_data/`).
- **Single ticking client.** In sync mode CARLA publishes only when its one
  designated client ticks. Extra ticking clients cause double-stepping or
  stalls; run everything through `run_carla_autoware.sh`.

---

## How this differs from `autoware_carla_interface`

The upstream `autoware_carla_interface` (Autoware Universe) is a Python ROS
node that connects to CARLA via the Python RPC API and re-publishes everything
into ROS2 — a bridge in the hot path, tied to `rclpy` and effectively to one
middleware configuration. In this integration the **simulator itself is the
DDS participant**: sensor data (including the XYZIRCAEDT lidar layout and the
six-camera rig) and vehicle status originate natively from the CARLA server,
and control commands flow straight back into it, with per-process RMW choice
across **Fast DDS, CycloneDDS, and Zenoh**. No serialization detour, lower and
more uniform latency, no Python GIL in the sensor path. We still reuse three
small stateless nodes *from* `autoware_carla_interface` in e2e mode (the
ground-truth localization converters), but nothing bridge-like sits between
sensors and the stack.
