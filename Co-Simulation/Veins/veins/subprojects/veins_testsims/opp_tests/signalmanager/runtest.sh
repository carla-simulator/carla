#!/bin/bash
#
# Copyright (C) 2018-2019 Dominik S. Buse <buse@ccs-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

set -e

TESTS="*.test"
VEINS_PATH="../../../../../src/"
EXTRA_INCLUDES="-I$VEINS_PATH -L$VEINS_PATH"

# ensure the working dir is ready
mkdir -p work

# generate test files
opp_test gen -v $TESTS

# build test files
(cd work; opp_makemake -f --deep -o work $EXTRA_INCLUDES ; make -j4 MODE=debug)

# run tests
opp_test run -v -p work_dbg $TESTS
