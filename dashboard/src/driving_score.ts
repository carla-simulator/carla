/**
 * CARLA Leaderboard-style driving score.
 *
 * The Leaderboard (github.com/carla-simulator/leaderboard) scores a route as
 *   driving_score = route_completion * infraction_penalty
 * where infraction_penalty is the product of a per-infraction coefficient for
 * every infraction that occurred. The coefficients below use the Leaderboard
 * 2.0 values for the infractions that map onto Atlas event classes and add
 * documented Atlas-specific coefficients for the safety rules that have no
 * Leaderboard equivalent (near misses, downhill overspeed, hard braking).
 *
 * Keep this table identical to toolkit/bhutan_sim/driving_score.py.
 */

export interface ScoreEvent {
  event_class: string;
  severity?: string | null;
  description?: string | null;
  data?: unknown;
}

export interface Infraction {
  event_class: string;
  kind: string;
  count: number;
  penalty: number;
}

export interface DrivingScore {
  driving_score: number;
  route_completion: number;
  infraction_penalty: number;
  route_completion_basis: string;
  infractions: Infraction[];
}

/** Coefficient applied once per occurrence. */
export const PENALTIES: Record<string, number> = {
  collision_pedestrian: 0.5,   // Leaderboard 2.0
  collision_vehicle: 0.6,      // Leaderboard 2.0
  collision_static: 0.65,      // Leaderboard 2.0
  driver_intervention: 0.6,    // Atlas: a disengagement is treated like an agent-blocked event
  ttc_low: 0.8,                // Atlas: near miss
  downhill_overspeed: 0.8,     // Atlas: SR-06
  speeding: 0.9,               // Atlas: SR-02 (Leaderboard has a min-speed rule instead)
  lane_departure: 0.95,        // Atlas: SR-08 (Leaderboard folds this into off-road completion)
  hard_brake: 0.97,            // Atlas: SR-01
  lateral_accel_high: 0.97,    // Atlas: SR-04
  lead_vehicle_close: 0.97,    // Atlas: SR-07
};

const PEDESTRIAN_WORDS = /walker|pedestrian|person|human|cyclist|bicycle|animal|livestock|cattle|dog/i;
const VEHICLE_WORDS = /vehicle|car|truck|bus|motor|bike|van|shuttle/i;

function collisionKind(event: ScoreEvent): string {
  const data = (event.data && typeof event.data === "object" ? (event.data as Record<string, unknown>) : {}) as Record<string, unknown>;
  const text = [event.description, data.source_event, data.other_actor, data.other, data.with].filter((s) => typeof s === "string").join(" ");
  if (PEDESTRIAN_WORDS.test(text)) return "collision_pedestrian";
  if (VEHICLE_WORDS.test(text)) return "collision_vehicle";
  return "collision_static";
}

export function infractionKind(event: ScoreEvent): string | null {
  if (event.event_class === "collision") return collisionKind(event);
  return event.event_class in PENALTIES ? event.event_class : null;
}

/**
 * @param events all events of the run (sensor events and rule violations)
 * @param actualDuration seconds driven
 * @param plannedDuration planned duration (scenario template) when known; null for real-world runs
 */
export function computeDrivingScore(events: ScoreEvent[], actualDuration: number | null, plannedDuration: number | null): DrivingScore {
  const counts = new Map<string, Infraction>();
  let penalty = 1;
  for (const event of events) {
    const kind = infractionKind(event);
    if (!kind) continue;
    const coefficient = PENALTIES[kind];
    penalty *= coefficient;
    const key = `${event.event_class}|${kind}`;
    const entry = counts.get(key) || { event_class: event.event_class, kind, count: 0, penalty: coefficient };
    entry.count += 1;
    counts.set(key, entry);
  }
  let completion = 1;
  let basis = "assumed_complete";
  if (plannedDuration && plannedDuration > 0 && actualDuration !== null && actualDuration >= 0) {
    completion = Math.min(1, actualDuration / plannedDuration);
    basis = "duration_vs_planned";
  }
  const infractions = Array.from(counts.values()).sort((a, b) => a.penalty - b.penalty || b.count - a.count);
  return {
    driving_score: round4(100 * completion * penalty),
    route_completion: round4(100 * completion),
    infraction_penalty: round4(penalty),
    route_completion_basis: basis,
    infractions,
  };
}

function round4(n: number): number {
  return Math.round(n * 10000) / 10000;
}
