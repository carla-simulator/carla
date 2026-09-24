import sys,struct,tempfile,unittest
from pathlib import Path
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from core import *

class CoreTests(unittest.TestCase):
    def test_receiver_transfer_preserves_relative_shadow_and_bounce(self):
        plate=np.ones((1,1,3))*.05;receiver=np.ones((1,1,3))*.1
        delta=np.array([[[-.02,.01,-.01]]])
        expected=np.array([[[-.01,.005,-.005]]])
        np.testing.assert_allclose(transfer_receiver_delta(delta,plate,receiver),expected)
        np.testing.assert_allclose(transfer_receiver_delta(delta*2,plate,receiver*2),expected)
        self.assertTrue(np.isfinite(transfer_receiver_delta(delta,plate,receiver*0)).all())
    def test_projection_engine_basis(self):
        t=optical_pose([10,20,3],[0,90,0]);K=np.array([[100,0,50],[0,100,30],[0,0,1]])
        pixels,z=project([[10,30,3],[9,30,3],[10,30,4]],t,K)
        np.testing.assert_allclose(pixels,[[50,30],[60,30],[50,20]],atol=1e-10)
        np.testing.assert_allclose(z,10)
    def test_distance_conversion(self):
        K=np.array([[1,0,0],[0,1,0],[0,0,1]])
        np.testing.assert_allclose(ray_to_z(np.array([[2,2*np.sqrt(2)]]),K),2)
    def test_protocol_preserves_hdr(self):
        rgb=np.array([[[2.,-.2,0.1234567]]],np.float32);d=np.array([[100000.]],np.float32);a=np.array([[.3]],np.float32)
        p=b'HYF1'+struct.pack('<II',1,1)+rgb.transpose(2,0,1).tobytes()+d.tobytes()+a.tobytes()
        for actual,expected in zip(decode_float_payload(p),(rgb,d,a)):np.testing.assert_array_equal(actual,expected)
        with self.assertRaises(ValueError):decode_float_payload(p[:-1])
        with self.assertRaises(ValueError):decode_float_payload(b'HYBR'+p[4:])
    def test_partial_occluder(self):
        plate=np.ones((1,1,3));fg=np.array([[[.5,0,0]]]);a=np.array([[.5]])
        # A fully nearer splat with 50% coverage leaves 50% of the foreground.
        result=merge(plate,np.array([[.5]]),np.array([[1.]]),fg,a,np.array([[2.]]))
        np.testing.assert_allclose(result,[[[1,.75,.75]]])
    def test_empty_splat_and_infinite_depth(self):
        result=merge(np.ones((1,1,3)),np.zeros((1,1)),np.full((1,1),np.inf),np.zeros((1,1,3)),np.ones((1,1)),np.ones((1,1)))
        np.testing.assert_array_equal(result,0)
    def test_contact_depth_bias_preserves_equal_surface(self):
        plate=np.ones((1,1,3));fg=np.zeros((1,1,3));alpha=np.ones((1,1))
        # A planted shoe must not turn half-transparent at equal road depth.
        result=merge(plate,alpha,np.array([[2.]]),fg,alpha,np.array([[2.]]),tolerance=.03,depth_bias=.03)
        np.testing.assert_allclose(result,fg)
        # A genuinely nearer occluder still fully blocks it.
        result=merge(plate,alpha,np.array([[1.9]]),fg,alpha,np.array([[2.]]),tolerance=.03,depth_bias=.03)
        np.testing.assert_allclose(result,plate)
    def test_color_no_silent_assumption(self):
        with self.assertRaises(ValueError):to_acescg(np.ones((1,1,3)),'unverified_native')
        np.testing.assert_allclose(to_acescg(np.ones((1,1,3))*4,'linear_srgb'),4,atol=1e-6)
    def test_exr_precision(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'test.exr';v=np.array([[123456.125,.012345678]],np.float32)
            write_exr(p,{'Z':v});np.testing.assert_array_equal(read_exr(p)['Z'],v)
            write_exr(p,{'R':np.array([[4]],np.float32)},half=True)
            self.assertEqual(read_exr(p)['R'][0,0],4)

if __name__=='__main__':unittest.main()
