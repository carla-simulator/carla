"""Matched vehicle/prop look tests using a saved NuRec hybrid camera/background.

Replaces the map on a dedicated CARLA server. Reuses the reference capture's
NuRec pixels, camera calibration and measured lighting without rerunning NuRec.
Writes full CARLA, filtered actor layer, masks, and matched NuRec composites.
"""
import argparse,json,math,queue,socket,sys,zipfile
from pathlib import Path

import carla
import msgpack
import numpy as np
from PIL import Image
from scipy.ndimage import binary_erosion,gaussian_filter,distance_transform_edt

HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parent))
from projection_functions import get_t_rig_enu_from_ecef
import skymap
import exposure_calib
from prop_grounding import ground_prop

VEHICLES=['vehicle.tesla.model3','vehicle.lincoln.mkz','vehicle.dodge.charger',
          'vehicle.nissan.patrol','vehicle.mini.cooper','vehicle.sprinter.mercedes']

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--reference',type=Path,required=True)
 ap.add_argument('--usdz',type=Path,required=True)
 ap.add_argument('--out',type=Path,required=True)
 ap.add_argument('--port',type=int,default=4690)
 ap.add_argument('--frame',type=int,default=0)
 ap.add_argument('--vehicles',nargs='+',default=VEHICLES)
 ap.add_argument('--color',help='Common CARLA paint RGB, e.g. 32,36,40; omitted preserves the reference paints')
 ap.add_argument('--views',nargs='+',default=['rear_near','rear_far','front_quarter'])
 ap.add_argument('--spp',type=int,default=32)
 ap.add_argument('--exposure-comp',type=float,help='Use a previously measured sensor EV instead of the reference capture EV')
 ap.add_argument('--denoiser',choices=['true','false'],default='true')
 ap.add_argument('--sky-intensity',type=float,default=skymap.DEFAULT_INTENSITY)
 ap.add_argument('--sky-indirect',type=float,help='Diagnostic override of the sky bounce multiplier')
 ap.add_argument('--visible-sky',action='store_true',help='Show environment radiance for sky calibration diagnostics')
 ap.add_argument('--direct-sun',action='store_true',help='Diagnostic: apply weather angles directly to the sun component')
 ap.add_argument('--props',action='store_true')
 ap.add_argument('--prop-poses',type=Path,help='Replay specific prop poses from a JSON list')
 ap.add_argument('--ground-props',action='store_true')
 ap.add_argument('--calibrate-exposure',action='store_true',help='Re-solve road exposure for this lighting condition')
 ap.add_argument('--calibrate-sky',metavar='X0,Y0,X1,Y1',help='Overcast exposure reference: a visually verified clear-sky ROI in normalized image coordinates')
 ap.add_argument('--balance-road',action='store_true',help='With --calibrate-sky, fit sky and road log-luminance residuals with equal weight')
 a=ap.parse_args();a.out.mkdir(parents=True,exist_ok=True)
 if a.color:
  try:
   rgb=[int(v) for v in a.color.split(',')]
   if len(rgb)!=3 or any(v<0 or v>255 for v in rgb):raise ValueError()
  except ValueError:ap.error('--color requires three integer channels in 0..255')
 meta=json.loads((a.reference/'meta.json').read_text())
 sky_roi=None
 if a.balance_road and not a.calibrate_sky:ap.error('--balance-road requires --calibrate-sky')
 if a.calibrate_sky:
  if a.calibrate_exposure or not meta.get('diffuse_only'):
   ap.error('--calibrate-sky requires a diffuse-only reference and excludes --calibrate-exposure')
  try:
   sky_roi=[float(v) for v in a.calibrate_sky.split(',')]
   if len(sky_roi)!=4 or not (0<=sky_roi[0]<sky_roi[2]<=1 and 0<=sky_roi[1]<sky_roi[3]<=1):raise ValueError()
  except ValueError:ap.error('--calibrate-sky requires X0,Y0,X1,Y1 in 0..1')
 with np.load(a.reference/f'B_{a.frame:04d}.npz') as f:tf_vals=f['cam_tf'].copy()
 with np.load(a.reference/f'E_{a.frame:04d}.npz') as f:neural={k:f[k].copy() for k in f.files}
 cam_tf=carla.Transform(carla.Location(*map(float,tf_vals[:3])),carla.Rotation(pitch=float(tf_vals[3]),yaw=float(tf_vals[4]),roll=float(tf_vals[5])))
 with zipfile.ZipFile(a.usdz) as z:
  xodr=z.read('map.xodr').decode();rig=json.loads(z.read('rig_trajectories.json'))
 t_sc=get_t_rig_enu_from_ecef(np.array(rig['T_world_base']),xodr)
 def console(cmd):
  with socket.create_connection(('localhost',a.port),timeout=20) as s:
   s.sendall(msgpack.packb([0,1,'console_command',[[False],cmd]],use_bin_type=True));s.recv(1<<16)
 c=carla.Client('localhost',a.port);c.set_timeout(180)
 console('carla.OpenDrive.StreetFurniture 0')
 w=c.generate_opendrive_world(xodr,carla.OpendriveGenerationParameters(vertex_distance=2,max_road_length=500,wall_height=0,additional_width=.6,smooth_junctions=True,enable_mesh_visibility=True))
 settings=w.get_settings();original=w.get_settings();settings.synchronous_mode=True;settings.fixed_delta_seconds=.05
 settings.substepping=True;settings.max_substep_delta_time=.01;settings.max_substeps=10;settings.no_rendering_mode=False
 w.apply_settings(settings)
 actors=[];cams=[];report=[]
 try:
  az,alt,cloud=map(float,meta['sun'].split(','));w.set_weather(carla.WeatherParameters(sun_azimuth_angle=az,sun_altitude_angle=alt,cloudiness=cloud))
  pano,il=skymap.load_probe(str(a.reference/'illum.jpg'),str(a.reference/'illum.json'))
  skymap.apply(w,skymap.rig_pano_to_carla(pano,skymap.rig_to_carla_matrix(rig_pose=t_sc@np.array(il['rig_pose']))),a.sky_intensity,512)
  if meta.get('diffuse_only'):
   console("py import unreal; w=unreal.find_object(None,'/Game/Carla/Maps/OpenDriveMap.OpenDriveMap'); [c.set_intensity(0.0) for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) for c in a.get_components_by_class(unreal.DirectionalLightComponent)]")
  if a.direct_sun:
   console("py import unreal; w=unreal.find_object(None,'/Game/Carla/Maps/OpenDriveMap.OpenDriveMap'); [c.set_world_rotation(unreal.Rotator(pitch="+str(-alt)+",yaw="+str(az)+",roll=0),False,True) for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) for c in a.get_components_by_class(unreal.DirectionalLightComponent) if c.get_name()=='DirectionalLightComponentSun']")
  if a.sky_indirect is not None:
   console("py import unreal; w=unreal.find_object(None,'/Game/Carla/Maps/OpenDriveMap.OpenDriveMap'); [c.set_indirect_lighting_intensity("+str(float(a.sky_indirect))+") for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor) for c in a.get_components_by_class(unreal.SkyLightComponent)]")
  console('r.MotionBlur.Amount 0');console('r.EyeAdaptation.MethodOverride 3');console('r.EyeAdaptation.LensAttenuation 0.44')
  if a.visible_sky or sky_roi:console('r.PathTracing.VisibleLights 2')
  ft=meta['ftheta'];width,height=meta['w'],meta['h'];ev=meta['rt']['exposure_comp'] if a.exposure_comp is None else a.exposure_comp
  attrs=dict(camera_model='ftheta',distortion_coeffs=','.join(map(str,ft['poly'])),theta_max_deg=str(np.degrees(ft['max_angle'])),fx=str(1/width),fy=str(1/height),cx=str(ft['cx']/width),cy=str(ft['cy']/height),image_size_x=str(width),image_size_y=str(height),samples_per_pixel=str(a.spp),enable_denoiser=a.denoiser,exposure_mode='manual',exposure_compensation=str(ev))
  bpl=w.get_blueprint_library();cmap=w.get_map()
  def spawn_sensor(kind,extra):
   bp=bpl.find(kind)
   for k,v in {**attrs,**extra}.items():bp.set_attribute(k,v)
   if bp.has_attribute('enable_dlss'):bp.set_attribute('enable_dlss','false')
   sensor=w.spawn_actor(bp,cam_tf);q=queue.Queue();sensor.listen(q.put);cams.append(sensor);return sensor,q
  def capture(sensors,n=1):
   result=None
   for _ in range(n):
    frame=w.tick();result={}
    for name,(_,q) in sensors.items():
     while True:
      im=q.get(timeout=45)
      if im.frame>=frame:break
     if im.frame!=frame:raise RuntimeError(f'Frame mismatch {name}: {im.frame} vs {frame}')
     if name=='depth':result[name]=np.frombuffer(im.raw_data,np.float32).reshape(height,width).copy()
     else:result[name]=np.frombuffer(im.raw_data,np.uint8).reshape(height,width,4)[:,:,2::-1].copy()
   return result
  def destroy_sensors():
   for sensor in cams:sensor.stop()
   w.tick()
   for sensor in cams:sensor.destroy()
   cams.clear();w.tick()
  if a.calibrate_exposure or sky_roi:
   def measure(ec):
    extra=dict(show_only_tags='Roads,Sidewalks,RoadLines',exposure_compensation=str(ec))
    sensors={'catch':spawn_sensor('sensor.camera.rt_lens',extra),
             'tags':spawn_sensor('sensor.camera.rt_lens_instance',extra),
             'depth':spawn_sensor('sensor.camera.rt_lens_distance',extra)}
    try:
     f=capture(sensors,30)
     if sky_roi:
      x0,y0,x1,y1=sky_roi;roi=np.zeros((height,width),bool)
      roi[int(y0*height):int(y1*height),int(x0*width):int(x1*width)]=True
      lc=exposure_calib.luminance(f['catch']);le=exposure_calib.luminance(neural['color'])
      valid=roi&(f['tags'][:,:,0]==0)&(lc>1e-3)&(le>1e-3)&(f['catch'].max(2)<250)&(neural['color'].max(2)<250)
      if valid.sum()<2000:raise RuntimeError('Too few unclipped clear-sky reference pixels')
      ratio=float(np.median(np.log2(lc[valid]/le[valid])))
      # The existing secant solver uses legacy road_* statistic keys; the
      # saved report explicitly identifies this reference as visible sky.
      st=dict(road_ev=ratio,road_px=int(valid.sum()),lines_ev=float('nan'),mask=dict(sky_roi=sky_roi))
      st['sky_ev']=ratio
      if a.balance_road:
       road_st,_=exposure_calib.frame_stats(f['catch'],f['tags'][:,:,0],f['depth'],neural['color'],neural['distance'],neural['opacity'])
       if not np.isfinite(road_st['road_ev']):raise RuntimeError('Too few comparable road pixels for the balanced reference')
       st.update(proxy_road_ev=road_st['road_ev'],proxy_road_px=road_st['road_px'],
                 road_ev=(ratio+road_st['road_ev'])/2,lines_ev=road_st['lines_ev'])
     else:
      st,_=exposure_calib.frame_stats(f['catch'],f['tags'][:,:,0],f['depth'],neural['color'],neural['distance'],neural['opacity'])
     return [st]
    finally:destroy_sensors()
   ev,calib=exposure_calib.solve_ev(measure,ec0=ev)
   if sky_roi:calib.update(reference=('Equal-weight overcast sky and proxy-road log-luminance residuals' if a.balance_road else 'Visible overcast sky luminance')+'; road_* keys are legacy solver statistic names',sky_roi=sky_roi)
   (a.out/'exposure_calib.json').write_text(json.dumps(calib,indent=2))
   if not calib['converged']:raise RuntimeError('Exposure reference failed to converge')
   attrs['exposure_compensation']=str(ev)
   if sky_roi and not a.visible_sky:console('r.PathTracing.VisibleLights 0')
  bg=capture({'catch':spawn_sensor('sensor.camera.rt_lens',dict(show_only_tags='Roads,Sidewalks,RoadLines'))},20)['catch']
  destroy_sensors();Image.fromarray(neural['color']).save(a.out/'neural.png');Image.fromarray(bg).save(a.out/'catchers.png')
  center=cmap.get_waypoint(cam_tf.location)
  (a.out/'setup.json').write_text(json.dumps(dict(reference=str(a.reference.resolve()),frame=a.frame,usdz=str(a.usdz.resolve()),camera=tf_vals.tolist(),spp=a.spp,denoiser=a.denoiser,sky_indirect=a.sky_indirect,direct_sun=a.direct_sun,visible_sky=a.visible_sky,sky_intensity=a.sky_intensity,exposure_comp=ev,sun=meta['sun'],diffuse_only=meta.get('diffuse_only',False),vehicles=a.vehicles,color=a.color),indent=2))
  specs=[(bp,view) for bp in a.vehicles for view in a.views]
  if a.props:specs += [(bp,'prop') for bp in ['static.prop.constructioncone','static.prop.streetbarrier','static.prop.warningconstruction']]
  prop_poses={}
  if a.prop_poses:
   prop_poses={f'prop_{i}':r for i,r in enumerate(json.loads(a.prop_poses.read_text()))}
   specs=[(r['blueprint'],view) for view,r in prop_poses.items()]
  for bp_id,view in specs:
   distance=25 if view=='rear_far' else 12
   wp=center.next(distance)[0];tf=wp.transform
   if view=='front_quarter':tf.rotation.yaw+=145
   if view=='prop':tf.rotation.yaw+=90
   tf.location.z+=.5 if bp_id.startswith('vehicle.') else 0
   if view in prop_poses:
    r=prop_poses[view]
    tf=carla.Transform(carla.Location(r['x'],r['y'],r['z']),carla.Rotation(yaw=r['yaw']))
   bp=bpl.find(bp_id)
   paint=a.color or ('140,35,45' if bp_id=='vehicle.tesla.model3' else '85,105,125')
   if bp.has_attribute('color'):bp.set_attribute('color',paint)
   actor=w.spawn_actor(bp,tf);actors.append(actor)
   is_car=isinstance(actor,carla.Vehicle)
   if is_car:
    actor.apply_control(carla.VehicleControl(brake=1,manual_gear_shift=False))
    for _ in range(80):w.tick()
   else:
    actor.set_simulate_physics(False)
    for _ in range(5):w.tick()
   grounding=ground_prop(w,actor) if a.ground_props else None
   if grounding:w.tick()
   settled=actor.get_transform();bb=actor.bounding_box
   corners=bb.get_world_vertices(settled)
   point=cmap.get_waypoint(settled.location)
   record=dict(blueprint=bp_id,view=view,location=[settled.location.x,settled.location.y,settled.location.z],
       rotation=[settled.rotation.pitch,settled.rotation.yaw,settled.rotation.roll],
       bbox_bottom_m=min(v.z for v in corners) if is_car else None,road_z=point.transform.location.z,
       bbox_bottom_vs_road_m=min(v.z for v in corners)-point.transform.location.z if is_car else None,
       bounds_note='Vehicle bounds versus road-center height; not a tire contact measurement.' if is_car else 'Static prop API bounds require separate validation; use the rendered ground contact.',
       semantic_tags=list(actor.semantic_tags))
   record['grounding']=grounding
   record['requested_color']=paint if bp.has_attribute('color') else None
   sensors={'beauty':spawn_sensor('sensor.camera.rt_lens',{}),
            'layer':spawn_sensor('sensor.camera.rt_lens',dict(show_only_actor_ids=str(actor.id),shadow_catcher_tags='Roads,Sidewalks,RoadLines')),
            'tags':spawn_sensor('sensor.camera.rt_lens_instance',dict(show_only_actor_ids=str(actor.id),shadow_catcher_tags='Roads,Sidewalks,RoadLines')),
            'depth':spawn_sensor('sensor.camera.rt_lens_distance',dict(show_only_actor_ids=str(actor.id),shadow_catcher_tags='Roads,Sidewalks,RoadLines'))}
   frames=capture(sensors,30)
   sem=frames['tags'][:,:,0];mask=np.isin(sem,actor.semantic_tags)
   label=f"{bp_id.replace('vehicle.','').replace('static.prop.','')}-{view}";dest=a.out/label;dest.mkdir(exist_ok=True)
   for name in ['beauty','layer']:Image.fromarray(frames[name]).save(dest/f'{name}.png')
   Image.fromarray(mask.astype(np.uint8)*255).save(dest/'mask.png')
   np.savez_compressed(dest/'layers.npz',beauty=frames['beauty'],rgb=frames['layer'],mask=mask,sem=sem,dist=frames['depth'])
   # Same blend as hybrid_video_finish.py, plus diagnostic hard-matte alternative.
   def composite(erode,sigma):
    m=binary_erosion(mask,iterations=erode) if erode else mask
    R=frames['layer'].astype(np.float32);dc=frames['depth'];mf=m.astype(np.float32)
    if m.any():
     iy,ix=distance_transform_edt(~m,return_distances=False,return_indices=True);R=R[iy,ix]
    vis=np.clip(.5+(neural['distance']-dc+.3)/(2*np.maximum(.5,.02*dc)),0,1)
    vis=np.where(neural['opacity']<.6,1.,vis)
    alpha=gaussian_filter(mf*vis,sigma)*(gaussian_filter(mf,sigma)>.02) if sigma else mf*vis
    ratio=np.clip(gaussian_filter(frames['layer'].mean(2),1.5)/np.maximum(gaussian_filter(bg.mean(2),1.5),1),.3,1)
    ground=np.isin(sem,[1,2,24,22,10,9])&~m
    vis0=np.clip(.5+(neural['distance']-dc)/(2*.5),0,1);vis0=np.where(neural['opacity']<.05,0.,vis0)
    shadow=gaussian_filter(1-(1-ratio)*ground*vis0,1)
    return np.clip(alpha[:,:,None]*R+(1-alpha[:,:,None])*neural['color']*shadow[:,:,None],0,255).astype(np.uint8)
   Image.fromarray(composite(1,.5)).save(dest/'composite.png')
   Image.fromarray(composite(1,0)).save(dest/'hard_matte.png')
   ys,xs=np.where(mask)
   record.update(pixels=int(mask.sum()),image_bounds=[int(xs.min()),int(ys.min()),int(xs.max()),int(ys.max())] if mask.any() else None,
                 raw_white_fraction=float((frames['layer'][mask].min(1)>230).mean()) if mask.any() else None)
   report.append(record);(a.out/'results.json').write_text(json.dumps(report,indent=2));print(label,record['pixels'],'pixels','bbox_ground',round(record['bbox_bottom_vs_road_m'],3) if is_car else 'unvalidated',flush=True)
   destroy_sensors();actor.destroy();actors.remove(actor);w.tick()
 finally:
  for sensor in cams:
   if sensor.is_alive:sensor.stop();sensor.destroy()
  for actor in actors:
   if actor.is_alive:actor.destroy()
  if a.visible_sky or sky_roi:console('r.PathTracing.VisibleLights 0')
  w.clear_sky_light_map();console('r.MotionBlur.Amount 0.5');console('r.EyeAdaptation.MethodOverride -1');console('r.EyeAdaptation.LensAttenuation 0.78');w.apply_settings(original)

if __name__=='__main__':main()
