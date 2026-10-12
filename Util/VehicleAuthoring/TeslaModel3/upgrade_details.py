"""Add modeled construction detail to the reviewed surface master.
All additions are artist reconstructions, not manufacturer tooling geometry.
"""
import bpy,bmesh,math,json,os
import numpy as np
from pathlib import Path
from mathutils import Vector
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
bpy.ops.wm.open_mainfile(filepath=str(P/'source/tesla-model3-surface-upgrade.blend'))
C=bpy.data.collections['Tesla_Model3'];report={'added':[],'bevelled':[]}
def mat(name,col,metal=0,rough=.6):
 m=bpy.data.materials.new(name);m.use_nodes=True;b=m.node_tree.nodes.get('Principled BSDF');b.inputs['Base Color'].default_value=(*col,1);b.inputs['Metallic'].default_value=metal;b.inputs['Roughness'].default_value=rough;return m
liner=mat('SurfaceDetail_WheelLiner',(.013,.016,.019),0,.86)
shield=mat('SurfaceDetail_Undertray',(.025,.028,.031),.15,.7)
seal=mat('SurfaceDetail_Seal',(.009,.011,.013),0,.7)
reflector=mat('SurfaceDetail_Reflector',(.68,.7,.72),1,.13)
lens=mat('SurfaceDetail_CameraLens',(.012,.018,.025),.4,.095)
def mesh(name,vs,fs,m,smooth=True):
 d=bpy.data.meshes.new(name);d.from_pydata(vs,[],fs);d.update();o=bpy.data.objects.new(name,d);C.objects.link(o);d.materials.append(m)
 for f in d.polygons:f.use_smooth=smooth
 report['added'].append(name);return o

def box(name,loc,size,m,bevel=.001):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.name=name;o.scale=size;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 for c in list(o.users_collection):c.objects.unlink(o)
 C.objects.link(o);o.data.materials.append(m)
 mod=o.modifiers.new('Manufactured edge radius','BEVEL');mod.width=bevel;mod.segments=3
 bpy.context.view_layer.objects.active=o;bpy.ops.object.modifier_apply(modifier=mod.name)
 bpy.context.view_layer.update()
 for v in o.data.vertices:v.co=o.matrix_world@v.co
 o.matrix_world.identity();report['added'].append(name);return o
# Keep full tire/suspension clearance: these non-colliding shells follow the arch,
# and stay behind its painted lip. There is no geometry inside the tire volume.
for axle,xc in [('Front',1.4375),('Rear',-1.4375)]:
 for side in [-1,1]:
  vs=[];fs=[];n=112
  for i in range(n+1):
   a=math.radians(-14+208*i/n)
   for y,r in [(.56,.39),(.68,.391),(.80,.391),(.865,.382)]:
    vs.append((xc+r*math.cos(a),side*y,.33625+r*math.sin(a)))
  for i in range(n):
   for j in range(3):
    f=(i*4+j,i*4+j+1,(i+1)*4+j+1,(i+1)*4+j);fs.append(f if side<0 else f[::-1])
  o=mesh('SurfaceDetail_'+axle+'Liner_'+str(side),vs,fs,liner)
  sol=o.modifiers.new('Liner thickness','SOLIDIFY');sol.thickness=.002
  # Captive fasteners set into the liner edge rather than floating in the arch.
  for a in [.18,.7,1.2,1.8,2.4,2.95]:
   pos=(xc+.384*math.cos(a),side*.85,.33625+.384*math.sin(a))
   bpy.ops.mesh.primitive_uv_sphere_add(segments=12,ring_count=6,radius=1,location=pos);o=bpy.context.object;o.name='SurfaceDetail_LinerFastener';o.scale=(.004,.0015,.004)
   for c in list(o.users_collection):c.objects.unlink(o)
   C.objects.link(o);o.data.materials.append(shield)
   bpy.context.view_layer.update()
   for v in o.data.vertices:v.co=o.matrix_world@v.co
   o.matrix_world.identity()
undertray=box('SurfaceDetail_BatteryUndertray',(-.1,0,.16),(2.15,1.30,.025),shield,.006)
# Thin the formed panel and place it below the original underside, avoiding overlap.
for v in undertray.data.vertices:v.co.z=.12+(v.co.z-.16)*.6
for x in [-.95,-.5,0,.5,.9]:box('SurfaceDetail_UndertrayJoint',(x,0,.111),(.007,1.24,.003),seal,.001)
# Put a recessed optical element inside each existing repeater garnish.
for side in [-1,1]:
 bpy.ops.mesh.primitive_uv_sphere_add(segments=32,ring_count=16,radius=1,location=(1.073,side*.882,.694));o=bpy.context.object;o.name='SurfaceDetail_RepeaterLens';o.scale=(.008,.0025,.008)
 for c in list(o.users_collection):c.objects.unlink(o)
 C.objects.link(o);o.data.materials.append(lens)
 bpy.context.view_layer.update()
 for v in o.data.vertices:v.co=o.matrix_world@v.co
 o.matrix_world.identity()
# Small manufactured radii on metallic fittings, preserving their articulation
# prefixes. Weld UV seams first so bevels cannot split individual triangles.
for o in list(C.objects):
 if o.type!='MESH' or o.name.startswith(('wheels','hub_','SurfaceDetail_')):continue
 if not any(m.name in ['movsteer_1.0.1','aluminium2.0'] for m in o.data.materials):continue
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000005)
 edges=[e for e in bm.edges if e.is_manifold and e.calc_face_angle(0)>math.radians(40)]
 if edges:
  bmesh.ops.bevel(bm,geom=edges,offset=.00045,segments=3,affect='EDGES',clamp_overlap=True)
  bm.to_mesh(o.data);report['bevelled'].append(o.name)
 bm.free()
# Three faceted reflector chambers fit within each existing low-beam opening.
# The lighting material names stay intact for CARLA's light-state controls.
for side,light in [(-1,'right_front_light'),(1,'left_front_light')]:
 normal=Vector((.82,side*.32,.46)).normalized();u=Vector((-side*.32,.82,0)).normalized();v=normal.cross(u).normalized()
 for k in range(3):
  center=Vector((1.921,side*(.494+k*.048),.655+k*.003))
  vs=[];fs=[]
  for depth,w,h in [(-.019,.011,.009),(-.014,.015,.012),(0,.022,.022),(.001,.023,.023)]:
   for a,b in [(-1,-1),(1,-1),(1,1),(-1,1)]:vs.append(center+u*w*a+v*h*b+normal*depth)
  for ring in range(3):
   for j in range(4):fs.append((ring*4+j,ring*4+(j+1)%4,(ring+1)*4+(j+1)%4,(ring+1)*4+j))
  mesh('SurfaceDetail_LowBeamReflector',vs,fs,reflector,False)
  pts=[center+u*a*.009+v*b*.006-normal*.0185 for a,b in [(-1,-1),(1,-1),(1,1),(-1,1)]]
  mesh('SurfaceDetail_LowBeamEmitter',pts,[(0,1,2,3)],bpy.data.materials[light],False)
# Eliminate the old flat emissive faces occupying the new cavity mouths.
for o in list(C.objects):
 if o.name.startswith('chrome_Lights_head_l_'):bpy.data.objects.remove(o,do_unlink=True)
# Refine shading continuity without changing panel positions or joint gaps.
# Range/normal gates avoid blending opposite skins and manufactured creases.
from mathutils.kdtree import KDTree
report['normal_refinement']=[]
for o in C.objects:
 if o.type!='MESH' or len(o.data.vertices)<1000 or not any(m.name=='Bodywork_Mat' for m in o.data.materials):continue
 m=o.data;vn={}
 for loop,n in zip(m.loops,m.corner_normals):vn.setdefault(loop.vertex_index,n.vector.copy())
 kd=KDTree(len(vn))
 for i in vn:kd.insert(m.vertices[i].co,i)
 kd.balance();sm={}
 for i,n in vn.items():
  value=Vector();weight=0
  for co,j,d in kd.find_range(m.vertices[i].co,.055):
   if n.dot(vn[j])<.94:continue
   w=math.exp(-(d/.024)**2);value+=vn[j]*w;weight+=w
  sm[i]=(value/weight).normalized() if weight else n
 m.normals_split_custom_set([sm[l.vertex_index] for l in m.loops]);report['normal_refinement'].append(o.name)
# The upper window gasket remains on the body when the frameless door opens.
# Extract it from the actual glass perimeter; omit the door-mounted lower edge.
for o in list(C.objects):
 if not o.name.startswith('door_') or not any(m.name.startswith('glass') for m in o.data.materials):continue
 side=1 if sum(v.co.y for v in o.data.vertices)>0 else -1
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000001);bm.normal_update()
 outer={f for f in bm.faces if f.normal.y*side>.5};bottom=min(v.co.z for v in bm.verts)
 edges=[e for e in bm.edges if sum(f in outer for f in e.link_faces)==1 and min(v.co.z for v in e.verts)>1.02]
 adj={}
 for e in edges:
  a,b=e.verts;adj.setdefault(a,[]).append(b);adj.setdefault(b,[]).append(a)
 remaining=set(edges)
 while remaining:
  e=next(iter(remaining));start=next((v for v in adj if len(adj[v])==1 and any(ed in remaining for ed in v.link_edges)),e.verts[0]);pts=[];v=start
  while True:
   pts.append(v.co.copy()+Vector((0,-side*.001,0)));ed=next((ed for ed in v.link_edges if ed in remaining),None)
   if ed is None:break
   remaining.remove(ed);v=ed.other_vert(v)
   if v==start:pts.append(pts[0]);break
  if len(pts)<3:continue
  curve=bpy.data.curves.new('Window gasket','CURVE');curve.dimensions='3D';curve.bevel_depth=.002;curve.bevel_resolution=2;sp=curve.splines.new('POLY');sp.points.add(len(pts)-1)
  for v,co in zip(sp.points,pts):v.co=(*co,1)
  gasket=bpy.data.objects.new('SurfaceDetail_WindowSeal',curve);C.objects.link(gasket);curve.materials.append(seal);bpy.ops.object.select_all(action='DESELECT');gasket.select_set(True);bpy.context.view_layer.objects.active=gasket;bpy.ops.object.convert(target='MESH');report['added'].append(gasket.name)
 bm.free()
# Visible dark stitching on the driver's side of the steering-wheel rim.
thread=mat('SurfaceDetail_DarkThread',(.028,.03,.033),0,.82)
from mathutils.bvhtree import BVHTree
steering=bpy.data.objects['movsteer_1.0_movsteer_1.0.0_0'].data
steering_surface=BVHTree.FromPolygons([v.co for v in steering.vertices],[list(p.vertices) for p in steering.polygons])
vs=[];fs=[]
for i in range(156):
 a=2*math.pi*i/156
 for end in [-1,1]:
  t=a+end*.007;z=.837+.153*math.sin(t);center=Vector((.447+.32*(z-.837)-.022,.398+.153*math.cos(t),z))
  hit,normal,_,_=steering_surface.ray_cast(Vector((-.1,center.y,center.z)),Vector((1,0,0)),1)
  if hit is None:raise RuntimeError('Steering stitch misses rim surface')
  if normal.x>0:normal=-normal
  center=hit+normal*.00032
  radial=Vector((0,math.cos(t),math.sin(t)));radial=(radial-normal*radial.dot(normal)).normalized()
  for j in range(6):vs.append(center+.00032*(normal*math.cos(j*math.pi/3)+radial*math.sin(j*math.pi/3)))
 base=i*12
 for j in range(6):fs.append((base+j,base+(j+1)%6,base+6+(j+1)%6,base+6+j))
mesh('SurfaceDetail_SteeringStitches',vs,fs,thread)
# Give the rear light guides physical depth where the source was an open skin.
report['rear_light_guides']=[]
for o in C.objects:
 if o.type!='MESH' or not o.name.startswith(('rear_lights','light_breake','breake_int')):continue
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000001);closed=all(e.is_manifold for e in bm.edges);bm.free()
 if not closed:
  mod=o.modifiers.new('Moulded light guide depth','SOLIDIFY');mod.thickness=.003;mod.offset=-1;report['rear_light_guides'].append(o.name)
# Fine original upholstery/plastic grain, with independent roughness maps.
# Keep the amplitude restrained; the seams and large forms remain geometry.
for name,rough,strength in [('Seat_Leather_white.0',.53,.42),('movsteer_1.0.0',.52,.38),('Plastic.0',.64,.22),('SurfaceDetail_Headliner',.92,.28)]:
 m=bpy.data.materials.get(name)
 if not m:continue
 n=1024;rng=np.random.default_rng(739);raw=rng.normal(0,1,(n,n));freq=np.fft.fftfreq(n);f2=freq[:,None]**2+freq[None,:]**2
 h=np.fft.ifft2(np.fft.fft2(raw)*np.exp(-f2*38)).real;h=h/(h.std()+1e-9)*.06
 dx=(np.roll(h,-1,1)-np.roll(h,1,1))*strength;dy=(np.roll(h,-1,0)-np.roll(h,1,0))*strength
 normal=np.dstack((-dx,-dy,np.ones_like(h)));normal/=np.linalg.norm(normal,axis=2)[:,:,None]
 for suffix,data in [('Normal',normal*.5+.5),('Roughness',np.repeat(np.clip(rough+h*.35,0,1)[:,:,None],3,2))]:
  im=bpy.data.images.new('SurfaceDetail_'+name+'_'+suffix,n,n);im.colorspace_settings.name='Non-Color';rgba=np.concatenate((data,np.ones((n,n,1))),axis=2).astype(np.float32);im.pixels.foreach_set(rgba.ravel());im.pack()
  tex=m.node_tree.nodes.new('ShaderNodeTexImage');tex.image=im;b=m.node_tree.nodes.get('Principled BSDF')
  if suffix=='Normal':
   node=m.node_tree.nodes.new('ShaderNodeNormalMap');m.node_tree.links.new(tex.outputs['Color'],node.inputs['Color']);m.node_tree.links.new(node.outputs['Normal'],b.inputs['Normal']);m['carla_normal_texture']=im.name
  else:m.node_tree.links.new(tex.outputs['Color'],b.inputs['Roughness']);m['carla_roughness_texture']=im.name
s=bpy.context.scene;cam=s.camera;s.cycles.samples=40;s.render.resolution_x=1600;s.render.resolution_y=1000
cam.location=(5,-5,2.1);cam.rotation_euler=(Vector((0,0,.7))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=55
for screen in bpy.data.screens:
 for area in screen.areas:
  if area.type=='VIEW_3D':
   area.spaces.active.region_3d.view_distance=6;area.spaces.active.region_3d.view_location=(0,0,.7);area.spaces.active.region_3d.view_rotation=cam.rotation_euler.to_quaternion();area.spaces.active.shading.type='MATERIAL'
bpy.ops.object.select_all(action='DESELECT')
bpy.ops.wm.save_as_mainfile(filepath=str(P/'source/tesla-model3-detail-upgrade.blend'))
(P/'audit/detail-upgrade.json').write_text(json.dumps(report,indent=2))
for name,loc,target,lens_mm in [('detail-headlight',(3,-2,1.15),(1.88,-.65,.7),75),('detail-low-front',(3.4,-3.4,.45),(1.2,-.6,.42),55),('detail-fittings',(1.9,-2.4,1.15),(.7,-.85,.82),65),('detail-cabin',(-.25,-.30,1.3),(.42,.38,.78),32),('detail-rear',(-3.5,-2,1.3),(-2.1,-.5,.86),65),('detail-stitching',(.08,.53,.92),(.40,.53,.92),65)]:
 cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=lens_mm;s.render.filepath=str(P/'renders'/f'{name}.png');bpy.ops.render.render(write_still=True)
