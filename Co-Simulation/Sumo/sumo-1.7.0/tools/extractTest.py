#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    extractTest.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2009-07-08

"""
Extract all files for a test case into a new dir.
It may copy more files than needed because it copies everything
that is mentioned in the config under copy_test_path.
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from os.path import join
import optparse
import glob
import shutil
import shlex
import subprocess
from collections import defaultdict

THIS_DIR = os.path.abspath(os.path.dirname(__file__))
SUMO_HOME = os.path.dirname(THIS_DIR)
sys.path.append(join(SUMO_HOME, "tools"))

from sumolib import checkBinary  # noqa

# cannot use ':' because it is a component of absolute paths on windows
SOURCE_DEST_SEP = ';'


def get_options(args=None):
    optParser = optparse.OptionParser(usage="%prog <options> <test directory>")
    optParser.add_option("-o", "--output", default=".", help="send output to directory")
    optParser.add_option("-f", "--file", help="read list of source and target dirs from")
    optParser.add_option("-p", "--python-script",
                         help="name of a python script to generate for a batch run")
    optParser.add_option("-i", "--intelligent-names", dest="names", action="store_true",
                         default=False, help="generate cfg name from directory name")
    optParser.add_option("-v", "--verbose", action="store_true", default=False, help="more information")
    optParser.add_option("-a", "--application", help="sets the application to be used")
    optParser.add_option("-s", "--skip-configuration", default=False, action="store_true",
                         help="skips creation of an application config from the options.app file")
    optParser.add_option("-x", "--skip-validation", default=False, action="store_true",
                         help="remove all options related to XML validation")
    optParser.add_option("-d", "--no-subdir", dest="noSubdir", action="store_true",
                         default=False, help="store test files directly in the output directory")
    options, args = optParser.parse_args(args=args)
    if not options.file and len(args) == 0:
        optParser.print_help()
        sys.exit(1)
    options.args = args
    return options


def copy_merge(srcDir, dstDir, merge, exclude):
    """merge contents of srcDir recursively into dstDir"""
    for dir, subdirs, files in os.walk(srcDir):
        for ex in exclude:
            if ex in subdirs:
                subdirs.remove(ex)
        dst = dir.replace(srcDir, dstDir)
        if os.path.exists(dst) and not merge:
            shutil.rmtree(dst)
        if not os.path.exists(dst):
            # print "creating dir '%s' as a copy of '%s'" % (dst, srcDir)
            os.mkdir(dst)
        for file in files:
            # print("copying file '%s' to '%s'" % (join(dir, file), join(dst, file)))
            shutil.copy(join(dir, file), join(dst, file))


def generateTargetName(baseDir, source):
    return source[len(os.path.commonprefix([baseDir, source])):].replace(os.sep, '_')


def main(options):
    targets = []
    if options.file:
        dirname = os.path.dirname(options.file)
        for line in open(options.file):
            line = line.strip()
            if line and line[0] != '#':
                ls = line.split(SOURCE_DEST_SEP) + [""]
                ls[0] = join(dirname, ls[0])
                ls[1] = join(dirname, ls[1])
                targets.append(ls[:3])
    for val in options.args:
        source_and_maybe_target = val.split(SOURCE_DEST_SEP) + ["", ""]
        targets.append(source_and_maybe_target[:3])

    if options.python_script:
        if not os.path.exists(os.path.dirname(options.python_script)):
            os.makedirs(os.path.dirname(options.python_script))
        pyBatch = open(options.python_script, 'w')
        pyBatch.write('''import subprocess, sys, os
from os.path import abspath, dirname, join
THIS_DIR = abspath(dirname(__file__))
SUMO_HOME = os.environ.get("SUMO_HOME", dirname(dirname(THIS_DIR)))
os.environ["SUMO_HOME"] = SUMO_HOME
for p in [
''')
    for source, target, app in targets:
        outputFiles = glob.glob(join(source, "output.[0-9a-z]*"))
        # print source, target, outputFiles
        # XXX we should collect the options.app.variant files in all parent
        # directories instead. This would allow us to save config files for all
        # variants
        appName = set([f.split('.')[-1] for f in outputFiles])
        if len(appName) != 1:
            if options.application in appName:
                appName = set([options.application])
            elif app in appName:
                appName = set([app])
            else:
                print(("Skipping %s because the application was not unique (found %s).") % (
                    source, appName), file=sys.stderr)
                continue
        app = next(iter(appName))
        optionsFiles = []
        configFiles = []
        potentials = defaultdict(list)
        source = os.path.realpath(source)
        curDir = source
        if curDir[-1] == os.path.sep:
            curDir = os.path.dirname(curDir)
        while True:
            for f in os.listdir(curDir):
                path = join(curDir, f)
                if f not in potentials or os.path.isdir(path):
                    potentials[f].append(path)
                if f == "options." + app:
                    optionsFiles.append(path)
            config = join(curDir, "config." + app)
            if curDir == join(SUMO_HOME, "tests") or curDir == os.path.dirname(curDir):
                break
            if os.path.exists(config):
                configFiles.append(config)
            curDir = os.path.dirname(curDir)
        if not configFiles:
            print("Config not found for %s." % source, file=sys.stderr)
            continue
        if target == "" and not options.noSubdir:
            target = generateTargetName(
                os.path.dirname(configFiles[-1]), source)
        testPath = os.path.abspath(join(options.output, target))
        if not os.path.exists(testPath):
            os.makedirs(testPath)
        net = None
        skip = False
        appOptions = []
        for f in reversed(optionsFiles):
            for o in shlex.split(open(f).read()):
                if skip:
                    skip = False
                    continue
                if o == "--xml-validation" and options.skip_validation:
                    skip = True
                    continue
                if o == "{CLEAR}":
                    appOptions = []
                    continue
                appOptions.append(o)
                if "=" in o:
                    o = o.split("=")[-1]
                if o[-8:] == ".net.xml":
                    net = o
        nameBase = "test"
        if options.names:
            nameBase = os.path.basename(target)
        exclude = []
        # gather copy_test_path exclusions
        for config in configFiles:
            for line in open(config):
                entry = line.strip().split(':')
                if entry and entry[0] == "test_data_ignore":
                    exclude.append(entry[1])
        # copy test data from the tree
        for config in configFiles:
            for line in open(config):
                entry = line.strip().split(':')
                if entry and "copy_test_path" in entry[0] and entry[1] in potentials:
                    if "net" in app or not net or entry[1][-8:] != ".net.xml" or entry[1] == net:
                        toCopy = potentials[entry[1]][0]
                        if os.path.isdir(toCopy):
                            # copy from least specific to most specific
                            merge = entry[0] == "copy_test_path_merge"
                            for toCopy in reversed(potentials[entry[1]]):
                                copy_merge(
                                    toCopy, join(testPath, os.path.basename(toCopy)), merge, exclude)
                        else:
                            shutil.copy2(toCopy, testPath)
        if options.python_script:
            if app == "netgen":
                call = ['join(SUMO_HOME, "bin", "netgenerate")'] + ['"%s"' % a for a in appOptions]
            elif app == "tools":
                call = ['"python"'] + ['"%s"' % a for a in appOptions]
                call[1] = 'join(SUMO_HOME, "%s")' % appOptions[0]
            elif app == "complex":
                call = ['"python"']
                for o in appOptions:
                    if o.endswith(".py"):
                        call.insert(1, '"./%s"' % os.path.basename(o))
                    else:
                        call.append('"%s"' % o)
            else:
                call = ['join(SUMO_HOME, "bin", "%s")' % app] + ['"%s"' % a for a in appOptions]
            prefix = os.path.commonprefix((testPath, os.path.abspath(pyBatch.name)))
            up = os.path.abspath(pyBatch.name)[len(prefix):].count(os.sep) * "../"
            pyBatch.write('    subprocess.Popen([%s], cwd=join(THIS_DIR, r"%s%s")),\n' %
                          (', '.join(call), up, testPath[len(prefix):]))
        if options.skip_configuration:
            continue
        oldWorkDir = os.getcwd()
        os.chdir(testPath)
        haveConfig = False
        if app in ["dfrouter", "duarouter", "jtrrouter", "marouter", "netconvert",
                   "netgen", "netgenerate", "od2trips", "polyconvert", "sumo", "activitygen"]:
            if app == "netgen":
                # binary is now called differently but app still has the old name
                app = "netgenerate"
            if options.verbose:
                print("calling %s for testPath '%s' with options '%s'" %
                      (checkBinary(app), testPath, " ".join(appOptions)))
            try:
                haveConfig = subprocess.call([checkBinary(app)] + appOptions +
                                             ['--save-configuration', '%s.%scfg' %
                                              (nameBase, app[:4])]) == 0
            except OSError:
                print("Executable %s not found, generating shell scripts instead of config." % app, file=sys.stderr)
            if not haveConfig:
                appOptions.insert(0, "$SUMO_HOME/bin/" + app)
        elif app == "tools":
            for i, a in enumerate(appOptions):
                if a.endswith(".py"):
                    del appOptions[i:i+1]
                    appOptions[0:0] = [os.environ.get("PYTHON", "python"), "$SUMO_HOME/" + a]
                    break
                if a.endswith(".jar"):
                    del appOptions[i:i+1]
                    appOptions[0:0] = ["java", "-jar", "$SUMO_HOME/" + a]
                    break
        elif app == "complex":
            for i, a in enumerate(appOptions):
                if a.endswith(".py"):
                    if os.path.exists(join(testPath, os.path.basename(a))):
                        a = os.path.basename(a)
                    del appOptions[i:i+1]
                    appOptions[0:0] = [os.environ.get("PYTHON", "python"), a]
                    break
        if not haveConfig:
            if options.verbose:
                print("generating shell scripts for testPath '%s' with call '%s'" %
                      (testPath, " ".join(appOptions)))
            cmd = [o if " " not in o else "'%s'" % o for o in appOptions]
            open(nameBase + ".sh", "w").write(" ".join(cmd))
            cmd = [o.replace("$SUMO_HOME", "%SUMO_HOME%") if " " not in o else '"%s"' % o for o in appOptions]
            open(nameBase + ".bat", "w").write(" ".join(cmd))
        os.chdir(oldWorkDir)
    if options.python_script:
        pyBatch.write(']:\n    if p.wait() != 0:\n        sys.exit(1)\n')


if __name__ == "__main__":
    main(get_options())
