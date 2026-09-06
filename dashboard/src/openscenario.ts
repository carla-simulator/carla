/**
 * ASAM OpenSCENARIO 1.2 export of a scenario template.
 *
 * The output runs in CARLA ScenarioRunner (`scenario_runner.py --openscenario
 * file.xosc`), esmini and other OpenSCENARIO players. Road geometry is not
 * part of a template, so positions are expressed relative to the ego lane
 * and the road, lane and start offset are parameters the player can override
 * (`--openscenarioparams` in ScenarioRunner, `--param` in esmini).
 *
 * toolkit/bhutan_sim/openscenario.py produces the same document.
 */

export interface XoscScenario {
  scenario_id: string;
  family: string;
  name: string;
  description?: string | null;
  version?: string | null;
  content_hash?: string | null;
  params: Record<string, unknown>;
  actors: Array<Record<string, unknown>>;
  expected_events?: string[];
}

interface Weather {
  cloudState: string;
  precipitationType: string;
  precipitationIntensity: number;
  fogVisualRange: number;
  sunElevation: number;
  sunAzimuth: number;
  friction: number;
}

const HOUR_BY_TIME: Record<string, string> = { dawn: "06:15:00", day: "12:00:00", dusk: "18:15:00", night: "22:00:00" };
const ELEVATION_BY_TIME: Record<string, number> = { dawn: 8, day: 60, dusk: 8, night: -30 };
const AZIMUTH_BY_TIME: Record<string, number> = { dawn: 90, day: 150, dusk: 270, night: 0 };

// Mirrors bhutan_sim/weather.py presets, reduced to what OpenSCENARIO expresses.
const PRESETS: Record<string, Partial<Weather>> = {
  clear_day: { cloudState: "free", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, friction: 1.0 },
  overcast: { cloudState: "overcast", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, friction: 1.0 },
  light_rain: { cloudState: "overcast", precipitationType: "rain", precipitationIntensity: 0.3, fogVisualRange: 100000, friction: 0.8 },
  heavy_rain: { cloudState: "rainy", precipitationType: "rain", precipitationIntensity: 0.8, fogVisualRange: 400, friction: 0.65 },
  monsoon_storm: { cloudState: "rainy", precipitationType: "rain", precipitationIntensity: 1.0, fogVisualRange: 300, friction: 0.55 },
  wet_road: { cloudState: "cloudy", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, friction: 0.7 },
  valley_fog: { cloudState: "cloudy", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 80, friction: 0.9 },
  dense_fog: { cloudState: "overcast", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 30, friction: 0.9 },
  low_sun: { cloudState: "free", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, sunElevation: 6, sunAzimuth: 0, friction: 1.0 },
  low_sun_haze: { cloudState: "free", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 600, sunElevation: 4, sunAzimuth: 0, friction: 1.0 },
  clear_night: { cloudState: "free", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, sunElevation: -30, friction: 1.0 },
  wet_night: { cloudState: "overcast", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 100000, sunElevation: -30, friction: 0.7 },
  dust_haze: { cloudState: "cloudy", precipitationType: "dry", precipitationIntensity: 0, fogVisualRange: 800, friction: 1.0 },
};

function resolveWeather(preset: string, timeOfDay: string): Weather {
  const base = PRESETS[preset] || PRESETS.clear_day;
  return {
    cloudState: base.cloudState || "free",
    precipitationType: base.precipitationType || "dry",
    precipitationIntensity: base.precipitationIntensity ?? 0,
    fogVisualRange: base.fogVisualRange ?? 100000,
    sunElevation: base.sunElevation ?? ELEVATION_BY_TIME[timeOfDay] ?? 60,
    sunAzimuth: base.sunAzimuth ?? AZIMUTH_BY_TIME[timeOfDay] ?? 150,
    friction: base.friction ?? 1.0,
  };
}

interface EntityDef {
  kind: "vehicle" | "pedestrian" | "misc";
  category: string;
  model: string;
  length: number;
  width: number;
  height: number;
  mass: number;
}

const ROLE_ENTITIES: Record<string, EntityDef> = {
  oncoming_truck: { kind: "vehicle", category: "truck", model: "vehicle.carlamotors.carlacola", length: 8.0, width: 2.5, height: 3.4, mass: 12000 },
  oncoming_vehicle: { kind: "vehicle", category: "car", model: "vehicle.tesla.model3", length: 4.7, width: 1.9, height: 1.5, mass: 1800 },
  lead_vehicle: { kind: "vehicle", category: "car", model: "vehicle.toyota.prius", length: 4.5, width: 1.8, height: 1.5, mass: 1500 },
  lead_vehicle_stopping: { kind: "vehicle", category: "car", model: "vehicle.toyota.prius", length: 4.5, width: 1.8, height: 1.5, mass: 1500 },
  following_vehicle: { kind: "vehicle", category: "car", model: "vehicle.audi.tt", length: 4.2, width: 1.8, height: 1.4, mass: 1400 },
  stalled_vehicle: { kind: "vehicle", category: "truck", model: "vehicle.carlamotors.carlacola", length: 8.0, width: 2.5, height: 3.4, mass: 12000 },
  parked_vehicle: { kind: "vehicle", category: "car", model: "vehicle.nissan.patrol", length: 4.9, width: 1.9, height: 1.9, mass: 2200 },
  motorcycle: { kind: "vehicle", category: "motorbike", model: "vehicle.yamaha.yzf", length: 2.1, width: 0.8, height: 1.2, mass: 200 },
  pedestrian: { kind: "pedestrian", category: "pedestrian", model: "walker.pedestrian.0001", length: 0.5, width: 0.6, height: 1.8, mass: 75 },
  pedestrian_crossing: { kind: "pedestrian", category: "pedestrian", model: "walker.pedestrian.0002", length: 0.5, width: 0.6, height: 1.8, mass: 75 },
  worker: { kind: "pedestrian", category: "pedestrian", model: "walker.pedestrian.0010", length: 0.5, width: 0.6, height: 1.8, mass: 80 },
  livestock: { kind: "pedestrian", category: "animal", model: "walker.pedestrian.0001", length: 2.0, width: 0.8, height: 1.4, mass: 350 },
  debris: { kind: "misc", category: "obstacle", model: "static.prop.rock", length: 1.5, width: 1.5, height: 0.8, mass: 900 },
  cones: { kind: "misc", category: "obstacle", model: "static.prop.trafficcone01", length: 0.4, width: 0.4, height: 0.7, mass: 3 },
};

const DEFAULT_ENTITY: EntityDef = { kind: "vehicle", category: "car", model: "vehicle.tesla.model3", length: 4.7, width: 1.9, height: 1.5, mass: 1800 };

const EGO_MODELS: Record<string, { category: string; model: string; length: number; width: number; height: number; mass: number }> = {
  truck: { category: "truck", model: "vehicle.carlamotors.carlacola", length: 8.0, width: 2.5, height: 3.4, mass: 12000 },
  shuttle: { category: "bus", model: "vehicle.mitsubishi.fusorosa", length: 7.0, width: 2.3, height: 2.9, mass: 6000 },
  car: { category: "car", model: "vehicle.tesla.model3", length: 4.7, width: 1.9, height: 1.5, mass: 1800 },
};

/** Lane offset relative to the ego lane; "opposite" lanes use the $oncoming_dlane parameter (+1 for right-hand traffic). */
function laneExpr(lane: string): string {
  switch (lane) {
    case "opposite":
    case "opposite_encroaching":
      return "$oncoming_dlane";
    case "shoulder":
    case "crossing":
    case "ego":
    case "ego_behind":
    default:
      return "0";
  }
}

function lateralOffset(lane: string, lateral: number, laneWidth: number): number {
  if (lane === "shoulder") return -(laneWidth / 2 + 0.5) + lateral;
  if (lane === "opposite_encroaching") return -(laneWidth * 0.35) + lateral;
  if (lane === "crossing") return laneWidth / 2 + lateral;
  return lateral;
}

function esc(value: unknown): string {
  return String(value ?? "").replace(/[<>&"']/g, (c) => ({ "<": "&lt;", ">": "&gt;", "&": "&amp;", '"': "&quot;", "'": "&apos;" }[c] as string));
}

function num(value: unknown, fallback: number): number {
  const n = Number(value);
  return Number.isFinite(n) ? n : fallback;
}

function safeName(value: string): string {
  return value.replace(/[^A-Za-z0-9_]/g, "_");
}

export function scenarioToXosc(s: XoscScenario, generatedAt = new Date()): string {
  const p = s.params || {};
  const vehicleClass = String(p.vehicle_class || "truck");
  const ego = EGO_MODELS[vehicleClass] || EGO_MODELS.car;
  const speedLimit = num(p.speed_limit_kph, 40);
  const laneWidth = num(p.lane_width_m, 3.3);
  const duration = num(p.duration_s, 60);
  const timeOfDay = String(p.time_of_day || "day");
  const weather = resolveWeather(String(p.weather_preset || "clear_day"), timeOfDay);
  const degradation = (p.sensor_degradation && typeof p.sensor_degradation === "object" ? p.sensor_degradation : {}) as Record<string, unknown>;
  const events = (s.expected_events || []).join(",");
  const date = generatedAt.toISOString().slice(0, 19);
  const dateTime = `${generatedAt.toISOString().slice(0, 10)}T${HOUR_BY_TIME[timeOfDay] || "12:00:00"}`;

  const lines: string[] = [];
  const out = (line: string) => lines.push(line);
  out('<?xml version="1.0" encoding="UTF-8"?>');
  out('<OpenSCENARIO xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="OpenScenario.xsd">');
  out(`  <FileHeader revMajor="1" revMinor="2" date="${date}" description="${esc(s.name)} [${esc(s.scenario_id)}] ${esc(s.description || "")}" author="Bhutan Mobility Atlas"/>`);
  out("  <ParameterDeclarations>");
  out('    <ParameterDeclaration name="road_id" parameterType="string" value="0"/>');
  out('    <ParameterDeclaration name="ego_lane_id" parameterType="integer" value="-1"/>');
  out('    <ParameterDeclaration name="ego_s" parameterType="double" value="20.0"/>');
  out('    <ParameterDeclaration name="oncoming_dlane" parameterType="integer" value="1"/>');
  out(`    <ParameterDeclaration name="ego_speed" parameterType="double" value="${(speedLimit / 3.6).toFixed(3)}"/>`);
  out(`    <ParameterDeclaration name="speed_limit_kph" parameterType="double" value="${speedLimit}"/>`);
  out(`    <ParameterDeclaration name="scenario_duration" parameterType="double" value="${duration}"/>`);
  out(`    <ParameterDeclaration name="road_curvature" parameterType="double" value="${num(p.road_curvature, 0)}"/>`);
  out(`    <ParameterDeclaration name="grade_pct" parameterType="double" value="${num(p.grade_pct, 0)}"/>`);
  out(`    <ParameterDeclaration name="lane_quality" parameterType="double" value="${num(p.lane_quality, 1)}"/>`);
  out(`    <ParameterDeclaration name="traffic_density" parameterType="double" value="${num(p.traffic_density, 0)}"/>`);
  out(`    <ParameterDeclaration name="payload_kg" parameterType="double" value="${num(p.payload_kg, 0)}"/>`);
  for (const [key, value] of Object.entries(degradation)) {
    out(`    <ParameterDeclaration name="sensor_${esc(key)}" parameterType="double" value="${num(value, 0)}"/>`);
  }
  out(`    <ParameterDeclaration name="atlas_family" parameterType="string" value="${esc(s.family)}"/>`);
  out(`    <ParameterDeclaration name="atlas_content_hash" parameterType="string" value="${esc(s.content_hash || "")}"/>`);
  out(`    <ParameterDeclaration name="atlas_expected_events" parameterType="string" value="${esc(events)}"/>`);
  out("  </ParameterDeclarations>");
  out("  <CatalogLocations/>");
  out("  <RoadNetwork>");
  out(`    <LogicFile filepath="${esc(String(p.map_name || "Town10HD_Opt"))}"/>`);
  out("  </RoadNetwork>");
  out("  <Entities>");
  out('    <ScenarioObject name="hero">');
  out(`      <Vehicle name="${ego.model}" vehicleCategory="${ego.category}" mass="${ego.mass + num(p.payload_kg, 0)}">`);
  out("        <ParameterDeclarations/>");
  out(`        <Performance maxSpeed="${(speedLimit / 3.6 * 1.5).toFixed(2)}" maxAcceleration="3.0" maxDeceleration="8.0"/>`);
  out(`        <BoundingBox><Center x="${(ego.length / 2).toFixed(2)}" y="0.0" z="${(ego.height / 2).toFixed(2)}"/><Dimensions width="${ego.width}" length="${ego.length}" height="${ego.height}"/></BoundingBox>`);
  out(`        <Axles><FrontAxle maxSteering="0.5" wheelDiameter="0.9" trackWidth="${ego.width}" positionX="${(ego.length * 0.75).toFixed(2)}" positionZ="0.45"/><RearAxle maxSteering="0.0" wheelDiameter="0.9" trackWidth="${ego.width}" positionX="0.0" positionZ="0.45"/></Axles>`);
  out('        <Properties><Property name="type" value="ego_vehicle"/><Property name="color" value="255,255,255"/></Properties>');
  out("      </Vehicle>");
  out("    </ScenarioObject>");
  const actors = (s.actors || []).map((a, i) => {
    const role = String(a.role || "vehicle");
    const def = ROLE_ENTITIES[role] || DEFAULT_ENTITY;
    return { name: `${safeName(role)}_${i + 1}`, role, def, lane: String(a.lane || "ego"), distance: num(a.distance_m, 50), lateral: num(a.lateral_m, 0), speed: a.speed_mps === null || a.speed_mps === undefined ? null : num(a.speed_mps, 0) };
  });
  for (const a of actors) {
    out(`    <ScenarioObject name="${a.name}">`);
    const bbox = `<BoundingBox><Center x="${(a.def.length / 2).toFixed(2)}" y="0.0" z="${(a.def.height / 2).toFixed(2)}"/><Dimensions width="${a.def.width}" length="${a.def.length}" height="${a.def.height}"/></BoundingBox>`;
    if (a.def.kind === "vehicle") {
      out(`      <Vehicle name="${a.def.model}" vehicleCategory="${a.def.category}" mass="${a.def.mass}">`);
      out("        <ParameterDeclarations/>");
      out('        <Performance maxSpeed="40.0" maxAcceleration="3.0" maxDeceleration="8.0"/>');
      out(`        ${bbox}`);
      out(`        <Axles><FrontAxle maxSteering="0.5" wheelDiameter="0.7" trackWidth="${a.def.width}" positionX="${(a.def.length * 0.75).toFixed(2)}" positionZ="0.35"/><RearAxle maxSteering="0.0" wheelDiameter="0.7" trackWidth="${a.def.width}" positionX="0.0" positionZ="0.35"/></Axles>`);
      out(`        <Properties><Property name="type" value="simulation"/><Property name="atlas_role" value="${esc(a.role)}"/></Properties>`);
      out("      </Vehicle>");
    } else if (a.def.kind === "pedestrian") {
      out(`      <Pedestrian name="${a.def.model}" model="${a.def.model}" mass="${a.def.mass}" pedestrianCategory="${a.def.category}">`);
      out("        <ParameterDeclarations/>");
      out(`        ${bbox}`);
      out(`        <Properties><Property name="type" value="simulation"/><Property name="atlas_role" value="${esc(a.role)}"/></Properties>`);
      out("      </Pedestrian>");
    } else {
      out(`      <MiscObject name="${a.def.model}" mass="${a.def.mass}" miscObjectCategory="${a.def.category}">`);
      out("        <ParameterDeclarations/>");
      out(`        ${bbox}`);
      out(`        <Properties><Property name="type" value="simulation"/><Property name="atlas_role" value="${esc(a.role)}"/></Properties>`);
      out("      </MiscObject>");
    }
    out("    </ScenarioObject>");
  }
  out("  </Entities>");
  out("  <Storyboard>");
  out("    <Init>");
  out("      <Actions>");
  out("        <GlobalAction>");
  out("          <EnvironmentAction>");
  out(`            <Environment name="${esc(String(p.weather_preset || "clear_day"))}_${esc(timeOfDay)}">`);
  out(`              <TimeOfDay animation="false" dateTime="${dateTime}"/>`);
  out(`              <Weather cloudState="${weather.cloudState}">`);
  out(`                <Sun intensity="${weather.sunElevation > 0 ? "1.0" : "0.05"}" azimuth="${(weather.sunAzimuth * Math.PI / 180).toFixed(4)}" elevation="${(weather.sunElevation * Math.PI / 180).toFixed(4)}"/>`);
  out(`                <Fog visualRange="${weather.fogVisualRange}"/>`);
  out(`                <Precipitation precipitationType="${weather.precipitationType}" intensity="${weather.precipitationIntensity}"/>`);
  out("              </Weather>");
  out(`              <RoadCondition frictionScaleFactor="${weather.friction}"/>`);
  out("            </Environment>");
  out("          </EnvironmentAction>");
  out("        </GlobalAction>");
  out('        <Private entityRef="hero">');
  out('          <PrivateAction><TeleportAction><Position><LanePosition roadId="$road_id" laneId="$ego_lane_id" offset="0.0" s="$ego_s"/></Position></TeleportAction></PrivateAction>');
  out('          <PrivateAction><LongitudinalAction><SpeedAction><SpeedActionDynamics dynamicsShape="step" value="0" dynamicsDimension="time"/><SpeedActionTarget><AbsoluteTargetSpeed value="$ego_speed"/></SpeedActionTarget></SpeedAction></LongitudinalAction></PrivateAction>');
  out("        </Private>");
  for (const a of actors) {
    const ds = a.lane === "ego_behind" ? -Math.abs(a.distance) : a.distance;
    const offset = lateralOffset(a.lane, a.lateral, laneWidth);
    out(`        <Private entityRef="${a.name}">`);
    out(`          <PrivateAction><TeleportAction><Position><RelativeLanePosition entityRef="hero" dLane="${laneExpr(a.lane)}" ds="${ds.toFixed(2)}" offset="${offset.toFixed(2)}"/></Position></TeleportAction></PrivateAction>`);
    if (a.speed !== null && a.def.kind !== "misc") {
      out(`          <PrivateAction><LongitudinalAction><SpeedAction><SpeedActionDynamics dynamicsShape="step" value="0" dynamicsDimension="time"/><SpeedActionTarget><AbsoluteTargetSpeed value="${a.speed.toFixed(2)}"/></SpeedActionTarget></SpeedAction></LongitudinalAction></PrivateAction>`);
    }
    out("        </Private>");
  }
  out("      </Actions>");
  out("    </Init>");
  out(`    <Story name="${safeName(s.scenario_id)}">`);
  out("      <ParameterDeclarations/>");
  out(`      <Act name="${safeName(s.family)}">`);
  out('        <ManeuverGroup name="ego_drive" maximumExecutionCount="1">');
  out('          <Actors selectTriggeringEntities="false"><EntityRef entityRef="hero"/></Actors>');
  out('          <Maneuver name="ego_cruise">');
  out('            <Event name="ego_cruise_event" priority="overwrite">');
  out('              <Action name="ego_cruise_speed"><PrivateAction><LongitudinalAction><SpeedAction><SpeedActionDynamics dynamicsShape="linear" value="1.0" dynamicsDimension="rate"/><SpeedActionTarget><AbsoluteTargetSpeed value="$ego_speed"/></SpeedActionTarget></SpeedAction></LongitudinalAction></PrivateAction></Action>');
  out('              <StartTrigger><ConditionGroup><Condition name="start" delay="0" conditionEdge="none"><ByValueCondition><SimulationTimeCondition value="0" rule="greaterThan"/></ByValueCondition></Condition></ConditionGroup></StartTrigger>');
  out("            </Event>");
  out("          </Maneuver>");
  out("        </ManeuverGroup>");
  const stoppers = actors.filter((a) => a.role === "lead_vehicle_stopping");
  for (const a of stoppers) {
    out(`        <ManeuverGroup name="${a.name}_stop" maximumExecutionCount="1">`);
    out(`          <Actors selectTriggeringEntities="false"><EntityRef entityRef="${a.name}"/></Actors>`);
    out(`          <Maneuver name="${a.name}_brake">`);
    out(`            <Event name="${a.name}_brake_event" priority="overwrite">`);
    out(`              <Action name="${a.name}_brake_to_stop"><PrivateAction><LongitudinalAction><SpeedAction><SpeedActionDynamics dynamicsShape="linear" value="4.0" dynamicsDimension="rate"/><SpeedActionTarget><AbsoluteTargetSpeed value="0.0"/></SpeedActionTarget></SpeedAction></LongitudinalAction></PrivateAction></Action>`);
    out(`              <StartTrigger><ConditionGroup><Condition name="${a.name}_close" delay="0" conditionEdge="rising"><ByEntityCondition><TriggeringEntities triggeringEntitiesRule="any"><EntityRef entityRef="hero"/></TriggeringEntities><EntityCondition><RelativeDistanceCondition entityRef="${a.name}" relativeDistanceType="longitudinal" value="${Math.max(10, a.distance * 0.6).toFixed(1)}" freespace="true" rule="lessThan"/></EntityCondition></ByEntityCondition></Condition></ConditionGroup></StartTrigger>`);
    out("            </Event>");
    out("          </Maneuver>");
    out("        </ManeuverGroup>");
  }
  out('        <StartTrigger><ConditionGroup><Condition name="act_start" delay="0" conditionEdge="none"><ByValueCondition><SimulationTimeCondition value="0" rule="greaterThan"/></ByValueCondition></Condition></ConditionGroup></StartTrigger>');
  out("      </Act>");
  out("    </Story>");
  out('    <StopTrigger><ConditionGroup><Condition name="timeout" delay="0" conditionEdge="rising"><ByValueCondition><SimulationTimeCondition value="$scenario_duration" rule="greaterThan"/></ByValueCondition></Condition></ConditionGroup></StopTrigger>');
  out("  </Storyboard>");
  out("</OpenSCENARIO>");
  return lines.join("\n") + "\n";
}
