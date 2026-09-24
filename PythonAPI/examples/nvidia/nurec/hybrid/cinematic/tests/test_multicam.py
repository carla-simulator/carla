"""Camera-rig checks independent of Unreal availability."""
import copy
import sys
from pathlib import Path
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from core import camera_rays, camera_world_points, optical_pose
from prepare_multicam import camera_pose


def test_ftheta_off_axis_distance_is_not_pinhole_depth():
    m={'resolution':[4,2],'camera_model':'ftheta',
       'ftheta':{'principal_point_x':2.,'principal_point_y':1.,'pixeldist_to_angle_poly':[0,.2]}}
    rays=camera_rays(m)
    angle=.2*np.hypot(1.5,.5)
    np.testing.assert_allclose(rays[0,0,2],np.cos(angle))
    distance=7/rays[...,2]
    points=camera_world_points(distance*rays[...,2],m,np.eye(4))
    np.testing.assert_allclose(points[...,2],7)
    np.testing.assert_allclose(np.linalg.norm(points,axis=-1),distance)


def test_scaled_ftheta_preserves_native_pixel_rays():
    # A pixel at half resolution samples native coordinate 2*i+1. Here
    # choose an odd scale to match an actual high-resolution pixel centre.
    m={'resolution':[12,6],'camera_model':'ftheta',
       'ftheta':{'principal_point_x':5.7,'principal_point_y':3.9,'pixeldist_to_angle_poly':[0,.09,1e-5]}}
    small=copy.deepcopy(m);small['resolution']=[4,2]
    f=small['ftheta'];f['principal_point_x']/=3;f['principal_point_y']/=3
    f['pixeldist_to_angle_poly']=[c*3**i for i,c in enumerate(f['pixeldist_to_angle_poly'])]
    np.testing.assert_allclose(camera_rays(small),camera_rays(m)[1::3,1::3],atol=1e-12)


def test_moving_rig_relative_pose_and_optical_handedness():
    front=optical_pose([1.6,-.06,1.2],[.3,-.2,.4])
    side=optical_pose([2.2,-.9,.8],[-1,-66,.2])
    relative=np.linalg.inv(front)@side
    for yaw in [-179,-20,0,60,179]:
        moving=optical_pose([23,51,482],[2,yaw,-1])
        camera=moving@relative
        np.testing.assert_allclose(optical_pose(**camera_pose(camera)),camera,atol=1e-10)
        np.testing.assert_allclose(np.linalg.inv(moving)@camera,relative,atol=1e-10)
