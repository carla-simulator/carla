"""Moving lead slows near the intersection; late crossing forces ego braking.

Use a2a4322c's proxy road. Parameters support frozen-onset counterfactuals.
The pedestrian continuously crosses from the mapped outer road edge (+7m, source-image candidate).
"""
model scenic.simulators.carla.model
from nurec_lib import *
from scenic.simulators.carla.actions import SetWalkingDirectionAction, SetWalkingSpeedAction
from late_crossing_helpers import *

param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param ego_speed = 20.0
param lead_gap = 17.0
param lead_speed = 20.0
param lead_slow_speed = 12.0
param lead_slow_after = 1.5
param crossing_distance = 37.0
param lead_stop_distance = 43.0
param crossing_half_width = 2.0
param pedestrian_start_lateral = 7.0
param commitment_lateral = 2.0
param pedestrian_speed = 1.047794
param trigger_ttc = 1.8
param reaction_delay = 0.35
param emergency_brake = 0.55
param ego_reacts = True
param pedestrian_present = True
param pedestrian_start_s = -1.0
param duration = 13.0

reset_events()
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
CROSSING = new OrientedPoint at (follow roadDirection from EGO_START for globalParameters.crossing_distance), facing roadDirection
LEAD_STOP = new OrientedPoint at (follow roadDirection from EGO_START for globalParameters.lead_stop_distance), facing roadDirection

behavior MovingLead():
    do LaneKeep(globalParameters.lead_speed * KMH) for globalParameters.lead_slow_after seconds
    mark_event('lead_slowdown_command', simulation().currentTime * simulation().timestep)
    do LaneKeep(globalParameters.lead_slow_speed * KMH) until fwd_lat_xy(self, LEAD_STOP.position.x, LEAD_STOP.position.y)[0] < 0
    do Stop(0.4)

behavior FollowingEgo():
    path = LanePath()
    ctrl = SpeedControl(simulation().timestep, max_brake=0.5)
    past_steer = 0.0
    while True:
        path.update(self._lane)
        gap = fwd_lat(self, lead)[0] - (self.length + lead.length)/2
        desired = min(globalParameters.ego_speed * KMH, max(0.0, lead.speed + 0.9 * (gap-max(5.0, 1.5*self.speed))))
        thr, brk = ctrl.step(desired, self.speed)
        steer = pure_pursuit(self, path, self.speed)
        steer = max(past_steer-0.15, min(past_steer+0.15, steer))
        past_steer = steer
        t = simulation().currentTime * simulation().timestep
        onset = event_time('pedestrian_commitment')
        if globalParameters.ego_reacts and onset is not None and t >= onset + globalParameters.reaction_delay:
            mark_event('ego_emergency_brake', t)
            thr = 0.0
            brk = globalParameters.emergency_brake
        take SetThrottleAction(thr), SetBrakeAction(brk), SetSteerAction(steer)

behavior LateCrossing():
    # Continuous motion from the outer road edge; no invented refuge in lane 2.
    t = 0.0
    while t < max(0.0, globalParameters.pedestrian_start_s):
        take SetWalkingSpeedAction(0.0)
        t = simulation().currentTime * simulation().timestep
    mark_event('pedestrian_start', t, crossing_x=float(CROSSING.position.x), crossing_y=float(CROSSING.position.y))
    while (self.position-CROSSING.position).rotatedBy(-CROSSING.heading).x > -globalParameters.crossing_half_width:
        lateral = (self.position-CROSSING.position).rotatedBy(-CROSSING.heading).x
        if lateral <= globalParameters.commitment_lateral:
            mark_event('pedestrian_commitment', simulation().currentTime * simulation().timestep)
        take SetWalkingDirectionAction(CROSSING.heading + 90 deg), SetWalkingSpeedAction(globalParameters.pedestrian_speed)
    mark_event('pedestrian_cleared', simulation().currentTime * simulation().timestep)
    while True:
        take SetWalkingSpeedAction(0.0)

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10,0.10,0.12), with rolename 'hero',
    with speed globalParameters.ego_speed * KMH, with behavior FollowingEgo()
lead = new Car at (follow roadDirection from EGO_START for globalParameters.lead_gap),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.6,0.2,0.12), with rolename 'lead',
    with speed globalParameters.lead_speed * KMH, with behavior MovingLead()
if globalParameters.pedestrian_present:
    pedestrian = new Pedestrian at (CROSSING offset by (globalParameters.pedestrian_start_lateral @ 0)),
        facing CROSSING.heading + 90 deg, with blueprint 'walker.pedestrian.german', with rolename 'mary_proxy',
        with regionContainedIn everywhere, with requireVisible False, with behavior LateCrossing()

terminate after globalParameters.duration seconds
