# -*- coding: utf-8 -*-
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

# @file    exceptions.py
# @author  Michael Behrisch
# @author  Lena Kalleske
# @author  Mario Krumnow
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @date    2008-10-09

from __future__ import print_function
from __future__ import absolute_import


class TraCIException(Exception):

    """Exception class for all TraCI errors which keep the connection intact"""

    def __init__(self, desc, command=None, errorType=None):
        Exception.__init__(self, desc)
        self._command = command
        self._type = errorType

    def getCommand(self):
        return self._command

    def getType(self):
        return self._type


class FatalTraCIError(Exception):

    """Exception class for all TraCI errors which do not allow for continuation"""

    def __init__(self, desc):
        Exception.__init__(self, desc)
