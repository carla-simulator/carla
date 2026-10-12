"""Behaviours shared by the NuRec hybrid Scenic scenarios (`from nurec_lib import *`; helpers in nurec_helpers.py).

Scenic's stock FollowLaneBehavior steers with a PID on the cross-track error whose gains are tuned for ~10 m/s; at
highway speed they saturate the steering and the car spins. LaneKeep below steers by pure pursuit on the lane centreline
(the lane followed by its successors, so road boundaries of the exported OpenDRIVE are invisible), holds a target speed
with a feed-forward + PI throttle / proportional brake, and optionally keeps a time headway behind the nearest car ahead
in its lane. Scenic's RegulatedControlAction cannot brake (it zeroes the throttle before taking its magnitude), so the
behaviours take SetThrottleAction / SetBrakeAction / SetSteerAction explicitly.
"""
from scenic.domains.driving.actions import SetThrottleAction, SetSteerAction, SetBrakeAction
from nurec_helpers import *


behavior LaneKeep(target_speed, leads=(), headway=1.4, min_gap=6.0, lane=None, max_throttle=0.9, max_brake=0.7, lat_tol=1.8):
    """Follow the current lane (or `lane`) at target_speed (m/s); with `leads`, hold a time headway behind the nearest one
    ahead in the lane (a car counts as ahead within `lat_tol` m of this car's axis; widen it to react to a car angling in from
    the kerb or the next lane). Never terminates on its own: pair it with `until` / `for`."""
    path = LanePath(); ctrl = SpeedControl(simulation().timestep, max_brake=max_brake); past_steer = 0.0
    while True:
        path.update(lane if lane is not None else self._lane)
        if path.line is None:
            take SetThrottleAction(0.0), SetBrakeAction(1.0), SetSteerAction(0.0)
            continue
        v_des = target_speed
        lead = nearest_ahead(self, leads, lat_tol=lat_tol)
        if lead is not None:
            gap = fwd_lat(self, lead)[0] - (self.length + lead.length) / 2.0
            v_des = min(target_speed, max(0.0, lead.speed + 0.9 * (gap - max(min_gap, headway * self.speed))))
        thr, brk = ctrl.step(v_des, self.speed)
        steer = pure_pursuit(self, path, self.speed)
        steer = max(past_steer - 0.15, min(past_steer + 0.15, steer)); past_steer = steer
        take SetThrottleAction(min(thr, max_throttle)), SetBrakeAction(brk), SetSteerAction(steer)


behavior Stop(brake=0.6):
    """Brake to a standstill and hold."""
    while True:
        take SetThrottleAction(0.0), SetBrakeAction(brake), SetSteerAction(0.0)
