"""Fast diagnostic raster ego view plus measured overhead trajectories."""
import json,math,subprocess
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFont


def save_preview(data,images,out,fps):
    out=Path(out);folder=out/'preview';folder.mkdir(exist_ok=True)
    initial={a['role']:a for a in data['frames'][0]['actors']};hero=initial['hero']
    yaw=math.radians(hero['yaw']);forward=np.array([math.cos(yaw),math.sin(yaw)]);right=np.array([-math.sin(yaw),math.cos(yaw)])
    origin=np.array([hero['x'],hero['y']]);font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
    colors={'hero':'#55aaff','lead':'#ffb45b','mary_proxy':'#54df85'}
    events=data.get('events',[])
    event_times={e['name']:e['time_s'] for e in events}
    def xy(point):
        d=np.array(point)-origin
        return (800+float(d@right)*18,394-float(d@forward)*7.0)
    for i,f in enumerate(data['frames']):
        image=images.get(f['frame'])
        if image is None:raise ValueError(f'Missing camera frame {f["frame"]}')
        pixels=np.frombuffer(image.raw_data,np.uint8).reshape(image.height,image.width,4)[...,:3][...,::-1]
        canvas=Image.new('RGB',(960,432),'#181c22');canvas.paste(Image.fromarray(pixels),(0,40));draw=ImageDraw.Draw(canvas)
        t=f['t'];stage='FOLLOWING LEAD'
        if t>=event_times.get('lead_slowdown_command',math.inf):stage='LEAD SLOWING'
        if t>=event_times.get('pedestrian_start',math.inf):stage='PEDESTRIAN CROSSING'
        if t>=event_times.get('pedestrian_commitment',math.inf):stage='PEDESTRIAN APPROACHING EGO LANE'
        if t>=event_times.get('ego_emergency_brake',math.inf):stage='EGO BRAKING / HOLDING'
        if t>=event_times.get('pedestrian_cleared',math.inf):stage='PEDESTRIAN CLEARED'
        draw.text((12,10),f'FAST BEHAVIOR PREVIEW | {t:05.2f}s | {stage}',font=font,fill='white')
        draw.line([(800,44),(800,399)],fill='#76808a',width=1)
        draw.text((670,42),'Schematic lane widths',font=font,fill='#aeb9c6')
        for lateral in [-1.64,1.64,5.59]:draw.line([(800+lateral*18,44),(800+lateral*18,399)],fill='#525961',width=1)
        for distance in range(0,51,10):
            y=394-distance*7;draw.text((646,y-8),f'{distance}m',font=font,fill='#aeb9c6')
        for a in f['actors']:
            points=np.array(a['footprint']);center=points.mean(0);points=points[np.argsort(np.arctan2(points[:,1]-center[1],points[:,0]-center[0]))]
            draw.polygon([xy(p) for p in points],fill=colors.get(a['role'],'white'))
            x,y=xy((a['x'],a['y']));draw.text((x+10,y-8),a['role'],font=font,fill=colors.get(a['role'],'white'))
        rows={a['role']:a for a in f['actors']};e=rows['hero'];l=rows['lead']
        draw.text((12,407),f'Ego {e["speed"]*3.6:4.1f} km/h | Lead {l["speed"]*3.6:4.1f} km/h | Brake {e["brake"]:.2f}',font=font,fill='white')
        canvas.save(folder/f'{i:04d}.png')
    video=out/'preview.mp4'
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-framerate',str(fps),'-i',str(folder/'%04d.png'),'-frames:v',str(len(data['frames'])),'-c:v','libx264','-crf','20','-pix_fmt','yuv420p','-movflags','+faststart',str(video)],check=True)
    subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-xerror','-i',str(video),'-f','null','-'],check=True)
    (out/'preview-validation.json').write_text(json.dumps({'frames':len(data['frames']),'fps':fps,'video':str(video.resolve()),'scope':'Diagnostic CARLA raster + telemetry, not final NuRec beauty','camera_frame_ids_match_telemetry':True,'decode_passed':True},indent=2))
    sheet=Image.new('RGB',(960,216*5))
    for n,i in enumerate(np.linspace(0,len(data['frames'])-1,10).astype(int)):
        im=Image.open(folder/f'{i:04d}.png');im.thumbnail((480,216));sheet.paste(im,(480*(n%2),216*(n//2)))
    sheet.save(out/'contact-sheet.jpg',quality=92)
