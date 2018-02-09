# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import math

from collections import namedtuple

try:
    import numpy
except ImportError:
    raise RuntimeError(
        'cannot import numpy, make sure numpy package is installed.')

try:
    from . import carla_server_pb2 as carla_protocol
except ImportError:
    raise RuntimeError('cannot import "carla_server_pb2.py", run '
                       'the protobuf compiler to generate this file')


Translation = namedtuple('Translation', 'x y z')
Translation.__new__.__defaults__ = (0.0, 0.0, 0.0)

Rotation = namedtuple('Rotation', 'pitch yaw roll')
Rotation.__new__.__defaults__ = (0.0, 0.0, 0.0)

Scale = namedtuple('Scale', 'x y z')
Scale.__new__.__defaults__ = (1.0, 1.0, 1.0)


class Transform(object):
    """A 3D transformation.

    The transformation is applied in the order: scale, rotation, translation.
    """

    def __init__(self, *args, **kwargs):
        if 'matrix' in kwargs:
            self.matrix = kwargs['matrix']
            return
        if isinstance(args[0], carla_protocol.Transform):
            args = [
                Translation(
                    args[0].location.x,
                    args[0].location.y,
                    args[0].location.z),
                Rotation(
                    args[0].rotation.pitch,
                    args[0].rotation.yaw,
                    args[0].rotation.roll)
            ]
        self.matrix = numpy.matrix(numpy.identity(4))
        self.set(*args, **kwargs)

    def set(self, *args):
        """Builds the transform matrix given a Translate, Rotation
        and Scale.
        """
        translation = Translation()
        rotation = Rotation()
        scale = Scale()

        if len(args) > 3:
            raise ValueError("'Transform' accepts 3 values as maximum.")

        def get_single_obj_type(obj_type):
            """Returns the unique object contained in the
            arguments lists that is instance of 'obj_type'.
            """
            obj = [x for x in args if isinstance(x, obj_type)]
            if len(obj) > 1:
                raise ValueError("Transform only accepts one instances of " +
                                 str(obj_type) + " as a parameter")
            elif not obj:
                # Create an instance of the type that is 'obj_type'
                return obj_type()
            return obj[0]

        translation = get_single_obj_type(Translation)
        rotation = get_single_obj_type(Rotation)
        scale = get_single_obj_type(Scale)

        for param in args:
            if not isinstance(param, Translation) and \
               not isinstance(param, Rotation) and \
               not isinstance(param, Scale):
                raise TypeError(
                    "'" + str(type(param)) + "' type not match with \
                    'Translation', 'Rotation' or 'Scale'")

        # Transformation matrix
        cy = math.cos(numpy.radians(rotation.yaw))
        sy = math.sin(numpy.radians(rotation.yaw))
        cr = math.cos(numpy.radians(rotation.roll))
        sr = math.sin(numpy.radians(rotation.roll))
        cp = math.cos(numpy.radians(rotation.pitch))
        sp = math.sin(numpy.radians(rotation.pitch))
        self.matrix[0, 3] = translation.x
        self.matrix[1, 3] = translation.y
        self.matrix[2, 3] = translation.z
        self.matrix[0, 0] = scale.x * (cp * cy)
        self.matrix[0, 1] = scale.y * (cy * sp * sr - sy * cr)
        self.matrix[0, 2] = -scale.z * (cy * sp * cr + sy * sr)
        self.matrix[1, 0] = scale.x * (sy * cp)
        self.matrix[1, 1] = scale.y * (sy * sp * sr + cy * cr)
        self.matrix[1, 2] = scale.z * (cy * sr - sy * sp * cr)
        self.matrix[2, 0] = scale.x * (sp)
        self.matrix[2, 1] = -scale.y * (cp * sr)
        self.matrix[2, 2] = scale.z * (cp * cr)

    def inverse(self):
        """Return the inverse transform."""
        return Transform(matrix=numpy.linalg.inv(self.matrix))

    def transform_points(self, points):
        """
        Given a 4x4 transformation matrix, transform an array of 3D points.
        Expected point foramt: [[X0,Y0,Z0],..[Xn,Yn,Zn]]
        """
        # Needed foramt: [[X0,..Xn],[Z0,..Zn],[Z0,..Zn]]. So let's transpose
        # the point matrix.
        points = points.transpose()
        # Add 0s row: [[X0..,Xn],[Y0..,Yn],[Z0..,Zn],[0,..0]]
        points = numpy.append(points, numpy.ones((1, points.shape[1])), axis=0)
        # Point transformation
        points = self.matrix * points
        # Return all but last row
        return points[0:3].transpose()

    def __mul__(self, other):
        return Transform(matrix=numpy.dot(self.matrix, other.matrix))

    def __str__(self):
        return str(self.matrix)
