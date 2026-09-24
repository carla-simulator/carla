import json,sys,math
from pathlib import Path
import numpy as np
from shapely.geometry import MultiPoint
root=Path(sys.argv[1]); data={name:json.loads((root/name/'behavior.json').read_text()) for name in ['main','no_response','absent','replay'] if (root/name/'behavior.json').exists()}
def actors(d):return [{a['role']:a for a in f['actors']} for f in d['frames']]
def poly(a):return MultiPoint(a['footprint']).convex_hull
m=data['main']; rows=actors(m);t=np.array([f['t'] for f in m['frames']]);e=np.array([r['hero']['speed'] for r in rows]);brake=next(ev['time_s'] for ev in m['events'] if ev['name']=='ego_emergency_brake');peak=np.max((e[:-4]-e[4:])/(t[4:]-t[:-4]));report={'events':m['events'],'collisions':m['collisions'],'peak_deceleration_4_frames_mps2':float(peak),'minimum_footprint_gaps_m':{role: min(poly(r['hero']).distance(poly(r[role])) for r in rows) for role in ['lead','mary_proxy']},'lead_pedestrian_minimum_gap_m':min(poly(r['lead']).distance(poly(r['mary_proxy'])) for r in rows)}
for name,d in data.items():
 if name=='main':continue
 rr=actors(d);stop=min([next((i for i,f in enumerate(d['frames']) if f['frame']==c['frame']),len(rr)) for c in d['collisions']]+[len(rr)])
 roles=['lead','hero']+(['mary_proxy'] if name!='absent' else [])
 errs={}
 for role in roles:
  inds=[i for i in range(min(len(rr),len(rows),stop)) if role!='hero' or name=='replay' or t[i]<=brake]
  errs[role]={'position_m':max(float(np.linalg.norm([rows[i][role][k]-rr[i][role][k] for k in ['x','y','z']])) for i in inds),'speed_mps':max(abs(rows[i][role]['speed']-rr[i][role]['speed']) for i in inds)}
 report[name]={'collisions':d['collisions'],'matched_errors':errs}
 if name=='absent':
  s=np.array([r['hero']['speed'] for r in rr]);delta=(e[:-4]-e[4:]-s[:-4]+s[4:])/(t[4:]-t[:-4]);report[name]['maximum_additional_deceleration_mps2']=float(delta.max())
 if name=='replay':report[name]['event_time_error_s']={v['name']:v['time_s']-next(x['time_s'] for x in m['events'] if x['name']==v['name']) for v in d['events']}
 if name=='no_response':
  ev=next(x for x in d['events'] if x['name']=='pedestrian_start');cross=np.array([ev['crossing_x'],-ev['crossing_y']]);yaw=math.radians(rr[0]['mary_proxy']['yaw']+90);fwd=np.array([math.cos(yaw),math.sin(yaw)]);right=np.array([-fwd[1],fwd[0]])
  for i,r in enumerate(rr[:stop+1]):
   ego=np.array(r['hero']['footprint']);ped=np.array(r['mary_proxy']['footprint']);el=ego@right;pl=ped@right
   if max(el.min(),pl.min())<=min(el.max(),pl.max()):
    dist=float(cross@fwd-(ego@fwd).max());report[name]['corridor_entry']={'time_s':float(t[i]),'front_to_crossing_m':dist,'front_ttc_s':dist/r['hero']['speed'],'time_to_sensor_contact_s':float(t[stop]-t[i]) if stop<len(t) else None};break
(root/'measurements.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
