#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Runs two independent, asynchronous weather cycles against a live CARLA server:

  - A day/night cycle: sun_altitude_angle/sun_azimuth_angle sweep smoothly
    over a configurable day length.
  - A storm cycle: cloudiness, precipitation, wetness, wind_intensity,
    precipitation_deposits and fog build up and recede in stages, timed so
    each effect kicks in once the previous one is well underway (see
    STORM_KEYFRAMES below), then settles back to clear.

Both run on independent periods and phase-shift relative to each other over
time (a storm can start at any point in the day/night cycle), driven by
world.on_tick so nothing here blocks the caller -- run() returns a handle
that can be stopped with .stop().

    python3 dynamic_day_night_weather_cycle.py --day-length 180 --storm-length 240

Ctrl+C stops the cycle and leaves the last computed weather in place.
"""

import argparse
import math
import shutil
import sys
import time

import carla


# --- Storm cycle -------------------------------------------------------
#
# Each parameter is its own list of (time_fraction, value) keyframes over
# one storm cycle (0.0 -> 1.0, then wraps to a fresh clear start). Values
# between keyframes are linearly interpolated; before the first keyframe or
# after the last, the nearest endpoint value holds. Keeping every parameter
# on its own timeline (rather than one shared if/elif ladder) is what lets
# each effect visibly lag the one that triggers it, matching how real
# weather builds: clouds first, then rain once clouds are dense, then wind
# and road wetness as rain picks up, then puddles once the road is properly
# wet, with a light haze/fog only at the height of the storm.
STORM_KEYFRAMES = {
    "cloudiness": [
        (0.00, 0), (0.15, 90), (0.20, 100), (0.75, 100), (0.85, 85), (1.00, 0),
    ],
    "precipitation": [
        (0.00, 0), (0.19, 0), (0.20, 5), (0.35, 45), (0.55, 100),
        (0.72, 100), (0.80, 40), (0.90, 0), (1.00, 0),
    ],
    "wind_intensity": [
        (0.00, 0), (0.30, 0), (0.35, 20), (0.55, 55), (0.72, 100),
        (0.85, 60), (0.95, 10), (1.00, 0),
    ],
    # Ramps fast as soon as rain starts (t=0.20) -- the ground gets visibly
    # wet well before puddles have any reason to form -- then keeps climbing
    # more gently toward the peak. Reaches 60 by t=0.30, a full 0.14 ahead of
    # precipitation_deposits' start at 0.44.
    "wetness": [
        (0.00, 0), (0.20, 0), (0.22, 10), (0.30, 60), (0.38, 80),
        (0.55, 95), (0.72, 100), (0.85, 70), (0.95, 15), (1.00, 0),
    ],
    "precipitation_deposits": [
        (0.00, 0), (0.44, 0), (0.45, 10), (0.60, 55), (0.72, 100),
        (0.88, 60), (0.97, 10), (1.00, 0),
    ],
    "fog_density": [
        (0.00, 0), (0.44, 0), (0.50, 8), (0.65, 20), (0.72, 30),
        (0.85, 15), (0.95, 0), (1.00, 0),
    ],
    "fog_distance": [
        (0.00, 100), (0.50, 100), (0.65, 60), (0.72, 35),
        (0.85, 70), (1.00, 100),
    ],
    "fog_falloff": [
        (0.00, 1.0), (0.50, 1.0), (0.72, 2.0), (1.00, 1.0),
    ],
    "scattering_intensity": [
        (0.00, 1.0), (0.60, 1.0), (0.72, 2.5), (0.90, 1.0), (1.00, 1.0),
    ],
}


# Every field WeatherParameters exposes -- used both for the warm-up blend
# (lerp every field, not just the ones the storm cycle drives) and the
# live status line.
WEATHER_FIELDS = [
    'cloudiness', 'precipitation', 'precipitation_deposits', 'wind_intensity',
    'sun_azimuth_angle', 'sun_altitude_angle', 'fog_density', 'fog_distance',
    'fog_falloff', 'wetness', 'scattering_intensity', 'mie_scattering_scale',
    'rayleigh_scattering_scale', 'dust_storm',
]


def _sample(keyframes, t):
    """Linear interpolation of a (time_fraction, value) keyframe list at t."""
    if t <= keyframes[0][0]:
        return keyframes[0][1]
    if t >= keyframes[-1][0]:
        return keyframes[-1][1]
    for (t0, v0), (t1, v1) in zip(keyframes, keyframes[1:]):
        if t0 <= t <= t1:
            span = t1 - t0
            alpha = (t - t0) / span if span > 0 else 0.0
            return v0 + (v1 - v0) * alpha
    return keyframes[-1][1]  # unreachable, keeps type-checkers happy


def storm_weather_at(t_frac):
    """t_frac in [0, 1): one full storm cycle. Returns a dict of overrides."""
    return {name: _sample(kf, t_frac) for name, kf in STORM_KEYFRAMES.items()}


def sun_angles_at(t_frac):
    """t_frac in [0, 1): one full day/night cycle.

    altitude: -90 at midnight (t=0), +90 at solar noon (t=0.5), back to -90
    at the next midnight (t=1) -- a plain cosine gives that shape directly.
    azimuth: sweeps a full 0-360 turn across the same period.
    """
    altitude = -90.0 * math.cos(2.0 * math.pi * t_frac)
    azimuth = (t_frac * 360.0) % 360.0
    return altitude, azimuth


def _lerp_weather(a, b, alpha):
    """Field-by-field lerp between two WeatherParameters. Azimuth isn't
    unwrapped for shortest-path -- a short warm-up blend sweeping the sun
    the "long way" round for a couple of seconds is a harmless cosmetic
    quirk, not worth the extra complexity."""
    wp = carla.WeatherParameters()
    for name in WEATHER_FIELDS:
        va, vb = getattr(a, name), getattr(b, name)
        setattr(wp, name, va + (vb - va) * alpha)
    return wp


class DayNightWeatherCycle:
    """Drives sun + storm cycles off world.on_tick -- non-blocking.

    day_length / storm_length are in simulated seconds (i.e. real seconds
    at a 1:1 fixed/variable timestep world, the common case), divided by
    speed -- speed=2 runs the whole thing twice as fast. The two periods
    are independent on purpose: a storm can start at any point in the
    day/night cycle, same as real weather.

    The very first update snaps straight from whatever weather the world
    already had to the cycle's computed state in one RPC call -- confirmed
    this can visibly glitch the sky/cloud rendering (atmosphere not
    rendering) on that first jump specifically, while every later update is
    a small per-tick delta and renders fine. warmup_seconds blends from the
    world's current weather into the cycle over that many real seconds
    instead of snapping, which sidesteps it.
    """

    def __init__(self, world, day_length=180.0, storm_length=240.0,
                 speed=1.0, start_time=0.0, warmup_seconds=5.0,
                 set_weather_hz=10.0, on_update=None,
                 animate_time=True, animate_weather=True):
        self.world = world
        self.day_length = day_length
        self.storm_length = storm_length
        self.speed = speed
        self.elapsed = start_time
        self.warmup_seconds = warmup_seconds
        self.set_weather_hz = set_weather_hz
        self.on_update = on_update
        self.animate_time = animate_time
        self.animate_weather = animate_weather
        self._last_sim_time = None
        self._warmup_start_weather = None
        self._warmup_elapsed = 0.0
        self._callback_id = None
        self._last_print_wall_time = 0.0
        self._last_set_weather_wall_time = 0.0
        self._fixed_sun = None  # (altitude, azimuth), set on start() if animate_time is off

    def _compute_weather(self):
        if self.animate_time:
            day_t = (self.elapsed % self.day_length) / self.day_length
            altitude, azimuth = sun_angles_at(day_t)
        else:
            altitude, azimuth = self._fixed_sun

        wp = carla.WeatherParameters(
            sun_altitude_angle=altitude,
            sun_azimuth_angle=azimuth,
        )
        # storm_t=0.0 (the cycle's own definition of clear, see
        # STORM_KEYFRAMES) when weather isn't animating, rather than reusing
        # whatever the world happened to have -- keeps a time-only run clean
        # and predictable instead of possibly stuck mid-storm.
        storm_t = (self.elapsed % self.storm_length) / self.storm_length if self.animate_weather else 0.0
        for name, value in storm_weather_at(storm_t).items():
            setattr(wp, name, value)
        return wp

    def _on_tick(self, snapshot):
        if self._last_sim_time is None:
            self._last_sim_time = snapshot.timestamp.elapsed_seconds
            self._warmup_start_weather = self.world.get_weather()
            if not self.animate_time:
                self._fixed_sun = (self._warmup_start_weather.sun_altitude_angle,
                                    self._warmup_start_weather.sun_azimuth_angle)
            return
        dt = snapshot.timestamp.elapsed_seconds - self._last_sim_time
        self._last_sim_time = snapshot.timestamp.elapsed_seconds
        if dt <= 0.0:
            return
        dt *= self.speed
        self.elapsed += dt

        target = self._compute_weather()
        if self._warmup_elapsed < self.warmup_seconds:
            self._warmup_elapsed += dt
            alpha = min(1.0, self._warmup_elapsed / self.warmup_seconds)
            wp = _lerp_weather(self._warmup_start_weather, target, alpha)
        else:
            wp = target

        # set_weather() itself is throttled to set_weather_hz (wall-clock),
        # NOT called every on_tick (which can fire 30-60+ times a second):
        # confirmed calling it that often breaks the map's sky Blueprint --
        # multiple directional lights end up competing at night, and by day
        # only the directional light renders (SkyLight/atmosphere stop
        # updating), both clearing on a fresh map load. elapsed still
        # accumulates every tick above, so the motion stays smooth; only the
        # RPC/Blueprint call rate drops.
        now = time.time()
        if now - self._last_set_weather_wall_time >= 1.0 / self.set_weather_hz:
            self._last_set_weather_wall_time = now
            self.world.set_weather(wp)

        if self.on_update and now - self._last_print_wall_time >= 0.2:
            self._last_print_wall_time = now
            self.on_update(self.elapsed, wp)

    def start(self):
        self._callback_id = self.world.on_tick(self._on_tick)
        return self

    def stop(self):
        if self._callback_id is not None:
            self.world.remove_on_tick(self._callback_id)
            self._callback_id = None


def _print_status_line(elapsed, wp):
    # Confirmed the earlier fixed set of fields (~95 chars) still wrapped
    # onto a second terminal row in a narrow pane -- \r then only rewinds
    # that wrapped row, not the whole logical line, which is what made each
    # update look like a fresh line instead of overwriting in place. Fields
    # ordered by priority and trimmed to whatever actually fits this
    # terminal's real width (falls back to 80 columns if it can't be
    # detected, e.g. output piped to a file).
    fields = [
        ('alt', 'sun_altitude_angle'), ('cloud', 'cloudiness'),
        ('rain', 'precipitation'), ('wet', 'wetness'),
        ('dep', 'precipitation_deposits'), ('wind', 'wind_intensity'),
        ('fog', 'fog_density'), ('az', 'sun_azimuth_angle'),
    ]
    width = shutil.get_terminal_size(fallback=(80, 24)).columns
    prefix = 't=%7.1fs' % elapsed
    line = prefix
    for label, attr in fields:
        piece = ' %s=%5.1f' % (label, getattr(wp, attr))
        if len(line) + len(piece) > width - 1:
            break
        line += piece
    # sys.stdout.write + explicit flush instead of print(..., end=''): some
    # terminals only honor \r-overwrite reliably this way. "\x1b[K" (erase
    # to end of line) clears any leftover tail from a longer previous line.
    sys.stdout.write('\r' + line + '\x1b[K')
    sys.stdout.flush()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--day-length', type=float, default=180.0,
                         help='seconds for one full day/night cycle')
    parser.add_argument('--storm-length', type=float, default=240.0,
                         help='seconds for one full clear->storm->clear cycle')
    parser.add_argument('--speed', type=float, default=1.0,
                         help='time multiplier -- 2.0 runs twice as fast, '
                              '0.5 half as fast')
    parser.add_argument('--warmup', type=float, default=5.0,
                         help='seconds to blend from the current weather into '
                              'the cycle, instead of snapping on the first update')
    parser.add_argument('--set-weather-hz', type=float, default=10.0,
                         help='how often per second to actually push the weather '
                              'update -- calling it every simulation tick (30-60+/s) '
                              'is confirmed to break the sky Blueprint')
    parser.add_argument('--quiet', action='store_true',
                         help="don't print the live status line")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--time-only', action='store_true',
                       help='animate only the day/night sun cycle -- weather '
                            'stays clear (storm cycle frozen at t=0)')
    mode.add_argument('--weather-only', action='store_true',
                       help='animate only the storm cycle -- sun position stays '
                            "fixed at whatever the world's current weather has")
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()

    cycle = DayNightWeatherCycle(
        world, day_length=args.day_length, storm_length=args.storm_length,
        speed=args.speed, warmup_seconds=args.warmup,
        set_weather_hz=args.set_weather_hz,
        animate_time=not args.weather_only,
        animate_weather=not args.time_only,
        on_update=None if args.quiet else _print_status_line).start()

    print('running day/night + storm cycle -- Ctrl+C to stop')
    try:
        while True:
            time.sleep(1.0)
    except KeyboardInterrupt:
        pass
    finally:
        cycle.stop()
        print('\nstopped, last weather left in place')


if __name__ == '__main__':
    main()
