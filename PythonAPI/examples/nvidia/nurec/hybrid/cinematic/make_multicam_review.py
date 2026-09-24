"""Assemble synchronized camera-wall video and preserve individual camera outputs."""
import argparse
import json
import subprocess
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('rig',type=Path)
    ap.add_argument('--variant',choices=['raw','single','temporal'],default='raw')
    a=ap.parse_args();root=a.rig.resolve()
    delivery=root if a.variant=='raw' else root/'diffusion'/a.variant
    delivery.mkdir(parents=True,exist_ok=True)
    rig=json.loads((root/'rig.json').read_text());views={v['camera_index']:v for v in rig['views']}
    order=[0,1,2,3,6,5];labels=['FRONT LEFT','FRONT WIDE','FRONT RIGHT','REAR LEFT','FRONT TELEPHOTO','REAR RIGHT']
    fontpath='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
    font=ImageFont.truetype(fontpath,19);large=ImageFont.truetype(fontpath,27)
    start,end=rig['frame_start'],rig['frame_end'];count=end-start+1;frames=delivery/'wall-frames';frames.mkdir(exist_ok=True)
    expected_clock=None;index=[]
    for camera in order:
        v=views[camera];p=root/v['view']
        if not (p/'cinematic-complete.json').exists():raise ValueError('Incomplete view '+v['view'])
        if a.variant!='raw' and not (p/'diffusion'/f'{a.variant}-complete.json').exists():raise ValueError('Incomplete diffusion view '+v['view'])
        gate=json.loads((p/'camera-render-gate.json').read_text())
        if not gate['passed']:raise ValueError('Failed native lens gate')
        capture=json.loads((p/'capture.json').read_text())
        clock=[(f['frame'],f['simulation_frame'],f['simulation_time_s'],f['timestamp_us']) for f in capture]
        if expected_clock is None:expected_clock=clock
        if clock!=expected_clock:raise ValueError('View clocks differ')
        index.append({'camera_index':camera,'name':v['name'],'video':str(p/'comp/review.mp4' if a.variant=='raw' else p/'diffusion'/f'{a.variant}.mp4')})
    for frame in range(start,end+1):
        image=Image.new('RGB',(1920,864),(14,19,27));draw=ImageDraw.Draw(image);seconds=(frame-start)/rig['fps']
        draw.text((20,18),'Intersection · late pedestrian crossing',font=large,fill=(237,243,250))
        state='Ego follows lead' if frame-start<36 else 'Lead slows near intersection' if frame-start<124 else 'Pedestrian crossing / ego yielding'
        draw.text((880,25),state,font=font,fill=(128,208,209))
        draw.text((1700,22),f'{seconds:04.2f} / 8.50 s',font=font,fill=(237,243,250))
        for j,camera in enumerate(order):
            v=views[camera];x=16+(j%3)*632;y=64+(j//3)*392
            draw.rectangle((x,y,x+623,y+382),fill=(27,34,45))
            source_path=root/v['view']/f'comp/preview/{frame}.png' if a.variant=='raw' else root/v['view']/'diffusion'/a.variant/f'{frame}.png'
            with Image.open(source_path) as source:
                image.paste(source.resize((624,351),Image.Resampling.LANCZOS),(x,y))
            draw.text((x+10,y+356),labels[j],font=font,fill=(237,243,250))
            draw.text((x+490,y+356),f'Camera {camera}',font=font,fill=(160,175,192))
        draw.rectangle((16,848,1903,851),fill=(44,57,73))
        draw.rectangle((16,848,16+round(1887*(frame-start)/(count-1)),851),fill=(95,211,206))
        image.save(frames/f'{frame}.png')
    video=delivery/'multicam.mp4'
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-framerate',str(rig['fps']),'-start_number',str(start),
                    '-i',str(frames/'%04d.png'),'-frames:v',str(count),'-c:v','libx264','-crf','17','-preset','medium',
                    '-pix_fmt','yuv420p','-movflags','+faststart',str(video)],check=True)
    subprocess.run(['ffmpeg','-v','error','-xerror','-i',str(video),'-f','null','-'],check=True)
    probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','v:0','-show_entries',
                'stream=nb_frames,width,height,r_frame_rate,duration','-of','json',str(video)]))['streams'][0]
    if int(probe['nb_frames'])!=count or probe['r_frame_rate']!=f"{rig['fps']}/1":raise ValueError('Wall timing mismatch')
    (delivery/'video-validation.json').write_text(json.dumps({'variant':a.variant,'full_decode_passed':True,'synchronized_clocks':True,'probe':probe,'individual_views':index},indent=2))
    with Image.open(frames/f'{start+124}.png') as image:image.save(delivery/'poster.jpg',quality=94)


if __name__=='__main__':main()
