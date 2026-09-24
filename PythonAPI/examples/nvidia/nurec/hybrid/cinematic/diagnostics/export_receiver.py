import bpy,numpy as np
from pathlib import Path
r=Path('/home/german/Projects/CARLA_SOURCE/artifacts/hybrid-cinematic-mvp/sh031_contactfix');d=np.load(r/'receiver-mesh.npz');v=d['vertices'];v[:,1]*=-1;ny,nx=d['shape'];faces=[]
for y in range(ny-1):
 for x in range(nx-1):
  i=y*nx+x;faces.extend([(i,i+nx+1,i+1),(i,i+nx,i+nx+1)])
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);m=bpy.data.meshes.new('Receiver');m.from_pydata(v,[],faces);m.update();o=bpy.data.objects.new('Receiver',m);bpy.context.collection.objects.link(o);o.select_set(True);bpy.context.view_layer.objects.active=o;bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
bpy.ops.export_scene.fbx(filepath=str(r/'receiver.fbx'),use_selection=True,global_scale=1,apply_unit_scale=True,axis_forward='-Y',axis_up='Z',bake_anim=False)
print('RECEIVER_EXPORTED',len(v),len(faces))
