"""Controlled vehicle impacts; use a dedicated Town10HD_Opt server."""
import argparse,json,math,os,time
from pathlib import Path
import carla
p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=4674);p.add_argument('--label',default='fixed');args=p.parse_args()
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
c=carla.Client('localhost',args.port);c.set_timeout(30);w=c.get_world();original=w.get_settings();results=[]
try:
 for hz,kind,speed in [(60,'rear',10),(30,'rear',10),(30,'side',10),(30,'barrier',10),(30,'barrier',20)]:
  settings=w.get_settings();settings.synchronous_mode=True;settings.fixed_delta_seconds=1/hz;settings.substepping=True;settings.max_substep_delta_time=.01;settings.max_substeps=10;w.apply_settings(settings)
  actors=[];events=[]
  try:
   tf=w.get_map().get_spawn_points()[0];vehicle=w.spawn_actor(w.get_blueprint_library().find('vehicle.tesla.model3'),tf);actors.append(vehicle)
   relative=(0,-5,0) if kind=='side' else (6.5,0,0)
   target_tf=carla.Transform(tf.transform(carla.Location(*relative)),carla.Rotation(yaw=tf.rotation.yaw+(90 if kind=='side' else 0)))
   target=w.spawn_actor(w.get_blueprint_library().find('vehicle.dodge.charger'),target_tf);actors.append(target);target.apply_control(carla.VehicleControl(brake=1,hand_brake=True))
   sensor=w.spawn_actor(w.get_blueprint_library().find('sensor.other.collision'),carla.Transform(),attach_to=vehicle);actors.append(sensor)
   sensor.listen(lambda e:events.append({'frame':e.frame,'other':e.other_actor.type_id,'impulse_ns':e.normal_impulse.length()}))
   for _ in range(2*hz):w.tick()
   z0=vehicle.get_location().z;samples=[]
   if kind=='side':
    vehicle.apply_control(carla.VehicleControl(brake=1));target.apply_control(carla.VehicleControl());target.set_target_velocity(target_tf.get_forward_vector()*speed)
   else:
    if kind=='barrier':target.set_simulate_physics(False)
    vehicle.set_target_velocity(tf.get_forward_vector()*speed);vehicle.apply_control(carla.VehicleControl(throttle=.6))
   for i in range(4*hz):
    w.tick();t=vehicle.get_transform();velocity=vehicle.get_velocity()
    samples.append({'height_m':t.location.z-z0,'speed_mps':velocity.length(),'vertical_speed_mps':velocity.z,'pitch_deg':t.rotation.pitch,'roll_deg':t.rotation.roll})
    if events:vehicle.apply_control(carla.VehicleControl(brake=1))
   row={'hz':hz,'case':kind,'initial_speed_mps':speed,'contacts':len(events),'peak_height_m':max(s['height_m'] for s in samples),'peak_speed_mps':max(s['speed_mps'] for s in samples),'peak_vertical_speed_mps':max(s['vertical_speed_mps'] for s in samples),'peak_roll_deg':max(abs(s['roll_deg']) for s in samples),'events':events,'samples':samples}
   row['target_contacts']=sum(e['other']==target.type_id for e in events)
   row['passed']=row['target_contacts']>0 and row['peak_speed_mps']<speed*1.25+2 and row['peak_height_m']<1.0
   results.append(row);print({k:v for k,v in row.items() if k not in ['events','samples']},flush=True)
  finally:
   for actor in reversed(actors):
    if actor.type_id.startswith('sensor.'):actor.stop()
    actor.destroy()
   w.tick();time.sleep(.2)
finally:
 w.apply_settings(original);(P/'audit'/('collision-regression-'+args.label+'.json')).write_text(json.dumps(results,indent=2))
assert len(results)==5 and all(r['passed'] for r in results),'An impact failed; inspect collision regression report.'
