import carla,time,json,math,queue,argparse,os
from pathlib import Path
from PIL import Image
parser=argparse.ArgumentParser(description='Model 3 integration check; use an isolated CARLA server.')
parser.add_argument('--port',type=int,default=4674);args=parser.parse_args()
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'));c=carla.Client('localhost',args.port);c.set_timeout(20)
for _ in range(240):
 try:
  c=carla.Client('localhost',args.port);c.set_timeout(20);w=c.get_world();break
 except RuntimeError:time.sleep(1)
else:raise RuntimeError('Server not ready')
old=w.get_settings();weather=w.get_weather();v=cam=None;r={}
try:
 s=w.get_settings();s.synchronous_mode=True;s.fixed_delta_seconds=1/30;w.apply_settings(s);w.set_weather(carla.WeatherParameters.ClearNoon)
 bp=w.get_blueprint_library().find('vehicle.tesla.model3');bp.set_attribute('role_name','tesla_fidelity_test');bp.set_attribute('color','235,238,240');tf=w.get_map().get_spawn_points()[0];v=w.spawn_actor(bp,tf)
 for _ in range(90):w.tick()
 assert 2.3<v.bounding_box.extent.x<2.4 and 1.0<v.bounding_box.extent.y<1.1, 'Incorrect sensor bounds'
 calipers=[n for n in v.get_component_names() if n.startswith('Caliper')]
 assert len(calipers)==4, 'Expected four independent calipers'
 r['blueprint']=v.type_id;r['initial_position']=str(v.get_location());r['bounds']=str(v.bounding_box);r['physics']=str(v.get_physics_control());r['alive_after_settle']=v.is_alive;print(json.dumps(r),flush=True)
 cb=w.get_blueprint_library().find('sensor.camera.rgb');cb.set_attribute('image_size_x','1600');cb.set_attribute('image_size_y','1000');cb.set_attribute('fov','55');cam=w.spawn_actor(cb,carla.Transform());q=queue.Queue();cam.listen(q.put)
 def capture(name,relative,target=(0,0,.7)):
  loc=v.get_transform().transform(carla.Location(*relative));aim=v.get_transform().transform(carla.Location(*target));d=aim-loc;cam.set_transform(carla.Transform(loc,carla.Rotation(pitch=math.degrees(math.atan2(d.z,math.hypot(d.x,d.y))),yaw=math.degrees(math.atan2(d.y,d.x)))))
  for _ in range(35):
   frame=w.tick()
   while True:
    im=q.get(timeout=30)
    if im.frame>=frame:break
  Image.frombytes('RGBA',(im.width,im.height),bytes(im.raw_data),'raw','BGRA').convert('RGB').save(P/'renders'/('carla-'+name+'.png'))
 capture('front',(6,-6,2.7));capture('rear',(-6,-6,2.7));capture('side',(0,-7,1.3))
 initial=v.get_location();v.apply_control(carla.VehicleControl(throttle=.35))
 speeds=[]
 caliper_tilt=[]
 for frame_index in range(120):
  w.tick();speeds.append(v.get_velocity().length()*3.6)
  if frame_index%10==0:
   for name in calipers:
    rot=v.get_component_relative_transform(name).rotation;caliper_tilt.extend([abs(rot.pitch),abs(rot.roll)])
 r['max_caliper_spin_deg']=max(caliper_tilt)
 assert r['max_caliper_spin_deg']<.5, 'Calipers spin with the wheels'
 r['drive_distance_m']=v.get_location().distance(initial);r['max_speed_kmh']=max(speeds);v.apply_control(carla.VehicleControl(brake=1))
 for _ in range(90):w.tick()
 r['braked_speed_kmh']=v.get_velocity().length()*3.6;print(json.dumps(r),flush=True);(P/'audit/runtime.json').write_text(json.dumps(r,indent=2))
 v.set_light_state(carla.VehicleLightState(carla.VehicleLightState.LowBeam|carla.VehicleLightState.Position|carla.VehicleLightState.Brake));capture('lights',(5,-4,1.3));r['light_state']=str(v.get_light_state())
 v.open_door(carla.VehicleDoor.FL)
 for _ in range(40):w.tick()
 capture('door',(3,-5,2));v.close_door(carla.VehicleDoor.All)
 for door in [carla.VehicleDoor.FR,carla.VehicleDoor.RL,carla.VehicleDoor.RR,carla.VehicleDoor.All]:
  v.open_door(door)
  for _ in range(15):w.tick()
  v.close_door(door)
  for _ in range(15):w.tick()
 v.apply_control(carla.VehicleControl(steer=.6,brake=1))
 for _ in range(30):w.tick()
 r['front_steer_deg']=v.get_wheel_steer_angle(carla.VehicleWheelLocation.FL_Wheel)
 r['caliper_steer_deg']=v.get_component_relative_transform(next(n for n in calipers if n.startswith('CaliperFL'))).rotation.yaw
 assert abs(r['caliper_steer_deg']-r['front_steer_deg'])<1, 'Caliper steering mismatch'
 assert abs(r['front_steer_deg'])>5, 'Steering did not respond'
 capture('steered',(5,-4,1.3))
 assert r['drive_distance_m']>3, 'Vehicle did not drive'
 assert r['max_speed_kmh']>5, 'Vehicle failed acceleration'
 assert r['braked_speed_kmh']<1, 'Vehicle failed braking'
 r['status']='passed motion/braking; images require visual review';(P/'audit/runtime.json').write_text(json.dumps(r,indent=2));print(json.dumps(r,indent=2),flush=True)
finally:
 if cam:cam.stop();cam.destroy();cam=None;w.tick();time.sleep(.2)
 if v:v.destroy();v=None
 # Weather restoration queues a native LightManager callback. Keep the client
 # alive through the subsequent user callback phase before releasing it.
 restored_frames=queue.Queue()
 callback_id=w.on_tick(lambda snapshot:restored_frames.put(snapshot.frame))
 try:
  w.set_weather(weather)
  for _ in range(3):
   target=w.tick()
   while restored_frames.get(timeout=30)<target:pass
 finally:
  w.remove_on_tick(callback_id)
  w.apply_settings(old)
