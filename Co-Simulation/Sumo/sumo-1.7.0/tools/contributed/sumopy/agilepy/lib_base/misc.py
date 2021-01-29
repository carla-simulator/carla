# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    misc.py
# @author  Joerg Schweizer
# @date


import types
import numpy as np
import time

# default file path priming
# this did depend on operating system, now " for all
P = '"'
#import platform
# if platform.system()=='Windows':
#    P = '"'
# else:
#    P=''


def get_inversemap(m):
    return {v: k for k, v in m.items()}


def random_choice_dist2(n, b):
    """
    Returns the absolute distribution of a random choice sample of size n
    having the choice between len(b) options where each option has
    the probability represented in vector b.
    """


def random_choice_dist(n, b):
    """
    Returns the absolute distribution of a random choice sample of size n
    having the choice between len(b) options where each option has
    the probability represented in vector b.
    """
    if np.__version__ >= '1.7.0':
        return np.bincount(np.random.choice(b.size, n, p=b.flat),
                           minlength=b.size).reshape(b.shape)
    else:
        return np.bincount(np.searchsorted(np.cumsum(b), np.random.random(n)), minlength=b.size).reshape(b.shape)


def random_choice(n, b):
    """
    Returns the  random choice sample of size n
    having the choice between len(b) options where each option has
    the probability represented in vector b.
    """
    # print 'misc.random_choice'
    if np.__version__ >= '1.7.0':
        # print '  b',b.size,b,b.flat
        # print '  n',n
        return np.clip(np.random.choice(b.size, n, p=b.flat), 0, len(b)-1)
    else:
        return np.clip(np.searchsorted(np.cumsum(b), np.random.random(n)), 0, len(b)-1)

# def random_choice1d(n, b):
#    """
#    Returns the  random choice sample of size n
#    having the choice between len(b) options where each option has
#    the probability represented in vector b.
#    """
#
#    return np.argmax(np.random.rand(n)*b.flat)


def get_seconds_from_timestr(t_data, t_offset=None,
                             sep_date_clock=' ', sep_date='-', sep_clock=':',
                             is_float=True):
    """
    Returns time in seconds after t_offset.
    If no offset is geven, the year 1970 is used.
    Time string format:
        2012-05-02 12:57:08.0
    """
    if t_offset is None:
        t_offset = time.mktime((1970, 1, 1, 0, 0, 0, 0, 0, 0))  # year 2000

    if len(t_data.split(sep_date_clock)) != 2:
        return None
    (date, clock) = t_data.split(sep_date_clock)

    if (len(clock.split(sep_clock)) == 3) & (len(date.split(sep_date)) == 3):
        (day_str, month_str, year_str) = date.split(sep_date)
        (hours_str, minutes_str, seconds_str) = clock.split(sep_clock)
        t = time.mktime((int(year_str), int(month_str), int(day_str),
                         int(hours_str), int(minutes_str), int(float(seconds_str)), 0, 0, 0))-t_offset
        if is_float:
            return float(t)
        else:
            return int(t)
    else:
        return None


def format_filepath(filepath):
    return ff(filepath)


def ff(filepath):
    return P+filepath+P


def filepathlist_to_filepathstring(filepathlist, sep=','):
    if type(filepathlist) == types.ListType:
        if len(filepathlist) == 0:
            return ''
        else:
            filepathstring = ''
            for filepath in filepathlist[:-1]:
                fp = filepath.replace('"', '')
                filepathstring += P+fp+P+sep
            filepathstring += P+filepathlist[-1]+P
            return filepathstring
    else:
        fp = filepathlist.replace('"', '')
        return P+filepathlist+P


def filepathstring_to_filepathlist(filepathstring, sep=','):
    filepaths = []
    for filepath in filepathstring.split(sep):
        filepaths.append(P+filepath.strip().replace('"', '')+P)
    return filepaths
