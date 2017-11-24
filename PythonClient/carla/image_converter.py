# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
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


try:
    import numpy
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
        0: [0, 0, 0],        # None
        1: [70, 70, 70],     # Buildings
        2: [190, 153, 153],  # Fences
        3: [72, 0, 90],      # Other
        4: [220, 20, 60],    # Pedestrians
        5: [153, 153, 153],  # Poles
        6: [157, 234, 50],   # RoadLines
        7: [128, 64, 128],   # Roads
        8: [244, 35, 232],   # Sidewalks
        9: [107, 142, 35],   # Vegetation
        10: [0, 0, 255],     # Vehicles
        11: [102, 102, 156], # Walls
        12: [220, 220, 0]    # TrafficSigns
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
    grayscale = numpy.dot(array[:, :, :3], [256.0 * 256.0, 256.0, 1.0])
    grayscale /= (256.0 * 256.0 * 256.0 - 1.0)
    return grayscale


def depth_to_logarithmic_grayscale(image):
    """
    Convert an image containing CARLA encoded depth-map to a logarithmic
    grayscale image array.
    """
    grayscale = depth_to_array(image)
    # Convert to logarithmic depth.
    logdepth = numpy.ones(grayscale.shape) + (numpy.log(grayscale) / 5.70378)
    logdepth = numpy.clip(logdepth, 0.0, 1.0)
    logdepth *= 255.0
    # Expand to three colors.
    return numpy.repeat(logdepth[:, :, numpy.newaxis], 3, axis=2)
