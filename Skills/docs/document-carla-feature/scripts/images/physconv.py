"""Physics substepping convergence on Chaos: same drive (throttle then brake) on Town10HD_Opt spawn 79
for several fixed_delta_seconds / max_substep_delta_time; writes JSON with speed and z-acceleration per tick."""
import carla, json, sys, math
c = carla.Client('localhost', 2000); c.set_timeout(60); w = c.get_world()
sp = w.get_map().get_spawn_points()[79]; sp.location.z += 0.2
bp = w.get_blueprint_library().find('vehicle.lincoln.mkz')
RUNS = [(dt, 0.01) for dt in (0.01, 0.02, 0.04, 0.05)] + [(0.04, s) for s in (0.005, 0.02, 0.04)]
out = {}
orig = w.get_settings()
try:
    for dt, sub in RUNS:
        s = w.get_settings(); s.synchronous_mode = True; s.fixed_delta_seconds = dt
        s.substepping = True; s.max_substep_delta_time = sub; s.max_substeps = max(1, math.ceil(dt / sub - 1e-9)); w.apply_settings(s)
        v = w.spawn_actor(bp, sp); w.tick()
        t0 = w.get_snapshot().timestamp.elapsed_seconds; rec = []
        while True:
            t = w.get_snapshot().timestamp.elapsed_seconds - t0
            if t < 1.5: ctl = carla.VehicleControl(brake=1.0)
            elif t < 6.5: ctl = carla.VehicleControl(throttle=0.7)
            else: ctl = carla.VehicleControl(brake=0.4)
            v.apply_control(ctl); w.tick()
            vel = v.get_velocity(); acc = v.get_acceleration()
            rec.append((round(t, 4), math.hypot(vel.x, vel.y), acc.z))
            if t > 14: break
        out[f'{dt}_{sub}'] = rec; print(dt, sub, s.max_substeps, 'max speed', max(r[1] for r in rec))
        v.destroy(); w.tick()
finally:
    w.apply_settings(orig)
json.dump(out, open(sys.argv[1], 'w'))
