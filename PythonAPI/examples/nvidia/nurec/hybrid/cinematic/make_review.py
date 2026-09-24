"""Encode a completed cinematic preview and verify its frame count and decode."""
import argparse
import json
import subprocess
from pathlib import Path
from PIL import Image, ImageDraw


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('shot',type=Path)
    a=ap.parse_args();root=a.shot.resolve()
    m=json.loads((root/'manifest.json').read_text())
    count=m['frame_end']-m['frame_start']+1
    checks=json.loads((root/'ue-pass-validation.json').read_text())
    composite=json.loads((root/'composite-report.json').read_text())
    render=json.loads((root/'unreal-render-status.json').read_text())
    if not render['success'] or not checks['artifact_passes_valid'] or len(checks['frames'])!=count or len(composite['frames'])!=count:
        raise SystemExit('Complete validated render/composite required')
    for frame in range(m['frame_start'],m['frame_end']+1):
        with Image.open(root/f'comp/preview/{frame}.png') as im:
            if im.size!=tuple(m['resolution']):raise ValueError('Preview dimensions differ')
            im.verify()
    video=root/'comp/review.mp4'
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-framerate',str(m['fps']),
                    '-start_number',str(m['frame_start']),'-i',str(root/'comp/preview/%04d.png'),
                    '-frames:v',str(count),'-vf','scale=in_range=full:out_range=tv:out_color_matrix=bt709',
                    '-c:v','libx264','-crf','16','-preset','medium','-pix_fmt','yuv420p',
                    '-color_primaries','bt709','-color_trc','iec61966-2-1','-colorspace','bt709','-color_range','tv',
                    '-movflags','+faststart',str(video)],check=True)
    probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','v:0',
                        '-show_entries','stream=nb_frames,width,height,r_frame_rate,duration','-of','json',str(video)]))['streams'][0]
    if int(probe['nb_frames'])!=count or probe['r_frame_rate']!=f"{m['fps']}/1":
        raise ValueError('Encoded timing mismatch')
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-xerror','-i',str(video),'-f','null','-'],check=True)
    samples=[m['frame_start']+round(i*(count-1)/9) for i in range(10)]
    sheet=Image.new('RGB',(960,5*300),(22,22,22));draw=ImageDraw.Draw(sheet)
    for i,frame in enumerate(samples):
        im=Image.open(root/f'comp/preview/{frame}.png');im.thumbnail((480,270))
        x=(i%2)*480;y=(i//2)*300;sheet.paste(im,(x,y))
        stage='approach / wait' if frame<m.get('crossing_start_frame',frame) else 'crossing' if frame<=m.get('crossing_end_frame',frame) else 'crossing complete'
        draw.text((x+10,y+276),f'{(frame-m["frame_start"])/m["fps"]:.2f}s | {stage}',fill='white')
    sheet.save(root/'comp/contact-sheet.jpg',quality=92)
    poster=m.get('crossing_start_frame',m['frame_start'])+round(3*m['fps'])
    poster=min(poster,m['frame_end'])
    Image.open(root/f'comp/preview/{poster}.png').save(root/'comp/poster.jpg',quality=95)
    (root/'video-validation.json').write_text(json.dumps({'video':str(video),'probe':probe,'full_decode_passed':True,
        'input_preview_frames_verified':count,'visual_review':'pending','production_ready':False},indent=2))
    print(video)


if __name__=='__main__':main()
