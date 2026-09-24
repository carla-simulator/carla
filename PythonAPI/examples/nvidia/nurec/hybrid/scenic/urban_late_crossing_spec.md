# Urban intersection: slowing lead vehicle and late pedestrian crossing

Status: v1.2 — user confirmed **brake**; candidate 005 passed independent fast behavior review. Final cinematic rendering remains deferred.

## Requested narrative

> Near an intersection, a vehicle is driving ahead of the ego, reducing the
> speed. Ego follows at a distance, but ego is forced to speed because a
> pedestrian crosses last second (right to left).

User clarification: “brake.” The intended narrative is: near an intersection,
a moving lead vehicle slows; ego follows at a distance, then must brake suddenly
because a pedestrian crosses at the last second from right to left. The previously
rendered stopped-queue scenario is a baseline to critique, not an accepted
implementation of this revised narrative.

## Setting and cast

- Use the same PhysicalAI/NuRec log, `a2a4322c-3f99-40c3-94df-17a67f56f55d`,
  initially. Keep the camera within the reconstructed route.
- Identify and record the actual intersection approach, crossing line, lane
  centreline, curb positions, and usable reconstruction bounds. Include a map
  or annotated image showing why the selected location is near an intersection.
  A filename or a junction label alone is insufficient evidence.
- Ego: API-controlled CARLA vehicle, driven by Scenic, with an ego-view camera.
- Lead: one clearly visible moving vehicle in the ego lane, also controlled by
  Scenic. It must be driving and visibly reducing speed, not parked from frame 1.
- Pedestrian: CARLA walker for behavior iteration; Mary for the final cinematic
  derivative. Cross from the camera's right to its left. Preserve realistic
  walking speed and ground contact; do not accelerate a walking performance to
  disguise incorrect event timing.
- Extra vehicles are optional and must not obscure the lead/pedestrian event.

## Required sequence

1. Establish the intersection approach with lead and ego both moving. The lead
   is ahead in the same lane; ego follows with visible, positive separation.
2. Lead reduces speed. Ego reacts to the lead and continues following at a
   controlled distance. Show enough motion to make the lead's slowdown legible.
3. Pedestrian begins crossing from the right while ego is still moving. Proposed
   staging: lead has cleared the crossing line and pedestrian enters the space
   between lead and ego. This staging is provisional, not an additional user
   requirement; revisit it if the clarified maneuver requires another layout.
4. The pedestrian creates a late conflict with ego's projected path. Trigger
   the braking response from measured pedestrian commitment geometry and validate
   time to conflict; do not require ego to have already stopped.
5. Ego brakes suddenly because of the pedestrian. This response
   must be measurably distinct from normal following of the slowing lead.
6. Pedestrian continues right to left and clears the ego's swept path. Hold the
   shot long enough to see the outcome. Subsequent ego motion is optional.

## Candidate tuning envelope

These are initial search ranges, not acceptance claims or fixed physical facts.
Tune against the actual log geometry, vehicle dynamics, and pedestrian asset.

- Initial urban speed: approximately 15–25 km/h.
- Establish moving following for at least 1.5 seconds before pedestrian commitment into the ego lane.
- Lead slowdown should be visible in footage and measured in telemetry; target
  a speed reduction of at least 20%, while remaining moving at pedestrian onset.
- Following distance: report bumper-to-bumper gap and time headway, not only
  distances between vehicle origins. Start with roughly 1.5–2 seconds headway.
- Mary walking reference: approximately 1.05 m/s, from the supplied performance.
- “Last second”: quantify the predicted conflict using both actor footprints
  and crossing motion. Initial candidate time-to-conflict is 1–2 seconds, to be
  adjusted for feasible response and visibility. Longitudinal TTC alone does
  not establish a collision with a laterally moving pedestrian.
- No lead/pedestrian contact, vehicle/vehicle contact, or ego/pedestrian contact
  in an accepted successful avoidance run. Minimum clearances must be reported.

## Acceptance evidence

The independent critic evaluates the specification and raw evidence, not only
the implementation's own pass flag. Missing evidence is “unproven,” not a pass.

| Requirement | Required evidence |
| --- | --- |
| Near an intersection | Annotated source/map view and recorded crossing coordinates |
| Moving lead slows ahead | Ego-view preview and lead speed/deceleration trace |
| Ego follows at a distance | Vehicle footprint gap/headway traces; no overlap |
| Pedestrian crosses late | Separate first-visibility, first-motion, corridor-entry, and ego-response timestamps; ego speed at each; projected footprint conflict |
| Right-to-left crossing | Ego-view video and signed lateral trajectory |
| Pedestrian causes ego response | Matched pedestrian-absent baseline and pedestrian-present/response-disabled run, leaving lead-following behavior unchanged |
| Outcome is physically coherent | Actor footprints, controls, braking/acceleration traces, collision sensors, and minimum clearances |
| Timing is reproducible | Same-seed replay comparison with explicit position, speed, and event-time tolerances |
| Story is readable | Critic review of both ego-view and overhead previews, with event timecodes |

The ego-view preview must visibly communicate entry and response. A geometric
conflict that occurs offscreen does not prove the requested visual narrative.
Do not conflate first pedestrian visibility, initial walking motion, and entry
into the ego's swept corridor. Record each event separately, together with
response onset. Evaluate impending conflict against the unbraked counterfactual,
as successful braking changes the actual ego time to the crossing.

Use two counterfactuals:

1. **Pedestrian absent:** measure normal lead-following speed and controls.
2. **Pedestrian present, ego pedestrian-response disabled:** measure whether the
   crossing creates a footprint conflict without the avoidance response.

Freeze the candidate's lead trajectory and pedestrian onset/path in the matched
runs. Do not recalculate the trigger from counterfactual ego motion. Check that
initial state and pre-response ego behavior match; record divergence time.
Stop conflict analysis at first contact rather than comparing post-collision
trajectories. Report any replay mismatch instead of treating it as causal evidence.

The counterfactuals establish whether the pedestrian actually matters. A
scenario fails the intended causality if following the lead would produce the
same ego response without the pedestrian. It also fails if ego has already
stopped before the pedestrian starts, even if collision checks pass.

### Braking acceptance criteria

- Ego is moving above 2 m/s at first walking motion; it must not already have
  stopped for the lead. The lead remains moving above 1 m/s at that time.
- Pedestrian-specific braking begins 0.25–0.60 s after commitment toward the ego lane;
  initial tuning value is 0.35 s. Commitment is crossing the +2 m lateral line
  continuously from the source-aligned curb candidate at +7 m. Record first walking and actual
  footprint corridor entry separately; neither is the commitment event.
- Peak ego deceleration is 3–9 m/s² over a 0.15-second measurement window,
  and at least 2 m/s² stronger than normal following in the matched absent run.
- Accepted run has no collisions and at least 0.5 m footprint clearance from
  pedestrian and lead; report actual minimums, not only a boolean pass.
- The response-disabled run must demonstrate a time-overlapping footprint
  conflict or contact. A late crossing which ego simply passes before the
  pedestrian arrives does not pass.
- At predicted pedestrian corridor intrusion, target 1–2 seconds unbraked
  ego-front time to the crossing. If exact measured geometry warrants another
  threshold, document and ask the critic to assess the change before acceptance.
- Same-seed replay: maximum position difference 1 cm, speed difference 0.02 m/s,
  and event-time difference no more than one simulation step. These are review
  tolerances, not a claim that the existing simulator is mathematically exact.

## Fast iteration and independent critic loop

1. Save each candidate as a numbered iteration with its parameters, scenario
   hash, source log, seed, simulator/build version, and fixed timestep.
2. Run Scenic/CARLA physics first. Reject spawn failures, premature stopping,
   wrong crossing direction, missing slowdown, and invalid timing immediately.
3. Generate a fast rasterized ego-view preview and an overhead trajectory view
   with time, actor speeds, lead gap, crossing trigger, and response onset.
   These are diagnostic previews, not final beauty footage. Use simple walker
   assets and no expensive Mary texture import, path tracing, or final EXRs.
4. Run the independent critic on the specification, preview videos, event
   annotations, telemetry, collision evidence, and both matched counterfactuals.
5. Save `critique.md` with verdict `PASS`, `REVISE`, or `UNPROVEN`, timestamped
   observations, requirement-by-requirement evidence, and at most three
   prioritized changes.
6. Apply the critic's actionable changes to the next candidate. Record why each
   change was made and compare the next result against the previous one. Do not
   weaken the narrative or thresholds simply to obtain a pass.
7. Repeat until all required criteria have evidence and the critic has no
   blocking findings. Repeat the accepted candidate to check reproducibility.
   If several iterations do not improve the same failure, diagnose the cause
   and document it rather than continuing an unbounded parameter search.
8. Present the accepted fast preview and critic verdict for scenario review.
   Final-quality rendering belongs to the later rendering stage, once the
   scenario is good; it is not part of each tuning iteration.

The primary agent implements changes. The critic remains independent and
read-only, and must not mark a candidate accurate merely because it renders.
Preserve previous iteration artifacts and the existing `sh020` video.

## Initial baseline

Existing scenario: `urban_traffic_crossing.scenic`.
Existing render: `artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing/comp/review.mp4`.
Existing behavior: `artifacts/hybrid-cinematic-mvp/traffic_crossing/behavior_v2/behavior.json`.

Known mismatch: its lead vehicles are stationary throughout, and its pedestrian
waits for ego to stop before crossing. It therefore does not establish a moving
lead slowdown, a late conflict, or a pedestrian-induced emergency response.

## Critic revision history

- v0.1: initial draft and independent rejection of the previous scenario.
- v0.2: incorporated the critic's three revisions: distinguish proposed staging
  from user requirements; use two matched counterfactuals with frozen event
  schedules; distinguish visible entry, walking onset, corridor intrusion, and
  ego response. The unresolved response remains explicit.
- v1.0: user confirmed braking. Added braking response, causal counterfactual,
  clearance, and repeatability criteria before the first revised run.

- v1.1: critic found +2 m was inside a second driving lane, not a curb.
  Candidate 002 starts at the mapped outer road edge +5.8 m and walks
  continuously, without pausing in traffic. First motion now precedes the
  last-second intrusion into the ego lane; response timing is measured from
  commitment at +2 m. This is an explicit staging interpretation for review,
  not a claim of a suddenly appearing pedestrian. The 1–2 s intrusion TTC
  and counterfactual collision criteria are retained. Source-image curb
  correspondence and narrative legibility remain acceptance gates.

- v1.2: source-image projection moves the starting candidate to +7 m.
  Crossing line is route arc 37 m, initial ego/lead speed 20 km/h, initial
  lead center gap 17 m. Prior candidates failed the retained intrusion TTC
  gate. Synchronous control batches address observed one-tick command latency.

## Reviewed candidate 005

- Lead slows from approximately 20 to 12 km/h; initial centre gap 17 m.
- Pedestrian walks continuously from the right curb candidate (+7 m) at
  native Mary reference speed, crossing at route arc37 m. The pedestrian is
  visible early; the late event is entry into ego's lane.
- Commitment 4.79167 s, braking command 5.16667 s, unbraked corridor entry
  5.625 s. Intrusion front TTC 1.17153 s. Response-disabled contact 6.70833 s.
- Main has no collisions; minimum ego/pedestrian footprint gap 3.987 m.
  Peak deceleration 3.905 m/s². Replay positions, speeds and event times match.
- Independent verdict: PASS for fast behavior, not cinematic integration.
  Review artifacts: `artifacts/hybrid-cinematic-mvp/late_crossing/iterations/005`.
- Final asset gates: verify Mary's mesh against the 0.346 m minimum lead/proxy
  separation, refine feet/curb contact in NuRec, then review final integration.
