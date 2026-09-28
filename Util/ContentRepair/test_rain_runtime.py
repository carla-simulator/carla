"""Capture rain transitions and late sensor creation on a running CARLA server.

Example: python test_rain_runtime.py --port 4654 --town Town03_Opt --load
This changes weather and spectator pose; --load also replaces the world.
Inspect the saved images for falling streaks and lens droplets. API assertions
check state and image delivery, not visual quality.
"""
import argparse
import json
from pathlib import Path
import queue
import time

import carla


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=2000)
    parser.add_argument('--town', default='Town10HD_Opt')
    parser.add_argument('--load', action='store_true')
    parser.add_argument('--output', type=Path, default=Path('rain-runtime-results'))
    args = parser.parse_args()
    client = carla.Client('localhost', args.port)
    client.set_timeout(180)
    world = client.load_world(args.town) if args.load else client.get_world()
    if world.get_settings().synchronous_mode:
        raise RuntimeError('Use an asynchronous test server.')
    output = args.output / world.get_map().name.rsplit('/', 1)[-1]
    output.mkdir(parents=True, exist_ok=True)
    spectator = world.get_spectator()
    old_pose, old_weather = spectator.get_transform(), world.get_weather()
    poses = world.get_map().get_spawn_points()
    near = poses[0]
    far = next(p for p in poses[1:] if p.location.distance(near.location) > 80)
    near.location.z += 1.5
    far.location.z += 1.5
    blueprint = world.get_blueprint_library().find('sensor.camera.rgb')
    blueprint.set_attribute('image_size_x', '1280')
    blueprint.set_attribute('image_size_y', '720')
    cameras = []
    queues = []

    def spawn(pose):
        camera = world.spawn_actor(blueprint, pose)
        cameras.append(camera)
        frames = queue.Queue(maxsize=2)
        queues.append(frames)

        def receive(image):
            try:
                frames.put_nowait(image)
            except queue.Full:
                pass
        camera.listen(receive)

    def capture(label):
        time.sleep(3)
        for frames in queues:
            while not frames.empty():
                frames.get_nowait()
        for index, frames in enumerate(queues):
            image = frames.get(timeout=45)
            assert (image.width, image.height) == (1280, 720)
            image.save_to_disk(str(output / f'{label}-camera{index}.png'))
        print(f'{label}: captured {len(cameras)} cameras', flush=True)

    try:
        spectator.set_transform(near)
        spawn(near)
        spawn(far)
        weather = carla.WeatherParameters(
            cloudiness=95, precipitation_deposits=70, wetness=85,
            wind_intensity=0, sun_altitude_angle=35, fog_density=7)
        for index, rain in enumerate((0, 20, 85, 0)):
            weather.precipitation = rain
            world.set_weather(weather)
            assert abs(world.get_weather().precipitation - rain) < 1e-5
            capture(f'{index:02d}-{rain}')
        weather.precipitation = 85
        world.set_weather(weather)
        spawn(near)  # Same position: must not multiply another view's rain.
        capture('late')
        (output / 'result.json').write_text(json.dumps({
            'map': world.get_map().name,
            'transitions': [0, 20, 85, 0],
            'late_camera_during_rain': True,
            'visual_review_required': True,
        }, indent=2))
    finally:
        for camera in cameras:
            camera.stop()
            camera.destroy()
        world.set_weather(old_weather)
        spectator.set_transform(old_pose)


if __name__ == '__main__':
    main()
