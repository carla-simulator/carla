# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    rmsd.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-03-31

from __future__ import absolute_import
from __future__ import print_function

from numpy import array, dot, shape, transpose
from numpy.linalg import det, svd


def superpose(nodes1, nodes2, select1, select2):
    vecs1 = array(nodes1)[array(select1)]
    vecs2 = array(nodes2)[array(select2)]
    print(vecs1, vecs2)

    n_vec, vec_size = shape(vecs1)
    center1 = sum(vecs1, 0) / float(n_vec)
    center2 = sum(vecs2, 0) / float(n_vec)
    vecs1 -= center1
    vecs2 -= center2

    V, S, W_trans = svd(dot(transpose(vecs2), vecs1))

    is_reflection = (det(V) * det(W_trans)) < 0.0
    if is_reflection:
        V[-1, :] = V[-1, :] * (-1.0)

    optimal_rotation = dot(V, W_trans)
    return dot(array(nodes2) - center2, optimal_rotation) + center1

# a = [ (1,1), (4,4), (1,4) ]
# b = [ (0,3), (3,0), (3,3), (5,5) ]

# print superpose(a, b, (0,1,2), (0,1,2))
