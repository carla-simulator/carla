#!/usr/bin/env python3
"""Preview camera rain on a dedicated CARLA server; replaces its current map.

python rain_lens_demo.py --port 4654
Requires pygame and numpy. 1-4 select rain; N switches day/night;
Space toggles automatic cycling; Esc closes and restores world settings.
"""
import argparse
import queue
import time
from pathlib import Path

import carla
import numpy as np
import pygame


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, required=True)
    parser.add_argument("--map", default="Town10HD_Opt")
    parser.add_argument("--capture-dir", type=Path)
    args = parser.parse_args()
    if args.capture_dir:
        args.capture_dir.mkdir(parents=True, exist_ok=True)
    client = carla.Client(args.host, args.port)
    client.set_timeout(120)
    world = client.load_world(args.map)
    original = world.get_settings()
    old_weather = world.get_weather()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    camera = None
    pygame.init()
    try:
        world.apply_settings(settings)
        display = pygame.display.set_mode((1280, 768))
        pygame.display.set_caption("CARLA — Cinematic rain preview")
        font = pygame.font.Font(None, 24)
        clock = pygame.time.Clock()
        bp = world.get_blueprint_library().find("sensor.camera.rgb")
        bp.set_attribute("image_size_x", "1280")
        bp.set_attribute("image_size_y", "720")
        bp.set_attribute("fov", "90")
        transform = world.get_map().get_spawn_points()[0]
        transform.location.z += 1.5
        camera = world.spawn_actor(bp, transform)
        frames = queue.Queue()
        camera.listen(frames.put)
        world.get_spectator().set_transform(transform)
        levels = (0, 20, 85, 100)
        level, night, automatic = 3, False, False
        dirty, running = True, True
        last_switch = time.monotonic()
        saved = set()
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    elif event.key in (pygame.K_1, pygame.K_2, pygame.K_3, pygame.K_4):
                        level = event.key - pygame.K_1
                        automatic, dirty = False, True
                    elif event.key == pygame.K_n:
                        night, dirty = not night, True
                    elif event.key == pygame.K_SPACE:
                        automatic = not automatic
                        last_switch = time.monotonic()
            if automatic and time.monotonic() - last_switch > 12:
                level, dirty = (level + 1) % len(levels), True
            if dirty:
                world.set_weather(carla.WeatherParameters(
                    cloudiness=95, precipitation=levels[level],
                    precipitation_deposits=70, wetness=85, wind_intensity=35,
                    sun_altitude_angle=-10 if night else 35, fog_density=7))
                dirty = False
                last_switch = time.monotonic()
                print(f"Rain {levels[level]}%, {'night' if night else 'day'}", flush=True)
            world.tick()
            image = None
            while not frames.empty():
                image = frames.get_nowait()
            if image is not None:
                rgb = np.frombuffer(image.raw_data, dtype=np.uint8).reshape(720, 1280, 4)
                surface = pygame.surfarray.make_surface(rgb[:, :, :3][:, :, ::-1].swapaxes(0, 1))
                display.blit(surface, (0, 48))
                tag = f"rain_{levels[level]}_{'night' if night else 'day'}"
                if args.capture_dir and tag not in saved and time.monotonic() - last_switch > 7:
                    image.save_to_disk(str(args.capture_dir / (tag + ".png")))
                    saved.add(tag)
            display.fill((18, 23, 30), (0, 0, 1280, 48))
            label = (f"CINEMATIC RAIN  {levels[level]}%  |  {'NIGHT' if night else 'DAY'}  |  "
                     f"1 Dry   2 Light   3 Heavy   4 Storm   N Day/Night   "
                     f"Space {'Stop' if automatic else 'Start'} cycle   Esc Close")
            display.blit(font.render(label, True, (225, 235, 245)), (18, 15))
            pygame.display.flip()
            clock.tick(20)
    finally:
        if camera is not None:
            camera.stop()
            camera.destroy()
        world.set_weather(old_weather)
        world.apply_settings(original)
        pygame.quit()


if __name__ == "__main__":
    main()
