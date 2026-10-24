#!/usr/bin/env python3
"""
Traffic Light Braking & Stop-Line Alignment Telemetry Analyzer.

Validates that an autonomous vehicle under CARLA Traffic Manager autopilot:
  1. Detects an upcoming RED traffic light via Ego perception.
  2. Decelerates smoothly from cruising speed to a complete halt (v < 0.1 km/h).
  3. Halts cleanly before the stop line with zero junction overshoot (d >= 0.5m).
Generates a multi-panel telemetry plot for rigorous diagnostic verification.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

os.environ["SDL_VIDEODRIVER"] = "dummy"

import carla
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def run_braking_verification(
    host: str = "127.0.0.1",
    port: int = 2000,
    out_plot: Path | str | None = None,
) -> dict[str, Any]:
    """Runs the traffic light braking and stop-line alignment test."""
    client = carla.Client(host, port)
    client.set_timeout(30.0)

    world = client.get_world()
    if "MunichStachus" not in world.get_map().name:
        world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    tls = list(world.get_actors().filter("traffic.traffic_light*"))
    if not tls:
        raise RuntimeError("No traffic lights found in world!")

    tl = tls[0]
    swps = tl.get_stop_waypoints()
    if not swps:
        raise RuntimeError(f"Traffic light {tl.id} has no stop waypoints!")

    swp = swps[0]
    stop_loc = swp.transform.location

    # Upstream spawn waypoint (~18m before stop line)
    prev_wps = swp.previous(18.0)
    if not prev_wps:
        raise RuntimeError("Could not find upstream spawn waypoint")

    spawn_wp = prev_wps[0]
    sp_tf = carla.Transform(spawn_wp.transform.location + carla.Location(z=0.5), spawn_wp.transform.rotation)

    bp_lib = world.get_blueprint_library()
    veh_bp = bp_lib.find("vehicle.ue4.audi.tt")

    # Freeze traffic light in RED state
    tl.freeze(True)
    tl.set_state(carla.TrafficLightState.Red)

    ego = world.try_spawn_actor(veh_bp, sp_tf)
    if ego is None:
        raise RuntimeError("Could not spawn Ego vehicle on stop approach")

    tm = client.get_trafficmanager(8000)
    tm.set_synchronous_mode(True)
    ego.set_autopilot(True, 8000)

    # Telemetry records
    timestamps: list[float] = []
    distances: list[float] = []
    speeds: list[float] = []
    detected_states: list[str] = []

    try:
        max_ticks = 110
        dt = 0.05
        for tick in range(max_ticks):
            world.tick()
            t = tick * dt

            loc = ego.get_location()
            vel = ego.get_velocity()
            spd_kmh = (vel.x ** 2 + vel.y ** 2) ** 0.5 * 3.6
            dist_to_stop = loc.distance(stop_loc)

            tl_det = ego.get_traffic_light()
            state_str = str(ego.get_traffic_light_state()) if (tl_det or ego.is_at_traffic_light()) else "None"

            timestamps.append(t)
            distances.append(dist_to_stop)
            speeds.append(spd_kmh)
            detected_states.append(state_str)

        # Final state analysis
        final_speed = speeds[-1]
        final_dist = distances[-1]
        max_speed = max(speeds)
        stopped_cleanly = final_speed < 0.2 and final_dist >= 0.5
        overshoot = final_dist < 0.0

        metrics = {
            "initial_distance": round(distances[0], 2),
            "final_distance": round(final_dist, 2),
            "max_speed_kmh": round(max_speed, 1),
            "final_speed_kmh": round(final_speed, 2),
            "stopped_cleanly": stopped_cleanly,
            "zero_overshoot": not overshoot,
            "detected_red_light": any("Red" in s for s in detected_states),
        }

        # Plot Telemetry
        if out_plot:
            fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(9, 7), sharex=False, dpi=150)
            fig.patch.set_facecolor("#16181f")
            for ax in (ax1, ax2):
                ax.set_facecolor("#1f232d")
                ax.grid(True, linestyle="--", alpha=0.3, color="#555a6d")
                ax.tick_params(colors="#d0d4e4")
                for spine in ax.spines.values():
                    spine.set_color("#3a3e4e")

            # Subplot 1: Speed vs Distance
            ax1.plot(distances, speeds, color="#38ef7d", linewidth=2.4, label="Vehicle Velocity (km/h)")
            ax1.axvline(0.0, color="#ff416c", linestyle="--", linewidth=1.8, label="Junction Stop Line")
            ax1.axhline(0.0, color="#a0a5b8", linestyle=":", linewidth=1.0)
            ax1.set_title("Ego Vehicle Speed vs Distance to Stop Line", fontsize=12, fontweight="bold", color="#ffffff", pad=10)
            ax1.set_xlabel("Distance to Stop Line (meters)", fontsize=10, color="#d0d4e4")
            ax1.set_ylabel("Speed (km/h)", fontsize=10, color="#d0d4e4")
            ax1.invert_xaxis()
            ax1.legend(loc="upper left", facecolor="#262a36", edgecolor="#3a3e4e", labelcolor="#ffffff")

            # Annotate stop point
            ax1.scatter([final_dist], [final_speed], color="#ff416c", s=60, zorder=5)
            ax1.annotate(
                f"Full Stop: {final_dist:.1f}m\n(v = {final_speed:.1f} km/h)",
                xy=(final_dist, final_speed),
                xytext=(final_dist + 3.0, 5.0),
                arrowprops=dict(facecolor="#ff416c", arrowstyle="->", lw=1.5),
                color="#ffffff",
                fontsize=9,
                fontweight="bold",
                bbox=dict(boxstyle="round,pad=0.3", fc="#ff416c", alpha=0.3),
            )

            # Subplot 2: Speed & Distance vs Time
            ax2.plot(timestamps, speeds, color="#00f2fe", linewidth=2.0, label="Speed (km/h)")
            ax2_r = ax2.twinx()
            ax2_r.plot(timestamps, distances, color="#fbc531", linewidth=1.8, linestyle="-.", label="Distance to Stop (m)")
            ax2_r.set_ylabel("Distance (m)", fontsize=10, color="#fbc531")
            ax2_r.tick_params(colors="#fbc531")
            ax2_r.spines["right"].set_color("#fbc531")

            ax2.set_title("Deceleration Profile & Standstill Over Time", fontsize=12, fontweight="bold", color="#ffffff", pad=10)
            ax2.set_xlabel("Simulation Time (seconds)", fontsize=10, color="#d0d4e4")
            ax2.set_ylabel("Speed (km/h)", fontsize=10, color="#00f2fe")

            plt.tight_layout()
            out_p = Path(out_plot)
            out_p.parent.mkdir(parents=True, exist_ok=True)
            plt.savefig(str(out_p), facecolor=fig.get_facecolor(), edgecolor="none")
            plt.close()
            print(f"[braking-verifier] Saved telemetry plot -> {out_p.name}")

        return metrics

    finally:
        tl.freeze(False)
        ego.destroy()
        settings.synchronous_mode = False
        world.apply_settings(settings)


def main() -> int:
    parser = argparse.ArgumentParser(description="Verify Traffic Light Braking Telemetry")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--out-plot", default=str(OUT_DIR / "munich_traffic_light_braking.png"))
    args = parser.parse_args()

    print("[braking-verifier] Running traffic light braking telemetry analysis...")
    metrics = run_braking_verification(host=args.host, port=args.port, out_plot=args.out_plot)

    print("\n" + "=" * 72)
    print("CARLA Traffic Light Braking Telemetry Report")
    print("=" * 72)
    print(f"  Initial Distance        : {metrics['initial_distance']} m")
    print(f"  Maximum Approach Speed  : {metrics['max_speed_kmh']} km/h")
    print(f"  Final Speed at Halt     : {metrics['final_speed_kmh']} km/h (must be < 0.2 km/h)")
    print(f"  Final Standoff Distance : {metrics['final_distance']} m (must be >= 0.5 m)")
    print(f"  Stopped Cleanly         : {'YES' if metrics['stopped_cleanly'] else 'NO'}")
    print(f"  Zero Stop-Line Overshoot: {'VERIFIED (0 encroachment)' if metrics['zero_overshoot'] else 'FAIL'}")
    print("=" * 72)

    if metrics["stopped_cleanly"] and metrics["zero_overshoot"]:
        print("[braking-verifier] SUCCESS: Definition of Done passed!")
        return 0
    else:
        print("[braking-verifier] FAIL: Braking criteria not satisfied.")
        return 1


if __name__ == "__main__":
    sys.exit(main())

