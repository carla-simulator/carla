"""Reference-led wheel detail pass. Keeps the original Blender source untouched.
Geometry is an artist reconstruction, not Michelin manufacturing CAD.
Run with Blender --background --python <this file>.
"""
import bpy,bmesh,math,json,os
import numpy as np
from pathlib import Path
from mathutils import Vector,Matrix,Quaternion
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
bpy.ops.wm.open_mainfile(filepath=str(P/'source/tesla-model3-fidelity.blend'))
collection=bpy.data.collections['Tesla_Model3'];R=.33625;report={'reference':'https://www.tesla.com/ownersmanual/2017_2023_model3/en_us/GUID-ECA7C07B-7944-496B-8FC5-12762BF061F1.html','parts':[]}
def material(name,color,metal,rough):
 m=bpy.data.materials.get(name) or bpy.data.materials.new(name);m.use_nodes=True;b=m.node_tree.nodes.get('Principled BSDF')
 for inp in ['Base Color','Metallic','Roughness','Normal']:
  for link in list(b.inputs[inp].links):m.node_tree.links.remove(link)
 b.inputs['Base Color'].default_value=(*color,1);b.inputs['Metallic'].default_value=metal;b.inputs['Roughness'].default_value=rough
 return m,b
rubber,b=material('WheelDetail_Rubber',(.012,.014,.016),0,.72);b.inputs['Specular IOR Level'].default_value=.28
sidewall,b=material('WheelDetail_Sidewall',(.013,.015,.017),0,.68);b.inputs['Specular IOR Level'].default_value=.28
raised,_=material('WheelDetail_RaisedRubber',(.027,.030,.033),0,.68)
velvet,b=material('WheelDetail_Velvet',(.003,.004,.005),0,.91);b.inputs['Specular IOR Level'].default_value=.2
silver,b=material('wheels.6',(.46,.49,.52),.72,.26);b.inputs['Coat Weight'].default_value=.6;b.inputs['Coat Roughness'].default_value=.18
rotor,b=material('WheelDetail_Rotor',(.23,.24,.25),.94,.34)
hat,b=material('WheelDetail_Hat',(.045,.048,.05),.7,.45)
valve,b=material('WheelDetail_Valve',(.009,.010,.012),0,.65)
# Original, tileable surface maps. No lighting or photographic shadows in albedo.
def surface_maps(mat,kind):
 n=512 if kind=='rotor' else 2048;y,x=np.mgrid[0:n,0:n]/n;rng=np.random.default_rng(348)
 if kind=='rotor':
  h=.5+.006*np.sin(y*2*np.pi*151)+.003*np.sin(y*2*np.pi*227)+rng.normal(0,.003,(n,n));rough=.33+.015*np.sin(y*2*np.pi*73)+rng.normal(0,.005,(n,n));strength=.12
 else:
  h=rng.normal(.5,.018,(n,n));rough=rng.normal(.73 if kind=='tread' else .68,.025,(n,n));strength=.4
  if kind=='tread':
   yy=np.interp(y,np.linspace(0,1,len(profile)),[p[0] for p in profile])
   phase=(x*64+yy*8+np.where(yy>0,.24,0))%1
   h-=.18*np.maximum(0,1-np.abs(phase-.5)/.06)*(np.abs(yy)<.103)
   strength=1.6
  if kind=='sidewall':
   # Fine molded radial grain, strongest beside the shoulder lettering.
   band=np.exp(-((y-.13)/.09)**2)+np.exp(-((y-.87)/.09)**2)
   h+=.018*np.sin(x*2*np.pi*650)*band
   rough+=.025*band
 dx=(np.roll(h,-1,1)-np.roll(h,1,1))*strength;dy=(np.roll(h,-1,0)-np.roll(h,1,0))*strength
 normals=np.dstack((-dx,-dy,np.ones_like(h)));normals/=np.linalg.norm(normals,axis=2)[:,:,None]
 for suffix,data in [('Normal',normals*.5+.5),('Roughness',np.repeat(rough[:,:,None],3,axis=2))]:
  name=mat.name+'_'+suffix;im=bpy.data.images.new(name,n,n);im.colorspace_settings.name='Non-Color';rgba=np.concatenate((np.clip(data,0,1),np.ones((n,n,1))),axis=2).astype(np.float32);im.pixels.foreach_set(rgba.ravel());im.pack()
  node=mat.node_tree.nodes.new('ShaderNodeTexImage');node.image=im;bsdf=mat.node_tree.nodes.get('Principled BSDF')
  if suffix=='Normal':
   normal=mat.node_tree.nodes.new('ShaderNodeNormalMap');mat.node_tree.links.new(node.outputs['Color'],normal.inputs['Color']);mat.node_tree.links.new(normal.outputs['Normal'],bsdf.inputs['Normal']);mat['carla_normal_texture']=name
  else:mat.node_tree.links.new(node.outputs['Color'],bsdf.inputs['Roughness']);mat['carla_roughness_texture']=name

# The source caliper label is an opaque white rectangle with a logo printed
# into it. Replace the rectangle with actual paint lettering on the caliper.
ink,_=material('WheelDetail_CaliperInk',(.72,.73,.74),0,.38)
for old in list(collection.objects):
 if old.type!='MESH' or not old.name.startswith('hub_') or not any(m and m.name=='hub_rf.1' for m in old.data.materials):continue
 pts=[old.matrix_world@v.co for v in old.data.vertices];center=sum(pts,Vector())/len(pts);side=1 if center.y>0 else -1;prefix=old.name.split('_hub_')[0]+'_'
 bpy.data.objects.remove(old,do_unlink=True)
 curve=bpy.data.curves.new('Caliper paint lettering','FONT');curve.body='TESLA';curve.align_x='CENTER';curve.align_y='CENTER';curve.size=.024;curve.space_character=1.22;curve.resolution_u=5
 label=bpy.data.objects.new(prefix+'DetailLogo',curve);collection.objects.link(label);label.location=center+Vector((0,side*.00015,0));label.rotation_euler=Matrix((Vector((0,0,-1)),Vector((-side,0,0)),Vector((0,side,0)))).transposed().to_euler();label.data.materials.append(ink)
 bpy.ops.object.select_all(action='DESELECT');label.select_set(True);bpy.context.view_layer.objects.active=label;bpy.ops.object.convert(target='MESH')
 for v in label.data.vertices:v.co=label.matrix_world@v.co
 label.matrix_world.identity()
# Remove the old low-resolution tire and pre-lit disc, retaining the OEM spokes,
# centre cap and separate steering calipers.
for o in list(collection.objects):
 if o.name.startswith('wheels') and any(m and m.name in ['wheels.2','wheels.3','wheels.4'] for m in o.data.materials):bpy.data.objects.remove(o,do_unlink=True)
def mesh_object(name,verts,faces,mats,uvs=None):
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update();o=bpy.data.objects.new(name,mesh);collection.objects.link(o)
 for m in mats:mesh.materials.append(m)
 for p in mesh.polygons:p.use_smooth=True
 if uvs:
  layer=mesh.uv_layers.new(name='UVMap')
  for p,uv in zip(mesh.polygons,uvs):
   for li,coord in zip(p.loop_indices,uv):layer.data[li].uv=coord
 o['carla_wheel_detail']=True
 return o
# Radial cross-section: rim bead, rounded sidewall, crown with four recessed
# circumferential channels. Groove radius never exceeds the original tire.
profile=[(-.117,.258),(-.121,.266),(-.124,.279),(-.125,.291),(-.124,.304),(-.120,.314),(-.113,.324),(-.103,.331)]
ys=sorted(set([float(v) for v in np.linspace(-.099,.099,35)]+[c+d for c in [-.068,-.025,.023,.066] for d in [-.006,-.0045,-.0035,.0035,.0045,.006]]))
for y in ys:
 crown=R-.002*(abs(y)/.1)**2
 depth=max(.0065*max(0,min(1,(.0055-abs(y-c))/.002)) for c in [-.068,-.025,.023,.066])
 profile.append((y,crown-depth))
profile.extend([(.103,.331),(.113,.324),(.120,.314),(.124,.304),(.125,.291),(.124,.279),(.121,.266),(.117,.258)])
segments=1152
surface_maps(rubber,'tread');surface_maps(sidewall,'sidewall');surface_maps(rotor,'rotor')
for rear in [False,True]:
 xc=-1.4375 if rear else 1.4375;prefix='wheels.001_' if rear else 'wheels_'
 for side in [1,-1]:
  yc=side*.79;verts=[];faces=[];uvs=[];rings=len(profile)
  for i in range(segments):
   a=2*math.pi*i/segments
   for y,r in profile:
    # Staggered lateral shoulder cuts are shallow; main channels carry depth.
    phase=(i/segments*64 + y*8 + (0.24 if y>0 else 0))%1
    width=.13 if abs(y)>.073 else .075
    depth=.005 if abs(y)>.073 else .0013
    cut=depth*max(0,1-abs(phase-.5)/width)*max(0,min(1,(.103-abs(y))/.013)) if abs(y)<.103 else 0
    rr=r-cut*max(0,min(1,(r-.304)/.02));verts.append((xc+rr*math.cos(a),yc+y,R+rr*math.sin(a)))
  for i in range(segments):
   for j in range(rings-1):
    ni=(i+1)%segments;faces.append((i*rings+j,i*rings+j+1,ni*rings+j+1,ni*rings+j));uvs.append(((i/segments,j/(rings-1)),(i/segments,(j+1)/(rings-1)),((i+1)/segments,(j+1)/(rings-1)),((i+1)/segments,j/(rings-1))))
  tire=mesh_object(prefix+'DetailTire_'+str(side),verts,faces,[rubber,sidewall],uvs)
  tire.data.set_sharp_from_angle(angle=math.radians(38))
  for poly in tire.data.polygons:
   y=(profile[poly.index%(rings-1)][0]+profile[poly.index%(rings-1)+1][0])*.5;poly.material_index=1 if abs(y)>.112 else 0
  # Cylindrical, vented disc; two ring faces and discrete internal vanes.
  n=192;front=yc+side*.049;back=yc+side*.021;outer=.177 if not rear else .167;inner=.092
  vv=[];ff=[];uu=[]
  for ay,rad in [(front,inner),(front,outer),(back,inner),(back,outer)]:
   vv.extend((xc+rad*math.cos(2*math.pi*i/n),ay,R+rad*math.sin(2*math.pi*i/n)) for i in range(n))
  for i in range(n):
   k=(i+1)%n
   for inds in [(i,k,n+k,n+i),(2*n+i,3*n+i,3*n+k,2*n+k)]:ff.append(inds);uu.append(((i/n,0),((i+1)/n,0),((i+1)/n,1),(i/n,1)))
  disc=mesh_object(prefix+'DetailDisc_'+str(side),vv,ff,[rotor],uu)
  # Add the rotor's central hat under the wheel centre cap.
  bpy.ops.mesh.primitive_cylinder_add(vertices=96,radius=inner,depth=.030,location=(xc,yc+side*.027,R),rotation=(math.pi/2,0,0))
  h=bpy.context.object;h.name=prefix+'DetailHat_'+str(side);h.data.materials.append(hat)
  for c in list(h.users_collection):c.objects.unlink(h)
  collection.objects.link(h)
  for v in h.data.vertices:v.co=h.matrix_world@v.co
  h.matrix_world.identity()
  # Double-sided ring surfaces receive normals matching their actual winding.
  if side==-1:
   for poly in disc.data.polygons:poly.flip()
  for i in range(40):
   a=2*math.pi*i/40
   bpy.ops.mesh.primitive_cube_add(size=1,location=(xc+.136*math.cos(a),yc+side*.035,R+.136*math.sin(a)))
   o=bpy.context.object;o.name=prefix+'DetailVane_'+str(side)+'_'+str(i);o.scale=(.065,.012,.003);o.rotation_euler[1]=-a;o.data.materials.append(hat)
   for c in list(o.users_collection):c.objects.unlink(o)
   collection.objects.link(o);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
   # Bake transforms into coordinates; exporter expects world-space wheel data.
   for v in o.data.vertices:v.co=o.matrix_world@v.co
   o.matrix_world.identity()
  # Moulded annular ribs and the dark velvet branding panel follow the
  # sidewall curvature, rather than floating as flat decals above the tire.
  def side_y(rad):return float(np.interp(rad,[.258,.266,.279,.291,.304,.314,.324,.331],[.117,.121,.124,.125,.124,.120,.113,.103]))
  def side_band(name,radii,start,end,mat,lift):
   count=max(32,int((end-start)/(2*math.pi)*576));vv=[];ff=[]
   for i in range(count+1):
    a=start+(end-start)*i/count
    for j,rad in enumerate(radii):
     bump=lift*(math.sin(math.pi*j/(len(radii)-1)) if len(radii)>2 else 1)
     vv.append((xc+rad*math.cos(a),yc+side*(side_y(rad)+.00005+bump),R+rad*math.sin(a)))
   for i in range(count):
    for j in range(len(radii)-1):
     f=(i*len(radii)+j,(i+1)*len(radii)+j,(i+1)*len(radii)+j+1,i*len(radii)+j+1)
     ff.append(f if side==1 else tuple(reversed(f)))
   return mesh_object(prefix+name+'_'+str(side),vv,ff,[mat])
  for radius in [.269,.313,.317]:side_band('DetailMoldRing',[radius-.0004,radius,radius+.0004],0,2*math.pi,sidewall,.00023)
  side_band('DetailBrandPanel',sorted(set([.279,.291,.304]+list(np.linspace(.277,.312,18)))),math.pi/2-.48,math.pi/2+.48,velvet,.00008)
  # Subtle raised tire inscriptions; no inflated white lettering.
  for word,angle,size,radius in [('MICHELIN',math.pi/2,.038,.279),('PILOT SPORT 4 S',.30,.019,.291),('235/35 ZR20',-math.pi/2,.014,.286),('RADIAL',2.55,.008,.279),('TUBELESS',.92,.007,.270),('OUTSIDE',-2.5,.009,.288)]:
   for index,char in enumerate(word):
    if char==' ':continue
    a=angle+side*(index-(len(word)-1)/2)*size*.70/radius
    radial=Vector((math.cos(a),0,math.sin(a)));tangent=Vector((-side*math.sin(a),0,side*math.cos(a)));normal=Vector((0,side,0))
    curve=bpy.data.curves.new('Tire lettering','FONT');curve.body=char;
    if word=='MICHELIN':curve.font=bpy.data.fonts.load('/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf',check_existing=True)
    curve.align_x='CENTER';curve.size=size;curve.extrude=.00018;curve.bevel_depth=.000025;curve.resolution_u=3
    o=bpy.data.objects.new(prefix+'DetailLetter_'+str(side),curve);collection.objects.link(o);o.location=Vector((xc,yc+side*.1252,R))+radial*radius;o.rotation_euler=Matrix((tangent,radial,normal)).transposed().to_euler();o.data.materials.append(raised)
    bpy.context.view_layer.objects.active=o;o.select_set(True);bpy.ops.object.convert(target='MESH');o=bpy.context.object
    if size>.03:
     bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.triangulate(bm,faces=list(bm.faces));bmesh.ops.subdivide_edges(bm,edges=list(bm.edges),cuts=5,use_grid_fill=True);bm.to_mesh(o.data);bm.free()
    for v in o.data.vertices:
     pos=o.matrix_world@v.co;rad=math.hypot(pos.x-xc,pos.z-R);depth=side*(pos.y-yc)-.1252
     pos.y=yc+side*(float(np.interp(rad,[.258,.266,.279,.291,.304,.314,.324,.331],[.117,.121,.124,.125,.124,.120,.113,.103]))+.0004+depth);v.co=pos
    o.matrix_world.identity();o.select_set(False)
  # Valve stem sits inside the rim, not outside the tire silhouette.
  a=.35;pos=(xc+.238*math.cos(a),yc+side*.122,R+.238*math.sin(a))
  bpy.ops.mesh.primitive_cylinder_add(vertices=16,radius=.0035,depth=.014,location=pos,rotation=(math.pi/2,0,0));o=bpy.context.object;o.name=prefix+'DetailValve_'+str(side);o.data.materials.append(valve)
  for c in list(o.users_collection):c.objects.unlink(o)
  collection.objects.link(o)
  for v in o.data.vertices:v.co=o.matrix_world@v.co
  o.matrix_world.identity()
  report['parts'].append({'axle':'rear' if rear else 'front','side':side,'tire_triangles':len(faces)*2,'radius_m':R,'groove_depth_m':.0065})
# Preserve authored spoke normals: the source's UV-split triangles must not be
# independently bevelled (doing that opens seams and damages the spoke shading).
# Open on the wheel in material preview, preserving original file and scene.
scene=bpy.context.scene;cam=scene.camera;cam.location=(2.55,-2.5,1.03);target=Vector((1.4375,-.79,.36));cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=62
scene.render.resolution_x=1400;scene.render.resolution_y=1100;scene.cycles.samples=32
for screen in bpy.data.screens:
 for area in screen.areas:
  if area.type=='VIEW_3D':
   area.spaces.active.region_3d.view_distance=1.65;area.spaces.active.region_3d.view_location=target;area.spaces.active.region_3d.view_rotation=cam.rotation_euler.to_quaternion();area.spaces.active.shading.type='MATERIAL'
bpy.ops.object.select_all(action='DESELECT');bpy.ops.wm.save_as_mainfile(filepath=str(P/'source/tesla-model3-wheel-detail.blend'))
(P/'audit/wheel-detail.json').write_text(json.dumps(report,indent=2))
scene.render.filepath=str(P/'renders/wheel-detail-blender.png');bpy.ops.render.render(write_still=True)
# A second beauty view exposes the tread; posed only after saving the straight
# source so exported wheel pivots and physics anchors remain unchanged.
center=Vector((1.4375,-.79,R));turn=Matrix.Rotation(math.radians(-28),4,'Z')
for o in collection.objects:
 if o.type!='MESH':continue
 if o.name.startswith('wheels') and '.001_' not in o.name:
  for v in o.data.vertices:
   if v.co.y<0:v.co=center+turn@(v.co-center)
 elif o.name.startswith('hub_rf_'):
  for v in o.data.vertices:v.co=center+turn@(v.co-center)
cam.location=(2.65,-2.25,1.08);cam.rotation_euler=(Vector((1.4375,-.79,.40))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=58
scene.render.filepath=str(P/'renders/tire-detail-turned.png');bpy.ops.render.render(write_still=True)
# A separately named inspection file avoids changing the export source.
for screen in bpy.data.screens:
 for area in screen.areas:
  if area.type=='VIEW_3D':area.spaces.active.region_3d.view_rotation=cam.rotation_euler.to_quaternion()
bpy.ops.wm.save_as_mainfile(filepath=str(P/'source/tesla-model3-tire-review.blend'))
