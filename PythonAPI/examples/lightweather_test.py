#!/usr/bin/env python3
"""Light + weather API functional test against a running CARLA UE5 server."""
import time

import numpy as np
from PIL import Image
import carla

OUT = "/home/german/Projects/CARLA_SOURCE"


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
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
    client = carla.Client("localhost", port)
    client.set_timeout(10.0)
    world = client.get_world()
    original = world.get_weather()
    lm = world.get_lightmanager()

    lights = lm.get_all_lights(carla.LightGroup.Street)
    print("street lights:", len(lights))

    # camera pose near first street light (map may have real-world elevation)
    ref = lights[0].location
    loc = carla.Location(x=ref.x + 15.0, y=ref.y, z=ref.z + 2.0)
    rot = carla.Rotation(pitch=-8, yaw=180)

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
