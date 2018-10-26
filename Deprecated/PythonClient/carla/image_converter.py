# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Handy conversions for CARLA images.

The functions here are provided for real-time display, if you want to save the
converted images, save the images from Python without conversion and convert
them afterwards with the C++ implementation at "Util/ImageConverter" as it
provides considerably better performance.
"""

import math

try:
    import numpy
    from numpy.matlib import repmat
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


from . import sensor


def to_bgra_array(image):
    """Convert a CARLA raw image to a BGRA numpy array."""
    if not isinstance(image, sensor.Image):
        raise ValueError("Argument must be a carla.sensor.Image")
    array = numpy.frombuffer(image.raw_data, dtype=numpy.dtype("uint8"))
    array = numpy.reshape(array, (image.height, image.width, 4))
    return array


def to_rgb_array(image):
    """Convert a CARLA raw image to a RGB numpy array."""
    array = to_bgra_array(image)
    # Convert BGRA to RGB.
    array = array[:, :, :3]
    array = array[:, :, ::-1]
    return array


def labels_to_array(image):
    """
    Convert an image containing CARLA semantic segmentation labels to a 2D array
    containing the label of each pixel.
    """
    return to_bgra_array(image)[:, :, 2]


def labels_to_cityscapes_palette(image):
    """
    Convert an image containing CARLA semantic segmentation labels to
    Cityscapes palette.
    """
    classes = {
        0: [0, 0, 0],         # None
        1: [70, 70, 70],      # Buildings
        2: [190, 153, 153],   # Fences
        3: [72, 0, 90],       # Other
        4: [220, 20, 60],     # Pedestrians
        5: [153, 153, 153],   # Poles
        6: [157, 234, 50],    # RoadLines
        7: [128, 64, 128],    # Roads
        8: [244, 35, 232],    # Sidewalks
        9: [107, 142, 35],    # Vegetation
        10: [0, 0, 255],      # Vehicles
        11: [102, 102, 156],  # Walls
        12: [220, 220, 0]     # TrafficSigns
    }
    array = labels_to_array(image)
    result = numpy.zeros((array.shape[0], array.shape[1], 3))
    for key, value in classes.items():
        result[numpy.where(array == key)] = value
    return result


def depth_to_array(image):
    """
    Convert an image containing CARLA encoded depth-map to a 2D array containing
    the depth value of each pixel normalized between [0.0, 1.0].
    """
    array = to_bgra_array(image)
    array = array.astype(numpy.float32)
    # Apply (R + G * 256 + B * 256 * 256) / (256 * 256 * 256 - 1).
    normalized_depth = numpy.dot(array[:, :, :3], [65536.0, 256.0, 1.0])
    normalized_depth /= 16777215.0  # (256.0 * 256.0 * 256.0 - 1.0)
    return normalized_depth


def depth_to_logarithmic_grayscale(image):
    """
    Convert an image containing CARLA encoded depth-map to a logarithmic
    grayscale image array.
    "max_depth" is used to omit the points that are far enough.
    """
    normalized_depth = depth_to_array(image)
    # Convert to logarithmic depth.
    logdepth = numpy.ones(normalized_depth.shape) + \
        (numpy.log(normalized_depth) / 5.70378)
    logdepth = numpy.clip(logdepth, 0.0, 1.0)
    logdepth *= 255.0
    # Expand to three colors.
    return numpy.repeat(logdepth[:, :, numpy.newaxis], 3, axis=2)


def depth_to_local_point_cloud(image, color=None, max_depth=0.9):
    """
    Convert an image containing CARLA encoded depth-map to a 2D array containing
    the 3D position (relative to the camera) of each pixel and its corresponding
    RGB color of an array.
    "max_depth" is used to omit the points that are far enough.
    """
    far = 1000.0  # max depth in meters.
    normalized_depth = depth_to_array(image)

    # (Intrinsic) K Matrix
    k = numpy.identity(3)
    k[0, 2] = image.width / 2.0
    k[1, 2] = image.height / 2.0
    k[0, 0] = k[1, 1] = image.width / \
        (2.0 * math.tan(image.fov * math.pi / 360.0))

    # 2d pixel coordinates
    pixel_length = image.width * image.height
    u_coord = repmat(numpy.r_[image.width-1:-1:-1],
                     image.height, 1).reshape(pixel_length)
    v_coord = repmat(numpy.c_[image.height-1:-1:-1],
                     1, image.width).reshape(pixel_length)
    if color is not None:
        color = color.reshape(pixel_length, 3)
    normalized_depth = numpy.reshape(normalized_depth, pixel_length)

    # Search for pixels where the depth is greater than max_depth to
    # delete them
    max_depth_indexes = numpy.where(normalized_depth > max_depth)
    normalized_depth = numpy.delete(normalized_depth, max_depth_indexes)
    u_coord = numpy.delete(u_coord, max_depth_indexes)
    v_coord = numpy.delete(v_coord, max_depth_indexes)
    if color is not None:
        color = numpy.delete(color, max_depth_indexes, axis=0)

    # pd2 = [u,v,1]
    p2d = numpy.array([u_coord, v_coord, numpy.ones_like(u_coord)])

    # P = [X,Y,Z]
    p3d = numpy.dot(numpy.linalg.inv(k), p2d)
    p3d *= normalized_depth * far

    # Formating the output to:
    # [[X1,Y1,Z1,R1,G1,B1],[X2,Y2,Z2,R2,G2,B2], ... [Xn,Yn,Zn,Rn,Gn,Bn]]
    if color is not None:
        # numpy.concatenate((numpy.transpose(p3d), color), axis=1)
        return sensor.PointCloud(
            image.frame_number,
            numpy.transpose(p3d),
            color_array=color)
    # [[X1,Y1,Z1],[X2,Y2,Z2], ... [Xn,Yn,Zn]]
    return sensor.PointCloud(image.frame_number, numpy.transpose(p3d))
