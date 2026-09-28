"""Build the reviewed optical-detail revision from the detail master.

Reflector proportions follow the local original-generation headlight photograph.
These surfaces approximate visible construction; they are not optical CAD.
"""
import bpy
import bmesh
import json
import math
import os
import numpy as np
from pathlib import Path
from mathutils import Vector

P = Path(os.environ.get('CARLA_MODEL3_WORKDIR', Path(__file__).resolve().parents[4] / 'artifacts/tesla-model3'))
bpy.ops.wm.open_mainfile(filepath=str(P / 'source/tesla-model3-detail-upgrade.blend'))
collection = bpy.data.collections['Tesla_Model3']
for obj in list(collection.objects):
    if obj.name.startswith(('SurfaceDetail_LowBeamReflector', 'SurfaceDetail_LowBeamEmitter')):
        bpy.data.objects.remove(obj, do_unlink=True)

reflector = bpy.data.materials['SurfaceDetail_Reflector']
reflector.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value = .085
report = {'status': 'generated—requires review after changes', 'reflectors': []}

# The source duplicates the same inner rear light surface for reverse and
# indicators. Opaque overlapping copies hide the amber state. Use one surface
# per side and let the Unreal material select the requested light color.
report['combined_rear_lamps'] = []
reverse = bpy.data.objects.get('lightrevese_boot_revlight L_0')
if reverse is None:
    raise RuntimeError('Expected paired rear reverse-light surface is missing')
for side, sign in [('l', 1), ('r', -1)]:
    obj = reverse.copy()
    obj.data = reverse.data.copy()
    obj.name = 'SurfaceDetail_RearCombined_' + side
    collection.objects.link(obj)
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if v.co.y * sign < 0], context='VERTS')
    bm.to_mesh(obj.data)
    bm.free()
    material = bpy.data.materials['revlight_L'].copy()
    material.name = 'reverse_indicator_' + side
    obj.data.materials.clear()
    obj.data.materials.append(material)
    report['combined_rear_lamps'].append(obj.name)
bpy.data.objects.remove(reverse, do_unlink=True)
for obj in list(collection.objects):
    if obj.name.startswith(('light_turn_lr_boot_', 'light_turn_rr_boot_')):
        bpy.data.objects.remove(obj, do_unlink=True)

# Upper white position strips and lower fog lamps shared a source material.
# Separate the slots so CARLA can control the two assemblies independently.
for side in ('l', 'r'):
    material = bpy.data.materials['foglight_' + side].copy()
    material.name = 'front_position_' + side
    for obj in collection.objects:
        if obj.type == 'MESH' and obj.name.startswith('chrome_foglight_' + side + '_'):
            obj.data.materials.clear()
            obj.data.materials.append(material)

# The original Image_8 is a photographed/baked chrome reflection. It was wired
# into diffuse color, so its warped highlights never followed the environment.
report['removed_baked_reflections'] = []
for name in ('indicator_lf', 'indicator_rf', 'foglight_l', 'foglight_r', 'front_position_l', 'front_position_r'):
    material = bpy.data.materials[name]
    shader = material.node_tree.nodes.get('Principled BSDF')
    for link in list(shader.inputs['Base Color'].links):
        material.node_tree.links.remove(link)
    # Remove the disconnected sample so it cannot be accidentally relinked.
    for node in list(material.node_tree.nodes):
        if node.type == 'TEX_IMAGE':
            material.node_tree.nodes.remove(node)
    diffuser = name.startswith(('foglight', 'front_position'))
    shader.inputs['Base Color'].default_value = (.8, .8, .8, 1) if diffuser else (.65, .68, .72, 1)
    shader.inputs['Metallic'].default_value = 0 if diffuser else 1
    shader.inputs['Roughness'].default_value = .3 if diffuser else .18
    report['removed_baked_reflections'].append(name)

# Vertical optical flutes replace the photographed ribbing in the atlas.
# Assign a fresh planar UV frame to the two outer reflector faces so flute
# spacing follows their physical width, without changing lamp silhouettes.
size = 1024
nx = .22 * np.sin(np.arange(size) / size * 2 * math.pi * 14)
normal = np.zeros((size, size, 4), dtype=np.float32)
normal[:, :, 0] = nx * .5 + .5
normal[:, :, 1] = .5
normal[:, :, 2] = np.sqrt(1 - nx ** 2) * .5 + .5
normal[:, :, 3] = 1
image = bpy.data.images.new('SurfaceDetail_ReflectorFlutes_Normal', size, size)
image.colorspace_settings.name = 'Non-Color'
image.pixels.foreach_set(normal.ravel())
image.pack()
for name in ('indicator_lf', 'indicator_rf'):
    material = bpy.data.materials[name]
    texture = material.node_tree.nodes.new('ShaderNodeTexImage')
    texture.image = image
    mapping = material.node_tree.nodes.new('ShaderNodeNormalMap')
    material.node_tree.links.new(texture.outputs['Color'], mapping.inputs['Color'])
    material.node_tree.links.new(mapping.outputs['Normal'], material.node_tree.nodes.get('Principled BSDF').inputs['Normal'])
    material['carla_normal_texture'] = image.name
for obj in collection.objects:
    if obj.type != 'MESH' or not any(m.name in ('indicator_lf', 'indicator_rf') for m in obj.data.materials):
        continue
    uv = obj.data.uv_layers.active or obj.data.uv_layers.new()
    for loop in obj.data.loops:
        point = obj.matrix_world @ obj.data.vertices[loop.vertex_index].co
        uv.data[loop.index].uv = ((abs(point.y) - .63) / .16, (point.z - .62) / .12)

# The outer indicator reflector carries disconnected triangle corners and
# imported shading. Reconstruct shared normals before evaluating curvature.
report['outer_reflectors'] = []
for obj in collection.objects:
    if obj.type != 'MESH' or not obj.name.startswith('turn_indicat_'):
        continue
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=.000005)
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bm.to_mesh(obj.data)
    bm.free()
    obj.data.normals_split_custom_set([(0, 0, 0)] * len(obj.data.loops))
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    mod = obj.modifiers.new('Reflector surface refinement', 'SUBSURF')
    mod.levels = 1
    mod.render_levels = 1
    report['outer_reflectors'].append(obj.name)

def mesh(name, vertices, faces, material):
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name, data)
    collection.objects.link(obj)
    data.materials.append(material)
    for polygon in data.polygons:
        polygon.use_smooth = True
    return obj

for side, light in [(-1, 'right_front_light'), (1, 'left_front_light')]:
    normal = Vector((.82, side * .32, .46)).normalized()
    u = Vector((-side * .32, .82, 0)).normalized()
    v = normal.cross(u).normalized()
    for k in range(3):
        center = Vector((1.921, side * (.494 + k * .048), .655 + k * .003))
        vertices, faces = [], []
        segments, rings = 64, 12
        for ring in range(rings + 1):
            t = ring / rings
            width, height = .009 + .013 * t, .006 + .016 * t
            # A curved bowl replaces the four flat trapezoids. Small vertical
            # flutes split the reflected highlight, as in the reference photo.
            for j in range(segments):
                angle = 2 * math.pi * j / segments
                ca, sa = math.cos(angle), math.sin(angle)
                x = math.copysign(abs(ca) ** .35, ca)
                y = math.copysign(abs(sa) ** .35, sa)
                flute = .00035 * math.cos(x * math.pi * 6) * math.sin(math.pi * t) ** 2
                depth = -.025 * (1 - t * t) + flute
                vertices.append(center + u * width * x + v * height * y + normal * depth)
        for ring in range(rings):
            for j in range(segments):
                a = ring * segments + j
                b = ring * segments + (j + 1) % segments
                faces.append((a, a + segments, b + segments, b))
        obj = mesh('SurfaceDetail_LowBeamReflector', vertices, faces, reflector)
        report['reflectors'].append({'name': obj.name, 'quads': len(faces)})
        points = [center + u * a * .0085 + v * b * .0055 - normal * .0245
                  for a, b in [(-1, -1), (1, -1), (1, 1), (-1, 1)]]
        mesh('SurfaceDetail_LowBeamEmitter', points, [(0, 1, 2, 3)], bpy.data.materials[light])

scene = bpy.context.scene
camera = scene.camera
camera.location = (3, -2, 1.15)
camera.rotation_euler = (Vector((1.88, -.65, .7)) - camera.location).to_track_quat('-Z', 'Y').to_euler()
camera.data.lens = 75
scene.cycles.samples = 48
scene.render.resolution_x = 1600
scene.render.resolution_y = 1000
scene.render.filepath = str(P / 'renders/optics-candidate.png')
bpy.ops.wm.save_as_mainfile(filepath=str(P / 'source/tesla-model3-optics-upgrade.blend'))
(P / 'audit/optics-candidate.json').write_text(json.dumps(report, indent=2))
bpy.ops.render.render(write_still=True)
