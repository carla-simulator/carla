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

# @file    _meandata.py
# @author  Melanie Weber
# @date    2020-08-18

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain


class MeanDataDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "meandata", tc.CMD_GET_MEANDATA_VARIABLE, None,
                        tc.CMD_SUBSCRIBE_MEANDATA_VARIABLE, tc.RESPONSE_SUBSCRIBE_MEANDATA_VARIABLE,
                        tc.CMD_SUBSCRIBE_MEANDATA_CONTEXT, tc.RESPONSE_SUBSCRIBE_MEANDATA_CONTEXT)
