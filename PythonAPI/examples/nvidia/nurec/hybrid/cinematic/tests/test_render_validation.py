import json
import sys
import tempfile
import time
import unittest
from pathlib import Path
import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from check_render import check
from core import write_exr


class RenderValidationTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.shot=Path(self.tmp.name)
        manifest={'schema':'hybrid.cinematic.v1','camera_model':'pinhole',
                  'distortion_applied_during_render':False,'resolution':[8,4],
                  'K':[[4,0,4],[0,4,2],[0,0,1]],'fps':24,
                  'frame_start':1001,'frame_end':1048,'shutter_angle':0}
        (self.shot/'manifest.json').write_text(json.dumps(manifest))
        (self.shot/'capture.json').write_text(json.dumps([{'frame':1001}]))
        alpha=np.ones((4,8));alpha[0]=0
        self.data={'R':alpha*.1,'G':alpha*.2,'B':alpha*.3,'A':alpha}
        for layer in ['beauty','integration','clean']:
            write_exr(self.path(layer),self.data,half=True)
        write_exr(self.path('integration.depth_m'),{**{c:np.ones((4,8))*8 for c in 'RGB'},'A':np.ones((4,8))})

    def path(self,layer):return self.shot/f'renders/ue_review/.{layer}.1001.exr'

    def test_valid_passes_do_not_certify_production(self):
        report=check(self.shot,1)
        self.assertTrue(report['artifact_passes_valid'])
        self.assertFalse(report['production_ready'])

    def test_beauty_disguised_as_depth_is_rejected(self):
        write_exr(self.path('integration.depth_m'),self.data)
        report=check(self.shot,1)
        self.assertFalse(report['artifact_passes_valid'])
        self.assertIn('scalar',report['errors'][0]['error'])

    def test_opaque_alpha_is_rejected(self):
        write_exr(self.path('beauty'),dict(self.data,A=np.ones((4,8))),half=True)
        self.assertFalse(check(self.shot,1)['artifact_passes_valid'])

    def test_old_output_cannot_certify_restarted_render(self):
        (self.shot/'render-attempt.json').write_text(json.dumps({'started_unix':time.time()+1,'returncode':0}))
        report=check(self.shot,1)
        self.assertFalse(report['artifact_passes_valid'])
        self.assertIn('Stale',report['errors'][0]['error'])


if __name__=='__main__':unittest.main()
