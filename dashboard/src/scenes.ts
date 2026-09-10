/**
 * Synthetic demo scenes.
 *
 * The catalog tabs need a seeded database before they show anything. These
 * scenes need nothing: they are generated from a fixed seed, so the dashboard
 * can demonstrate the deck.gl scene viewer — ego trajectory, actor tracks,
 * lidar returns, detections — on an empty deployment, and so screenshots in
 * partner reports are reproducible byte for byte.
 *
 * The four scenes are deliberately placed in the ODD cells that the coverage
 * matrix (src/coverage.ts) reports as gaps for a young catalog: mountain
 * hairpins, monsoon rain, and night fog. Each carries the same
 * visibility/lighting/route classes the scenario library uses, so a scene can
 * be read against the matrix directly.
 *
 * Everything here is pure and deterministic — no D1, no clock, no Math.random —
 * so it unit-tests directly and two callers always get the same bytes.
 */

export type ActorClass = "car" | "truck" | "bus" | "motorcycle" | "cyclist" | "pedestrian";

export interface Vec2 {
  x: number;
  y: number;
}

/** A pose on the local metric grid; heading is radians CCW from +x. */
export interface Pose {
  t: number;
  x: number;
  y: number;
  heading: number;
  speed_mps: number;
}

export interface SceneActor {
  actor_id: string;
  class: ActorClass;
  length_m: number;
  width_m: number;
  height_m: number;
  /** Pose per frame; index i is t = i / hz. */
  track: Pose[];
}

export interface SceneEvent {
  t: number;
  class: string;
  severity: "info" | "warning" | "critical";
  description: string;
}

/**
 * Sensor envelope for the scene's conditions. Rain and fog cut usable lidar
 * range hard, which is the whole point of holding these ODD cells: an actor
 * outside `lidar_range_m` is not detected, and the viewer draws it as missed.
 */
export interface SensorModel {
  lidar_range_m: number;
  /** Returns synthesised per frame by the client from the scene geometry. */
  lidar_points_per_frame: number;
  /** Std. dev. of range noise, metres. */
  noise_m: number;
}

export interface RoadGeometry {
  /** Centreline in local metres, 2 m apart. */
  centerline: Vec2[];
  lane_width_m: number;
  lanes: number;
}

export interface Scene {
  scene_id: string;
  name: string;
  description: string;
  odd: { visibility_class: string; lighting_class: string; route_class: string };
  sensor: SensorModel;
  hz: number;
  duration_s: number;
  /** Local metric grid is anchored here so a scene can be put on a basemap. */
  origin: { lat: number; lon: number };
  road: RoadGeometry;
  ego: { length_m: number; width_m: number; height_m: number; track: Pose[] };
  actors: SceneActor[];
  events: SceneEvent[];
  seed: number;
}

export interface SceneSummary {
  scene_id: string;
  name: string;
  description: string;
  odd: Scene["odd"];
  duration_s: number;
  hz: number;
  actors: number;
  events: number;
  lidar_range_m: number;
}

// ------------------------------------------------------------------ geometry

/** One stretch of road: a length and a constant curvature (1/radius, m^-1). */
interface Segment {
  length_m: number;
  curvature: number;
}

const STEP_M = 2;

/**
 * Integrate a curvature profile into a centreline. Constant-curvature segments
 * are how road geometry is described in OpenDRIVE, so a scene defined this way
 * can be exported to a simulator later without being re-authored.
 */
export function centerlineFrom(segments: Segment[], step = STEP_M): Vec2[] {
  let x = 0;
  let y = 0;
  let heading = 0;
  const points: Vec2[] = [{ x, y }];
  for (const segment of segments) {
    const n = Math.max(1, Math.round(segment.length_m / step));
    const ds = segment.length_m / n;
    for (let i = 0; i < n; i += 1) {
      heading += segment.curvature * ds;
      x += Math.cos(heading) * ds;
      y += Math.sin(heading) * ds;
      points.push({ x: round(x), y: round(y) });
    }
  }
  return points;
}

const round = (n: number, digits = 3): number => {
  const f = 10 ** digits;
  return Math.round(n * f) / f;
};

/** Cumulative arc length at each centreline vertex. */
function arcLengths(points: Vec2[]): number[] {
  const s = [0];
  for (let i = 1; i < points.length; i += 1) {
    s.push(s[i - 1] + Math.hypot(points[i].x - points[i - 1].x, points[i].y - points[i - 1].y));
  }
  return s;
}

/**
 * Pose at arc length `distance`, offset `lateral` metres to the left of the
 * centreline (right-hand traffic uses a negative offset). Distances past
 * either end clamp to the end pose so an actor can drive off-scene and stay put.
 */
export function poseOnPath(points: Vec2[], cum: number[], distance: number, lateral = 0): { x: number; y: number; heading: number } {
  const total = cum[cum.length - 1];
  const d = Math.min(Math.max(distance, 0), total);
  let i = 1;
  while (i < cum.length - 1 && cum[i] < d) i += 1;
  const span = cum[i] - cum[i - 1] || 1;
  const f = (d - cum[i - 1]) / span;
  const a = points[i - 1];
  const b = points[i];
  const heading = Math.atan2(b.y - a.y, b.x - a.x);
  return {
    x: a.x + (b.x - a.x) * f - Math.sin(heading) * lateral,
    y: a.y + (b.y - a.y) * f + Math.cos(heading) * lateral,
    heading,
  };
}

// -------------------------------------------------------------------- actors

/** How an actor moves relative to the road, resolved into a track per frame. */
interface ActorSpec {
  actor_id: string;
  class: ActorClass;
  /** Arc length at t = 0. */
  start_m: number;
  /** Metres left of the centreline; negative is the oncoming side. */
  lateral_m: number;
  speed_mps: number;
  /** -1 drives against the ego. */
  direction?: 1 | -1;
  /**
   * Lateral metres per second — a pedestrian stepping into the road, or a
   * truck drifting across the centreline on a hairpin.
   */
  lateral_rate_mps?: number;
  /** Lateral drift only runs inside this window. */
  lateral_window?: [number, number];
}

const DIMENSIONS: Record<ActorClass, { length_m: number; width_m: number; height_m: number }> = {
  car: { length_m: 4.4, width_m: 1.8, height_m: 1.5 },
  truck: { length_m: 9.5, width_m: 2.5, height_m: 3.4 },
  bus: { length_m: 11, width_m: 2.55, height_m: 3.2 },
  motorcycle: { length_m: 2.1, width_m: 0.8, height_m: 1.4 },
  cyclist: { length_m: 1.8, width_m: 0.7, height_m: 1.7 },
  pedestrian: { length_m: 0.6, width_m: 0.6, height_m: 1.7 },
};

function buildActor(spec: ActorSpec, points: Vec2[], cum: number[], frames: number, hz: number): SceneActor {
  const dir = spec.direction ?? 1;
  const track: Pose[] = [];
  for (let i = 0; i < frames; i += 1) {
    const t = round(i / hz, 3);
    let lateral = spec.lateral_m;
    if (spec.lateral_rate_mps) {
      const [from, to] = spec.lateral_window ?? [0, frames / hz];
      const active = Math.min(Math.max(t, from), to) - from;
      lateral += spec.lateral_rate_mps * active;
    }
    const pose = poseOnPath(points, cum, spec.start_m + dir * spec.speed_mps * t, lateral);
    track.push({
      t,
      x: round(pose.x),
      y: round(pose.y),
      // An oncoming actor faces back down the road.
      heading: round(dir === 1 ? pose.heading : pose.heading + Math.PI, 4),
      speed_mps: spec.speed_mps,
    });
  }
  return { actor_id: spec.actor_id, class: spec.class, ...DIMENSIONS[spec.class], track };
}

// -------------------------------------------------------------------- scenes

/** A scene before its tracks are integrated. */
interface SceneSpec {
  scene_id: string;
  name: string;
  description: string;
  odd: Scene["odd"];
  sensor: SensorModel;
  duration_s: number;
  origin: { lat: number; lon: number };
  lanes: number;
  lane_width_m: number;
  segments: Segment[];
  /** Ego arc length at t = 0. */
  ego_start_m: number;
  ego_lateral_m: number;
  /** Ego speed in m/s at time t — where the braking events live. */
  speed: (t: number) => number;
  actors: ActorSpec[];
  events: SceneEvent[];
  seed: number;
}

const HZ = 10;

/** Smooth 0→1 ramp, so speed changes look like braking rather than a step. */
const ramp = (t: number, from: number, to: number): number => {
  if (t <= from) return 0;
  if (t >= to) return 1;
  const f = (t - from) / (to - from);
  return f * f * (3 - 2 * f);
};

/** Slow from `v0` to `v1` between `from` and `to`, then hold. */
const brake = (t: number, v0: number, v1: number, from: number, to: number): number => v0 + (v1 - v0) * ramp(t, from, to);

const SPECS: SceneSpec[] = [
  {
    scene_id: "thimphu_junction",
    name: "Thimphu junction · unsignalised crossing",
    description:
      "Urban approach to an unsignalised junction in clear daylight. A pedestrian steps off the kerb from behind a stopped bus, which is the occlusion case the perception benchmark is scored on.",
    odd: { visibility_class: "clear", lighting_class: "daylight", route_class: "urban_junction" },
    sensor: { lidar_range_m: 80, lidar_points_per_frame: 4200, noise_m: 0.02 },
    duration_s: 24,
    origin: { lat: 27.4712, lon: 89.639 },
    lanes: 2,
    lane_width_m: 3.25,
    segments: [
      { length_m: 120, curvature: 0 },
      { length_m: 60, curvature: 0.004 },
      { length_m: 120, curvature: 0 },
    ],
    ego_start_m: 10,
    ego_lateral_m: -1.6,
    // Cruise at 11 m/s, brake to 2 m/s for the pedestrian at t ≈ 11 s.
    speed: (t) => brake(t, 11, 2, 10.5, 13.5),
    actors: [
      // Parked in the layby at 144 m; its box sits on the ego's line of sight to ped-05.
      { actor_id: "bus-01", class: "bus", start_m: 144, lateral_m: -4, speed_mps: 0 },
      { actor_id: "car-02", class: "car", start_m: 240, lateral_m: 1.6, speed_mps: 8, direction: -1 },
      { actor_id: "car-03", class: "car", start_m: 40, lateral_m: -1.6, speed_mps: 9 },
      { actor_id: "moto-04", class: "motorcycle", start_m: 200, lateral_m: 1.6, speed_mps: 10, direction: -1 },
      // Steps off the kerb behind the bus at t = 9 s and crosses to the far side.
      { actor_id: "ped-05", class: "pedestrian", start_m: 152, lateral_m: -6, speed_mps: 0, lateral_rate_mps: 1.2, lateral_window: [9, 16] },
    ],
    events: [
      { t: 9.0, class: "occlusion", severity: "info", description: "Pedestrian steps off the kerb 43.8 m ahead, behind the parked bus" },
      { t: 11.3, class: "vru_conflict", severity: "critical", description: "Pedestrian crosses into the ego lane at 18.4 m" },
      { t: 12.0, class: "hard_brake", severity: "warning", description: "Deceleration 4.5 m/s^2" },
      { t: 16.9, class: "vru_pass", severity: "info", description: "Closest approach to ped-05: 4.0 m" },
    ],
    seed: 1041,
  },
  {
    scene_id: "dochula_switchback",
    name: "Dochula pass · hairpin with oncoming truck",
    description:
      "Mountain hairpin in clear daylight. An oncoming truck cuts the apex and crosses the centreline into the ego's lane — the geometry that makes hairpins the highest-value cell in the coverage matrix.",
    odd: { visibility_class: "clear", lighting_class: "daylight", route_class: "hairpin_mountain" },
    sensor: { lidar_range_m: 70, lidar_points_per_frame: 3800, noise_m: 0.03 },
    duration_s: 26,
    origin: { lat: 27.4906, lon: 89.7486 },
    lanes: 2,
    lane_width_m: 3,
    segments: [
      { length_m: 70, curvature: 0 },
      // ~18 m radius hairpin, the tightest geometry on the Thimphu–Punakha road.
      { length_m: 56, curvature: 0.055 },
      { length_m: 90, curvature: -0.008 },
    ],
    ego_start_m: 5,
    ego_lateral_m: -1.5,
    // Slow for the hairpin at t = 5-9 s, then again for the truck in the apex.
    speed: (t) => brake(t, 9, 4.5, 5, 9) - 2 * ramp(t, 13.5, 15.5),
    actors: [
      // Drifts 1.9 m across the centreline through the apex, where the ego meets it.
      { actor_id: "truck-01", class: "truck", start_m: 190, lateral_m: 1.5, speed_mps: 5.5, direction: -1, lateral_rate_mps: -0.6, lateral_window: [11, 16] },
      { actor_id: "car-02", class: "car", start_m: 30, lateral_m: -1.5, speed_mps: 8 },
      { actor_id: "cyclist-03", class: "cyclist", start_m: 40, lateral_m: -2.9, speed_mps: 3.2 },
    ],
    events: [
      { t: 6.1, class: "vru_pass", severity: "info", description: "Passing cyclist-03 with 1.40 m clearance" },
      { t: 6.7, class: "speed_reduction", severity: "info", description: "Slowing for the hairpin; deceleration 1.7 m/s^2" },
      { t: 13.5, class: "centreline_incursion", severity: "critical", description: "Oncoming truck crosses the centreline in the apex" },
      { t: 15.6, class: "near_miss", severity: "critical", description: "Closest approach to truck-01: 0.38 m, with the truck 1.26 m over the centreline" },
    ],
    seed: 2277,
  },
  {
    scene_id: "monsoon_descent",
    name: "Monsoon descent · standing water",
    description:
      "Heavy monsoon rain on a mountain descent. Spray and standing water cut lidar returns and the intensity channel collapses — the sensor-degradation case the evaluation tab reports recall against.",
    odd: { visibility_class: "heavy_rain", lighting_class: "daylight", route_class: "curve_mountain" },
    sensor: { lidar_range_m: 38, lidar_points_per_frame: 2200, noise_m: 0.12 },
    duration_s: 28,
    origin: { lat: 27.0975, lon: 89.8774 },
    lanes: 2,
    lane_width_m: 3,
    segments: [
      { length_m: 90, curvature: -0.012 },
      { length_m: 80, curvature: 0.02 },
      { length_m: 90, curvature: -0.006 },
    ],
    ego_start_m: 8,
    ego_lateral_m: -1.5,
    speed: (t) => brake(t, 10, 6, 12, 16),
    actors: [
      { actor_id: "truck-01", class: "truck", start_m: 60, lateral_m: -1.5, speed_mps: 7.5 },
      { actor_id: "car-02", class: "car", start_m: 210, lateral_m: 1.5, speed_mps: 9, direction: -1 },
      { actor_id: "car-03", class: "car", start_m: 175, lateral_m: 1.5, speed_mps: 8.5, direction: -1 },
    ],
    events: [
      { t: 5.5, class: "sensor_degradation", severity: "warning", description: "Lead truck acquired at 38 m — the usable lidar range in this rain" },
      { t: 9.0, class: "oncoming_pass", severity: "info", description: "Oncoming car-03 passes at 3.0 m" },
      { t: 13.9, class: "hydroplane", severity: "critical", description: "Standing water; deceleration 1.5 m/s^2 with 0.4 s of front-axle slip" },
      { t: 14.3, class: "spray_occlusion", severity: "warning", description: "Following truck-01 at 18.9 m in heavy spray" },
    ],
    seed: 3319,
  },
  {
    scene_id: "night_fog_pass",
    name: "Night fog · unlit mountain road",
    description:
      "Unlit road in dense night fog. Usable lidar range drops to 22 m, so the roadside pedestrian is not acquired until 21.9 m out and the ego is still braking as it passes: the worst cell in the matrix and the reason the night/fog row is held open.",
    odd: { visibility_class: "fog", lighting_class: "night", route_class: "straight_mountain" },
    sensor: { lidar_range_m: 22, lidar_points_per_frame: 1400, noise_m: 0.18 },
    duration_s: 26,
    origin: { lat: 27.2894, lon: 89.6421 },
    lanes: 2,
    lane_width_m: 2.9,
    segments: [
      { length_m: 130, curvature: 0.003 },
      { length_m: 120, curvature: -0.004 },
    ],
    ego_start_m: 6,
    ego_lateral_m: -1.45,
    speed: (t) => brake(t, 8, 3, 14, 17),
    actors: [
      { actor_id: "ped-01", class: "pedestrian", start_m: 128, lateral_m: -3.1, speed_mps: 0.9 },
      { actor_id: "truck-02", class: "truck", start_m: 235, lateral_m: 1.45, speed_mps: 7, direction: -1 },
      { actor_id: "car-03", class: "car", start_m: 34, lateral_m: -1.45, speed_mps: 7.5 },
    ],
    events: [
      { t: 3.0, class: "sensor_degradation", severity: "critical", description: "Fog; usable lidar range 22 m, camera unusable" },
      { t: 12.2, class: "late_detection", severity: "warning", description: "Lead car-03 acquired at 22.0 m" },
      { t: 14.1, class: "late_detection", severity: "critical", description: "Roadside pedestrian first detected at 21.9 m" },
      { t: 15.4, class: "hard_brake", severity: "warning", description: "Deceleration 2.5 m/s^2" },
      { t: 21.2, class: "vru_pass", severity: "critical", description: "Passing ped-01 with 1.65 m clearance" },
    ],
    seed: 4903,
  },
];

const EGO_DIMENSIONS = { length_m: 4.7, width_m: 1.9, height_m: 1.6 };

/** Integrate a spec into a full scene: ego track, actor tracks and geometry. */
function build(spec: SceneSpec): Scene {
  const centerline = centerlineFrom(spec.segments);
  const cum = arcLengths(centerline);
  const frames = Math.round(spec.duration_s * HZ) + 1;

  // Integrate the speed profile so the ego's distance matches its speed; a
  // trapezoid keeps the two consistent to within a frame.
  const track: Pose[] = [];
  let distance = spec.ego_start_m;
  let previous = spec.speed(0);
  for (let i = 0; i < frames; i += 1) {
    const t = round(i / HZ, 3);
    const speed = spec.speed(t);
    if (i > 0) distance += ((speed + previous) / 2) * (1 / HZ);
    previous = speed;
    const pose = poseOnPath(centerline, cum, distance, spec.ego_lateral_m);
    track.push({ t, x: round(pose.x), y: round(pose.y), heading: round(pose.heading, 4), speed_mps: round(speed, 2) });
  }

  return {
    scene_id: spec.scene_id,
    name: spec.name,
    description: spec.description,
    odd: spec.odd,
    sensor: spec.sensor,
    hz: HZ,
    duration_s: spec.duration_s,
    origin: spec.origin,
    road: { centerline, lane_width_m: spec.lane_width_m, lanes: spec.lanes },
    ego: { ...EGO_DIMENSIONS, track },
    actors: spec.actors.map((a) => buildActor(a, centerline, cum, frames, HZ)),
    events: spec.events,
    seed: spec.seed,
  };
}

/** Scene ids in catalog order. */
export const SCENE_IDS: string[] = SPECS.map((s) => s.scene_id);

/** Build one scene by id, or null when the id is unknown. */
export function buildScene(sceneId: string): Scene | null {
  const spec = SPECS.find((s) => s.scene_id === sceneId);
  return spec ? build(spec) : null;
}

/** The catalog, without the per-frame tracks — what the picker lists. */
export function listScenes(): SceneSummary[] {
  return SPECS.map((spec) => ({
    scene_id: spec.scene_id,
    name: spec.name,
    description: spec.description,
    odd: spec.odd,
    duration_s: spec.duration_s,
    hz: HZ,
    actors: spec.actors.length,
    events: spec.events.length,
    lidar_range_m: spec.sensor.lidar_range_m,
  }));
}

/**
 * Straight-line range from the ego to an actor at a frame. The viewer marks an
 * actor missed when this exceeds the scene's lidar range, which is what makes
 * the fog and rain scenes look different from the clear ones.
 */
export function rangeAt(scene: Scene, actor: SceneActor, frame: number): number {
  const e = scene.ego.track[Math.min(frame, scene.ego.track.length - 1)];
  const a = actor.track[Math.min(frame, actor.track.length - 1)];
  return round(Math.hypot(a.x - e.x, a.y - e.y), 2);
}
