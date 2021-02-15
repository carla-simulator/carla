#!/bin/bash

#
# Copyright (C) 2019 Julian Heinovski <heinovski@ccs-labs.org>
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

# Generates the version number used for the doxygen documentation
# by using git describe to analyze the current HEAD of the repository.
#
# Example: veins-5a1-16-gd81b4a14a0
# The current HEAD points at the commit d81b4a14a0, which is 16 commits
# newer than the tag veins-5a1 it is based on.
#
# In case git describe fails, the version is empty.

TAG=$(git describe --tags --always 2> /dev/null)
if [[ ($? -eq 0) && ($TAG =~ ^veins-) ]]; then
    echo $TAG | sed -n 's/^veins-\(.*\)$/\1/p'
else
    echo ""
fi
