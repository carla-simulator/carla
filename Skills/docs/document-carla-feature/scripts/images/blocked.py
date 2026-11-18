"""Record 60 s of Traffic Manager traffic on Town10HD_Opt, then query blocked actors."""
import carla, random, sys
c = carla.Client('localhost', 2000); c.set_timeout(60); w = c.get_world(); tm = c.get_trafficmanager(8000)
s = w.get_settings(); s.synchronous_mode = True; s.fixed_delta_seconds = 0.05; w.apply_settings(s); tm.set_synchronous_mode(True)
random.seed(4); tm.set_random_device_seed(4)
bps = [b for b in w.get_blueprint_library().filter('vehicle.*') if int(b.get_attribute('number_of_wheels')) == 4 and 'mining' not in b.id]
sps = w.get_map().get_spawn_points(); random.shuffle(sps)
vs = []
for sp in sps[:50]:
    v = w.try_spawn_actor(random.choice(bps), sp)
    if v: v.set_autopilot(True, 8000); vs.append(v)
log = sys.argv[1]
try:
    for _ in range(20): w.tick()
    c.start_recorder(log)
    for _ in range(1200): w.tick()
    c.stop_recorder()
finally:
    c.apply_batch_sync([carla.command.DestroyActor(v) for v in vs])
    s.synchronous_mode = False; s.fixed_delta_seconds = None; w.apply_settings(s); tm.set_synchronous_mode(False)
print(len(vs), 'vehicles')
print(c.show_recorder_actors_blocked(log, 20, 100))
