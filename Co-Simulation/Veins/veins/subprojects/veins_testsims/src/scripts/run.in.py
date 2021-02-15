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
Runs Veins simulation in current directory
"""

import os
import argparse
import subprocess

def relpath(s):
    veins_root = os.path.dirname(os.path.realpath(__file__))
    return os.path.relpath(os.path.join(veins_root, s), '.')

parser = argparse.ArgumentParser('Run a Veins simulation')
parser.add_argument('-d', '--debug', action='store_true', help='Run using opp_run_dbg (instead of opp_run)')
parser.add_argument('-t', '--tool', metavar='TOOL', dest='tool', choices=['lldb', 'gdb', 'memcheck'], help='Wrap opp_run execution in TOOL (lldb, gdb or memcheck)')
parser.add_argument('-v', '--verbose', action='store_true', help='Print command line before executing')
parser.add_argument('--', dest='arguments', help='Arguments to pass to opp_run')
args, omnet_args = parser.parse_known_args()
if (len(omnet_args) > 0) and omnet_args[0] == '--':
    omnet_args = omnet_args[1:]

run_libs = [relpath(s) for s in run_libs]
run_neds = [relpath(s) for s in run_neds] + ['.']
run_imgs = [relpath(s) for s in run_imgs]

opp_run = 'opp_run'
if args.debug:
    opp_run = 'opp_run_dbg'

lib_flags = ['-l%s' % s for s in run_libs]
ned_flags = ['-n' + ';'.join(run_neds)]
img_flags = ['--image-path=' + ';'.join(run_imgs)]

prefix = []
if args.tool == 'lldb':
    prefix = ['lldb', '--']
if args.tool == 'gdb':
    prefix = ['gdb', '--args']
if args.tool == 'memcheck':
    prefix = ['valgrind', '--tool=memcheck', '--leak-check=full', '--dsymutil=yes', '--log-file=valgrind.out']

cmdline = prefix + [opp_run] + lib_flags + ned_flags + img_flags + omnet_args

if args.verbose:
    print "Running with command line arguments: %s" % ' '.join(['"%s"' % arg for arg in cmdline])

if os.name == 'nt':
    subprocess.call(['env'] + cmdline)
else:
    os.execvp('env', ['env'] + cmdline)

