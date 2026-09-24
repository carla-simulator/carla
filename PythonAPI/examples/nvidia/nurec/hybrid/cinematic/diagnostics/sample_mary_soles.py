import bpy,json,numpy as np
from pathlib import Path
r=Path('/home/german/Projects/CARLA_SOURCE/artifacts/hybrid-cinematic-mvp/sh031_contactfix');m=json.loads((r/'manifest.json').read_text());source=Path(m['source_asset'])
for c in bpy.data.cache_files:c.filepath=str(source/'rp_mary_4d_006_walkingCalling.abc')
mesh=next(o for o in bpy.context.scene.objects if o.type=='MESH');frames=sorted({f['mary_source_frame'] for f in json.loads((r/'capture.json').read_text())});out=r/'sole-samples';out.mkdir(exist_ok=True)
for frame in frames:
 bpy.context.scene.frame_set(frame);o=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get());geo=o.to_mesh();v=np.empty(len(geo.vertices)*3,dtype=np.float32);geo.vertices.foreach_get('co',v);v=v.reshape(-1,3);matrix=np.array(o.matrix_world);v=v@matrix[:3,:3].T+matrix[:3,3];v=v[v[:,2]<.15];np.save(out/f'{frame:04d}.npy',v);o.to_mesh_clear()
print('SOLE_SAMPLES_DONE',len(frames))
