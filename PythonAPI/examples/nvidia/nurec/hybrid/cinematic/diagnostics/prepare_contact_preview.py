from pathlib import Path
import json,sys,numpy as np
sys.path.insert(0,'carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic');from core import read_exr,write_exr
r=Path('artifacts/hybrid-cinematic-mvp/sh031_contactfix');out=r/'preview540';out.mkdir(exist_ok=True)
for p in r.iterdir():
 if p.suffix in ('.json','.hdr','.fbx') and p.name!='manifest.json':
  target=out/p.name
  if not target.exists():target.symlink_to(p.resolve())
for name in ['capture']:
 target=out/name
 if not target.exists():target.symlink_to((r/name).resolve(),target_is_directory=True)
m=json.loads((r/'manifest.json').read_text());m['resolution']=[960,540];m['K']=(np.array(m['K'])*np.array([[.5,.5,.5],[.5,.5,.5],[1,1,1]])).tolist();m['preview_of']='../manifest.json';m['preview_resolution_note']='960x540 review render; source plate RGB area reduced, alpha/depth point sampled on half-resolution calibrated grid';(out/'manifest.json').write_text(json.dumps(m,indent=2))
# Render reports must be local rather than overwrite the diagnostic audit.
for name in ['unreal-render-status.json','wheel-render-validation.json','mary-sequence-validation.json','traffic-sequence-validation.json','composite-report.json','render-attempt.json','utility-materials.json']:
 p=out/name
 if p.is_symlink():p.unlink()
for k in range(1001,1205):
 for layer in ['gs_beauty','gs_depth']:
  v=read_exr(r/f'renders/{layer}/{k}.exr');small={}
  for ch,a in v.items():
   small[ch]=a.reshape(540,2,960,2).mean(axis=(1,3)) if ch in 'RGB' else a[::2,::2]
  write_exr(out/f'renders/{layer}/{k}.exr',small,half=layer=='gs_beauty')
print('PREVIEW_PLATES_READY',flush=True)
