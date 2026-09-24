"""Run the existing single and temporal harmonizers independently on each view."""
import argparse
import hashlib
import json
import os
import shutil
from pathlib import Path
import subprocess
import time
from PIL import Image


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('rig',type=Path)
    args=ap.parse_args()
    root=args.rig.resolve();code=Path(__file__).resolve().parent;repo=code.parents[6]
    h=repo/'harmonizer';py=repo/'.venv-rtaov/bin/python'
    rig=json.loads((root/'rig.json').read_text());start,end=rig['frame_start'],rig['frame_end'];count=end-start+1
    out=root/'diffusion';out.mkdir(exist_ok=True)
    state={}
    def status(**kw):
        state.update(kw,updated_unix=time.time());(out/'status.json').write_text(json.dumps(state,indent=2));print(json.dumps(state),flush=True)
    def run(cmd,log):
        checkpoint=log.with_suffix('.inference-complete.json') if log.name in ('temporal.log','single.log') else None
        if checkpoint is not None and checkpoint.exists():return
        with log.open('w') as f:subprocess.run(list(map(str,cmd)),stdout=f,stderr=subprocess.STDOUT,check=True)
        if checkpoint is not None:checkpoint.write_text(json.dumps({'completed_unix':time.time(),'command':list(map(str,cmd))}))
    config={'temporal':{'timestep':250,'resolution':1360,'offsets':[-1,-2,-3,-4],'use_sched':True},'single':{'resolution':[1088,1920],'color_transfer':False},'stacked':False,'cross_camera_conditioning':False,'seedvr2':False}
    (out/'configuration.json').write_text(json.dumps(config,indent=2))
    try:
        for variant in ['temporal','single']:
            for item in sorted(rig['views'],key=lambda v: 0 if v['camera_index']==1 else v['camera_index']+1):
                view=root/item['view'];d=view/'diffusion';d.mkdir(exist_ok=True)
                done=d/f'{variant}-complete.json'
                if done.exists():continue
                if not (view/'cinematic-complete.json').exists():raise ValueError('Unfinished source view')
                tin=d/'input';tin.mkdir(exist_ok=True)
                hashes={}
                for frame in range(start,end+1):
                    src=view/f'comp/preview/{frame}.png';dst=tin/f'comp_{frame:04d}.png'
                    hashes[str(frame)]=hashlib.sha256(src.read_bytes()).hexdigest()
                    if not dst.exists():os.link(src,dst)
                    elif hashlib.sha256(dst.read_bytes()).hexdigest()!=hashes[str(frame)]:raise ValueError('Changed source frame')
                (d/'input-sha256.json').write_text(json.dumps(hashes,indent=2))
                status(stage='inference',camera=view.name,variant=variant)
                if variant=='temporal':
                    run(['docker','run','--rm','--name','sh040-temporal','--gpus','all','--ipc=host','-v',f'{h}:/work','-v',f'{d}:/trial','-v',f'{h}/text2image_patched.py:/usr/local/lib/python3.12/dist-packages/cosmos_predict2/pipelines/text2image.py:ro','-w','/work/src','--entrypoint','python3','harmonizer-cosmos-env','inference_pix2pix_turbo_harmonizer.py','--input_image','/trial/input','--model_path','/work/models/diffusion_harmonizer.pkl','--model_identifier','temporal','--timestep','250','--resolution','1360','--use_sched'],d/'temporal.log')
                    generated=d/'input_temporal';prefix='comp_'
                else:
                    run(['docker','start','nurec-cinematic-mvp'],d/'container-start.log')
                    mounted='/work/'+str(tin.relative_to(repo))
                    run(['docker','exec','-e','RUNFILES_DIR=/app/run.runfiles','-w','/app','nurec-cinematic-mvp','/tmp/hybrid/runpy','/work/carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/engine_patch/harmonize.py',mounted,'--res=1088x1920'],d/'single.log')
                    generated=tin;prefix='harm_'
                frames=d/variant;frames.mkdir(exist_ok=True)
                for frame in range(start,end+1):
                    src=generated/f'{prefix}{frame:04d}.png'
                    with Image.open(src) as im:
                        im.load()
                        if im.size!=tuple(rig['resolution'] if 'resolution' in rig else [960,540]):raise ValueError('Unexpected output size')
                    dst=frames/f'{frame}.png'
                    if dst.exists():dst.unlink()
                    shutil.copyfile(src,dst)
                status(stage='encode_validate',camera=view.name,variant=variant)
                video=d/f'{variant}.mp4'
                run(['ffmpeg','-hide_banner','-loglevel','error','-y','-framerate',rig['fps'],'-start_number',start,'-i',frames/'%04d.png','-frames:v',count,'-c:v','libx264','-crf','16','-preset','fast','-pix_fmt','yuv420p','-movflags','+faststart',video],d/f'{variant}-encode.log')
                run(['ffmpeg','-v','error','-xerror','-i',video,'-f','null','-'],d/f'{variant}-decode.log')
                probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','v:0','-show_entries','stream=nb_frames,r_frame_rate,width,height,duration','-of','json',str(video)]))['streams'][0]
                if int(probe['nb_frames'])!=count or probe['r_frame_rate']!=f"{rig['fps']}/1":raise ValueError('Output timing mismatch')
                done.write_text(json.dumps({'completed_unix':time.time(),'full_decode_passed':True,'probe':probe,'config':config[variant]},indent=2))
            status(stage='assemble_wall',variant=variant,camera=None)
            run([py,code/'make_multicam_review.py',root,'--variant',variant],out/f'{variant}-wall.log')
        status(stage='ready_for_visual_review',camera=None,variant='both')
    except Exception as exc:
        status(stage='failed',error=str(exc));raise


if __name__=='__main__':main()
