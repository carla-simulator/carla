#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    traciToHex.py
# @author  Michael Behrisch
# @date    2010-09-08


from __future__ import print_function
from __future__ import absolute_import
import os
import sys

mRoot = "."
if len(sys.argv) > 1:
    mRoot = sys.argv[1]
for root, dirs, files in os.walk(mRoot):
    if ".svn" in dirs:
        dirs.remove(".svn")
    for file in files:
        if file == "testclient.prog":
            full = os.path.join(root, file)
            out = open(full + ".hex", 'w')
            change = False
            for line in open(full):
                ls = line.split()
                if ls and ls[0] in ["setvalue", "getvalue", "getvariable", "getvariable_plus"]:
                    if not ls[1][:2] == "0x":
                        ls[1] = "0x%x" % int(ls[1])
                        change = True
                    if not ls[2][:2] == "0x":
                        ls[2] = "0x%x" % int(ls[2])
                        change = True
                print(" ".join(ls), file=out)
            out.close()
            if change:
                if os.name != "posix":
                    os.remove(full)
                os.rename(out.name, full)
            else:
                os.remove(out.name)
