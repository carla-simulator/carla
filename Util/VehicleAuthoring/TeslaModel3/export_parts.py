import bpy,bmesh,json
from pathlib import Path
import os
from mathutils import Vector
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'));D=P/'export';D.mkdir(exist_ok=True);(D/'textures').mkdir(exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=os.environ.get('CARLA_MODEL3_BLEND',str(P/'source/tesla-model3-fidelity.blend')))
vehicle=list(bpy.data.collections['Tesla_Model3'].objects)
# Export texture pixels explicitly; Unreal material authoring uses the manifest.
images={}
for im in bpy.data.images:
 if im.size[0]>0 and im.name!='Render Result':
  im.filepath_raw=str(D/'textures'/(im.name+'.png'));im.file_format='PNG';im.save();images[im.name]=str(im.filepath_raw)
mats=[]
for m in bpy.data.materials:
 if not m.use_nodes or m.name=='Floor':continue
 b=m.node_tree.nodes.get('Principled BSDF')
 if not b:continue
 base=None
 for link in b.inputs['Base Color'].links:
  if link.from_node.type=='TEX_IMAGE':base=link.from_node.image.name
 # glTF often inserts multiply nodes; trace upstream to the base-color image.
 def image_up(node,seen):
  if node in seen:return None
  seen.add(node)
  if node.type=='TEX_IMAGE' and node.image:return node.image.name
  for inp in node.inputs:
   for link in inp.links:
    found=image_up(link.from_node,seen)
    if found:return found
  return None
 if not base and b.inputs['Base Color'].is_linked:base=image_up(b.inputs['Base Color'].links[0].from_node,set())
 mats.append(dict(name=m.name,base=list(b.inputs['Base Color'].default_value),roughness=b.inputs['Roughness'].default_value,metallic=b.inputs['Metallic'].default_value,coat=b.inputs['Coat Weight'].default_value,coat_roughness=b.inputs['Coat Roughness'].default_value,specular=b.inputs['Specular IOR Level'].default_value,transmission=b.inputs['Transmission Weight'].default_value,texture=base,normal_texture=m.get('carla_normal_texture'),roughness_texture=m.get('carla_roughness_texture')))
# Separate paired wheel meshes and retain independent moving wheels.
groups={n:[] for n in ['Body','DoorFL','DoorFR','DoorRL','DoorRR','Hood','Trunk','WheelFL','WheelFR','WheelRL','WheelRR','CaliperFL','CaliperFR','CaliperRL','CaliperRR']}
anchors={'Body':(0,0,0),'DoorFL':(.90,.74,.75),'DoorFR':(.90,-.74,.75),'DoorRL':(-.15,.78,.78),'DoorRR':(-.15,-.78,.78),'Hood':(1.14,0,.91),'Trunk':(-1.60,0,1.03),'WheelFL':(1.4375,.79,.33625),'WheelFR':(1.4375,-.79,.33625),'WheelRL':(-1.4375,.79,.33625),'WheelRR':(-1.4375,-.79,.33625)}
for part in ['FL','FR','RL','RR']:anchors['Caliper'+part]=anchors['Wheel'+part]
for o in vehicle:
 if o.name.startswith('wheels'):
  rear='.001_' in o.name
  for side in ['L','R']:
   du=o.copy();du.data=o.data.copy();bpy.context.collection.objects.link(du);bm=bmesh.new();bm.from_mesh(du.data);bmesh.ops.delete(bm,geom=[v for v in bm.verts if (v.co.y<0 if side=='L' else v.co.y>0)],context='VERTS');bm.to_mesh(du.data);bm.free();groups['Wheel'+('R' if rear else 'F')+side].append(du)
 else:
  name=o.name
  caliper=next((part for prefix,part in [('hub_lf_','FL'),('hub_rf_','FR'),('hub_lb_','RL'),('hub_rb_','RR')] if name.startswith(prefix)),None)
  if caliper:groups['Caliper'+caliper].append(o);continue
  g=next((group for prefix,group in [('door_lf','DoorFL'),('door_rf','DoorFR'),('door_lr','DoorRL'),('door_rr','DoorRR'),('bonnet','Hood'),('boot_','Trunk')] if name.startswith(prefix)),'Body');groups[g].append(o)
report=[]
for name,parts in groups.items():
 bpy.ops.object.select_all(action='DESELECT')
 # Bake modifiers, consolidate materials and move geometry into the joint's local frame.
 for o in parts:
  o.select_set(True);bpy.context.view_layer.objects.active=o
  for mod in list(o.modifiers):bpy.ops.object.modifier_apply(modifier=mod.name)
 bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name='SM_Model3_'+name
 anchor=Vector(anchors[name])
 for v in o.data.vertices:v.co-=anchor
 bpy.ops.export_scene.fbx(filepath=str(D/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',use_mesh_modifiers=True,bake_anim=False,path_mode='COPY',embed_textures=False)
 report.append(dict(part=name,file=str(D/(name+'.fbx')),anchor_m=anchors[name],triangles=sum(len(p.vertices)-2 for p in o.data.polygons)))
(D/'manifest.json').write_text(json.dumps(dict(parts=report,materials=mats,images=images,source='https://sketchfab.com/3d-models/tesla-2018-model-3-5ef9b845aaf44203b6d04e2c677e444f',author='Ameer Studio',license='CC-BY-4.0'),indent=2))
print('EXPORT COMPLETE',report)
