"""Prepare the CC BY Tesla 2018 Model 3 candidate for fidelity review (Blender).
Source: Ameer Studio, Sketchfab 5ef9b845aaf44203b6d04e2c677e444f.
Source retrieved from the public Objaverse collection; attribution retained.
This is a derived asset, not a model authored from scratch.
"""
import bpy,bmesh,math,json
from pathlib import Path
import os
from mathutils import Vector
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(P/'reference/candidate-model.glb'))
# Original vector display artwork replaces the source's mismatched Model S screen.
display=bpy.data.images.load(str(P/'reference/Model3_Display.png'))
for node in bpy.data.materials['LCDs.0'].node_tree.nodes:
 if node.type=='TEX_IMAGE':node.image=display
objs=[o for o in bpy.context.scene.objects if o.type=='MESH']
# All coordinates are baked from the scene graph before axis/metric conversion.
world={o.name:[o.matrix_world@v.co for v in o.data.vertices] for o in objs}
knots=[(-261.964,-2.4155),(-144.721,-1.4375),(175.683,1.4375),(263.880,2.2785)]
def mx(x):
 for (a,b),(c,d) in zip(knots,knots[1:]):
  if x<=c:return b+(x-a)*(d-b)/(c-a)
 return 2.2785+(x-263.880)*.009
R=.254+.235*.35
wheel_centers={}
for rear in [False,True]:
 pref='wheels.001_wheels.3' if rear else 'wheels_wheels.3'
 o=next(o for o in objs if o.name.startswith(pref));points=world[o.name]
 for side in [-1,1]:
  pp=[v for v in points if v.x*side>0];lo=Vector([min(v[i] for v in pp) for i in range(3)]);hi=Vector([max(v[i] for v in pp) for i in range(3)]);wheel_centers[(rear,side)]=((lo+hi)/2,(hi.z-lo.z)/2)
for o in objs:
 points=world[o.name];normal_matrix=o.matrix_world.to_3x3().inverted().transposed();source_normals=[normal_matrix@n.vector for n in o.data.corner_normals];o.parent=None;o.matrix_world.identity()
 for v,p in zip(o.data.vertices,points):
  if o.name.startswith('wheels'):
   rear='.001_' in o.name;side=1 if p.x>0 else -1;center,r=wheel_centers[(rear,side)]
   v.co=Vector(((-1.4375 if rear else 1.4375)+(p.y-center.y)*R/r,-side*.79-(p.x-center.x)*R/r,R+(p.z-center.z)*R/r))
  else:v.co=Vector((mx(p.y),-p.x*(1.85/218.322),R+(p.z+37.17)*(1.443-R)/(82.65+37.17)))
 for poly in o.data.polygons:poly.use_smooth=True
 normals=[]
 for loop,n in zip(o.data.loops,source_normals):
  y=points[loop.vertex_index].y
  dx=next(((d-b)/(c-a) for (a,b),(c,d) in zip(knots,knots[1:]) if y<=c),.009)
  if o.name.startswith('wheels'):normals.append(Vector((n.y,-n.x,n.z)).normalized())
  else:normals.append(Vector((n.y/dx,-n.x/(1.85/218.322),n.z/((1.443-R)/(82.65+37.17)))).normalized())
 o.data.normals_split_custom_set(normals)
for o in list(bpy.data.objects):
 if o.type!='MESH':bpy.data.objects.remove(o,do_unlink=True)
# The source display is too tall; match the original Model 3 landscape bezel.
display_mesh=bpy.data.objects['LCDs_LCDs.0_0']
pts=[v.co.copy() for v in display_mesh.data.vertices]
lo=Vector([min(v[i] for v in pts) for i in range(3)]);hi=Vector([max(v[i] for v in pts) for i in range(3)])
center=(lo+hi)/2;sy=.348/(hi.y-lo.y);sz=.214/(hi.z-lo.z)
normals=[n.vector.copy() for n in display_mesh.data.corner_normals]
for v in display_mesh.data.vertices:
 v.co.x=center.x+(v.co.x-center.x)*sz;v.co.y=center.y+(v.co.y-center.y)*sy;v.co.z=hi.z-(hi.z-v.co.z)*sz
display_mesh.data.normals_split_custom_set([Vector((n.x/sz,n.y/sy,n.z/sz)).normalized() for n in normals])
# Original texture detail remains. Replace flat imported response with physical materials.
material_report=[]
for m in bpy.data.materials:
 if not m.use_nodes:continue
 b=m.node_tree.nodes.get('Principled BSDF');name=m.name.lower()
 if not b:continue
 b.inputs['Alpha'].default_value=1
 for link in list(b.inputs['Alpha'].links):m.node_tree.links.remove(link)
 if name.startswith('primary'):
  for link in list(b.inputs['Base Color'].links):m.node_tree.links.remove(link)
  b.inputs['Base Color'].default_value=(.53,.57,.61,1);b.inputs['Metallic'].default_value=.12;b.inputs['Roughness'].default_value=.23;b.inputs['Coat Weight'].default_value=1;b.inputs['Coat Roughness'].default_value=.08
 elif name.startswith('glass') or name.startswith('tembus'):
  for link in list(b.inputs['Base Color'].links):m.node_tree.links.remove(link)
  b.inputs['Base Color'].default_value=(.72,.8,.83,1) if name.startswith('glass') else (.96,.97,1,1)
  b.inputs['Transmission Weight'].default_value=1;b.inputs['Roughness'].default_value=.045;b.inputs['IOR'].default_value=1.5
 elif any(t in name for t in ['chrome','aluminium']):b.inputs['Metallic'].default_value=1;b.inputs['Roughness'].default_value=.23
 elif name.startswith('wheels.6'):b.inputs['Metallic'].default_value=.9;b.inputs['Roughness'].default_value=.27
 elif name.startswith('wheels.3'):b.inputs['Roughness'].default_value=.68
 elif 'leather' in name or 'movsteer_1.0.0' in name:b.inputs['Roughness'].default_value=.48
 elif 'carpet' in name:b.inputs['Roughness'].default_value=.9
 else:b.inputs['Roughness'].default_value=.4
 material_report.append({'name':m.name,'roughness':b.inputs['Roughness'].default_value,'metallic':b.inputs['Metallic'].default_value,'textures':[n.image.name for n in m.node_tree.nodes if n.type=='TEX_IMAGE' and n.image]})
# Optical surfaces need physical thickness to avoid air/glass interface artifacts.
for o in objs:
 if any(m and (m.name.startswith('glass') or m.name.startswith('tembus')) for m in o.data.materials):
  # Source windows are already closed volumes. Thickening those again creates
  # overlapping optical interfaces; only open lens surfaces need solidification.
  bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000001)
  closed=all(e.is_manifold for e in bm.edges);bm.free()
  if not closed:
   sol=o.modifiers.new('Glass thickness','SOLIDIFY');sol.thickness=.0025;sol.offset=-1
# Unique paint slot for the CARLA color API (all painted parts consolidated on export).
paint=bpy.data.materials.get('primary');paint.name='Bodywork_Mat'
for o in objs:
 for slot in o.material_slots:
  if slot.material and slot.material.name.startswith('primary'):slot.material=paint
# Source remains editable; stage is excluded from future exports by collection.
vehicle=bpy.data.collections.new('Tesla_Model3');bpy.context.scene.collection.children.link(vehicle)
for o in objs:
 for c in list(o.users_collection):c.objects.unlink(o)
 vehicle.objects.link(o)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=48;scene.render.resolution_x=1800;scene.render.resolution_y=1100;scene.render.resolution_percentage=100
scene.world=bpy.data.worlds.new('Neutral_Studio');scene.world.use_nodes=True;scene.world.node_tree.nodes['Background'].inputs[0].default_value=(.16,.18,.21,1)
for loc,power,size in [((1,-4,5),1100,5),((-3,2,3),1400,4),((5,1,2.5),900,3)]:
 bpy.ops.object.light_add(type='AREA',location=loc);o=bpy.context.object;o.data.energy=power;o.data.shape='RECTANGLE';o.data.size=size;o.data.size_y=2;o.rotation_euler=(Vector((0,0,.7))-o.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.mesh.primitive_plane_add(size=200);floor=bpy.context.object;floor.name='Studio_Floor';m=bpy.data.materials.new('Floor');m.diffuse_color=(.1,.11,.13,1);m.use_nodes=True;m.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.1,.11,.13,1);floor.data.materials.append(m)
bpy.ops.object.camera_add();cam=bpy.context.object;scene.camera=cam;cam.data.lens=55
cam.location=(6.8,-6.8,2.9);cam.rotation_euler=(Vector((0,0,.68))-cam.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.wm.save_as_mainfile(filepath=str(P/'source/tesla-model3-fidelity.blend'))
(P/'audit/prepared-materials.json').write_text(json.dumps(material_report,indent=2))
(P/'audit/prepared-dimensions.json').write_text(json.dumps({'wheelbase_m':2.875,'track_m':1.58,'tire_radius_m':R,'body_length_m':4.694,'body_width_m':1.85,'height_target_m':1.443,'source':'CC BY Ameer Studio Tesla 2018 Model 3','status':'visual review required'},indent=2))
for name,loc,target in [('front-quarter',(6.8,-6.8,2.9),(0,0,.68)),('side',(0,-9,1.1),(0,0,.72)),('rear-quarter',(-6.8,-6.8,2.7),(0,0,.7)),('interior',(.25,.40,1.13),(1.0,-.05,.90))]:
 cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=24 if name=='interior' else 55;scene.render.filepath=str(P/'renders'/('fidelity-'+name+'.png'));bpy.ops.render.render(write_still=True)
