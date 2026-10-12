"""Fetch the attributed source mesh and verify its exact bytes before authoring."""
import hashlib,json,os,subprocess
from pathlib import Path
import requests
P=Path(os.environ.get('CARLA_MODEL3_WORKDIR',Path(__file__).resolve().parents[4]/'artifacts/tesla-model3'))
for name in ['reference','audit','source','renders','export']: (P/name).mkdir(parents=True,exist_ok=True)
UID='5ef9b845aaf44203b6d04e2c677e444f'
SHA256='d6d78c9bd1bd9c7ca87a07509a2e7b6585995fcbdec054f297167ba7c15cd878'
path=P/'reference/candidate-model.glb'
if not path.exists():
 r=requests.get(f'https://huggingface.co/datasets/allenai/objaverse/resolve/main/glbs/000-119/{UID}.glb',timeout=120);r.raise_for_status();data=r.content
 if hashlib.sha256(data).hexdigest()!=SHA256:raise RuntimeError('Source hash changed; review provenance before using it')
 path.write_bytes(data)
assert hashlib.sha256(path.read_bytes()).hexdigest()==SHA256
r=requests.get(f'https://api.sketchfab.com/v3/models/{UID}',timeout=30);r.raise_for_status();metadata=r.json()
assert metadata['license']['slug']=='by', 'Review changed source license'
(P/'reference/candidate-model-metadata.json').write_text(json.dumps(metadata,indent=2))
subprocess.run(['convert','-background','none',str(Path(__file__).with_name('model3_display.svg')),str(P/'reference/Model3_Display.png')],check=True)
print('Verified CC BY source:',SHA256)
