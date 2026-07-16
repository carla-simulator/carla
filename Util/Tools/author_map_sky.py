#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Author daytime lighting into a CARLA map by upgrading the map's own native
lights in place (adds a SkyAtmosphere, switches the SkyLight to real-time
capture, and tunes the sun intensity).

Runs INSIDE the UE editor's embedded Python (PythonScriptPlugin), headless.
Config comes from environment variables (UE's -run=pythonscript arg passing is
unreliable, so env vars are used instead):

  MODE               list-native | apply-native
  TARGET_MAP         /Game path of the map to author (default Town04_Opt)
  # apply-native only:
  ADD_ATMOSPHERE     add a SkyAtmosphere if none present (default 1)
  SKYLIGHT_CAPTURED  switch the native SkyLight to real-time captured scene (default 1)
  SKYLIGHT_INTENSITY captured-SkyLight intensity scale (default 1.0)
  SUN_INTENSITY      set the native DirectionalLight intensity (default: leave as-is)
  ADD_PP             add an unbound histogram PostProcessVolume (default 0; viewport only)
  PP_BIAS            AutoExposureBias for that PostProcessVolume (default 1.2)

apply-native (native rig): for maps lit at runtime by their OWN native
DirectionalLight + SkyLight with no SkyAtmosphere (e.g. Town04_Opt), which
render as dim dusk. It KEEPS the native sun and converts the rig to daytime by
adding a SkyAtmosphere, switching the SkyLight to real-time capture, and
raising the sun intensity into the CARLA sensor camera's auto-exposure window
(that camera clamps adaptation luminance to [10,12] cd/m², so the scene must
be physically bright). Verified daytime recipe for Town04_Opt:
    MODE=apply-native TARGET_MAP=/Game/Carla/Maps/Town04_Opt \
    SKYLIGHT_INTENSITY=1.0 SUN_INTENSITY=20000

Runner note:
  Both modes run in the -run=pythonscript commandlet. The commandlet's async
  shader compiler (under -AllowCommandletRendering) can crash a worker thread
  on shutdown AFTER the save completes -- judge apply-native by the
  "saved ..." log line and the post-save list-native check, not the exit code.
"""

import math
import os

import unreal


def log(msg):
    unreal.log(f"[author_map_sky] {msg}")


def load_map(game_path):
    unreal.EditorLoadingAndSavingUtils.load_map(game_path)
    log(f"loaded map {game_path}")


def all_actors():
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    return eas.get_all_level_actors()


def _try(getter, default=None):
    """Read-oriented reflection helper. A miss here is non-fatal (optional
    snapshot reads), so it is swallowed and logged as a 'read miss'. Do NOT use
    this for authoring writes — use _set/_call so failures are surfaced."""
    try:
        return getter()
    except Exception as e:  # reflection misses are non-fatal; snapshot what we can
        log(f"  read miss: {e}")
        return default


# Module-level tally of authoring problems (failed writes AND requested steps
# that could not run). mode_apply_native flags a prominent WARNING (and still
# saves, so the partial state is inspectable) when non-zero, so an incomplete
# authoring can never hide behind the "saved" log line.
_write_failures = 0


def _authoring_error(msg):
    """A requested authoring step could not run (e.g. its target actor is
    absent -- possibly stranded in an unloaded streamed sub-level -- or a config
    value was malformed). Count it like a failed write and log it loudly so the
    run reports 'authoring incomplete' instead of a silent skip."""
    global _write_failures
    _write_failures += 1
    log(f"  write failed: {msg}")


def _values_match(got, want):
    """Tolerant equality for a write read-back: float values compare with a
    small tolerance, everything else (bool/enum/int/str) by ==."""
    if isinstance(got, float) or isinstance(want, float):
        try:
            return math.isclose(float(got), float(want), rel_tol=1e-6, abs_tol=1e-6)
        except (TypeError, ValueError):
            return got == want
    return got == want


def _set(obj, prop, value):
    """Authoring WRITE. Unlike _try, a failure is loud: it logs 'write failed'
    (not 'read miss') and records a module-level failure so incomplete authoring
    is reported instead of silently succeeding.

    UE's set_editor_property can log-and-continue (returning without raising) on
    a rejected/read-only property, so an exception alone is not proof the write
    took. For scalar/enum/bool writes we read the property back and count a
    silent no-op as a failure too. Struct values (e.g. PostProcessSettings) do
    not compare reliably, so verification is skipped for them."""
    global _write_failures
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        _write_failures += 1
        log(f"  write failed: {prop}: {e}")
        return False
    if not isinstance(value, unreal.StructBase):
        try:
            got = obj.get_editor_property(prop)
        except Exception as e:
            log(f"  (write verify skipped for {prop}: {e})")
            return True
        if not _values_match(got, value):
            _write_failures += 1
            log(f"  write failed (no-op): {prop}: set {value!r} but read back {got!r}")
            return False
    return True


def _call(obj, method):
    """Authoring method call (e.g. recapture_sky). Same loud-failure contract
    as _set."""
    global _write_failures
    try:
        getattr(obj, method)()
        return True
    except Exception as e:
        _write_failures += 1
        log(f"  write failed: {method}(): {e}")
        return False


# ---------------------------------------------------------------------------
# Native-lighting authoring (apply-native)
#
# Some CARLA maps (e.g. Town04_Opt) are lit at runtime by their OWN native
# standalone DirectionalLight + SkyLight actors (living in a streamed
# rendering/lighting sub-level) and have NO SkyAtmosphere. That renders as dim
# dusk: the sun is present but there is no scattering sky and the SkyLight is a
# dark baked cubemap, so both the sky and the ambient are dark.
#
# apply-native converts that native rig to daytime WITHOUT transplanting
# BP_Carla_Sky and WITHOUT removing the native lights (they are the real
# runtime sun). It:
#   1. keeps the native DirectionalLight (ensures atmosphere_sun_light=True;
#      optional intensity tune via SUN_INTENSITY),
#   2. adds a SkyAtmosphere driven by that sun (bright daytime sky + aerial
#      scattering — the single biggest missing piece),
#   3. switches the native SkyLight to a real-time captured-scene skylight so it
#      captures the now-bright atmosphere for bright daytime ambient (instead of
#      the dark baked cubemap), at a sane intensity scale,
#   4. optionally adds an unbound histogram PostProcessVolume (ADD_PP=1) for the
#      game viewport — note CARLA sensor cameras override their own exposure so
#      this does NOT affect sensor captures; off by default.
# ---------------------------------------------------------------------------


def _envf(name, default):
    """Parse a float env var. A malformed value is loud (logged + counted as an
    authoring failure) and falls back to the default, rather than raising and
    crashing the run mid-authoring (discarding edits already applied but not yet
    saved)."""
    v = os.environ.get(name)
    if v in (None, ""):
        return default
    try:
        return float(v)
    except ValueError:
        _authoring_error(f"invalid {name}={v!r}; using default {default}")
        return default


def _env_opt_f(name):
    """Optional float env var: None when unset (leave the property as-is). A
    malformed value is loud (logged + counted) and treated as unset instead of
    crashing the run."""
    v = os.environ.get(name)
    if v in (None, ""):
        return None
    try:
        return float(v)
    except ValueError:
        _authoring_error(f"invalid {name}={v!r}; leaving property unchanged")
        return None


def _envb(name, default):
    v = os.environ.get(name)
    if v in (None, ""):
        return default
    return v.strip().lower() not in ("0", "false", "no", "off")


def _actors_by_class(class_name):
    return [a for a in all_actors() if a.get_class().get_name() == class_name]


def _component_by_class(actor, comp_class):
    return _try(lambda: actor.get_component_by_class(comp_class))


def _log_directional(dl):
    c = _component_by_class(dl, unreal.DirectionalLightComponent)
    if c is None:
        return
    rot = _try(lambda: c.get_relative_transform().rotation.rotator())
    log(f"  DirectionalLight '{dl.get_actor_label()}': "
        f"intensity={_try(lambda: c.get_editor_property('intensity'))} "
        f"units={_try(lambda: c.get_editor_property('intensity_units'))} "
        f"atmosphere_sun={_try(lambda: c.get_editor_property('atmosphere_sun_light'))} "
        f"rot=({rot.pitch if rot else '?'},{rot.yaw if rot else '?'},{rot.roll if rot else '?'})")


def _log_skylight(sl):
    c = _component_by_class(sl, unreal.SkyLightComponent)
    if c is None:
        return
    log(f"  SkyLight '{sl.get_actor_label()}': "
        f"source={_try(lambda: c.get_editor_property('source_type'))} "
        f"real_time_capture={_try(lambda: c.get_editor_property('real_time_capture'))} "
        f"intensity={_try(lambda: c.get_editor_property('intensity'))} "
        f"mobility={_try(lambda: c.get_editor_property('mobility'))}")


def _log_atmosphere(atm):
    c = _component_by_class(atm, unreal.SkyAtmosphereComponent)
    log(f"  SkyAtmosphere '{atm.get_actor_label()}': "
        f"component={'present' if c is not None else 'MISSING'}")


def _log_postprocess(ppv):
    s = _try(lambda: ppv.get_editor_property("settings"))
    log(f"  PostProcessVolume '{ppv.get_actor_label()}': "
        f"unbound={_try(lambda: ppv.get_editor_property('unbound'))} "
        f"priority={_try(lambda: ppv.get_editor_property('priority'))} "
        f"auto_exposure_method="
        f"{_try(lambda: s.get_editor_property('auto_exposure_method')) if s else '?'} "
        f"auto_exposure_bias="
        f"{_try(lambda: s.get_editor_property('auto_exposure_bias')) if s else '?'}")


def _map_prefix(target):
    """Actor-label prefix derived from the target map's leaf name so a general
    map does not get misleading Town04_-prefixed actors. e.g.
    /Game/Carla/Maps/Town04_Opt -> 'Town04' (a trailing _Opt is stripped)."""
    leaf = target.rstrip("/").rsplit("/", 1)[-1]
    if leaf.endswith("_Opt"):
        leaf = leaf[:-len("_Opt")]
    return leaf or "Map"


# A single OpenDriveToMap instance is reused for every spawn (see _get_spawner).
_spawner = None


def _get_spawner():
    """Lazily construct and cache the OpenDriveToMap spawner used by
    _spawn_native.

    DELIBERATE workaround, not a gratuitous CarlaTools dependency:
    EditorActorSubsystem.spawn_actor_from_class() SIGSEGVs in the headless
    -run=pythonscript commandlet (placement-subsystem null deref at 0x38) --
    empirically verified for the NATIVE classes this tool spawns (SkyAtmosphere,
    PostProcessVolume), not only for Blueprint classes.
    OpenDriveToMap.spawn_actor_in_editor_world() spawns into the editor world
    without touching the placement subsystem, so it is the only spawn path that
    survives the commandlet."""
    global _spawner
    if _spawner is None:
        if not hasattr(unreal, "OpenDriveToMap"):
            raise RuntimeError(
                "apply-native: unreal.OpenDriveToMap is unavailable -- the "
                "CarlaTools plugin and its Python bindings must be enabled. It "
                "is required because EditorActorSubsystem.spawn_actor_from_class()"
                " SIGSEGVs in the headless commandlet; OpenDriveToMap is the "
                "working spawn path.")
        _spawner = unreal.OpenDriveToMap()
    return _spawner


def _spawn_native(actor_class, label):
    actor = _get_spawner().spawn_actor_in_editor_world(
        actor_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    if actor is None:
        raise RuntimeError(f"apply-native: spawn of {label} returned None")
    _try(lambda: actor.set_actor_label(label))
    log(f"spawned {actor.get_class().get_name()} '{label}'")
    return actor


def _add_histogram_pp(bias, prefix):
    """Unbound PostProcessVolume with Town10HD-style histogram exposure. Only
    affects the game viewport; CARLA sensor cameras override their own exposure.
    """
    ppv = _spawn_native(unreal.PostProcessVolume.static_class(),
                        f"{prefix}_ExposurePP")
    _set(ppv, "unbound", True)
    _set(ppv, "priority", 1.0)
    s = ppv.get_editor_property("settings")
    # -10/20 EV are fixed Town10HD-style defaults (not map-tuned). Flags and
    # value writes both route through _set so a failure on either is loud.
    _set(s, "auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
    _set(s, "auto_exposure_min_brightness", -10.0)
    _set(s, "auto_exposure_max_brightness", 20.0)
    _set(s, "auto_exposure_bias", bias)
    for flag in ("override_auto_exposure_method",
                 "override_auto_exposure_min_brightness",
                 "override_auto_exposure_max_brightness",
                 "override_auto_exposure_bias"):
        _set(s, flag, True)
    _set(ppv, "settings", s)
    log(f"added unbound histogram PostProcessVolume (bias={bias})")


def mode_list_native():
    load_map(os.environ.get("TARGET_MAP", "/Game/Carla/Maps/Town04_Opt"))
    dls = _actors_by_class("DirectionalLight")
    sls = _actors_by_class("SkyLight")
    atm = _actors_by_class("SkyAtmosphere")
    ppv = _actors_by_class("PostProcessVolume")
    log(f"native lighting: DirectionalLight={len(dls)} SkyLight={len(sls)} "
        f"SkyAtmosphere={len(atm)} PostProcessVolume={len(ppv)}")
    for dl in dls:
        _log_directional(dl)
    for sl in sls:
        _log_skylight(sl)
    for a in atm:
        _log_atmosphere(a)
    for p in ppv:
        _log_postprocess(p)


def mode_apply_native():
    target = os.environ.get("TARGET_MAP", "/Game/Carla/Maps/Town04_Opt")
    prefix = _map_prefix(target)
    load_map(target)

    dls = _actors_by_class("DirectionalLight")
    sls = _actors_by_class("SkyLight")
    atm = _actors_by_class("SkyAtmosphere")
    log(f"native before: DirectionalLight={len(dls)} SkyLight={len(sls)} "
        f"SkyAtmosphere={len(atm)}")
    for dl in dls:
        _log_directional(dl)
    for sl in sls:
        _log_skylight(sl)

    # A differently-shaped map may lack the native lights this tool tunes -- and
    # note the SkyLight in particular lives in a streamed sub-level, so an empty
    # result can mean "stranded in an unloaded sub-level" rather than "absent".
    # Either way, when the step that needs the actor was actually requested,
    # count it as an authoring failure so the run reports incompleteness instead
    # of silently no-op'ing (a bare WARNING is easy to miss in the log).
    if not dls:
        _authoring_error("no DirectionalLight found on target map "
                         "(nothing to drive the atmosphere sun)")
    if not sls and _envb("SKYLIGHT_CAPTURED", True):
        _authoring_error("no SkyLight found on target map "
                         "(SKYLIGHT_CAPTURED requested but cannot run -- it may "
                         "be in an unloaded streamed sub-level)")

    # 1. Native DirectionalLight = the runtime sun. Keep it; ensure it drives the
    # atmosphere. Optional intensity tune (SUN_INTENSITY) once atmosphere+ambient
    # are in and the ground is still over/under-lit.
    sun_intensity = _env_opt_f("SUN_INTENSITY")
    for dl in dls:
        c = _component_by_class(dl, unreal.DirectionalLightComponent)
        if c is None:
            continue
        _set(c, "atmosphere_sun_light", True)
        if sun_intensity is not None:
            _set(c, "intensity", sun_intensity)
            log(f"  set DirectionalLight intensity -> {sun_intensity}")

    # 2. SkyAtmosphere (the dominant missing piece) — defaults give an Earth-like
    # scattering sky driven by the atmosphere-sun DirectionalLight.
    if _envb("ADD_ATMOSPHERE", True):
        if atm:
            log("SkyAtmosphere already present; not adding another")
        else:
            _spawn_native(unreal.SkyAtmosphere.static_class(),
                         f"{prefix}_SkyAtmosphere")

    # 3. SkyLight -> real-time captured scene (bright daytime ambient from the
    # now-bright atmosphere) at a sane intensity scale (baked 540 would blow out
    # when re-pointed at a live bright sky).
    if _envb("SKYLIGHT_CAPTURED", True):
        sky_intensity = _envf("SKYLIGHT_INTENSITY", 1.0)
        for sl in sls:
            c = _component_by_class(sl, unreal.SkyLightComponent)
            if c is None:
                continue
            _set(c, "mobility", unreal.ComponentMobility.MOVABLE)
            _set(c, "source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
            _set(c, "real_time_capture", True)
            _set(c, "intensity", sky_intensity)
            _call(c, "recapture_sky")
            log(f"  SkyLight -> captured-scene real-time, intensity={sky_intensity}")

    # 4. Optional histogram PostProcessVolume (viewport only; not sensors).
    if _envb("ADD_PP", False):
        if _actors_by_class("PostProcessVolume"):
            log("PostProcessVolume already present; not adding another")
        else:
            _add_histogram_pp(_envf("PP_BIAS", 1.2), prefix)

    # Report final state before save.
    log("native after:")
    for dl in _actors_by_class("DirectionalLight"):
        _log_directional(dl)
    for sl in _actors_by_class("SkyLight"):
        _log_skylight(sl)
    log(f"  SkyAtmosphere count={len(_actors_by_class('SkyAtmosphere'))}")

    # Persist: the SkyAtmosphere/PP land in the persistent map; the SkyLight edit
    # dirties its streamed sub-level. Save the persistent level then every dirty
    # map package so the sub-level change persists too.
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.save_current_level()
    ok = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, False)
    log(f"saved {target}; save_dirty_packages(maps) -> {ok}")

    # Still saved above (so the partial state is inspectable), but if any
    # authoring write failed, make the incompleteness loud rather than let it
    # hide behind the "saved" line.
    if _write_failures:
        log(f"WARNING: authoring incomplete -- {_write_failures} write(s) failed")


def main():
    mode = os.environ.get("MODE", "list-native")
    log(f"MODE={mode}")
    if mode == "list-native":
        mode_list_native()
    elif mode == "apply-native":
        mode_apply_native()
    else:
        log(f"unknown MODE {mode}")


main()
