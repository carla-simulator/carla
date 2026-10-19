"""Shared helpers: sync mode, traffic population, freezing, cleanup."""
import random, time, carla

def connect():
    c = carla.Client("localhost", 2000); c.set_timeout(60)
    return c, c.get_world()

def set_sync(world, tm=None, on=True, dt=0.05):
    s = world.get_settings(); s.synchronous_mode = on; s.fixed_delta_seconds = dt if on else None
    world.apply_settings(s)
    if tm is not None:
        tm.set_synchronous_mode(on)

def populate(client, world, center, n_veh=40, n_walk=30, radius=120, seed=7, exclude=4.0):
    random.seed(seed)
    tm = client.get_trafficmanager(8000); tm.set_random_device_seed(seed)
    lib = world.get_blueprint_library()
    vbps = [b for b in lib.filter("vehicle.*") if int(b.get_attribute("number_of_wheels")) == 4
            and "mine" not in b.id and "firetruck" not in b.id]
    sps = [p for p in world.get_map().get_spawn_points()
           if p.location.distance(center) < radius and p.location.distance(center) > exclude]
    random.shuffle(sps)
    vehicles = []
    for p in sps[:n_veh]:
        bp = random.choice(vbps)
        if bp.has_attribute("color"):
            bp.set_attribute("color", random.choice(bp.get_attribute("color").recommended_values))
        v = world.try_spawn_actor(bp, p)
        if v:
            v.set_autopilot(True, tm.get_port()); vehicles.append(v)
    wbps = lib.filter("walker.pedestrian.*")
    walkers, ctrls = [], []
    tries = 0
    while len(walkers) < n_walk and tries < n_walk * 40:
        tries += 1
        loc = world.get_random_location_from_navigation()
        if loc is None or loc.distance(center) > radius * 0.6:
            continue
        bp = random.choice(wbps)
        if bp.has_attribute("is_invincible"):
            bp.set_attribute("is_invincible", "false")
        w = world.try_spawn_actor(bp, carla.Transform(loc + carla.Location(z=1)))
        if w:
            walkers.append(w)
    world.tick()
    cbp = lib.find("controller.ai.walker")
    for w in walkers:
        c = world.spawn_actor(cbp, carla.Transform(), attach_to=w); ctrls.append(c)
    world.tick()
    for c in ctrls:
        c.start(); c.go_to_location(world.get_random_location_from_navigation()); c.set_max_speed(1.2)
    return tm, vehicles, walkers, ctrls

def freeze(vehicles, ctrls, walkers):
    for v in vehicles:
        v.set_autopilot(False); v.set_simulate_physics(False)
    for c in ctrls:
        c.stop()
    for w in walkers:
        w.apply_control(carla.WalkerControl(speed=0.0))

def cleanup(client, world, tm, actors_lists):
    for c in actors_lists[-1]:
        try: c.stop()
        except Exception: pass
    ids = [a.id for lst in actors_lists for a in lst]
    client.apply_batch_sync([carla.command.DestroyActor(i) for i in ids])
    set_sync(world, tm, on=False)

def stage_front(world, t, dists=(11, 20, 30, 42, 58), lanes=True, walkers=18, seed=3, near=7, far=40):
    """Static vehicles on the lanes in front of transform t and walkers in its front cone."""
    import math
    random.seed(seed)
    lib = world.get_blueprint_library(); m = world.get_map()
    vbps = [b for b in lib.filter("vehicle.*") if int(b.get_attribute("number_of_wheels")) == 4
            and "mine" not in b.id and "firetruck" not in b.id and "ambulance" not in b.id]
    base = m.get_waypoint(t.location)
    rows = [base]
    if lanes:
        for side in ("get_left_lane", "get_right_lane"):
            w = getattr(base, side)()
            if w and w.lane_type == carla.LaneType.Driving:
                rows.append(w)
    cars = []
    for i, w0 in enumerate(rows):
        for j, d in enumerate(dists):
            if (i + j) % 2 and i:
                continue
            nx = w0.next(d) if w0.lane_id * base.lane_id > 0 else w0.previous(d)
            if not nx:
                continue
            wp = nx[0]; tr = wp.transform; tr.location.z += 0.3
            bp = random.choice(vbps)
            if bp.has_attribute("color"):
                bp.set_attribute("color", random.choice(bp.get_attribute("color").recommended_values))
            v = world.try_spawn_actor(bp, tr)
            if v:
                cars.append(v)
    f = t.get_forward_vector(); peds = []
    wbps = lib.filter("walker.pedestrian.*"); tries = 0
    while len(peds) < walkers and tries < 4000:
        tries += 1
        loc = world.get_random_location_from_navigation()
        if loc is None: continue
        d = loc - t.location; dist = math.hypot(d.x, d.y)
        if not (near < dist < far) or (d.x * f.x + d.y * f.y) / dist < 0.75:
            continue
        bp = random.choice(wbps)
        yaw = random.uniform(0, 360)
        p = world.try_spawn_actor(bp, carla.Transform(loc + carla.Location(z=1.0), carla.Rotation(yaw=yaw)))
        if p: peds.append(p)
    return cars, peds
