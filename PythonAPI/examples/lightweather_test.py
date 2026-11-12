#!/usr/bin/env python3
"""Light + weather API functional test against a running CARLA UE5 server."""
import argparse
import time

import numpy as np
from PIL import Image
import carla

OUT = "."


def snap(world, tag, loc, rot):
    bp = world.get_blueprint_library().find("sensor.camera.rgb")
    bp.set_attribute("image_size_x", "960")
    bp.set_attribute("image_size_y", "540")
    cam = world.spawn_actor(bp, carla.Transform(loc, rot))
    holder = []
    cam.listen(lambda img: holder.append(img) if not holder else None)
    deadline = time.time() + 10
    while not holder and time.time() < deadline:
        time.sleep(0.1)
    cam.stop()
    cam.destroy()
    assert holder, f"no frame for {tag}"
    img = holder[0]
    arr = np.frombuffer(img.raw_data, dtype=np.uint8).reshape(img.height, img.width, 4)
    Image.fromarray(arr[:, :, [2, 1, 0]]).save(f"{OUT}/lw_{tag}.png")
    print("saved", f"lw_{tag}.png")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("-p", "--port", type=int, default=2000)
    args = ap.parse_args()
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()
    original = world.get_weather()
    lm = world.get_lightmanager()

    lights = lm.get_all_lights(carla.LightGroup.Street)
    print("street lights:", len(lights))

    if lights:
        # camera pose near first street light (map may have real-world elevation)
        ref = lights[0].location
        loc = carla.Location(x=ref.x + 15.0, y=ref.y, z=ref.z + 2.0)
        rot = carla.Rotation(pitch=-8, yaw=180)
    else:
        # some maps (e.g. Town11) register no street lights: still test the
        # weather, from the first spawn point, and skip the light checks
        print("no street lights on this map: testing the weather only")
        sp = world.get_map().get_spawn_points()[0]
        loc = carla.Location(x=sp.location.x, y=sp.location.y, z=sp.location.z + 2.0)
        rot = carla.Rotation(pitch=-8, yaw=sp.rotation.yaw)

    # World Partition maps only stream in the cells around the spectator and
    # the vehicles: move the spectator to the camera so the scene is loaded
    world.get_spectator().set_transform(carla.Transform(loc, rot))
    time.sleep(5.0)

    try:
        # --- day ---
        world.set_weather(carla.WeatherParameters.ClearNoon)
        time.sleep(2.0)
        on_day = sum(1 for l in lm.get_all_lights(carla.LightGroup.Street) if l.is_on)
        print("day: lights on =", on_day)
        snap(world, "day", loc, rot)

        # --- night ---
        world.set_weather(carla.WeatherParameters.ClearNight)
        time.sleep(3.0)
        on_night = sum(1 for l in lm.get_all_lights(carla.LightGroup.Street) if l.is_on)
        print("night: lights on =", on_night)
        snap(world, "night", loc, rot)

        if not lights:
            print("PASS (weather only, light checks skipped)")
            return

        # --- manual toggle: switch all street lights off at night ---
        lm.turn_off(lights)
        time.sleep(2.0)
        on_off = sum(1 for l in lm.get_all_lights(carla.LightGroup.Street) if l.is_on)
        print("after turn_off: lights on =", on_off)
        snap(world, "night_lights_off", loc, rot)

        # --- back on ---
        lm.turn_on(lights)
        time.sleep(2.0)
        on_on = sum(1 for l in lm.get_all_lights(carla.LightGroup.Street) if l.is_on)
        print("after turn_on: lights on =", on_on)

        # verdicts
        assert on_night > on_day, "day/night cycle did not turn lights on at night"
        assert on_off < on_night, "turn_off had no effect"
        assert on_on >= on_night, "turn_on had no effect"
        print("PASS")
    finally:
        world.set_weather(original)
        print("weather restored")


if __name__ == "__main__":
    main()
