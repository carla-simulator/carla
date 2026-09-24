"""Opt-in comfort controller; state is shared explicitly across maneuver phases."""
from nurec_lib import *
from refined_helpers import *

behavior SmoothKeep(target_speed, state, leads=(), route=None, lane=None, transition=0., lat_tol=2.2, headway=1.5):
    path = route if route is not None else LanePath()
    if route is None:
        path.update(lane if lane is not None else self._lane)
    if transition > 0:
        path = transition_path(self, path, transition)
    if 'ctrl' not in state:
        state['ctrl'] = ComfortControl(simulation().timestep, self.carlaActor.get_velocity().length())
        state['steer'] = 0.
    while True:
        speed = self.carlaActor.get_velocity().length()
        desired = target_speed
        lead = nearest_ahead(self, leads, lat_tol=lat_tol)
        if lead is not None:
            gap = fwd_lat(self, lead)[0] - (self.length + lead.length)/2
            desired = min(desired, max(0., lead.speed + .55*(gap-max(7.,headway*speed))))
        throttle, brake = state['ctrl'].step(desired, speed)
        steer = pure_pursuit(self, path, speed, min_ld=7., k_ld=.65)
        delta = .3*simulation().timestep
        steer = max(state['steer']-delta,min(state['steer']+delta,steer))
        state['steer'] = steer
        take SetThrottleAction(throttle), SetBrakeAction(brake), SetSteerAction(steer)
