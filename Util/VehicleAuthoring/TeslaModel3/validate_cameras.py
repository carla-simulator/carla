import carla,time,queue,json,math,os,argparse
from pathlib import Path
from PIL import Image
parser=argparse.ArgumentParser(description='Model 3 camera and shared door-regression checks; use an isolated server.');parser.add_argument('--port',type=int,default=4674);parser.add_argument('--profile',default='Default');args=parser.parse_args()
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'));c=carla.Client('localhost',args.port);c.set_timeout(30);w=c.get_world();old=w.get_settings();weather=w.get_weather();v=cam=None;r={}
try:
 r['camera_profile']=args.profile
 s=w.get_settings();s.synchronous_mode=True;s.fixed_delta_seconds=1/30;w.apply_settings(s);w.set_weather(carla.WeatherParameters.ClearSunset)
 bp=w.get_blueprint_library().find('vehicle.tesla.model3');bp.set_attribute('color','140,12,20');v=w.spawn_actor(bp,w.get_map().get_spawn_points()[0])
 for _ in range(60):w.tick()
 r['semantic_tags']=list(v.semantic_tags);r['color']=v.attributes.get('color');assert 14 in v.semantic_tags
 def shot(name,sensor='sensor.camera.rgb'):
  global cam
  cb=w.get_blueprint_library().find(sensor);
  if cb.has_attribute('post_process_profile'):cb.set_attribute('post_process_profile',args.profile)
  cb.set_attribute('image_size_x','1600');cb.set_attribute('image_size_y','1000');cb.set_attribute('fov','90' if name=='interior' else '55')
  loc=v.get_transform().transform(carla.Location(-.15,-.38,1.14) if name=='interior' else carla.Location(6,-6,2.7));aim=v.get_transform().transform(carla.Location(1.1,0,.98) if name=='interior' else carla.Location(0,0,.7));d=aim-loc
  cam=w.spawn_actor(cb,carla.Transform(loc,carla.Rotation(pitch=math.degrees(math.atan2(d.z,math.hypot(d.x,d.y))),yaw=math.degrees(math.atan2(d.y,d.x)))));q=queue.Queue();cam.listen(q.put)
  for _ in range(35):
   f=w.tick()
   while True:
    im=q.get(timeout=30)
    if im.frame>=f:break
  if 'semantic' in sensor:
   raw=bytes(im.raw_data);r['car_label_pixels']=sum(x==14 for x in raw[2::4]);assert r['car_label_pixels']>10000;im.convert(carla.ColorConverter.CityScapesPalette)
  Image.frombytes('RGBA',(im.width,im.height),bytes(im.raw_data),'raw','BGRA').convert('RGB').save(P/'renders'/('carla-'+name+'.png'));cam.stop();cam.destroy();cam=None
 shot('red');shot('interior');shot('semantic','sensor.camera.semantic_segmentation')
 dark=w.get_weather();dark.sun_altitude_angle=-8;w.set_weather(dark)
 v.set_light_state(carla.VehicleLightState.NONE);shot('night-off')
 v.set_light_state(carla.VehicleLightState(carla.VehicleLightState.LowBeam|carla.VehicleLightState.Position|carla.VehicleLightState.LeftBlinker));shot('night-on')
 w.set_weather(carla.WeatherParameters.HardRainNoon);shot('rain')
 v.destroy();v=None
 r['door_regression']={}
 for name in ['vehicle.ue4.audi.tt','vehicle.dodge.charger']:
  b=w.get_blueprint_library().filter(name)
  assert b, 'Missing regression vehicle '+name
  v=w.spawn_actor(b[0],w.get_map().get_spawn_points()[0])
  for _ in range(30):w.tick()
  for _ in range(3):
   v.open_door(carla.VehicleDoor.All)
   for _ in range(15):w.tick()
   v.close_door(carla.VehicleDoor.All)
   for _ in range(15):w.tick()
  r['door_regression'][name]=v.is_alive;v.destroy();v=None
 (P/'audit/extra-checks.json').write_text(json.dumps(r,indent=2));print(r)
finally:
 if cam:cam.stop();cam.destroy()
 if v:v.destroy()
 w.set_weather(weather);w.apply_settings(old)
