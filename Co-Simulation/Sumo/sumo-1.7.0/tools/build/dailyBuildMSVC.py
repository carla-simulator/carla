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

# @file    dailyBuildMSVC.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @author  Laura Bieker
# @date    2008

"""
Does the nightly git pull on the windows server and the visual
studio build. The script is also used for the meso build.
Some paths especially for the names of the texttest output dirs are
hard coded into this script.
"""
from __future__ import absolute_import
from __future__ import print_function
import io
import datetime
import optparse
import os
import glob
import subprocess
import zipfile
import shutil
import sys

import status
import wix

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib  # noqa

BINARIES = ("activitygen", "emissionsDrivingCycle", "emissionsMap",
            "dfrouter", "duarouter", "jtrrouter", "marouter",
            "netconvert", "netedit", "netgenerate",
            "od2trips", "polyconvert", "sumo", "sumo-gui",
            "TraCITestClient")


def repositoryUpdate(options, log):
    gitrev = ""
    cwd = os.getcwd()
    for d in options.repositories.split(","):
        os.chdir(os.path.join(options.rootDir, d))
        subprocess.call(["git", "pull"], stdout=log, stderr=subprocess.STDOUT)
        subprocess.call(["git", "submodule", "update"], stdout=log, stderr=subprocess.STDOUT)
        if gitrev == "":
            gitrev = sumolib.version.gitDescribe()
    os.chdir(cwd)
    return gitrev


def killall(debugSuffix):
    bins = set([name + debugSuffix + ".exe" for name in BINARIES])
    for taskline in subprocess.check_output(["tasklist", "/nh"]).splitlines():
        task = taskline.split()
        if task and task[0] in bins:
            subprocess.call(["taskkill", "/f", "/im", task[0]])
            bins.remove(task[0])


def runTests(options, env, gitrev, log, debugSuffix=""):
    if not options.tests:
        return
    prefix = env["FILEPREFIX"] + debugSuffix
    env["SUMO_BATCH_RESULT"] = os.path.join(
        options.rootDir, prefix + "batch_result")
    env["SUMO_REPORT"] = os.path.join(options.remoteDir, prefix + "report")
    env["TEXTTEST_TMP"] = os.path.join(
        options.rootDir, prefix + "texttesttmp")
    env["TEXTTEST_HOME"] = os.path.join(options.rootDir, options.testsDir)
    shutil.rmtree(env["TEXTTEST_TMP"], True)
    if not os.path.exists(env["SUMO_REPORT"]):
        os.makedirs(env["SUMO_REPORT"])
    killall(debugSuffix)
    for name in BINARIES:
        binary = os.path.join(options.rootDir, options.binDir, name + debugSuffix + ".exe")
        if name == "sumo-gui":
            if os.path.exists(binary):
                env["GUISIM_BINARY"] = binary
        elif os.path.exists(binary):
            env[name.upper() + "_BINARY"] = binary
    # provide more information than just the date:
    fullOpt = ["-b", prefix, "-name", "%sr%s" %
               (datetime.date.today().strftime("%d%b%y"), gitrev)]
    ttBin = "texttestc.py"
    if options.suffix == "extra":
        runExtraTests.run(debugSuffix, fullOpt, log, True, True, debugSuffix == "")
    else:
        subprocess.call([ttBin] + fullOpt, env=env,
                        stdout=log, stderr=subprocess.STDOUT, shell=True)
        subprocess.call([ttBin, "-a", "sumo.gui"] + fullOpt, env=env,
                        stdout=log, stderr=subprocess.STDOUT, shell=True)
    subprocess.call([ttBin, "-b", env["FILEPREFIX"], "-coll"], env=env,
                    stdout=log, stderr=subprocess.STDOUT, shell=True)
    killall(debugSuffix)


def generateCMake(generator, log, checkOptionalLibs, python):
    buildDir = os.path.join(env["SUMO_HOME"], "build", "cmake-build-" + generator.replace(" ", "-"))
    cmakeOpt = ["-DCOMPILE_DEFINITIONS=MSVC_TEST_SERVER",
                "-DDEFAULT_LIBSUMO_PYTHON=False",
                "-DCHECK_OPTIONAL_LIBS=%s" % checkOptionalLibs]
    if python:
        cmakeOpt += ["-DPYTHON_EXECUTABLE=%s" % python]
    if checkOptionalLibs:
        cmakeOpt += ["-DSUMO_UTILS=True"]
    # Create directory or clear it if already exists
    if os.path.exists(buildDir):
        status.printLog("Cleaning directory of %s." % generator, log)
        shutil.rmtree(buildDir)
    os.makedirs(buildDir)
    status.printLog("Creating solution for %s." % generator, log)
    subprocess.call(["cmake", "../..", "-G", generator] + cmakeOpt, cwd=buildDir, stdout=log, stderr=subprocess.STDOUT)
    return buildDir


optParser = optparse.OptionParser()
optParser.add_option("-r", "--root-dir", dest="rootDir",
                     default=r"D:\Sumo", help="root for git and log output")
optParser.add_option("-s", "--suffix", default="", help="suffix to the fileprefix")
optParser.add_option("-b", "--bin-dir", dest="binDir", default=r"git\bin",
                     help="directory containg the binaries, relative to the root dir")
optParser.add_option("-t", "--tests-dir", dest="testsDir", default=r"git\tests",
                     help="directory containg the tests, relative to the root dir")
optParser.add_option("-m", "--remote-dir", dest="remoteDir", default="S:\\daily",
                     help="directory to move the results to")
optParser.add_option("-n", "--no-tests", dest="tests", action="store_false",
                     default=True, help="skip tests")
optParser.add_option("-x", "--x64only", action="store_true",
                     default=False, help="skip Win32 and debug build (as well as netedit tests)")
optParser.add_option("-p", "--python", help="path to python interpreter to use")
optParser.add_option("-u", "--repositories", default="git",
                     help="repositories to update")
(options, args) = optParser.parse_args()

sys.path.append(os.path.join(options.rootDir, options.testsDir))
import runExtraTests  # noqa

env = os.environ
if "SUMO_HOME" not in env:
    env["SUMO_HOME"] = os.path.dirname(
        os.path.dirname(os.path.dirname(__file__)))
env["PYTHON"] = "python"
env["SMTP_SERVER"] = "smtprelay.dlr.de"
msvcVersion = "msvc12"

maxTime = 0
sumoAllZip = None
for fname in glob.glob(os.path.join(options.remoteDir, "sumo-all-*.zip")):
    if os.path.getmtime(fname) > maxTime:
        maxTime = os.path.getmtime(fname)
        sumoAllZip = fname
for platform in (["x64"] if options.x64only else ["Win32", "x64"]):
    env["FILEPREFIX"] = msvcVersion + options.suffix + platform
    prefix = os.path.join(options.remoteDir, env["FILEPREFIX"])
    makeLog = prefix + "Release.log"
    makeAllLog = prefix + "Debug.log"
    statusLog = prefix + "status.log"
    binDir = "sumo-git/bin/"

    toClean = [makeLog, makeAllLog]
    toolsLibsumoDir = os.path.join(options.rootDir, options.binDir.replace("bin", "tools"), "libsumo")
    for ext in ("*.exe", "*.ilk", "*.pdb", "*.py", "*.pyd", "*.dll", "*.lib", "*.exp", "*.jar"):
        toClean += glob.glob(os.path.join(options.rootDir, options.binDir, ext))
    toClean += glob.glob(os.path.join(toolsLibsumoDir, "libsumo*"))
    for f in toClean:
        try:
            os.remove(f)
        except WindowsError:
            pass
    # we need to use io.open here due to http://bugs.python.org/issue16273
    with io.open(makeLog, 'a') as log:
        status.printLog("Running %s build using python %s." % (msvcVersion, sys.version), log)
        gitrev = repositoryUpdate(options, log)
        generator = "Visual Studio 12 2013"
        if platform == "x64":
            generator += " Win64"
        buildDir = generateCMake(generator, log, options.suffix == "extra", options.python)
        ret = subprocess.call(["cmake", "--build", ".", "--config", "Release"],
                              cwd=buildDir, stdout=log, stderr=subprocess.STDOUT)
        if os.path.exists(os.path.join("src", "libsumo", "_libsumo.vcxproj")):
            ret = subprocess.call(["cmake", "--build", ".", "--target", "_libsumo"],
                                  cwd=buildDir, stdout=log, stderr=subprocess.STDOUT)
        ret = subprocess.call(["cmake", "--build", ".", "--target", "lisum-gui"],
                              cwd=buildDir, stdout=log, stderr=subprocess.STDOUT)
        if ret == 0 and sumoAllZip:
            try:
                binaryZip = sumoAllZip.replace("-all-", "-%s%s-" %
                                               (platform.lower().replace("x", "win"), options.suffix))
                zipf = zipfile.ZipFile(binaryZip, 'w', zipfile.ZIP_DEFLATED)
                srcZip = zipfile.ZipFile(sumoAllZip)
                write = False
                for f in srcZip.namelist():
                    if f.count('/') == 1:
                        write = f.endswith(".md") or os.path.basename(f) in ["AUTHORS", "ChangeLog", "LICENSE"]
                    if f.endswith('/') and f.count('/') == 2:
                        write = f.endswith(('/bin/', '/tools/', '/data/', '/docs/'))
                        if f.endswith('/bin/'):
                            binDir = f
                    elif f.endswith('/') and '/docs/' in f and f.count('/') == 3:
                        write = not f.endswith('/doxygen/') and not f.endswith('/web/')
                    elif write and not f.endswith(".jar"):
                        zipf.writestr(f, srcZip.read(f))
                srcZip.close()
                for ext in ("*.exe", "*.dll", "*.lib", "*.exp", "*.jar"):
                    for f in sorted(glob.glob(os.path.join(options.rootDir, options.binDir, ext))):
                        base = os.path.basename(f)
                        nameInZip = os.path.join(binDir, base)
                        # filter debug dlls
                        if nameInZip[-5:] in ("d.dll", "D.dll") and nameInZip[:-5] + ".dll" in zipf.namelist():
                            write = False
                        elif ext == "*.exe":
                            write = any([base.startswith(b) for b in BINARIES])
                        else:
                            write = True
                        if write:
                            zipf.write(f, nameInZip)
                srcDir = os.path.join(options.rootDir, options.binDir.replace("bin", "src"))
                includeDir = binDir.replace("bin", "include")
                status.printLog("Creating sumo.zip.", log)
                for f in (glob.glob(os.path.join(srcDir, "libsumo", "*.h")) +
                          glob.glob(os.path.join(srcDir, "utils", "traci", "TraCIAPI.*")) +
                          glob.glob(os.path.join(srcDir, "foreign", "tcpip", "s*.*"))):
                    if os.path.basename(f) != "Helper.h":
                        zipf.write(f, includeDir + f[len(srcDir):])
                zipf.write(os.path.join(buildDir, "src", "version.h"), os.path.join(includeDir, "version.h"))
                for f in (glob.glob(os.path.join(toolsLibsumoDir, "*.py")) +
                          glob.glob(os.path.join(toolsLibsumoDir, "*.pyd"))):
                    # no os.path.join here, since the namelist uses only "/"
                    nameInZip = binDir.replace("bin", "tools") + "libsumo/" + os.path.basename(f)
                    if nameInZip not in zipf.namelist():
                        zipf.write(f, nameInZip)
                zipf.close()
                if options.suffix == "":
                    # installers only for the vanilla build
                    status.printLog("Creating sumo.msi.", log)
                    wix.buildMSI(binaryZip, binaryZip.replace(".zip", ".msi"), log=log)
            except IOError as ziperr:
                status.printLog("Warning: Could not zip to %s (%s)!" % (binaryZip, ziperr), log)
        if platform == "x64":
            status.printLog("Creating sumo-game.zip.", log)
            try:
                setup = os.path.join(env["SUMO_HOME"], 'tools', 'game', 'setup.py')
                subprocess.call(['python', setup, binaryZip], stdout=log, stderr=subprocess.STDOUT)
            except Exception as e:
                status.printLog("Warning: Could not create nightly sumo-game.zip! (%s)" % e, log)
        with open(makeAllLog, 'a') as debugLog:
            ret = subprocess.call(["cmake", "--build", ".", "--config", "Debug"],
                                  cwd=buildDir, stdout=debugLog, stderr=subprocess.STDOUT)
            if ret == 0 and sumoAllZip:
                status.printLog("Creating sumoDebug.zip.", debugLog)
                try:
                    debugZip = sumoAllZip.replace("-all-", "-%s%sDebug-" %
                                                  (platform.lower().replace("x", "win"), options.suffix))
                    zipf = zipfile.ZipFile(debugZip, 'w', zipfile.ZIP_DEFLATED)
                    for ext in ("*D.exe", "*.dll", "*D.pdb"):
                        for f in glob.glob(os.path.join(options.rootDir, options.binDir, ext)):
                            zipf.write(f, os.path.join(binDir, os.path.basename(f)))
                    zipf.close()
                except IOError as ziperr:
                    status.printLog("Warning: Could not zip to %s (%s)!" % (binaryZip, ziperr), debugLog)
        status.printLog("Running tests.", log)
        runTests(options, env, gitrev, log)
    with open(statusLog, 'w') as log:
        status.printStatus(makeLog, makeAllLog, env["SMTP_SERVER"], log)
if not options.x64only:
    with open(makeAllLog, 'a') as debugLog:
        status.printLog("Running debug tests.", debugLog)
        runTests(options, env, gitrev, debugLog, "D")
    with open(prefix + "Dstatus.log", 'w') as log:
        status.printStatus(makeAllLog, makeAllLog, env["SMTP_SERVER"], log)
