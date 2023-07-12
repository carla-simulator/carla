import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla

import weakref
import random
import math
import time
import queue
import cv2
import json

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


OBJECTS = dict()

# ==============================================================================
# -- functions -----------------------------------------------------------------
# ==============================================================================

def get_bb_from_seg(seg_im, object):
    """
    The input image should be a RGB image with dim (wxhx3).
    """
    np_img = np.array(seg_im)

    object_color = OBJECTS[object]
    object_mask = np.where(np_img == object_color)

    # reduce to 2 dimensions
    object_mask = object_mask[:,:,0]

    contours = cv2.findContours(object_mask)

    # convert contours to bounding boxes and return the bounding box