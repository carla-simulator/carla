import bpy
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:] # get all args after "--"

obj_in = argv[0]
fbx_out = argv[1]

bpy.ops.import_scene.obj(filepath=obj_in, axis_forward='-Z', axis_up='Y')
bpy.ops.export_scene.fbx(filepath=fbx_out, axis_forward='-Z', axis_up='Y')
