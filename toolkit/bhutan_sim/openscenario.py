# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""ASAM OpenSCENARIO 1.2 export of scenario templates.

The generated ``.xosc`` runs in CARLA ScenarioRunner
(``python scenario_runner.py --openscenario file.xosc``), esmini
(``esmini --osc file.xosc``) and other OpenSCENARIO players. Templates carry
no road geometry, so the ego is placed on a parameterised lane
(``road_id``, ``ego_lane_id``, ``ego_s``) and every other actor relative to
the ego; override the parameters per map with ``--openscenarioparams`` in
ScenarioRunner or ``--param`` in esmini.

``dashboard/src/openscenario.ts`` produces the same document; keep the
entity and weather tables in sync.
"""

from __future__ import annotations

import datetime as _dt
import math
import xml.etree.ElementTree as ET
from typing import Any, Dict, List, Optional
from xml.dom import minidom

from .scenario import ScenarioTemplate

HOUR_BY_TIME = {"dawn": "06:15:00", "day": "12:00:00", "dusk": "18:15:00", "night": "22:00:00"}
ELEVATION_BY_TIME = {"dawn": 8.0, "day": 60.0, "dusk": 8.0, "night": -30.0}
AZIMUTH_BY_TIME = {"dawn": 90.0, "day": 150.0, "dusk": 270.0, "night": 0.0}

# Mirrors bhutan_sim/weather.py presets reduced to what OpenSCENARIO expresses.
PRESETS: Dict[str, Dict[str, Any]] = {
    "clear_day": dict(cloudState="free", precipitationType="dry", intensity=0.0, fog=100000, friction=1.0),
    "overcast": dict(cloudState="overcast", precipitationType="dry", intensity=0.0, fog=100000, friction=1.0),
    "light_rain": dict(cloudState="overcast", precipitationType="rain", intensity=0.3, fog=100000, friction=0.8),
    "heavy_rain": dict(cloudState="rainy", precipitationType="rain", intensity=0.8, fog=400, friction=0.65),
    "monsoon_storm": dict(cloudState="rainy", precipitationType="rain", intensity=1.0, fog=300, friction=0.55),
    "wet_road": dict(cloudState="cloudy", precipitationType="dry", intensity=0.0, fog=100000, friction=0.7),
    "valley_fog": dict(cloudState="cloudy", precipitationType="dry", intensity=0.0, fog=80, friction=0.9),
    "dense_fog": dict(cloudState="overcast", precipitationType="dry", intensity=0.0, fog=30, friction=0.9),
    "low_sun": dict(cloudState="free", precipitationType="dry", intensity=0.0, fog=100000, elevation=6.0, azimuth=0.0, friction=1.0),
    "low_sun_haze": dict(cloudState="free", precipitationType="dry", intensity=0.0, fog=600, elevation=4.0, azimuth=0.0, friction=1.0),
    "clear_night": dict(cloudState="free", precipitationType="dry", intensity=0.0, fog=100000, elevation=-30.0, friction=1.0),
    "wet_night": dict(cloudState="overcast", precipitationType="dry", intensity=0.0, fog=100000, elevation=-30.0, friction=0.7),
    "dust_haze": dict(cloudState="cloudy", precipitationType="dry", intensity=0.0, fog=800, friction=1.0),
}

# role -> (kind, category, model, length, width, height, mass)
ROLE_ENTITIES: Dict[str, tuple] = {
    "oncoming_truck": ("vehicle", "truck", "vehicle.carlamotors.carlacola", 8.0, 2.5, 3.4, 12000),
    "oncoming_vehicle": ("vehicle", "car", "vehicle.tesla.model3", 4.7, 1.9, 1.5, 1800),
    "lead_vehicle": ("vehicle", "car", "vehicle.toyota.prius", 4.5, 1.8, 1.5, 1500),
    "lead_vehicle_stopping": ("vehicle", "car", "vehicle.toyota.prius", 4.5, 1.8, 1.5, 1500),
    "following_vehicle": ("vehicle", "car", "vehicle.audi.tt", 4.2, 1.8, 1.4, 1400),
    "stalled_vehicle": ("vehicle", "truck", "vehicle.carlamotors.carlacola", 8.0, 2.5, 3.4, 12000),
    "parked_vehicle": ("vehicle", "car", "vehicle.nissan.patrol", 4.9, 1.9, 1.9, 2200),
    "motorcycle": ("vehicle", "motorbike", "vehicle.yamaha.yzf", 2.1, 0.8, 1.2, 200),
    "pedestrian": ("pedestrian", "pedestrian", "walker.pedestrian.0001", 0.5, 0.6, 1.8, 75),
    "pedestrian_crossing": ("pedestrian", "pedestrian", "walker.pedestrian.0002", 0.5, 0.6, 1.8, 75),
    "worker": ("pedestrian", "pedestrian", "walker.pedestrian.0010", 0.5, 0.6, 1.8, 80),
    "livestock": ("pedestrian", "animal", "walker.pedestrian.0001", 2.0, 0.8, 1.4, 350),
    "debris": ("misc", "obstacle", "static.prop.rock", 1.5, 1.5, 0.8, 900),
    "cones": ("misc", "obstacle", "static.prop.trafficcone01", 0.4, 0.4, 0.7, 3),
}
DEFAULT_ENTITY = ("vehicle", "car", "vehicle.tesla.model3", 4.7, 1.9, 1.5, 1800)

EGO_MODELS = {
    "truck": ("truck", "vehicle.carlamotors.carlacola", 8.0, 2.5, 3.4, 12000),
    "shuttle": ("bus", "vehicle.mitsubishi.fusorosa", 7.0, 2.3, 2.9, 6000),
    "car": ("car", "vehicle.tesla.model3", 4.7, 1.9, 1.5, 1800),
}


def _resolve_weather(preset: str, time_of_day: str) -> Dict[str, Any]:
    base = PRESETS.get(preset, PRESETS["clear_day"])
    return {
        "cloudState": base["cloudState"],
        "precipitationType": base["precipitationType"],
        "intensity": base["intensity"],
        "fog": base["fog"],
        "elevation": base.get("elevation", ELEVATION_BY_TIME.get(time_of_day, 60.0)),
        "azimuth": base.get("azimuth", AZIMUTH_BY_TIME.get(time_of_day, 150.0)),
        "friction": base["friction"],
    }


def _lane_expr(lane: str) -> str:
    return "$oncoming_dlane" if lane in ("opposite", "opposite_encroaching") else "0"


def _lateral_offset(lane: str, lateral: float, lane_width: float) -> float:
    if lane == "shoulder":
        return -(lane_width / 2 + 0.5) + lateral
    if lane == "opposite_encroaching":
        return -(lane_width * 0.35) + lateral
    if lane == "crossing":
        return lane_width / 2 + lateral
    return lateral


def _safe(name: str) -> str:
    return "".join(ch if ch.isalnum() or ch == "_" else "_" for ch in name)


def _sub(parent: ET.Element, tag: str, **attrs: Any) -> ET.Element:
    return ET.SubElement(parent, tag, {k: str(v) for k, v in attrs.items()})


def _param(parent: ET.Element, name: str, ptype: str, value: Any) -> None:
    _sub(parent, "ParameterDeclaration", name=name, parameterType=ptype, value=value)


def _bbox(parent: ET.Element, length: float, width: float, height: float) -> None:
    box = _sub(parent, "BoundingBox")
    _sub(box, "Center", x="%.2f" % (length / 2), y="0.0", z="%.2f" % (height / 2))
    _sub(box, "Dimensions", width=width, length=length, height=height)


def _axles(parent: ET.Element, length: float, width: float, z: float, wheel: float) -> None:
    axles = _sub(parent, "Axles")
    _sub(axles, "FrontAxle", maxSteering="0.5", wheelDiameter=wheel, trackWidth=width, positionX="%.2f" % (length * 0.75), positionZ=z)
    _sub(axles, "RearAxle", maxSteering="0.0", wheelDiameter=wheel, trackWidth=width, positionX="0.0", positionZ=z)


def _speed_action(parent: ET.Element, value: Any, shape: str = "step", rate: Any = 0) -> None:
    action = _sub(_sub(_sub(parent, "PrivateAction"), "LongitudinalAction"), "SpeedAction")
    _sub(action, "SpeedActionDynamics", dynamicsShape=shape, value=rate, dynamicsDimension="time" if shape == "step" else "rate")
    _sub(_sub(action, "SpeedActionTarget"), "AbsoluteTargetSpeed", value=value)


def _time_trigger(parent: ET.Element, tag: str, name: str, value: Any, edge: str = "none") -> None:
    cond = _sub(_sub(_sub(parent, tag), "ConditionGroup"), "Condition", name=name, delay="0", conditionEdge=edge)
    _sub(_sub(cond, "ByValueCondition"), "SimulationTimeCondition", value=value, rule="greaterThan")


def scenario_to_xosc(template: ScenarioTemplate, generated_at: Optional[_dt.datetime] = None, map_name: str = "Town10HD_Opt") -> str:
    """Return the OpenSCENARIO document for a template as a pretty-printed string."""
    generated_at = generated_at or _dt.datetime.now(_dt.timezone.utc)
    data = template.to_dict()
    p = data["params"]
    ego_cat, ego_model, ego_len, ego_w, ego_h, ego_mass = EGO_MODELS.get(p.get("vehicle_class", "truck"), EGO_MODELS["car"])
    speed_limit = float(p.get("speed_limit_kph", 40.0))
    lane_width = float(p.get("lane_width_m", 3.3))
    duration = float(p.get("duration_s", 60.0))
    time_of_day = str(p.get("time_of_day", "day"))
    weather = _resolve_weather(str(p.get("weather_preset", "clear_day")), time_of_day)
    degradation = p.get("sensor_degradation") or {}

    root = ET.Element("OpenSCENARIO", {
        "xmlns:xsi": "http://www.w3.org/2001/XMLSchema-instance",
        "xsi:noNamespaceSchemaLocation": "OpenScenario.xsd",
    })
    _sub(root, "FileHeader", revMajor="1", revMinor="2", date=generated_at.strftime("%Y-%m-%dT%H:%M:%S"),
         description="%s [%s] %s" % (data["name"], data["id"], data.get("description", "")), author="Bhutan Mobility Atlas")
    params = _sub(root, "ParameterDeclarations")
    _param(params, "road_id", "string", "0")
    _param(params, "ego_lane_id", "integer", "-1")
    _param(params, "ego_s", "double", "20.0")
    _param(params, "oncoming_dlane", "integer", "1")
    _param(params, "ego_speed", "double", "%.3f" % (speed_limit / 3.6))
    _param(params, "speed_limit_kph", "double", speed_limit)
    _param(params, "scenario_duration", "double", duration)
    _param(params, "road_curvature", "double", p.get("road_curvature", 0.0))
    _param(params, "grade_pct", "double", p.get("grade_pct", 0.0))
    _param(params, "lane_quality", "double", p.get("lane_quality", 1.0))
    _param(params, "traffic_density", "double", p.get("traffic_density", 0.0))
    _param(params, "payload_kg", "double", p.get("payload_kg", 0.0))
    for key, value in degradation.items():
        _param(params, "sensor_%s" % key, "double", value)
    _param(params, "atlas_family", "string", data["family"])
    _param(params, "atlas_content_hash", "string", data["content_hash"])
    _param(params, "atlas_expected_events", "string", ",".join(data.get("expected_events", [])))
    _sub(root, "CatalogLocations")
    _sub(_sub(root, "RoadNetwork"), "LogicFile", filepath=map_name)

    entities = _sub(root, "Entities")
    hero = _sub(_sub(entities, "ScenarioObject", name="hero"), "Vehicle", name=ego_model, vehicleCategory=ego_cat, mass=int(ego_mass + float(p.get("payload_kg", 0.0))))
    _sub(hero, "ParameterDeclarations")
    _sub(hero, "Performance", maxSpeed="%.2f" % (speed_limit / 3.6 * 1.5), maxAcceleration="3.0", maxDeceleration="8.0")
    _bbox(hero, ego_len, ego_w, ego_h)
    _axles(hero, ego_len, ego_w, 0.45, 0.9)
    props = _sub(hero, "Properties")
    _sub(props, "Property", name="type", value="ego_vehicle")
    _sub(props, "Property", name="color", value="255,255,255")

    actors: List[Dict[str, Any]] = []
    for index, spec in enumerate(data.get("actors", [])):
        role = str(spec.get("role", "vehicle"))
        kind, category, model, length, width, height, mass = ROLE_ENTITIES.get(role, DEFAULT_ENTITY)
        actor = {"name": "%s_%d" % (_safe(role), index + 1), "role": role, "kind": kind, "lane": str(spec.get("lane", "ego")),
                 "distance": float(spec.get("distance_m", 50.0)), "lateral": float(spec.get("lateral_m", 0.0)), "speed": spec.get("speed_mps")}
        actors.append(actor)
        obj = _sub(entities, "ScenarioObject", name=actor["name"])
        if kind == "vehicle":
            el = _sub(obj, "Vehicle", name=model, vehicleCategory=category, mass=mass)
            _sub(el, "ParameterDeclarations")
            _sub(el, "Performance", maxSpeed="40.0", maxAcceleration="3.0", maxDeceleration="8.0")
            _bbox(el, length, width, height)
            _axles(el, length, width, 0.35, 0.7)
        elif kind == "pedestrian":
            el = _sub(obj, "Pedestrian", name=model, model=model, mass=mass, pedestrianCategory=category)
            _sub(el, "ParameterDeclarations")
            _bbox(el, length, width, height)
        else:
            el = _sub(obj, "MiscObject", name=model, mass=mass, miscObjectCategory=category)
            _sub(el, "ParameterDeclarations")
            _bbox(el, length, width, height)
        props = _sub(el, "Properties")
        _sub(props, "Property", name="type", value="simulation")
        _sub(props, "Property", name="atlas_role", value=role)

    storyboard = _sub(root, "Storyboard")
    init_actions = _sub(_sub(storyboard, "Init"), "Actions")
    env = _sub(_sub(_sub(init_actions, "GlobalAction"), "EnvironmentAction"), "Environment", name="%s_%s" % (p.get("weather_preset", "clear_day"), time_of_day))
    _sub(env, "TimeOfDay", animation="false", dateTime="%sT%s" % (generated_at.strftime("%Y-%m-%d"), HOUR_BY_TIME.get(time_of_day, "12:00:00")))
    w = _sub(env, "Weather", cloudState=weather["cloudState"])
    _sub(w, "Sun", intensity="1.0" if weather["elevation"] > 0 else "0.05", azimuth="%.4f" % math.radians(weather["azimuth"]), elevation="%.4f" % math.radians(weather["elevation"]))
    _sub(w, "Fog", visualRange=weather["fog"])
    _sub(w, "Precipitation", precipitationType=weather["precipitationType"], intensity=weather["intensity"])
    _sub(env, "RoadCondition", frictionScaleFactor=weather["friction"])
    ego_private = _sub(init_actions, "Private", entityRef="hero")
    _sub(_sub(_sub(_sub(ego_private, "PrivateAction"), "TeleportAction"), "Position"), "LanePosition", roadId="$road_id", laneId="$ego_lane_id", offset="0.0", s="$ego_s")
    _speed_action(ego_private, "$ego_speed")
    for actor in actors:
        ds = -abs(actor["distance"]) if actor["lane"] == "ego_behind" else actor["distance"]
        private = _sub(init_actions, "Private", entityRef=actor["name"])
        _sub(_sub(_sub(_sub(private, "PrivateAction"), "TeleportAction"), "Position"), "RelativeLanePosition",
             entityRef="hero", dLane=_lane_expr(actor["lane"]), ds="%.2f" % ds, offset="%.2f" % _lateral_offset(actor["lane"], actor["lateral"], lane_width))
        if actor["speed"] is not None and actor["kind"] != "misc":
            _speed_action(private, "%.2f" % float(actor["speed"]))

    story = _sub(storyboard, "Story", name=_safe(data["id"]))
    _sub(story, "ParameterDeclarations")
    act = _sub(story, "Act", name=_safe(data["family"]))
    group = _sub(act, "ManeuverGroup", name="ego_drive", maximumExecutionCount="1")
    _sub(_sub(group, "Actors", selectTriggeringEntities="false"), "EntityRef", entityRef="hero")
    event = _sub(_sub(group, "Maneuver", name="ego_cruise"), "Event", name="ego_cruise_event", priority="overwrite")
    _speed_action(_sub(event, "Action", name="ego_cruise_speed"), "$ego_speed", shape="linear", rate="1.0")
    _time_trigger(event, "StartTrigger", "start", "0")
    for actor in actors:
        if actor["role"] != "lead_vehicle_stopping":
            continue
        group = _sub(act, "ManeuverGroup", name="%s_stop" % actor["name"], maximumExecutionCount="1")
        _sub(_sub(group, "Actors", selectTriggeringEntities="false"), "EntityRef", entityRef=actor["name"])
        event = _sub(_sub(group, "Maneuver", name="%s_brake" % actor["name"]), "Event", name="%s_brake_event" % actor["name"], priority="overwrite")
        _speed_action(_sub(event, "Action", name="%s_brake_to_stop" % actor["name"]), "0.0", shape="linear", rate="4.0")
        cond = _sub(_sub(_sub(event, "StartTrigger"), "ConditionGroup"), "Condition", name="%s_close" % actor["name"], delay="0", conditionEdge="rising")
        by_entity = _sub(cond, "ByEntityCondition")
        _sub(_sub(by_entity, "TriggeringEntities", triggeringEntitiesRule="any"), "EntityRef", entityRef="hero")
        _sub(_sub(by_entity, "EntityCondition"), "RelativeDistanceCondition", entityRef=actor["name"], relativeDistanceType="longitudinal",
             value="%.1f" % max(10.0, actor["distance"] * 0.6), freespace="true", rule="lessThan")
    _time_trigger(act, "StartTrigger", "act_start", "0")
    _time_trigger(storyboard, "StopTrigger", "timeout", "$scenario_duration", edge="rising")

    raw = ET.tostring(root, encoding="unicode")
    pretty = minidom.parseString(raw).toprettyxml(indent="  ", encoding="UTF-8").decode("utf-8")
    return pretty


def write_xosc(template: ScenarioTemplate, path: str, **kwargs: Any) -> str:
    text = scenario_to_xosc(template, **kwargs)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(text)
    return path
