"""A: a vehicle follows a real right-hand ramp, then merges ahead of the ego.

Requires explicit main-road ego and on-ramp merger poses from a qualified source
log. The merger must start on the mapped ramp; no lateral-offset lane substitute.
Speeds are km/h. ego_reacts=0 is only for collision counterfactual validation.
"""
model scenic.simulators.carla.model
from refined_lib import *
param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param merger_x = 0.0
param merger_y = 0.0
param merger_heading = 0.0
param clip_end_x = 0.0
param clip_end_y = 0.0
param ego_speed = 75
param merge_speed = 58
param merge_slow = 48
param merge_delay = 2.0
param duration = 10
param ego_reacts = 1
param ego_blueprint = 'vehicle.lincoln.mkz'
param merge_blueprint = 'vehicle.tesla.model3'
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
RAMP_START = new OrientedPoint at (globalParameters.merger_x @ globalParameters.merger_y), facing globalParameters.merger_heading

param transition_length = 45.
behavior EgoBehavior():
    state = {}
    route = carla_lane_path(self.carlaActor)
    do SmoothKeep(globalParameters.ego_speed * KMH, state, leads=([merger] if globalParameters.ego_reacts else []), lat_tol=4.5, route=route)

behavior MergeBehavior():
    state = {}
    ramp = carla_lane_path(self.carlaActor)
    target = carla_lane_path(ego.carlaActor)
    do SmoothKeep(globalParameters.merge_speed * KMH, state, route=ramp) for globalParameters.merge_delay seconds
    do SmoothKeep(globalParameters.merge_slow * KMH, state, route=target, transition=globalParameters.transition_length) until target.lateral(self.position) < .25
    do SmoothKeep(globalParameters.merge_slow * KMH, state, route=target) for 2 seconds
    do SmoothKeep(globalParameters.ego_speed * KMH, state, route=target)

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint globalParameters.ego_blueprint, with color Color(0.15, 0.20, 0.28),
    with rolename 'hero', with speed globalParameters.ego_speed * KMH, with behavior EgoBehavior()
merger = new Car at RAMP_START, facing RAMP_START.heading,
    with blueprint globalParameters.merge_blueprint, with color Color(220/255, 220/255, 220/255),
    with rolename 'merger', with speed globalParameters.merge_speed * KMH, with behavior MergeBehavior()
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=15)
terminate after globalParameters.duration seconds
