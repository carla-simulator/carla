#!/usr/bin/env python
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

# @file    setup.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2010-05-23

from __future__ import absolute_import

from distutils.core import setup
# import is needed to make the module known
import py2exe  # noqa
import sys
import shutil
import os
import glob
import zipfile
import subprocess
import tempfile

for d in ["Program Files", "Program Files (x86)", "Programme"]:
    sevenZip = r'C:\%s\7-Zip\7z.exe' % d
    if os.path.exists(sevenZip):
        break

inZip = os.path.abspath(sys.argv[1])
sys.argv[1] = "py2exe"

base = os.path.abspath(os.path.dirname(__file__))
oldDir = os.getcwd()
os.chdir(base)
# add sumo configs and input files
files = [d for d in os.listdir(".") if os.path.isdir(d)]
for pattern in ['*.sumocfg', 'input_additional.add.xml', '*.gif']:
    files += glob.glob(pattern)
zipfName = inZip.replace("sumo-", "sumo-game-")
subprocess.call(['git', 'archive', 'HEAD', '-o', zipfName] + files)
# run py2exe
tmpDir = tempfile.mkdtemp()
os.chdir(tmpDir)
os.mkdir("dist")
setup(console=[os.path.join(base, 'runner.py')])
# collect sumo binaries and dlls
osgPlugins = None
with zipfile.ZipFile(inZip) as binZip:
    for f in binZip.namelist():
        if "osgPlugins" in f:
            extracted = binZip.extract(f)
            if osgPlugins is None:
                if f.endswith("/"):
                    osgPlugins = extracted
                else:
                    osgPlugins = os.path.dirname(extracted)
        elif f.endswith(".dll") or f.endswith("gui.exe") or f.endswith("sumo.exe"):
            extracted = binZip.extract(f)
            dest = os.path.join("dist", os.path.basename(f))
            if os.path.isfile(extracted) and not os.path.exists(dest):
                os.rename(extracted, dest)
os.chdir("dist")
if osgPlugins:
    os.rename(osgPlugins, os.path.basename(osgPlugins))
    for f in glob.glob(os.path.join(base, '..', '..', 'data', '3D', '*')):
        shutil.copy2(f, ".")
    os.mkdir("bs3d")
    os.chdir("bs3d")
    subprocess.call([sevenZip, 'x', os.path.join(
        os.path.dirname(inZip), '..', '3D_Modell_Forschungskreuzung_BS.7z')])
    os.chdir("..")
# package the zip
zipf = zipfile.ZipFile(zipfName, 'a', zipfile.ZIP_DEFLATED)
root_len = len(os.path.abspath("."))
for root, dirs, files in os.walk("."):
    archive_root = os.path.abspath(root)[root_len:]
    for f in files:
        fullpath = os.path.join(root, f)
        archive_name = os.path.join(archive_root, f)
        zipf.write(fullpath, archive_name)
zipf.close()
os.chdir(oldDir)
shutil.rmtree(tmpDir, True)
