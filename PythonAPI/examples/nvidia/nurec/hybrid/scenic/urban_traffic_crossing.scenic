"""Ego arrives at a queue, yields, then a pedestrian crosses right to left.

Source: a2a4322c, the Mary cinematic test's NuRec log. The CARLA walker is
simulation truth; the Mary geometry-cache performance is a render derivative.
Walking speed matches the measured Mary root speed. No cinematic render is
implied by running this scenario's physics validation.
"""
model scenic.simulators.carla.model
from nurec_lib import *
from scenic.simulators.carla.actions import SetWalkingDirectionAction, SetWalkingSpeedAction

param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param ego_speed = 15.0
param lead_distance = 22.0
param crossing_distance = 16.0
param crossing_half_width = 3.5
param pedestrian_speed = 1.047794
param stop_hold = 1.0

EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
CROSSING = new OrientedPoint at (follow roadDirection from EGO_START for globalParameters.crossing_distance), facing roadDirection

behavior ApproachQueue():
    do LaneKeep(globalParameters.ego_speed * KMH, leads=[lead, queue, standing], min_gap=8.0, headway=1.5) until self.speed < 0.15 and fwd_lat(self, lead)[0] < 15.0
    do Stop(1.0)

behavior CrossAfterYield():
    # Require an uninterrupted stop, not merely a brief low-speed sample.
    stopped = 0.0
    while stopped < globalParameters.stop_hold:
        if ego.speed < 0.15 and fwd_lat(ego, lead)[0] < 15.0 and fwd_lat(ego, self)[0] > 4.0:
            stopped += simulation().timestep
        else:
            stopped = 0.0
        take SetWalkingSpeedAction(0.0)
    while (self.position - CROSSING.position).rotatedBy(-CROSSING.heading).x > -globalParameters.crossing_half_width:
        take SetWalkingDirectionAction(CROSSING.heading + 90 deg), SetWalkingSpeedAction(globalParameters.pedestrian_speed)
    take SetWalkingSpeedAction(0.0)
    wait for 1 seconds
    terminate

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed globalParameters.ego_speed * KMH, with behavior ApproachQueue()

lead = new Car at (follow roadDirection from EGO_START for globalParameters.lead_distance),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.28, 0.30, 0.33), with rolename 'lead',
    with behavior Stop(1.0)
queue = new Car at (follow roadDirection from EGO_START for 30),
    with blueprint 'vehicle.mini.cooper', with color Color(0.75, 0.75, 0.72), with rolename 'queue',
    with behavior Stop(1.0)
standing = new Car at (follow roadDirection from EGO_START for 54),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.45, 0.06, 0.08), with rolename 'standing',
    with behavior Stop(1.0)
pedestrian = new Pedestrian at (CROSSING offset by (globalParameters.crossing_half_width @ 0)),
    facing CROSSING.heading + 90 deg, with blueprint 'walker.pedestrian.german',
    with rolename 'mary_proxy', with regionContainedIn everywhere,
    with requireVisible False, with behavior CrossAfterYield()

terminate after 20 seconds
