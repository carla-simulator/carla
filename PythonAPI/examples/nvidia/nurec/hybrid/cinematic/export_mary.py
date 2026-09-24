"""Run in Blender with the supplied Mary 4K blend; preserve its evaluated motion.

Writes a derivative only. The source blend contains object animation in addition
 to its Alembic mesh sequence; importing the source ABC alone loses locomotion.
"""
import bpy,json,os
from pathlib import Path
from mathutils import Vector
root=Path(os.environ['HYBRID_SHOT']);manifest=json.loads((root/'manifest.json').read_text());source=Path(manifest['source_asset'])
for cache in bpy.data.cache_files:cache.filepath=str(source/'rp_mary_4d_006_walkingCalling.abc')
scene=bpy.context.scene
assert scene.render.fps==30 and scene.render.fps_base==1
bpy.ops.object.select_all(action='DESELECT')
mesh=next(o for o in scene.objects if o.type=='MESH');mesh.select_set(True);bpy.context.view_layer.objects.active=mesh
samples=[]
end=int(manifest.get('mary_export_end',61))
for frame in range(1,end+1):
 scene.frame_set(frame);obj=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get());bounds=[obj.matrix_world@Vector(x) for x in obj.bound_box]
 samples.append({'source_frame':frame,'matrix_world':[list(row) for row in obj.matrix_world],'bounds_min_m':[min(v[i] for v in bounds) for i in range(3)],'bounds_max_m':[max(v[i] for v in bounds) for i in range(3)]})
output=root/'capture/mary';output.mkdir(parents=True,exist_ok=True)
(output/'blender-samples.json').write_text(json.dumps(samples,indent=2))
# Export centimeters, Y-up as standard Alembic; the UE Maya preset converts axes.
bpy.ops.wm.alembic_export(filepath=str(output/f'mary_0001_{end:04d}_cm.abc'),start=1,end=end,xsamples=1,gsamples=1,sh_open=0,sh_close=0,selected=True,flatten=True,global_scale=100,uvs=True,normals=True,face_sets=True,triangulate=True,as_background_job=False)
print('MARY_EXPORT_COMPLETE',output)
