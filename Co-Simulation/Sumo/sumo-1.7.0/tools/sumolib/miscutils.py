# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    miscutils.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-05-08

from __future__ import absolute_import
from __future__ import print_function
from __future__ import division
import sys
import time
import os
import math
import colorsys
import socket
import random
# needed for backward compatibility
from .statistics import Statistics, geh, uMax, uMin, round  # noqa


def benchmark(func):
    """
    decorator for timing a function
    """
    def benchmark_wrapper(*args, **kwargs):
        started = time.time()
        now = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.localtime())
        print('function %s called at %s' % (func.__name__, now))
        sys.stdout.flush()
        result = func(*args, **kwargs)
        print('function %s finished after %f seconds' %
              (func.__name__, time.time() - started))
        sys.stdout.flush()
        return result
    return benchmark_wrapper


class working_dir:
    """
    temporarily change working directory using 'with' statement
    """

    def __init__(self, dir):
        self.dir = dir
        self.origdir = os.getcwd()

    def __enter__(self):
        os.chdir(self.dir)

    def __exit__(self, type, value, traceback):
        os.chdir(self.origdir)


class Colorgen:
    DISTINCT = [
        (0.17, 1.0, 0.5),
        (0.0, 0.9, 1.0),
        (0.35, 0.67, 0.71),
        (0.14, 0.9, 1.0),
        (0.56, 1.0, 0.78),
        (0.07, 0.8, 0.96),
        (0.79, 0.83, 0.71),
        (0.5, 0.71, 0.94),
        (0.84, 0.79, 0.94),
        (0.2, 0.76, 0.96),
        (0.0, 0.24, 0.98),
        (0.5, 1.0, 0.5),
        (0.77, 0.25, 1.0),
        (0.09, 0.76, 0.67),
        (0.15, 0.22, 1.0),
        (0.0, 1.0, 0.5),
        (0.38, 0.33, 1.0),
        (0.67, 1.0, 0.5),
    ]

    def __init__(self, hsv, cycleLength=10.67):
        self.hsv = hsv
        self.cycle = [int(random.random() * 256) for x in self.hsv]
        self.cycleOffset = int(round(256 / cycleLength))
        self.distinctIndex = 0

    def get_value(self, opt, index):
        if opt == 'random':
            return random.random()
        elif opt == 'cycle':
            # the 255 below is intentional to get all color values when cycling long enough
            self.cycle[index] = (self.cycle[index] + self.cycleOffset) % 255
            return self.cycle[index] / 255.0
        elif opt == 'cycle':
            # the 255 below is intentional to get all color values when cycling long enough
            self.cycle[index] = (self.cycle[index] + self.cycleOffset) % 255
            return self.cycle[index] / 255.0
        elif opt == 'distinct':
            if index == 0:
                self.distinctIndex = (self.distinctIndex + 1) % len(self.DISTINCT)
            return self.DISTINCT[self.distinctIndex][index]
        else:
            return float(opt)

    def floatTuple(self):
        """return color as a tuple of floats each in [0,1]"""
        return colorsys.hsv_to_rgb(*[self.get_value(o, i) for i, o in enumerate(self.hsv)])

    def byteTuple(self):
        """return color as a tuple of bytes each in [0,255]"""
        return tuple([int(round(255 * x)) for x in self.floatTuple()])

    def __call__(self):
        """return constant or randomized rgb-color string"""
        return ','.join(map(str, self.byteTuple()))


def getFreeSocketPort(numTries=10):
    for _ in range(numTries):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind(('', 0))
            p = s.getsockname()[1]
            s.close()
            return p
        except socket.error:
            pass
    return None


def getSocketStream(port, mode='rb'):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("localhost", port))
    s.listen(1)
    conn, _ = s.accept()
    return conn.makefile(mode)


# euclidean distance between two coordinates in the plane
def euclidean(a, b):
    return math.sqrt((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2)


def humanReadableTime(seconds):
    result = ""
    ds = 3600 * 24
    if seconds > ds:
        result = "%s:" % int(seconds / ds)
        seconds = seconds % ds
    result += "%02i:" % int(seconds / 3600)
    seconds = seconds % 3600
    result += "%02i:" % int(seconds / 60)
    seconds = seconds % 60
    if seconds == int(seconds):
        seconds = int(seconds)
    result += "%02i" % seconds
    return result


def parseTime(t, factor=1):
    try:
        return float(t) * factor
    except ValueError:
        pass
    # prepended zero is ignored if the date value already contains days
    days, hours, minutes, seconds = ([0] + list(map(float, t.split(':'))))[-4:]
    return 3600 * 24 * days + 3600 * hours + 60 * minutes + seconds
