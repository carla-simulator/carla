"""Log get_telemetry_data during a hard launch, a lane-change steer and a hard stop (Town10HD_Opt spawn 79)."""
import carla, json, sys, math
c = carla.Client('localhost', 2000); c.set_timeout(60); w = c.get_world()
sp = w.get_map().get_spawn_points()[79]; sp.location.z += 0.2
orig = w.get_settings(); s = w.get_settings(); s.synchronous_mode = True; s.fixed_delta_seconds = 0.02
s.substepping = True; s.max_substep_delta_time = 0.01; s.max_substeps = 2; w.apply_settings(s)
v = w.spawn_actor(w.get_blueprint_library().find('vehicle.lincoln.mkz'), sp); rec = []
try:
    w.tick(); t0 = w.get_snapshot().timestamp.elapsed_seconds
    while True:
        t = w.get_snapshot().timestamp.elapsed_seconds - t0
        if t < 1.0: ctl = carla.VehicleControl(brake=1.0)
        elif t < 6.0: ctl = carla.VehicleControl(throttle=1.0)
        elif t < 7.0: ctl = carla.VehicleControl(throttle=0.4, steer=-0.15 if t < 6.5 else 0.15)
        else: ctl = carla.VehicleControl(brake=1.0)
        v.apply_control(ctl); w.tick()
        d = v.get_telemetry_data()
        rec.append(dict(t=round(t, 3), speed=d.speed, rpm=d.engine_rpm, gear=d.gear, throttle=d.throttle, brake=d.brake, steer=d.steer,
                        long=[x.long_slip for x in d.wheels], lat=[x.lat_slip for x in d.wheels], omega=[x.omega for x in d.wheels]))
        if t > 11: break
finally:
    v.destroy(); w.apply_settings(orig)
json.dump(rec, open(sys.argv[1], 'w')); print(len(rec), max(r['speed'] for r in rec), max(r['rpm'] for r in rec), sorted(set(r['gear'] for r in rec)))
print(rec[300])
