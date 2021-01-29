#!/usr/bin/env python2

# ^-- contents of out/config.py go here

#
# Copyright (C) 2013-2019 Christoph Sommer <sommer@ccs-labs.org>
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

"""
Wrapper around veins_catch binary
"""

import os
import argparse
import subprocess

parser = argparse.ArgumentParser('Run a Veins simulation')
parser.add_argument('-d', '--debug', action='store_true', help='Run using opp_run_dbg (instead of opp_run)')
parser.add_argument('-t', '--tool', metavar='TOOL', dest='tool', choices=['lldb', 'gdb', 'memcheck'], help='Wrap opp_run execution in TOOL (lldb, gdb or memcheck)')
parser.add_argument('-v', '--verbose', action='store_true', help='Print command line before executing')
parser.add_argument('--', dest='arguments', help='Arguments to pass to opp_run')
args, bin_args = parser.parse_known_args()
if (len(bin_args) > 0) and bin_args[0] == '--':
    bin_args = bin_args[1:]

bin_run = 'veins_catch'
if args.debug:
    bin_run = 'veins_catch_dbg'
prefix = []
if args.tool == 'lldb':
    prefix = ['lldb', '--']
if args.tool == 'gdb':
    prefix = ['gdb', '--args']
if args.tool == 'memcheck':
    prefix = ['valgrind', '--tool=memcheck', '--leak-check=full', '--dsymutil=yes', '--log-file=valgrind.out']

cmdline = prefix + [os.path.join('src', bin_run)] + bin_args

if args.verbose:
    print "Running with command line arguments: %s" % ' '.join(['"%s"' % arg for arg in cmdline])

os.environ["PATH"] += os.pathsep + os.pathsep.join(run_lib_paths)

if os.name == 'nt':
    subprocess.call(['env'] + cmdline)
else:
    os.execvp('env', ['env'] + cmdline)

