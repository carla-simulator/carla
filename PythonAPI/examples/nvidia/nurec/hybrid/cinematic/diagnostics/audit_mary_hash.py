exec(open('/tmp/audit_mary_export.py').read().split('for n in [')[0])
import hashlib
native_hash={}
for frame in range(1,262):
 bpy.context.scene.frame_set(frame);a=uv(src);native_hash[hashlib.sha256(a.tobytes()).hexdigest()]=frame
rows=[]
for frame in [1,2,3,4,5,20,40,124,155,156,157,158,204,255,261]:
 bpy.context.scene.frame_set(frame);a=uv(dst);h=hashlib.sha256(a.tobytes()).hexdigest();rows.append({'frame':frame,'native':native_hash.get(h),'cache_frame':[(c.filepath,c.frame) for c in bpy.data.cache_files]})
print('MATCHES',rows);(r/'export-uv-matches.json').write_text(json.dumps(rows,indent=2))
