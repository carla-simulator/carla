"""Reproducible surface revision; preserves the straight wheel-detail master.
Run in Blender. Artist material estimates; not measured manufacturer BRDFs.
"""
import bpy, bmesh, os, json
from pathlib import Path
from mathutils import Vector
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
bpy.ops.wm.open_mainfile(filepath=str(P/'source/tesla-model3-wheel-detail.blend'))
changes=[]
glass_repairs=[]
for o in bpy.data.collections['Tesla_Model3'].objects:
 if o.type!='MESH' or not any(m and m.name.startswith('glass') for m in o.data.materials):continue
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000001)
 closed=all(e.is_manifold for e in bm.edges);bm.free()
 if closed:
  removed=[m.name for m in o.modifiers if m.type=='SOLIDIFY']
  for m in list(o.modifiers):
   if m.type=='SOLIDIFY':o.modifiers.remove(m)
  glass_repairs.append({'object':o.name,'removed_nested_shell_modifiers':removed})

def surface(name,color,metal,rough,spec=.5,coat=0):
 m=bpy.data.materials.get(name)
 if not m:return
 b=m.node_tree.nodes.get('Principled BSDF')
 for field in ['Base Color','Metallic','Roughness','Specular IOR Level','Coat Weight']:
  for link in list(b.inputs[field].links):m.node_tree.links.remove(link)
 b.inputs['Base Color'].default_value=(*color,1);b.inputs['Metallic'].default_value=metal;b.inputs['Roughness'].default_value=rough;b.inputs['Specular IOR Level'].default_value=spec;b.inputs['Coat Weight'].default_value=coat;b.inputs['Coat Roughness'].default_value=.16
 changes.append(name)
 return m
surface('wheels.6',(.38,.40,.43),.95,.24,coat=.35)
surface('WheelDetail_RaisedRubber',(.014,.016,.018),0,.72,.28)
surface('WheelDetail_Velvet',(.009,.010,.012),0,.84,.22)
# Remove the synthetic branding panel's excessive color contrast, retaining relief.
surface('mirror_inside.0',(.88,.90,.92),1,.025)
surface('movsteer_1.0.1',(.55,.58,.61),1,.22)
surface('aluminium_light.0',(.65,.68,.72),1,.16)
surface('back_chrome_light.0',(.55,.58,.61),1,.19)
surface('black_lights.0',(.008,.010,.012),0,.32)
surface('movsteer_1.0.0',(.018,.020,.023),0,.52,.35)
surface('Plastic.0',(.016,.019,.022),0,.64,.32)
surface('Seat_Leather_white.0',(.65,.64,.61),0,.53,.35)
# Shared white trim includes the roof lining: split by object before detailing.
for o in bpy.data.collections['Tesla_Model3'].objects:
 if o.type!='MESH':continue
 if o.name.startswith('whiteleather_'):
  for slot in o.material_slots:
   if slot.material and slot.material.name=='Putih.0':
    m=slot.material.copy();m.name='SurfaceDetail_Headliner';slot.material=m
surface('SurfaceDetail_Headliner',(.51,.50,.47),0,.92,.2)
surface('Putih.0',(.67,.66,.63),0,.48,.35)
# Screen reflections must not wash away the display texture at driver eye height.
b=bpy.data.materials['LCDs.0'].node_tree.nodes.get('Principled BSDF');b.inputs['Roughness'].default_value=.28;b.inputs['Specular IOR Level'].default_value=.16
scene=bpy.context.scene;cam=scene.camera;scene.cycles.samples=40
scene.render.resolution_x=1600;scene.render.resolution_y=1000
bpy.ops.wm.save_as_mainfile(filepath=str(P/'source/tesla-model3-surface-upgrade.blend'))
(P/'audit/surface-upgrade.json').write_text(json.dumps({'materials':changes,'glass_repairs':glass_repairs,'status':'initial material pass; full upgrade checklist remains pending'},indent=2))
for name,loc,target,lens in [('surface-wheel',(2.55,-2.5,1.03),(1.4375,-.79,.36),62),('surface-interior',(.1,.4,1.15),(.9,-.05,.97),24),('surface-front',(5,-5,2.1),(0,0,.7),55)]:
 cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=lens;scene.render.filepath=str(P/'renders'/f'{name}.png');bpy.ops.render.render(write_still=True)
