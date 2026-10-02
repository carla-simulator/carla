"""Close-up fidelity review on a dedicated CARLA server; restores world settings."""
import argparse,carla,math,queue,json,os,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=4674);p.add_argument('--views',default='',help='Comma-separated view names; default captures all');p.add_argument('--light-audit',action='store_true',help='Capture isolated API light states at fixed front/rear/cabin/road views');a=p.parse_args()
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
c=carla.Client('localhost',a.port);c.set_timeout(30);w=c.get_world();settings=w.get_settings();weather=w.get_weather();vehicle=camera=None;r={}
try:
 s=w.get_settings();s.synchronous_mode=True;s.fixed_delta_seconds=1/30;s.substepping=True;s.max_substep_delta_time=.01;s.max_substeps=10;w.apply_settings(s);w.set_weather(carla.WeatherParameters.ClearNoon)
 bp=w.get_blueprint_library().find('vehicle.tesla.model3');bp.set_attribute('color','140,12,20');vehicle=w.spawn_actor(bp,w.get_map().get_spawn_points()[0]);vehicle.apply_control(carla.VehicleControl(brake=1))
 for _ in range(90):w.tick()
 bp=w.get_blueprint_library().find('sensor.camera.rgb');bp.set_attribute('image_size_x','1600');bp.set_attribute('image_size_y','1000');bp.set_attribute('fov','55');bp.set_attribute('post_process_profile','VehicleReview');camera=w.spawn_actor(bp,carla.Transform());q=queue.Queue();camera.listen(q.put)
 def capture(label,loc,aim):
  if a.light_audit and not label.startswith('audit-'):return
  if a.views and label not in a.views.split(','):return
  tf=vehicle.get_transform();loc=tf.transform(carla.Location(*loc));aim=tf.transform(carla.Location(*aim));d=aim-loc;camera.set_transform(carla.Transform(loc,carla.Rotation(pitch=math.degrees(math.atan2(d.z,math.hypot(d.x,d.y))),yaw=math.degrees(math.atan2(d.y,d.x)))))
  for _ in range(35):
   frame=w.tick()
   while True:
    im=q.get(timeout=30)
    if im.frame>=frame:break
  path=P/'renders'/('upgrade-carla-'+label+'.png');im.save_to_disk(str(path));r[label]={'file':str(path),'frame':im.frame};print(label,im.frame,flush=True)
 capture('front',(5,-5,2.1),(0,0,.7));capture('headlight',(2.9,-1.8,1.0),(1.87,-.65,.70));capture('rear',(-3.3,-2,1.25),(-2.12,-.5,.86));capture('glass',(0,-3,1.35),(0,-.3,1.05));capture('cabin',(-.25,.30,1.30),(.42,-.38,.78));capture('driver',(.03,-.40,1.14),(1.4,-.4,1.12));capture('mirror',(.14,-.10,1.19),(.298,0,1.227))
 vehicle.apply_control(carla.VehicleControl(brake=1,steer=-.8))
 for _ in range(30):w.tick()
 capture('steered',(2.6,-2.2,.85),(1.4375,-.79,.36))
 vehicle.open_door(carla.VehicleDoor.FR)
 for _ in range(30):w.tick()
 capture('door',(1.7,3,1.3),(.25,.55,.8));vehicle.close_door(carla.VehicleDoor.All)
 night=w.get_weather();night.sun_altitude_angle=-18;night.cloudiness=0;w.set_weather(night)
 # Weather/skylight and temporal histories need time to settle before an
 # on/off comparison. Drain the sensor every frame to avoid an image backlog.
 for _ in range(180):
  frame=w.tick()
  while q.get(timeout=30).frame<frame:pass
 vehicle.set_light_state(carla.VehicleLightState.NONE);capture('lights-off',(3,-2,1.1),(1.88,-.65,.7))
 vehicle.set_light_state(carla.VehicleLightState(carla.VehicleLightState.Position|carla.VehicleLightState.LowBeam));capture('lights-on',(3,-2,1.1),(1.88,-.65,.7));capture('rear-on',(-3.3,-2,1.25),(-2.12,-.5,.86))
 if a.light_audit:
  vehicle.apply_control(carla.VehicleControl(brake=1,steer=0))
  # Let the queued steering/control update settle before issuing isolated
  # light requests, including when the all-off capture is filtered out.
  for _ in range(30):
   frame=w.tick()
   while q.get(timeout=30).frame<frame:pass
  states=[('off',carla.VehicleLightState.NONE)]+[(name,getattr(carla.VehicleLightState,name)) for name in ['Position','LowBeam','HighBeam','Fog','Interior','Brake','Reverse','LeftBlinker','RightBlinker']]
  views=[('front',(3,-2,1.1),(1.88,-.65,.7)),('rear',(-3.3,-2,1.25),(-2.12,-.5,.86)),('rear-right',(-3.3,2,1.25),(-2.12,.5,.86)),('rear-lens',(-2.8,-.75,.92),(-2.24,-.5,.88)),('roof',(-.05,0,1.02),(.19,0,1.30)),('road',(.1,-.4,1.15),(15,0,.1))]
  for view,loc,aim in views:
   for name,state in states:
    label='audit-'+view+'-'+name
    if a.views and label not in a.views.split(','):continue
    vehicle.set_light_state(state)
    capture(label,loc,aim)
    if label in r:
     actual=vehicle.get_light_state()
     assert actual==state,(label,actual,state)
     r[label]['requested_state']=str(state);r[label]['observed_state']=str(actual)
 report=P/'audit'/('light-state-camera-review.json' if a.light_audit else 'upgrade-camera-review.json')
 if a.views and report.exists():r={**json.loads(report.read_text()),**r}
 report.write_text(json.dumps(r,indent=2))
finally:
 if camera:camera.stop();camera.destroy();camera=None;w.tick();time.sleep(.2)
 if vehicle:vehicle.destroy();vehicle=None
 # tick() observes the published frame before native light-update callbacks
 # finish. Keep the client alive until the restored day/night state has also
 # reached the user callback phase (which follows LightManager updates).
 restored_frames=queue.Queue()
 callback_id=w.on_tick(lambda snapshot: restored_frames.put(snapshot.frame))
 try:
  w.set_weather(weather)
  for _ in range(3):
   target=w.tick()
   while restored_frames.get(timeout=30)<target:pass
 finally:
  w.remove_on_tick(callback_id)
  w.apply_settings(settings)
