"""Resumable six-camera render with separate simulation and NuRec clocks."""
import argparse
import json
import os
import subprocess
import time
from pathlib import Path


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('rig',type=Path)
    ap.add_argument('--stage',choices=['plates','cinematic'],required=True)
    ap.add_argument('--spp',type=int,default=8);a=ap.parse_args()
    root=a.rig.resolve();rig=json.loads((root/'rig.json').read_text());code=Path(__file__).resolve().parent
    repo=code.parents[6];py=repo/'.venv-rtaov/bin/python';count=rig['frame_end']-rig['frame_start']+1
    statusfile=root/(a.stage+'-status.json')
    def run(view,stage,args,env=None):
        statusfile.write_text(json.dumps({'stage':stage,'camera':view.name,'time':time.time(),'frames_per_camera':count},indent=2))
        with (view/(stage+'.log')).open('w') as log:
            subprocess.run([str(py),str(code/args[0]),*map(str,args[1:])],stdout=log,stderr=subprocess.STDOUT,check=True,env=env)
    try:
        for item in rig['views']:
            view=root/item['view'];done=view/(a.stage+'-complete.json')
            if done.exists():continue
            if a.stage=='plates':
                run(view,'plates',['plates.py',view,'--limit',count])
            else:
                if not (view/'plates-complete.json').exists():raise RuntimeError('Plate stage incomplete')
                run(view,'build',['run_unreal.py','build',view])
                marker_env=os.environ.copy();marker_env['HYBRID_CAMERA_MARKERS']='1'
                run(view,'markers',['run_unreal.py','render',view,'--limit',1,'--spp',8,'--denoiser','raw'],env=marker_env)
                run(view,'camera_gate',['check_camera_markers.py',view])
                run(view,'render',['run_unreal.py','render',view,'--limit',count,'--spp',a.spp,'--denoiser','nne'])
                run(view,'check',['check_render.py',view,'--limit',count])
                run(view,'composite',['composite.py',view,'--limit',count,'--preview-srgb-assumption'])
                run(view,'video',['make_review.py',view])
            done.write_text(json.dumps({'completed_unix':time.time(),'frames':count,'spp':a.spp if a.stage=='cinematic' else None}))
        statusfile.write_text(json.dumps({'stage':'complete','cameras':len(rig['views']),'frames_per_camera':count}))
    except Exception as exc:
        state=json.loads(statusfile.read_text()) if statusfile.exists() else {}
        state.update(failed=True,error=str(exc));statusfile.write_text(json.dumps(state,indent=2));raise


if __name__=='__main__':main()
