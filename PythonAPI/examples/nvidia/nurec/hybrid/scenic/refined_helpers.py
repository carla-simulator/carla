"""Continuous trajectory and acceleration control for the opt-in refined scenarios."""
from nurec_helpers import *


def transition_path(me, target, length):
    """Quintic lateral offset matching initial heading and ending tangent to the road.

    Uses distance, not elapsed time, so braking cannot advance the lane change.
    The scalar offset ends with zero slope/curvature. Sampling the mapped
    polyline does not guarantee globally continuous road curvature.
    """
    origin = Point(me.position.x, me.position.y)
    start = target.line.project(origin)
    p = target.line.interpolate(start)
    q = target.line.interpolate(start + .2)
    dx, dy = q.x-p.x, q.y-p.y
    norm = math.hypot(dx, dy)
    nx, ny = -dy/norm, dx/norm
    offset = (origin.x-p.x)*nx + (origin.y-p.y)*ny
    forward = (-math.sin(me.heading), math.cos(me.heading))
    along = (forward[0]*dx + forward[1]*dy)/norm
    slope = (forward[0]*nx + forward[1]*ny)/max(.2, along)
    extent = target.line.length-start
    if extent < length + 5:
        raise ValueError('Insufficient mapped road for smooth lane transition')
    points = []
    for i in range(int(extent/.5)+1):
        s = min(i*.5, extent)
        p = target.line.interpolate(start+s)
        lo = target.line.interpolate(max(0, start+s-.25))
        hi = target.line.interpolate(min(target.line.length, start+s+.25))
        dx,dy = hi.x-lo.x,hi.y-lo.y
        norm = math.hypot(dx,dy)
        u = min(1.,s/length)
        blend = 10*u**3-15*u**4+6*u**5
        lateral = offset*(1-blend) + length*slope*(u-6*u**3+8*u**4-3*u**5)
        points.append((p.x-dy/norm*lateral,p.y+dx/norm*lateral))
    result = LanePath(); result.line = LineString(points)
    return result


class ComfortControl:
    """Acceleration feedback with jerk-limited demand; continuous across phases."""
    def __init__(self, dt, speed):
        self.dt=dt; self.previous_speed=speed; self.accel=0.; self.integral=0.; self.measured=0.; self.samples=0.

    def step(self, desired, speed):
        requested=max(-2.5,min(1.5,.65*(desired-speed)))
        self.accel += max(-1.2*self.dt,min(1.2*self.dt,requested-self.accel))
        measured=(speed-self.previous_speed)/self.dt if self.samples > 1 else 0.
        self.samples += 1
        self.measured += .2*(measured-self.measured)
        self.previous_speed=speed
        error=self.accel-self.measured
        self.integral=max(-.8,min(.8,self.integral+error*self.dt*.10))
        effort=.019*speed+.20*self.accel+.04*error+self.integral
        return max(0.,min(.75,effort)),max(0.,min(.7,-effort*.65))
