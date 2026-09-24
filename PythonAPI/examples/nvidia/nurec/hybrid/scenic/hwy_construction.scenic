"""B: cones and barriers close the ego lane; ego yields left, changes lane, continues.

The closure is synthetic CARLA geometry. Requires a same-direction left lane
throughout the closure. The default props are the available construction assets.
"""
model scenic.simulators.carla.model
from nurec_lib import *
param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param clip_end_x = 0.0
param clip_end_y = 0.0
param ego_speed = 80
param wait_speed = 60
param change_speed = 70
param passer_speed = 105
param passer_start = -18
param lane_left = -3.9
param closure_distance = 160
param ego_blueprint = 'vehicle.tesla.model3'
param passer_blueprint = 'vehicle.tesla.model3'
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
CLOSURE = new OrientedPoint at (follow roadDirection from EGO_START for globalParameters.closure_distance), facing roadDirection

behavior EgoBehavior():
    do LaneKeep(globalParameters.ego_speed * KMH, leads=[work_van]) until fwd_lat(self, barrier)[0] < 120
    do LaneKeep(globalParameters.wait_speed * KMH, leads=[barrier], min_gap=12) until fwd_lat(self, passer)[0] > 28
    target = self.laneSection._laneToLeft.lane
    do LaneKeep(globalParameters.change_speed * KMH, lane=target, leads=[barrier, passer]) until lateral_to_lane(self, target) < 0.3
    do LaneKeep(globalParameters.ego_speed * KMH, leads=[passer], headway=1.4)

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint globalParameters.ego_blueprint, with color Color(0.90, 0.90, 0.92),
    with rolename 'hero', with speed globalParameters.ego_speed * KMH, with behavior EgoBehavior()
passer = new Car at ((follow roadDirection from EGO_START for globalParameters.passer_start) offset by (globalParameters.lane_left @ 0)),
    with blueprint globalParameters.passer_blueprint, with color Color(32/255, 36/255, 40/255),
    with rolename 'passer', with speed globalParameters.passer_speed * KMH, with behavior LaneKeep(globalParameters.passer_speed * KMH)
work_van = new Car at (follow roadDirection from CLOSURE for 22),
    with blueprint 'vehicle.sprinter.mercedes', with color Color(0.90, 0.84, 0.64),
    with rolename 'work_van', with speed 0, with behavior Stop()
barrier = new Prop at CLOSURE, facing CLOSURE.heading + 90 deg,
    with blueprint 'static.prop.streetbarrier', with width 0.38, with length 1.215
barrier_left = new Prop at (CLOSURE offset by (-1.28 @ 0)), facing CLOSURE.heading + 90 deg,
    with blueprint 'static.prop.streetbarrier', with width 0.38, with length 1.215
barrier_right = new Prop at (CLOSURE offset by (1.28 @ 0)), facing CLOSURE.heading + 90 deg,
    with blueprint 'static.prop.streetbarrier', with width 0.38, with length 1.215
for i in range(8):
    cone = new Prop at ((follow roadDirection from CLOSURE for (-40 + 5*i)) offset by ((1.4 - 0.4*i) @ 0)),
        facing CLOSURE.heading, with blueprint 'static.prop.constructioncone', with width 0.48, with length 0.48
for i in range(5):
    cone = new Prop at ((follow roadDirection from CLOSURE for (5 + 5*i)) offset by (-1.45 @ 0)),
        facing CLOSURE.heading, with blueprint 'static.prop.constructioncone', with width 0.48, with length 0.48
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=15.0)
terminate after 24 seconds
