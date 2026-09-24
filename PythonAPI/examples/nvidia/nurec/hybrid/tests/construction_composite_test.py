"""Regression: scenario-selected prop labels composite, obey depth, preserve legacy masks."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image


def main():
    script=Path(__file__).resolve().parents[1]/'hybrid_video_finish.py'
    with tempfile.TemporaryDirectory() as tmp:
        root=Path(tmp)
        bg=np.full((12,12,3),40,dtype=np.uint8)
        rgb=np.full_like(bg,[220,110,20])
        sem=np.full((12,12),21,dtype=np.uint8)
        dist=np.full((12,12),10,dtype=np.float32)
        np.savez(root/'A_0000.npz',rgb=bg)
        np.savez(root/'B_0000.npz',rgb=rgb,sem=sem,dist=dist)
        for tags,depth,expected in [([14,21],20,rgb),([14,21],5,bg),(None,20,bg)]:
            meta={'frames':1}
            if tags is not None:meta['synthetic_tags']=tags
            (root/'meta.json').write_text(json.dumps(meta))
            np.savez(root/'E_0000.npz',color=bg,distance=np.full((12,12),depth),opacity=np.ones((12,12)))
            subprocess.run([sys.executable,str(script),str(root),'--jobs','1','--erode','0','--alpha-sigma','0'],check=True,capture_output=True)
            actual=np.asarray(Image.open(root/'frames/comp_0000.png'))
            np.testing.assert_array_equal(actual,expected)
        # A co-located receiver must retain its cast shadow when road-depth
        # tolerance is enabled, while genuinely nearer geometry stays unshadowed.
        (root/'meta.json').write_text(json.dumps({'frames':1}))
        np.savez(root/'A_0000.npz',rgb=np.full_like(bg,100))
        np.savez(root/'B_0000.npz',rgb=np.full_like(bg,50),sem=np.ones((12,12),dtype=np.uint8),dist=dist)
        for depth,opacity,expected in [(10,1,20),(5,1,40),(10,0,40)]:
            np.savez(root/'E_0000.npz',color=bg,distance=np.full((12,12),depth),opacity=np.full((12,12),opacity))
            subprocess.run([sys.executable,str(script),str(root),'--jobs','1','--shadow-depth-bias','0.5'],check=True,capture_output=True)
            actual=np.asarray(Image.open(root/'frames/comp_0000.png'))
            np.testing.assert_array_equal(actual,np.full_like(bg,expected))
    print('PASS: construction props visible, depth-occluded, legacy output unchanged')
    print('PASS: contact shadow retained on aligned road; foreground and empty space protected')


if __name__=='__main__': main()
