"""Blender: rigid shoe offsets fading below knees on the native animated cache.
A Geometry Nodes Set Position modifier preserves native topology and UV timing.
"""
import bpy,json,os,numpy as np
from pathlib import Path
r=Path(os.environ['HYBRID_SHOT']);m=json.loads((r/'manifest.json').read_text());source=Path(m['source_asset'])
for c in bpy.data.cache_files:c.filepath=str(source/'rp_mary_4d_006_walkingCalling.abc')
scene=bpy.context.scene;assert scene.render.fps==30
src=next(o for o in scene.objects if o.type=='MESH');capture=json.loads((r/'capture.json').read_text());contact=json.loads((r/'sole-contact.json').read_text());native=np.array([f['mary_source_frame'] for f in capture]);roots=np.array([f['actors'][0]['transform']['location'][:2] for f in capture]);body=np.array([f['root_height_m'] for f in contact['frames']]);th=np.radians(contact['yaw_degrees']);rot=np.array([[np.cos(th),-np.sin(th)],[np.sin(th),np.cos(th)]]);field=np.load(r/'receiver-heightfield.npz');xs=field['xs'];ys=field['ys'];h=field['height']
def surface(q):
 px=(q[:,0]-xs[0])/(xs[1]-xs[0]);py=(q[:,1]-ys[0])/(ys[1]-ys[0]);ix=np.clip(np.floor(px).astype(int),0,len(xs)-2);iy=np.clip(np.floor(py).astype(int),0,len(ys)-2);x=np.clip(px-ix,0,1);y=np.clip(py-iy,0,1);a=h[iy,ix];b=h[iy,ix+1];c=h[iy+1,ix+1];d=h[iy+1,ix];return np.where(x>=y,a+(b-a)*x+(c-b)*y,a+(c-d)*x+(d-a)*y)
report=[];parameters=[];end=int(m['mary_export_end'])
for frame in range(1,end+1):
 scene.frame_set(frame);obj=src.evaluated_get(bpy.context.evaluated_depsgraph_get());geo=obj.to_mesh();matrix=np.array(obj.matrix_world);v=np.empty(len(geo.vertices)*3,dtype=np.float32);geo.vertices.foreach_get('co',v);v=v.reshape(-1,3);local=v@matrix[:3,:3].T;xy=local[:,:2].copy();xy[:,1]*=-1;target=np.array([np.interp(frame,native,roots[:,i]) for i in range(2)]);xy=xy@rot.T+target;terrain=surface(xy);root_z=np.interp(frame,native,body);z=local[:,2];native_min=float(z.min());low=z<.18;foot_xy=v[low,:2];centres=np.array([foot_xy[np.argmin(foot_xy[:,0])],foot_xy[np.argmax(foot_xy[:,0])]])
 for _ in range(12):
  labels=np.argmin(((foot_xy[:,None,:]-centres[None,:,:])**2).sum(-1),axis=1)
  for j in range(2):
   if np.any(labels==j):centres[j]=foot_xy[labels==j].mean(0)
 labels=np.argmin(((v[:,:2,None]-centres.T[None,:,:])**2).sum(1),axis=1);shifts=[]
 for j in range(2):
  sole=low&(labels==j)
  if not sole.any():raise RuntimeError('Missing shoe cluster')
  lift=float(z[sole].min()-native_min);shifts.append(float(np.max(terrain[sole]-z[sole])+lift-root_z+.004))
 t=np.clip((z-.18)/.4,0,1);weight=1-t*t*(3-2*t);dz=np.array(shifts)[labels]*weight;clearance=z[low]+dz[low]+root_z-terrain[low]
 report.append({'source_frame':frame,'min_sole_clearance_m':float(clearance.min()),'max_vertex_z_correction_m':float(np.abs(dz).max()),'foot_offsets_m':shifts,'vertex_count':len(v)});parameters.append((frame,centres,shifts,float(matrix[2,2])));obj.to_mesh_clear()
# SetPosition on the existing animated mesh avoids replacing the object/data
# during Alembic sampling (which can desynchronize topology and texture time).
g=bpy.data.node_groups.new('RigidShoeGroundContact','GeometryNodeTree');g.interface.new_socket(name='Geometry',in_out='INPUT',socket_type='NodeSocketGeometry');g.interface.new_socket(name='Geometry',in_out='OUTPUT',socket_type='NodeSocketGeometry');nodes=g.nodes;links=g.links
inp=nodes.new('NodeGroupInput');out=nodes.new('NodeGroupOutput');pos=nodes.new('GeometryNodeInputPosition');sep=nodes.new('ShaderNodeSeparateXYZ');links.new(pos.outputs['Position'],sep.inputs[0]);xy=nodes.new('ShaderNodeCombineXYZ');links.new(sep.outputs['X'],xy.inputs['X']);links.new(sep.outputs['Y'],xy.inputs['Y'])
dist=[]
for _ in range(2):
 n=nodes.new('ShaderNodeVectorMath');n.operation='DISTANCE';links.new(xy.outputs[0],n.inputs[0]);dist.append(n)
select=nodes.new('ShaderNodeMath');select.operation='LESS_THAN';links.new(dist[0].outputs['Value'],select.inputs[0]);links.new(dist[1].outputs['Value'],select.inputs[1])
mix=nodes.new('ShaderNodeMapRange');mix.clamp=True;links.new(select.outputs[0],mix.inputs['Value']);mix.inputs['From Min'].default_value=0;mix.inputs['From Max'].default_value=1
fade=nodes.new('ShaderNodeMapRange');fade.interpolation_type='SMOOTHSTEP';fade.clamp=True;links.new(sep.outputs['Z'],fade.inputs['Value']);fade.inputs['To Min'].default_value=1;fade.inputs['To Max'].default_value=0
mul=nodes.new('ShaderNodeMath');mul.operation='MULTIPLY';links.new(mix.outputs[0],mul.inputs[0]);links.new(fade.outputs[0],mul.inputs[1]);offset=nodes.new('ShaderNodeCombineXYZ');links.new(mul.outputs[0],offset.inputs['Z']);setpos=nodes.new('GeometryNodeSetPosition');links.new(inp.outputs['Geometry'],setpos.inputs['Geometry']);links.new(offset.outputs[0],setpos.inputs['Offset']);links.new(setpos.outputs['Geometry'],out.inputs['Geometry'])
for frame,centres,shifts,scale in parameters:
 for j in range(2):
  socket=dist[j].inputs[1];socket.default_value=(*centres[j],0);socket.keyframe_insert(data_path='default_value',frame=frame)
 for socket,value in [(mix.inputs['To Min'],shifts[1]/scale),(mix.inputs['To Max'],shifts[0]/scale),(fade.inputs['From Min'],.18/scale),(fade.inputs['From Max'],.58/scale)]:socket.default_value=value;socket.keyframe_insert(data_path='default_value',frame=frame)
modifier=src.modifiers.new('CinematicShoeContact','NODES');modifier.node_group=g
bpy.ops.object.select_all(action='DESELECT');src.select_set(True);bpy.context.view_layer.objects.active=src
outdir=r/'capture/mary';outdir.mkdir(parents=True,exist_ok=True)
bpy.ops.wm.alembic_export(filepath=str(outdir/f'mary_0001_{end:04d}_cm.abc'),start=1,end=end,xsamples=1,gsamples=1,sh_open=0,sh_close=0,selected=True,flatten=True,global_scale=100,uvs=True,normals=True,face_sets=True,triangulate=True,as_background_job=False)
(r/'mary-deformation-validation.json').write_text(json.dumps({'method':'rigid shoe offsets on native mesh via SetPosition, smoothly fading below knees','frames':report},indent=2));print('ADAPTED_CACHE_COMPLETE',len(report))
