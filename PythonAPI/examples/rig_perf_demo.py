#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Multi-camera rig throughput benchmark with a two-pass video recording.

Pass 1 (``measure``) drives an autopilot vehicle carrying a rig of N cameras
(``sensor.camera.rgb`` or ``sensor.camera.rt_lens``, with or without DLSS) in
synchronous mode, headless: no window, no image decoding on the client, so the
measured step time is the server's cost of simulating and rendering the rig.
It writes the per-tick server FPS and a CARLA recorder log of the drive.

Pass 2 (``render``) replays that recording with the same rig attached to the
same vehicle, waits for every camera on every tick (speed is irrelevant here),
composes the tiles into a grid and stamps each frame with the FPS measured for
that very tick in pass 1. The video therefore shows exactly what the rig saw
while the labels are the live numbers, unperturbed by the video encoding.

    # pass 1
    python rig_perf_demo.py measure --rig rgb --cameras 5 --dlss --seconds 30 --out runs/rgb5_dlss
    # pass 2
    python rig_perf_demo.py render --run runs/rgb5_dlss
"""

import argparse
import json
import os
import subprocess
import time

import numpy as np

import carla

CAMERA_WIDTH = 1920
CAMERA_HEIGHT = 1080
TILE_WIDTH = 640
TILE_HEIGHT = 360

# name, yaw (deg), tile column/row in the 3x2 grid. The first N entries make an
# N-camera rig; the 3x2 grid keeps the centre-bottom tile for the summary.
RIG = [
    ('Center-Front', 0.0, 1, 0),
    ('Left-Front', -55.0, 0, 0),
    ('Right-Front', 55.0, 2, 0),
    ('Rear-Left', -140.0, 0, 1),
    ('Rear-Right', 140.0, 2, 1),
]

HERO_ROLE = 'rig_perf_hero'


# ---------------------------------------------------------------------------
# rig
# ---------------------------------------------------------------------------

def camera_blueprint(world, cfg):
    bp = world.get_blueprint_library().find(cfg['sensor'])
    bp.set_attribute('image_size_x', str(cfg['width']))
    bp.set_attribute('image_size_y', str(cfg['height']))
    bp.set_attribute('fov', '90')
    if cfg['sensor'] == 'sensor.camera.rt_lens':
        bp.set_attribute('camera_model', 'perspective')
        bp.set_attribute('samples_per_pixel', str(cfg['spp']))
        bp.set_attribute('enable_denoiser', 'true' if cfg['denoiser'] else 'false')
    if cfg['dlss']:
        if not bp.has_attribute('enable_dlss'):
            raise RuntimeError('%s has no enable_dlss attribute' % cfg['sensor'])
        bp.set_attribute('enable_dlss', 'true')
        bp.set_attribute('dlss_screen_percentage', str(cfg['dlss_percentage']))
    return bp


def rig_transform(yaw):
    x = 0.8 if abs(yaw) < 90.0 else -0.8
    return carla.Transform(carla.Location(x=x, z=1.8), carla.Rotation(yaw=yaw))


class Sink(object):
    """Receives one camera's images. keep_images=False only counts arrivals."""

    def __init__(self, name, keep_images):
        self.name = name
        self.keep_images = keep_images
        self.frames = {}      # frame -> image (render) or arrival time (measure)
        self.received = 0

    def __call__(self, image):
        self.received += 1
        if self.keep_images:
            self.frames[image.frame] = image
        else:
            self.frames[image.frame] = time.perf_counter()


def spawn_rig(world, vehicle, cfg, keep_images):
    cameras, sinks = [], []
    for name, yaw, _, _ in RIG[:cfg['cameras']]:
        bp = camera_blueprint(world, cfg)
        sensor = world.spawn_actor(bp, rig_transform(yaw), attach_to=vehicle)
        sink = Sink(name, keep_images)
        sensor.listen(sink)
        cameras.append(sensor)
        sinks.append(sink)
    return cameras, sinks


def wait_for_frame(sinks, frame, timeout):
    """Block until every sink has the given frame (synchronous delivery)."""
    t0 = time.perf_counter()
    while any(frame not in s.frames for s in sinks):
        if time.perf_counter() - t0 > timeout:
            missing = [s.name for s in sinks if frame not in s.frames]
            return missing
        time.sleep(0.0005)
    return []


def mode_label(cfg):
    if cfg['sensor'] == 'sensor.camera.rt_lens':
        parts = ['path traced, %d spp' % cfg['spp'],
                 'DLSS Ray Reconstruction' if cfg['denoiser'] else 'no denoiser']
    else:
        parts = ['raster']
    if cfg['dlss']:
        parts.append('DLSS SR %d%% -> %dx%d' % (cfg['dlss_percentage'], cfg['width'], cfg['height']))
    else:
        parts.append('native %dx%d' % (cfg['width'], cfg['height']))
    return ', '.join(parts)


# ---------------------------------------------------------------------------
# pass 1: measure
# ---------------------------------------------------------------------------

def find_hero(world):
    for actor in world.get_actors().filter('vehicle.*'):
        if actor.attributes.get('role_name') == HERO_ROLE:
            return actor
    return None


def measure(args):
    cfg = dict(sensor='sensor.camera.rt_lens' if args.rig == 'rt_lens' else 'sensor.camera.rgb',
               cameras=args.cameras, width=args.width, height=args.height,
               dlss=args.dlss, dlss_percentage=args.dlss_percentage,
               spp=args.spp, denoiser=not args.no_denoiser,
               fixed_dt=args.fixed_dt, seconds=args.seconds, traffic=args.traffic,
               seed=args.seed, map=None)
    os.makedirs(args.out, exist_ok=True)
    recording = os.path.abspath(os.path.join(args.out, 'drive.log'))

    client = carla.Client(args.host, args.port)
    client.set_timeout(300.0)
    world = client.get_world()
    cfg['map'] = world.get_map().name
    tm = client.get_trafficmanager(args.tm_port)
    original = world.get_settings()

    vehicles, cameras = [], []
    try:
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = args.fixed_dt
        world.apply_settings(settings)
        tm.set_synchronous_mode(True)
        tm.set_random_device_seed(args.seed)

        library = world.get_blueprint_library()
        spawn_points = world.get_map().get_spawn_points()
        hero_bp = library.filter('vehicle.lincoln.mkz*')[0]
        hero_bp.set_attribute('role_name', HERO_ROLE)
        hero = None
        for spawn_point in spawn_points[args.spawn_index:] + spawn_points[:args.spawn_index]:
            hero = world.try_spawn_actor(hero_bp, spawn_point)
            if hero is not None:
                break
        if hero is None:
            raise RuntimeError('no free spawn point for the hero')
        vehicles.append(hero)
        hero.set_autopilot(True, tm.get_port())

        rng = np.random.RandomState(args.seed)
        car_bps = [bp for bp in library.filter('vehicle.*')
                   if int(bp.get_attribute('number_of_wheels')) == 4]
        for spawn_point in rng.permutation(len(spawn_points)):
            if len(vehicles) - 1 >= args.traffic:
                break
            bp = car_bps[rng.randint(len(car_bps))]
            bp.set_attribute('role_name', 'autopilot')
            other = world.try_spawn_actor(bp, spawn_points[spawn_point])
            if other is not None:
                other.set_autopilot(True, tm.get_port())
                vehicles.append(other)
        print('hero %d + %d traffic vehicles' % (hero.id, len(vehicles) - 1), flush=True)

        cameras, sinks = spawn_rig(world, hero, cfg, keep_images=False)
        print('rig: %d x %s (%s)' % (cfg['cameras'], cfg['sensor'], mode_label(cfg)), flush=True)

        # Warm-up: shader/pipeline compilation, DLSS feature creation, TM settling.
        for _ in range(args.warmup):
            frame = world.tick()
            wait_for_frame(sinks, frame, 120.0)
        for s in sinks:
            s.frames.clear()

        client.start_recorder(recording, True)
        ticks = int(round(args.seconds / args.fixed_dt))
        records = []
        t_start = time.perf_counter()
        for i in range(ticks):
            t0 = time.perf_counter()
            frame = world.tick()
            t1 = time.perf_counter()
            missing = wait_for_frame(sinks, frame, 60.0)
            t2 = time.perf_counter()
            loc = hero.get_transform().location
            records.append(dict(i=i, frame=frame, tick_ms=(t1 - t0) * 1e3, delivered_ms=(t2 - t0) * 1e3,
                                missing=missing, hero=[loc.x, loc.y, loc.z],
                                sim_time=world.get_snapshot().timestamp.elapsed_seconds))
            if i % 40 == 0:
                print('tick %4d/%d  server %.1f fps (tick %.1f ms, delivered %.1f ms)' % (
                    i, ticks, 1e3 / records[-1]['delivered_ms'], records[-1]['tick_ms'], records[-1]['delivered_ms']), flush=True)
        wall = time.perf_counter() - t_start
        client.stop_recorder()

        delivered = np.array([r['delivered_ms'] for r in records])
        tick_only = np.array([r['tick_ms'] for r in records])
        summary = dict(
            ticks=ticks, wall_seconds=wall,
            fps_mean=float(1e3 / delivered.mean()), fps_median=float(1e3 / np.median(delivered)),
            fps_p05=float(1e3 / np.percentile(delivered, 95)), fps_p95=float(1e3 / np.percentile(delivered, 5)),
            delivered_ms_mean=float(delivered.mean()), tick_ms_mean=float(tick_only.mean()),
            frames_per_camera=[s.received for s in sinks],
            dropped_frames=int(sum(len(r['missing']) for r in records)),
        )
        print('RESULT %s: %.1f fps mean, %.1f median, p5 %.1f, p95 %.1f (%d cameras, %d ticks, drops %d)' % (
            args.out, summary['fps_mean'], summary['fps_median'], summary['fps_p05'], summary['fps_p95'],
            cfg['cameras'], ticks, summary['dropped_frames']), flush=True)
        with open(os.path.join(args.out, 'fps.json'), 'w') as f:
            json.dump(dict(config=cfg, summary=summary, records=records, recording=recording,
                           mode=mode_label(cfg)), f, indent=1)
    finally:
        for sensor in cameras:
            sensor.stop()
            sensor.destroy()
        for vehicle in vehicles:
            vehicle.destroy()
        tm.set_synchronous_mode(False)
        world.apply_settings(original)


# ---------------------------------------------------------------------------
# pass 2: render
# ---------------------------------------------------------------------------

def to_rgb(image):
    array = np.frombuffer(image.raw_data, dtype=np.uint8).reshape((image.height, image.width, 4))
    return np.ascontiguousarray(array[:, :, 2::-1])


def resize(rgb, w, h):
    import cv2
    return cv2.resize(rgb, (w, h), interpolation=cv2.INTER_AREA)


def render(args):
    import cv2
    with open(os.path.join(args.run, 'fps.json')) as f:
        run = json.load(f)
    cfg, records = run['config'], run['records']
    recording = run['recording']
    n = cfg['cameras']
    grid_w, grid_h = 3 * TILE_WIDTH, 2 * TILE_HEIGHT
    playback_fps = args.fps if args.fps > 0 else 1.0 / cfg['fixed_dt']
    out_path = os.path.join(args.run, 'video.mp4')

    client = carla.Client(args.host, args.port)
    client.set_timeout(300.0)
    world = client.get_world()
    original = world.get_settings()
    cameras = []
    ffmpeg = None
    try:
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = cfg['fixed_dt']
        world.apply_settings(settings)

        print(client.replay_file(recording, 0.0, 0.0, 0, False).strip(), flush=True)
        # The replayer recreates the recorded actors on the next ticks.
        hero = None
        for _ in range(50):
            world.tick()
            hero = find_hero(world)
            if hero is not None:
                break
        if hero is None:
            raise RuntimeError('replayed hero (role_name %s) not found' % HERO_ROLE)
        cameras, sinks = spawn_rig(world, hero, cfg, keep_images=True)

        # Align the replay with the pass-1 records by the hero's position.
        target = np.array(records[0]['hero'])
        best = None
        for offset in range(args.max_align_ticks):
            loc = hero.get_transform().location
            d = float(np.linalg.norm(np.array([loc.x, loc.y, loc.z]) - target))
            if best is None or d < best[1]:
                best = (offset, d)
            if d < 0.05:
                break
            frame = world.tick()
            wait_for_frame(sinks, frame, 120.0)
            for s in sinks:
                s.frames.clear()
        print('alignment: %d replay ticks skipped, hero %.2f m from the pass-1 start' % best, flush=True)

        ffmpeg = subprocess.Popen(
            ['ffmpeg', '-y', '-loglevel', 'error', '-f', 'rawvideo', '-pix_fmt', 'rgb24',
             '-s', '%dx%d' % (grid_w, grid_h), '-r', '%.3f' % playback_fps, '-i', '-',
             '-c:v', 'libx264', '-preset', 'medium', '-crf', '18', '-pix_fmt', 'yuv420p', out_path],
            stdin=subprocess.PIPE)
        assert ffmpeg.stdin is not None
        font = cv2.FONT_HERSHEY_SIMPLEX
        fps_values = np.array([1e3 / r['delivered_ms'] for r in records])
        canvas = np.zeros((grid_h, grid_w, 3), np.uint8)
        for i in range(len(records)):
            frame = world.tick()
            missing = wait_for_frame(sinks, frame, 120.0)
            canvas[:] = 15
            for sink, (name, _, col, row) in zip(sinks, RIG[:n]):
                x0, y0 = col * TILE_WIDTH, row * TILE_HEIGHT
                image = sink.frames.pop(frame, None)
                if image is not None:
                    canvas[y0:y0 + TILE_HEIGHT, x0:x0 + TILE_WIDTH] = resize(to_rgb(image), TILE_WIDTH, TILE_HEIGHT)
                sink.frames.clear()
                cv2.putText(canvas, name, (x0 + 10, y0 + 26), font, 0.7, (255, 255, 255), 2, cv2.LINE_AA)
            # Summary tile (centre-bottom): live numbers from pass 1 for this tick.
            x0, y0 = TILE_WIDTH, TILE_HEIGHT
            fps_now = fps_values[i]
            fps_avg = fps_values[max(0, i - 19):i + 1].mean()
            lines = [
                '%d x %s' % (n, cfg['sensor']),
                run['mode'],
                'server: %5.1f fps' % fps_now,
                '1 s avg: %5.1f fps' % fps_avg,
                'run mean %.1f / median %.1f fps' % (run['summary']['fps_mean'], run['summary']['fps_median']),
                'per camera %dx%d, %d vehicles' % (cfg['width'], cfg['height'], cfg['traffic'] + 1),
                'measured live (pass 1), rendered offline (pass 2)',
            ]
            for k, line in enumerate(lines):
                scale = 0.8 if k in (2, 3) else 0.55
                cv2.putText(canvas, line, (x0 + 16, y0 + 40 + 40 * k), font, scale, (200, 220, 255), 2, cv2.LINE_AA)
            # FPS strip along the bottom of the summary tile.
            strip_y = y0 + TILE_HEIGHT - 12
            fmax = max(1.0, float(fps_values.max()))
            for k in range(min(i + 1, TILE_WIDTH - 32)):
                v = fps_values[i - k]
                h = int(48 * v / fmax)
                cv2.line(canvas, (x0 + TILE_WIDTH - 16 - k, strip_y), (x0 + TILE_WIDTH - 16 - k, strip_y - h), (120, 200, 120), 1)
            ffmpeg.stdin.write(canvas.tobytes())
            if i % 40 == 0:
                print('rendered %d/%d frames%s' % (i, len(records), ' (missing %s)' % missing if missing else ''), flush=True)
        ffmpeg.stdin.close()
        ffmpeg.wait()
        loc = hero.get_transform().location
        drift = float(np.linalg.norm(np.array([loc.x, loc.y, loc.z]) - np.array(records[-1]['hero'])))
        print('alignment at the end: hero %.2f m from the pass-1 end position' % drift, flush=True)
        print('VIDEO %s (%d frames at %.1f fps playback)' % (out_path, len(records), playback_fps), flush=True)
    finally:
        if ffmpeg is not None and ffmpeg.poll() is None:
            ffmpeg.stdin.close()
            ffmpeg.wait()
        for sensor in cameras:
            sensor.stop()
            sensor.destroy()
        client.stop_replayer(False)
        world.tick()
        # The replayer's actors outlive stop_replayer; they carry the recorded
        # role names, so they can be told apart from anything else in the world.
        leftovers = [a for a in world.get_actors().filter('vehicle.*')
                     if a.attributes.get('role_name') in (HERO_ROLE, 'autopilot')]
        for actor in leftovers:
            actor.destroy()
        world.apply_settings(original)


# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    for p in (sub.add_parser('measure', help='pass 1: drive, measure, record'),
              sub.add_parser('render', help='pass 2: replay and write the labelled video')):
        p.add_argument('--host', default='127.0.0.1')
        p.add_argument('-p', '--port', default=2000, type=int)
    m = sub.choices['measure']
    m.add_argument('--tm-port', default=8000, type=int)
    m.add_argument('--rig', choices=['rgb', 'rt_lens'], default='rgb')
    m.add_argument('--cameras', default=5, type=int, help='1..5 cameras of the rig')
    m.add_argument('--width', default=CAMERA_WIDTH, type=int)
    m.add_argument('--height', default=CAMERA_HEIGHT, type=int)
    m.add_argument('--dlss', action='store_true', help='DLSS Super Resolution (enable_dlss) on every camera')
    m.add_argument('--dlss-percentage', default=50.0, type=float)
    m.add_argument('--spp', default=4, type=int, help='rt_lens samples_per_pixel')
    m.add_argument('--no-denoiser', action='store_true', help='rt_lens without DLSS Ray Reconstruction')
    m.add_argument('--fixed-dt', default=0.05, type=float)
    m.add_argument('--seconds', default=30.0, type=float, help='measured simulated seconds')
    m.add_argument('--warmup', default=40, type=int, help='ticks before measuring')
    m.add_argument('--traffic', default=20, type=int, help='autopilot vehicles besides the hero')
    m.add_argument('--seed', default=7, type=int)
    m.add_argument('--spawn-index', default=0, type=int)
    m.add_argument('--out', required=True)
    r = sub.choices['render']
    r.add_argument('--run', required=True, help='pass-1 output directory')
    r.add_argument('--fps', default=0.0, type=float, help='video playback fps (default 1/fixed_dt)')
    r.add_argument('--max-align-ticks', default=40, type=int)
    args = parser.parse_args()
    if args.command == 'measure':
        if not 1 <= args.cameras <= len(RIG):
            parser.error('--cameras must be 1..%d' % len(RIG))
        measure(args)
    else:
        render(args)


if __name__ == '__main__':
    main()
