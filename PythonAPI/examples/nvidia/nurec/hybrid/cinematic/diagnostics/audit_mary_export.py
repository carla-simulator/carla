import bpy,numpy as np,json
from pathlib import Path
r=Path('/home/german/Projects/CARLA_SOURCE/artifacts/hybrid-cinematic-mvp/sh031_contactfix');m=json.loads((r/'manifest.json').read_text())
for c in bpy.data.cache_files:c.filepath=str(Path(m['source_asset'])/'rp_mary_4d_006_walkingCalling.abc')
src=next(o for o in bpy.context.scene.objects if o.type=='MESH');before=set(bpy.data.objects);bpy.ops.wm.alembic_import(filepath=str(r/'capture/mary/mary_0001_0261_cm.abc'));dst=next(o for o in set(bpy.data.objects)-before if o.type=='MESH');out=[]
def uv(o):
 e=o.evaluated_get(bpy.context.evaluated_depsgraph_get());me=e.to_mesh();v=np.empty(len(me.uv_layers.active.data)*2,dtype=np.float32);me.uv_layers.active.data.foreach_get('uv',v);e.to_mesh_clear();return v
for n in [1,2,3,40,156,157,158]:
 bpy.context.scene.frame_set(n);a=uv(dst);row={'sample':n,'derived_uv_count':len(a),'matches':[]}
 for i in range(max(1,n-3),n+4):
  bpy.context.scene.frame_set(i);b=uv(src)
  if len(a)==len(b):row['matches'].append({'native':i,'rms':float(np.sqrt(np.mean((a-b)**2)))})
 out.append(row)
(r/'export-uv-audit.json').write_text(json.dumps(out,indent=2));print(out)
