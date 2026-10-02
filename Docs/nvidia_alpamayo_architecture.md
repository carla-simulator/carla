# Alpamayo integration architecture and control

This page describes how the CARLA Alpamayo example connects synchronized
sensors, NVIDIA Alpamayo 2 Super inference, trajectory tracking, NuRec, and the
dashboard recorder. For installation and commands, start with the
[Alpamayo user guide](nvidia_alpamayo.md).

---

## Process architecture

The integration separates simulation and inference because the two sides have
different Python and dependency requirements.

| Component | Runtime | Responsibility |
|---|---|---|
| CARLA server | UE58 process | World state, physics, CARLA sensors, actors, and RPC. |
| Control client | CARLA-compatible Python | Sole synchronous tick owner, observation history, controller, dashboard, recording, and cleanup. |
| Alpamayo model service | Isolated Python 3.12 | Model loading, request validation, inference, numerical trajectory, and reasoning response. |
| NRE service, optional | Pinned NVIDIA container | Neural camera rendering for the selected NuRec artifact. |

The control client and model service communicate over a versioned local TCP
protocol. Messages carry JSON metadata and contiguous NumPy arrays; Python
pickles are not used. The model can remain resident while control clients
disconnect and reconnect sequentially.

The normal launcher starts both the control process and, unless
`--external-model-server` is given, a model subprocess. NuRec uses the same
control process rather than running `nurec_runner.py` beside it.

---

## Closed-loop timing

The control process is the only owner of `world.tick()`. A normal cycle is:

1. Advance CARLA at a fixed 0.1-second control interval.
2. Collect a synchronized camera set and 1.6 seconds of ego-motion history.
3. Pause simulation advancement while Alpamayo inference runs.
4. Receive 64 ego-frame waypoints sampled at 10 Hz over 6.4 seconds.
5. Execute the first ten 0.1-second control steps by default.
6. Capture a fresh observation and replan.

NuRec internally configures a 20 Hz physics world while neural observations
are due at 10 Hz. Its scenario tick owns the intervening physics ticks and
batches all due NRE cameras into one gRPC render request. No second process may
advance that same world.

Inference latency does not advance simulation. This preserves synchronized
observations and deterministic control timing even when a prediction takes
several wall-clock seconds.

---

## Camera topology

The canonical Alpamayo request contains seven PhysicalAI-AV camera slots:

| Index | Camera |
|---:|---|
| 0 | Front cross-left, 120° |
| 1 | Front wide, 120° |
| 2 | Front cross-right, 120° |
| 3 | Rear-left, 70° |
| 4 | Rear telephoto context, 30° |
| 5 | Rear-right, 70° |
| 6 | Front telephoto, 30° |

The trajectory profile consumes indices 0, 1, 2, 3, 5, and 6. Index 4 remains
in the canonical request because upstream input validation requires all seven
slots.

The dashboard adds an eighth CARLA-only third-person camera. This view makes a
balanced two-by-four sensor wall and is never sent to the model.

### CARLA camera backend

The default backend attaches seven calibrated sensors to the ego. The public
PhysicalAI-AV translations and full roll, pitch, and yaw are converted from the
source rear-axle frame into the MKZ actor frame. The two forward-center cameras
retain their calibrated horizontal pose but are raised just enough to clear
the actual vehicle bounding box.

The default `sensor.camera.rt_lens` cameras use fitted Kannala-Brandt
coefficients and preserve the source fisheye projection. DLSS Ray
Reconstruction denoises the path-traced model cameras; DLSS Super Resolution
is used independently by the dashboard chase camera. `--no-dlss` disables
both DLSS paths without changing camera calibration.

### NuRec camera backend

NuRec artifacts expose the six cameras consumed by the trajectory profile.
NRE renders those virtual sensors over gRPC; they are not CARLA camera actors.
The canonical rear-tele context slot is filled with an explicitly identified,
synchronized duplicate of the rear-left neural view because that context slot
is discarded by the trajectory profile. The eighth chase camera remains a
normal CARLA RGB actor.

---

## Coordinate conversion

Alpamayo uses a right-handed ego frame with `x` forward and `y` left. CARLA
uses a left-handed frame. The bridge reflects the lateral coordinate:

```text
x_carla =  x_alpamayo
y_carla = -y_alpamayo
z_carla =  z_alpamayo
```

The same reflection is applied consistently to ego-motion history, predicted
waypoints, sensor extrinsics, and NuRec-to-CARLA poses. Trajectories recorded in
telemetry include both the raw Alpamayo representation and the converted CARLA
local representation for auditing.

---

## Trajectory-to-control conversion

Steering and longitudinal control intentionally use different trajectory
indices.

### Lateral control

A pure-pursuit tracker finds the nearest trajectory point and then selects the
first point at least `--lookahead` metres away. It transforms that point into
the current vehicle frame, computes curvature, maps the resulting wheel angle
to CARLA's normalized steering command, applies `--steering-gain`, and limits
changes with `--max-steer-rate`.

The telemetry fields `steering_target_index`, `requested_steer`, and
`applied_steer` expose the spatial target and rate limiting.

### Longitudinal control

Target speed is derived from consecutive waypoint displacement over the
matching 0.1-second trajectory interval. Control step `i` uses trajectory
speed index `i`; it does not reuse the spatial pure-pursuit index. This keeps
speed aligned with prediction time when the steering look-ahead jumps several
points forward.

A stateful PID converts target-speed error into one signed effort:

* Positive effort becomes throttle.
* Negative effort becomes brake.
* Conditional integration prevents windup at saturated pedals.
* Derivative-on-measurement prevents a derivative kick when a new trajectory
  changes target speed at a replan boundary.

The default gains are `Kp=0.35`, `Ki=0.04`, and `Kd=0.02`. A target at or
below 0.25 m/s is treated as a stop. The controller applies at least 0.25 brake
while moving, then full hold brake below 0.15 m/s. All thresholds and gains are
available as command-line arguments.

Reasoning text is not parsed into throttle, brake, or steering. Only the
numerical trajectory controls the vehicle.

---

## NuRec world and actor orchestration

Entering a NuRec scenario extracts `map.xodr` from the `.usdz` and calls
`generate_opendrive_world()`. The resulting world is distinct from Town10 or
any other named CARLA map. The scenario creates a physics-controlled ego and
proxy actors for recorded traffic.

Rigid replay becomes unsafe after a policy-controlled ego deviates from its
recorded position. The integration therefore provides three actor policies:

* `replay` follows dataset poses exactly.
* `carla` hands usable vehicles to Traffic Manager with collision avoidance
  and a speed-dependent following gap. Same-lane rear traffic remains held
  until the ego opens a safe gap. Route-less proxies remain stationary.
* `disabled` keeps non-ego proxies collision-free without Traffic Manager.

Tracks that cannot accept CARLA pose overrides remain neural-replay-only, and
their CARLA proxies do not collide. The ego is seeded with the recorded
starting velocity using a backward pose difference where possible and a
one-sided forward difference at the first recording timestamp.

---

## Dashboard recording

The dashboard displays two rows of four sensor views, a reshaped trajectory
plot, one unified inference/reasoning panel, and control telemetry.

The recorder serializes simulation state rather than display redraws. At the
default settings, the simulation produces observations at 10 Hz and the video
clock writes at 20 FPS, so each state contributes two identical video frames.
Redraws performed while inference has simulation paused contribute no frames.
The resulting video therefore plays at simulated real-time speed without long
frozen inference gaps.

The writer finalizes on normal completion, ESC/Q, or Ctrl-C. Abrupt process
termination can leave a partial container that requires repair or re-encoding.

---

## Telemetry contract

The JSON Lines sidecar contains one object per event. Important event types are:

| Event | Important fields |
|---|---|
| `prediction` | Cycle, CARLA frame, inference duration, reasoning, raw trajectory, converted trajectory. |
| `control` | Control step, spatial and temporal indices, actual and target speeds, PID effort, throttle, brake, steering, and hold state. |
| `collision` | Phase, cycle, frame, other actor identity, and collision impulse. |

For a time-aligned control run, every `control` event should satisfy:

```text
speed_target_index == control_step
```

The steering index is expected to differ because it is selected spatially.
Use the telemetry to determine whether unexpected behavior originated in the
model trajectory, coordinate conversion, controller, actuator limiting, or a
physical collision.

---

## Lifecycle and cleanup

On exit, the control process:

1. Applies brake and hand brake to the ego.
2. Destroys its own sensors.
3. Destroys an ego only if it spawned that vehicle.
4. Optionally restores autopilot on an attached ego.
5. Restores original world and Traffic Manager synchronization settings.
6. Closes model, display, telemetry, and video resources.

Externally created traffic is preserved. NuRec containers remain warm by
default so later runs can reuse the loaded scene; reuse checks the service,
scene identifier, render settings, and GPU selection before adoption.

---

## Extension points

The integration is split into small modules so research changes do not require
rewriting the orchestration:

* `alpamayo_rig.py`: camera calibration and dashboard ordering.
* `alpamayo_schema.py`: canonical request validation and tensor conversion.
* `alpamayo_protocol.py`: process transport.
* `alpamayo_model_server.py`: model adapter.
* `alpamayo_controller.py`: trajectory tracking and PID control.
* `alpamayo_nurec.py`: NuRec lifecycle, virtual cameras, and actor policy.
* `alpamayo_display.py`: dashboard and video clock.
* `alpamayo_control.py`: common orchestration and command-line interface.

Keep a single tick owner when adding scenario managers or additional sensors.
Long-lived clients should consume snapshots with `wait_for_tick()` instead of
calling `world.tick()`.
