"""Geometric regression: prop bases remain on sloped roads at arbitrary yaw."""
import math
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import carla
import numpy as np
from prop_grounding import BASE_Z_M, ground_prop, grounded_rotation

class PropGroundingTest(unittest.TestCase):
    def test_rotated_axes_lie_in_road_plane(self):
        for dx,dy in [(0.,0.),(.08,-.04),(-.12,.07)]:
            normal=np.array([-dx,-dy,1.]);normal/=np.linalg.norm(normal)
            for yaw in [-145,0,35,90,170]:
                r=grounded_rotation(yaw,dx,dy)
                # CARLA Transform.get_matrix currently disagrees with engine
                # pitch/roll, so check its documented Unreal rotation basis.
                p,y,t=map(math.radians,[r.pitch,r.yaw,r.roll])
                f=np.array([math.cos(p)*math.cos(y),math.cos(p)*math.sin(y),math.sin(p)])
                right=np.array([math.sin(t)*math.sin(p)*math.cos(y)-math.cos(t)*math.sin(y),math.sin(t)*math.sin(p)*math.sin(y)+math.cos(t)*math.cos(y),-math.sin(t)*math.cos(p)])
                self.assertAlmostEqual(float(f@normal),0.,places=7)
                self.assertAlmostEqual(float(right@normal),0.,places=7)
    def test_known_mesh_base_meets_sloped_road(self):
        dx,dy=.04,-.03
        class World:
            def cast_ray(self,start,end):
                class Hit:
                    label=carla.CityObjectLabel.Roads
                    location=carla.Location(start.x,start.y,100+dx*start.x+dy*start.y)
                return [Hit()]
        class Actor:
            id=1
            def __init__(self,bp):self.type_id=bp;self.tf=carla.Transform(carla.Location(5,6,100),carla.Rotation(yaw=70))
            def get_transform(self):return self.tf
            def set_transform(self,tf):self.tf=tf
        for bp,base in BASE_Z_M.items():
            a=Actor(bp);r=ground_prop(World(),a)
            signed_base_height=(a.tf.location.z-(100+dx*5+dy*6))/math.sqrt(1+dx*dx+dy*dy)+base
            self.assertAlmostEqual(signed_base_height,.001/math.sqrt(1+dx*dx+dy*dy),delta=2e-5)
        class Empty:
            def cast_ray(self,*args):return []
        with self.assertRaisesRegex(ValueError,'No collision road'):ground_prop(Empty(),Actor(next(iter(BASE_Z_M))))

if __name__=='__main__':unittest.main()
