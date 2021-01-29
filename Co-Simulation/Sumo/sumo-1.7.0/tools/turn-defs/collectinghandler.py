# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    collectinghandler.py
# @author  Karol Stosiek
# @date    2011-10-26

from __future__ import absolute_import

import logging


class CollectingHandler(logging.Handler):

    """ Handler for loggers from logging module. Collects all log messages. """

    def __init__(self, level=0):
        """ Constructor. The level parameter stands for logging level. """

        self.log_records = []
        logging.Handler.__init__(self, level)

    def handle(self, record):
        """ See logging.Handler.handle(self, record) docs. """

        self.log_records.append(record)

    def emit(self, record):
        """ See logging.Handler.emit(self, record) docs. """

        pass  # do not emit the record. Other handlers can do that.
