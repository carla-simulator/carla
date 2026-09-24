"""Check the urban traffic crossing's telemetry, including a second execution.

Produces a behavior-only review diagram and report; does not certify rendering.
"""
import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from shapely.geometry import MultiPoint


def check(primary, replay, out):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    data = json.loads(primary.read_text())
    again = json.loads(replay.read_text())
    frames = data['frames']
    rows = [{a['role']: a for a in f['actors']} for f in frames]
    times = np.array([f['t'] for f in frames])
    xy = {role: np.array([[r[role]['x'], r[role]['y']] for r in rows])
          for role in rows[0]}
    speeds = {role: np.array([r[role]['speed'] for r in rows]) for role in xy}
    moving = np.flatnonzero(speeds['mary_proxy'] > 0.2)
    if not len(moving):
        raise ValueError('Pedestrian never crossed')
    start, end = int(moving[0]), int(moving[-1])
    dt = float(data['params']['timestep'])
    held = int(round(1.0 / dt))
    separation = {}
    for role in ('hero', 'lead', 'queue', 'standing'):
        separation[role] = min(
            MultiPoint(r[role]['footprint']).convex_hull.distance(
                MultiPoint(r['mary_proxy']['footprint']).convex_hull)
            for r in rows)
    second = [{a['role']: a for a in f['actors']} for f in again['frames']]
    same_count = len(rows) == len(second)
    max_delta = max((float(np.linalg.norm(
        np.array([a[role][k] for k in ('x', 'y', 'z')]) -
        np.array([b[role][k] for k in ('x', 'y', 'z')])))
        for a, b in zip(rows, second) for role in xy), default=float('inf'))
    yaw_delta = max((abs((a[role]['yaw'] - b[role]['yaw'] + 180) % 360 - 180)
                     for a, b in zip(rows, second) for role in xy), default=float('inf'))
    travel = float(np.linalg.norm(xy['hero'][-1] - xy['hero'][0]))
    cross_distance = float(np.linalg.norm(xy['mary_proxy'][-1] - xy['mary_proxy'][0]))
    checks = {
        'no_reported_collisions_both_runs': not data['collisions'] and not again['collisions'],
        'ego_approaches_queue': 8 < travel < 13,
        'ego_stopped_for_one_second_before_crossing': start >= held and bool(np.all(speeds['hero'][start-held:start] < 0.15)),
        'ego_stays_stopped_during_crossing': bool(np.all(speeds['hero'][start:end+1] < 0.15)),
        'pedestrian_completes_seven_metre_crossing': 6.9 < cross_distance < 7.2,
        'pedestrian_clears_every_vehicle_by_one_metre': min(separation.values()) > 1,
        'queue_remains_stationary': all(np.max(np.linalg.norm(xy[r] - xy[r][0], axis=1)) < 0.1 for r in ('lead', 'queue', 'standing')),
        'scenario_completes_before_timeout': times[-1] < 20 and '"terminate" executed' in data['termination'],
        'replay_within_one_centimetre_and_point_one_degree': same_count and max_delta < 0.01 and yaw_delta < 0.1,
    }
    report = {
        'scope': 'Scenic/CARLA behavior only; Mary and NuRec cinematic rendering pending',
        'passed': all(checks.values()), 'checks': checks,
        'frames': len(rows), 'fps': 1 / dt, 'duration_s': float(times[-1]),
        'crossing_start_s': float(times[start]), 'crossing_end_s': float(times[end]),
        'ego_travel_m': travel, 'pedestrian_travel_m': cross_distance,
        'minimum_pedestrian_vehicle_footprint_gap_m': separation,
        'replay_max_position_difference_m': max_delta,
        'replay_max_yaw_difference_deg': yaw_delta,
        'inputs': {str(p.resolve()): hashlib.sha256(p.read_bytes()).hexdigest() for p in (primary, replay)},
    }
    out.mkdir(parents=True, exist_ok=True)
    (out / 'validation.json').write_text(json.dumps(report, indent=2))
    # Coordinates relative to initial ego: lateral-right and forward distance.
    yaw = np.deg2rad(rows[0]['hero']['yaw'])
    axes = np.array([[-np.sin(yaw), np.cos(yaw)], [np.cos(yaw), np.sin(yaw)]])
    local = {r: (p - xy['hero'][0]) @ axes.T for r, p in xy.items()}
    fig, (plan, chart) = plt.subplots(1, 2, figsize=(12, 6), layout='constrained')
    colors = dict(hero='#246ab3', lead='#666666', queue='#ad8428', standing='#a84343', mary_proxy='#228548')
    for role, points in local.items():
        plan.plot(points[:, 0], points[:, 1], color=colors[role], label=role)
        plan.scatter(*points[0], color=colors[role], marker='o')
        plan.scatter(*points[-1], color=colors[role], marker='x')
    plan.set(xlabel='Lateral offset (m)', ylabel='Forward distance (m)', title='Measured actor paths (plan view)', xlim=(-6, 6), ylim=(-3, 58))
    plan.legend(loc='upper left'); plan.grid(alpha=.25)
    chart.plot(times, speeds['hero'] * 3.6, label='Ego (km/h)', color=colors['hero'])
    chart.plot(times, speeds['mary_proxy'] * 3.6, label='Pedestrian (km/h)', color=colors['mary_proxy'])
    chart.axvspan(times[start], times[end], color=colors['mary_proxy'], alpha=.1, label='Crossing')
    chart.set(xlabel='Time (s)', ylabel='Speed (km/h)', title='Approach → stop → pedestrian crossing')
    chart.legend(); chart.grid(alpha=.25)
    fig.suptitle('NuRec a2a4322c — Scenic behavior validation (not a cinematic render)')
    fig.savefig(out / 'behavior-review.png', dpi=160)
    plt.close(fig)
    print(json.dumps(report, indent=2))
    if not report['passed']:
        raise SystemExit('Crossing validation failed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('primary', type=Path)
    parser.add_argument('replay', type=Path)
    parser.add_argument('out', type=Path)
    args = parser.parse_args()
    check(args.primary, args.replay, args.out)
