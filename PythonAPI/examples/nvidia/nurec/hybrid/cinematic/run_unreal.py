"""Launch isolated cinematic build/render using the workspace's UE5.8 editor."""
import argparse,json,os,subprocess,time
from pathlib import Path

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('stage',choices=['build','render']);ap.add_argument('shot',type=Path);ap.add_argument('--limit',type=int,default=48);ap.add_argument('--spp',type=int,default=32)
    ap.add_argument('--denoiser',choices=['raw','nne'],default='raw')
    ap.add_argument('--start-frame',type=int,help='First delivery frame, for isolated diagnostic renders')
    a=ap.parse_args();here=Path(__file__).resolve().parent;repo=here.parents[5];workspace=repo.parent
    env=os.environ.copy();env['HYBRID_SHOT']=str(a.shot.resolve());env['HYBRID_LIMIT']=str(a.limit);env['HYBRID_SPP']=str(a.spp);env.setdefault('DISPLAY',':1');env.setdefault('DLSS_SDK','/home/german/SDKs/DLSS')
    env['HYBRID_DENOISER']=a.denoiser
    editor=Path(env.get('UE_ROOT',workspace/'UnrealEngine_5'))/'Engine/Binaries/Linux/UnrealEditor'
    editor_log=a.shot.resolve()/f'{a.stage}-editor.log'
    manifest=json.loads((a.shot/'manifest.json').read_text());geometry_cache=manifest.get('actor_render_type')=='geometry_cache'
    start=manifest['frame_start'] if a.start_frame is None else a.start_frame
    env['HYBRID_START_FRAME']=str(start)
    plugins=['MovieRenderPipeline']+(['NNEDenoiser'] if a.denoiser=='nne' else [])
    if geometry_cache:plugins+=['GeometryCache','AlembicImporter']
    cmd=['flock',str(workspace/'.omc/ue.lock'),str(editor),str(repo/'Unreal/CarlaUnreal/CarlaUnreal.uproject'),'-EnablePlugins='+','.join(plugins),'-unattended','-nosplash','-nosound','-stdout',f'-abslog={editor_log}']
    if a.stage=='build':cmd+=['-run=pythonscript',f'-script={here}/'+('build_mary.py' if geometry_cache else 'build_unreal.py'),'-nullrhi']
    else:cmd+=['-RenderOffScreen',f'-ExecutePythonScript={here}/render_unreal.py']
    if start<manifest['frame_start'] or not 1 <= a.limit <= manifest['frame_end']-start+1 or a.spp < 1:
        ap.error('limit must fit the manifest frame range and spp must be positive')
    status=a.shot/('unreal-build.json' if a.stage=='build' else 'unreal-render-status.json')
    log=a.shot/f'{a.stage}-unreal.log'
    stamp=time.strftime('%Y%m%d-%H%M%S')
    # A successful report from before an outage must never certify a new run.
    error_report=a.shot/('unreal-error.txt' if a.stage=='build' else 'unreal-render-error.txt')
    for previous in (status,log,editor_log,error_report):
        if previous.exists():
            history=a.shot/'history';history.mkdir(exist_ok=True)
            previous.rename(history/f'{stamp}-{previous.name}')
    attempt={'stage':a.stage,'started_unix':time.time(),'start_frame':start,'limit':a.limit,'spp':a.spp,'denoiser':a.denoiser}
    (a.shot/f'{a.stage}-attempt.json').write_text(json.dumps(attempt,indent=2))
    with log.open('w') as f:result=subprocess.run(cmd,env=env,stdout=f,stderr=subprocess.STDOUT)
    attempt.update(returncode=result.returncode,finished_unix=time.time())
    (a.shot/f'{a.stage}-attempt.json').write_text(json.dumps(attempt,indent=2))
    result.check_returncode()
    if not status.exists():raise SystemExit(f'No completion report; inspect {log}')
    if a.stage=='render' and not json.loads(status.read_text()).get('success'):
        raise SystemExit(f'Render failed; inspect {status} and {log}')
    print(status.read_text())

if __name__=='__main__':main()
