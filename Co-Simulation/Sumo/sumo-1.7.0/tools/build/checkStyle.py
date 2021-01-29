#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    checkStyle.py
# @author  Michael Behrisch
# @date    2010-08-29

"""
Checks svn property settings for all files and pep8 for python
as well as file headers.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import subprocess
import multiprocessing
import xml.sax
import codecs
from optparse import OptionParser
try:
    import flake8  # noqa
    HAVE_FLAKE = True
except ImportError:
    print("Python flake not found. Python style checking is disabled.")
    HAVE_FLAKE = False
try:
    import autopep8  # noqa
    HAVE_AUTOPEP = True
except ImportError:
    HAVE_AUTOPEP = False
try:
    with open(os.devnull, 'w') as devnull:
        subprocess.check_call(['astyle', '--version'], stdout=devnull)
    HAVE_ASTYLE = True
except (OSError, subprocess.CalledProcessError):
    HAVE_ASTYLE = False

_SOURCE_EXT = set([".h", ".cpp", ".py", ".pyw", ".pl", ".java", ".am", ".cs", ".c"])
_TESTDATA_EXT = [".xml", ".prog", ".csv",
                 ".complex", ".dfrouter", ".duarouter", ".jtrrouter", ".marouter",
                 ".astar", ".chrouter", ".internal", ".tcl", ".txt",
                 ".netconvert", ".netedit", ".netgen",
                 ".od2trips", ".polyconvert", ".sumo",
                 ".meso", ".tools", ".traci", ".activitygen",
                 ".scenario", ".tapasVEU",
                 ".sumocfg", ".netccfg", ".netgcfg"]
_VS_EXT = [".vsprops", ".sln", ".vcproj",
           ".bat", ".props", ".vcxproj", ".filters"]
_IGNORE = set(["binstate.sumo", "binstate.sumo.meso", "image.tools"])
_KEYWORDS = "HeadURL Id LastChangedBy LastChangedDate LastChangedRevision"

SEPARATOR = "/****************************************************************************/\n"
EPL_HEADER = """/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
"""
EPL_GPL_HEADER = """/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
"""


class PropertyReader(xml.sax.handler.ContentHandler):

    """Reads the svn properties of files as written by svn pl -v --xml"""

    def __init__(self, doFix, doPep, doLicense):
        self._fix = doFix
        self._pep = doPep
        self._license = doFix or doLicense
        self._file = ""
        self._property = None
        self._value = ""
        self._hadEOL = False
        self._hadKeywords = False
        self._haveFixed = False

    def checkDoxyLines(self, lines, idx, comment="///"):
        fileRef = "%s @file    %s\n" % (comment, os.path.basename(self._file))
        try:
            s = lines[idx].split()
            if s != fileRef.split():
                print(self._file, "broken @file reference", lines[idx].rstrip())
                if self._fix and lines[idx].startswith("%s @file" % comment):
                    lines[idx] = fileRef
                    self._haveFixed = True
            idx += 1
            s = lines[idx].split()
            if s[:2] != [comment, "@author"]:
                print(self._file, "broken @author reference", s)
            idx += 1
            while lines[idx].split()[:2] == [comment, "@author"]:
                idx += 1
            s = lines[idx].split()
            if s[:2] != [comment, "@date"]:
                print(self._file, "broken @date reference", s)
            idx += 1
            if lines[idx] not in (comment + "\n", "\n"):
                print(self._file, "missing empty line", idx, lines[idx].rstrip())
        except IndexError:
            print(self._file, "seems to be empty")
        idx += 1
        while idx < len(lines) and lines[idx].split()[:1] == ["//"]:
            idx += 1
        return idx

    def checkFileHeader(self, ext):
        lines = open(self._file).readlines()
        if len(lines) == 0:
            print(self._file, "is empty")
            return
        self._haveFixed = False
        idx = 0
        if ext in (".cpp", ".h", ".java", ".c"):
            if lines[idx] == SEPARATOR:
                year = lines[idx + 2][17:21]
                end = lines.index(SEPARATOR, idx + 1) + 1
                license = EPL_HEADER.replace("2001", year)
                newLicense = EPL_GPL_HEADER.replace("2001", year)
                if "module" in lines[idx + 3]:
                    fileLicense = "".join(lines[idx:idx + 3]) + "".join(lines[idx + 5:end])
                else:
                    fileLicense = "".join(lines[idx:end])
                if fileLicense == license:
                    print(self._file, "old license")
                    if self._license:
                        if "module" in lines[idx + 3]:
                            lines[idx:idx+3] = newLicense.splitlines(True)[:3]
                            lines[idx+5:end] = newLicense.splitlines(True)[3:]
                        else:
                            lines[idx:end] = newLicense.splitlines(True)
                        end += 4
                        self._haveFixed = True
                elif fileLicense != newLicense:
                    print(self._file, "invalid license")
                    if options.verbose:
                        print(fileLicense)
                        print(license)
                self.checkDoxyLines(lines, end)
            else:
                print(self._file, "header does not start")
        if ext in (".py", ".pyw"):
            if lines[0][:2] == '#!':
                idx += 1
                if lines[0] not in ('#!/usr/bin/env python\n', '#!/usr/bin/env python3\n'):
                    print(self._file, "wrong shebang")
                    if self._fix:
                        lines[0] = '#!/usr/bin/env python\n'
                        self._haveFixed = True
            if lines[idx][:5] == '# -*-':
                idx += 1
            end = lines.index("\n", idx)
            if len(lines) < 13:
                print(self._file, "is too short (%s lines, at least 13 required for valid header)" % len(lines))
                return
            year = lines[idx + 1][16:20]
            license = EPL_HEADER.replace("//   ", "# ").replace("// ", "# ").replace("\n//", "")
            license = license.replace("2001", year).replace(SEPARATOR, "")
            newLicense = EPL_GPL_HEADER.replace("//   ", "# ").replace("// ", "# ").replace("\n//", "")
            newLicense = newLicense.replace("2001", year).replace(SEPARATOR, "")
            if "module" in lines[idx + 2]:
                fileLicense = "".join(lines[idx:idx + 2]) + "".join(lines[idx + 4:end])
            else:
                fileLicense = "".join(lines[idx:end])
            if fileLicense == license:
                print(self._file, "old license")
                if self._license:
                    if "module" in lines[idx + 2]:
                        lines[idx:idx+2] = newLicense.splitlines(True)[:2]
                        lines[idx+4:end] = newLicense.splitlines(True)[2:]
                    else:
                        lines[idx:end] = newLicense.splitlines(True)
                    end += 4
                    self._haveFixed = True
            elif fileLicense != newLicense:
                print(self._file, "different license:")
                print(fileLicense)
                if options.verbose:
                    print("!!%s!!" % os.path.commonprefix([fileLicense, license]))
                    print(license)
            self.checkDoxyLines(lines, end + 1, "#")
        if self._haveFixed:
            open(self._file, "w").write("".join(lines))

    def startElement(self, name, attrs):
        if name == 'target':
            self._file = attrs['path']
            seen.add(os.path.join(repoRoot, self._file))
        if name == 'property':
            self._property = attrs['name']

    def characters(self, content):
        if self._property:
            self._value += content

    def endElement(self, name):
        ext = os.path.splitext(self._file)[1]
        if name == 'property' and self._property == "svn:eol-style":
            self._hadEOL = True
        if name == 'property' and self._property == "svn:keywords":
            self._hadKeywords = True
        if os.path.basename(self._file) not in _IGNORE:
            if ext in _SOURCE_EXT or ext in _TESTDATA_EXT or ext in _VS_EXT:
                if (name == 'property' and self._property == "svn:executable" and
                        ext not in (".py", ".pyw", ".pl", ".bat")):
                    print(self._file, self._property, self._value)
                    if self._fix:
                        subprocess.call(
                            ["svn", "pd", "svn:executable", self._file])
                if name == 'property' and self._property == "svn:mime-type":
                    print(self._file, self._property, self._value)
                    if self._fix:
                        subprocess.call(
                            ["svn", "pd", "svn:mime-type", self._file])
            if ext in _SOURCE_EXT or ext in _TESTDATA_EXT:
                if ((name == 'property' and self._property == "svn:eol-style" and self._value != "LF") or
                        (name == "target" and not self._hadEOL)):
                    print(self._file, "svn:eol-style", self._value)
                    if self._fix:
                        if os.name == "posix":
                            subprocess.call(
                                ["sed", "-i", r's/\r$//', self._file])
                            subprocess.call(
                                ["sed", "-i", r's/\r/\n/g', self._file])
                        subprocess.call(
                            ["svn", "ps", "svn:eol-style", "LF", self._file])
            if ext in _SOURCE_EXT:
                if ((name == 'property' and self._property == "svn:keywords" and self._value != _KEYWORDS) or
                        (name == "target" and not self._hadKeywords)):
                    print(self._file, "svn:keywords", self._value)
                    if self._fix:
                        subprocess.call(
                            ["svn", "ps", "svn:keywords", _KEYWORDS, self._file])
                if name == 'target':
                    p = self.checkFile()
                    if p is not None:
                        p.wait()
            if ext in _VS_EXT:
                if ((name == 'property' and self._property == "svn:eol-style" and self._value != "CRLF") or
                        (name == "target" and not self._hadEOL)):
                    print(self._file, "svn:eol-style", self._value)
                    if self._fix:
                        subprocess.call(["svn", "ps", "svn:eol-style", "CRLF", self._file])
        if name == 'property':
            self._value = ""
            self._property = None
        if name == 'target':
            self._hadEOL = False
            self._hadKeywords = False

    def checkFile(self, fileName=None, exclude=None):
        if fileName is not None:
            self._file = fileName
        ext = os.path.splitext(self._file)[1]
        try:
            codecs.open(self._file, 'r', 'utf8').read()
        except UnicodeDecodeError as e:
            print(self._file, e)
        self.checkFileHeader(ext)
        if exclude:
            for x in exclude:
                if x + "/" in self._file:
                    return None
        if ext in (".cpp", ".h", ".java") and HAVE_ASTYLE and self._fix:
            subprocess.call(["astyle", "--style=java", "--unpad-paren", "--pad-header", "--pad-oper",
                             "--add-brackets", "--indent-switches", "--align-pointer=type",
                             "-n", os.path.abspath(self._file)])
            subprocess.call(["sed", "-i", "-e", '$a\\', self._file])
        if self._pep and ext == ".py":
            ret = 0
            if HAVE_FLAKE and os.path.getsize(self._file) < 1000000:  # flake hangs on very large files
                if not self._fix:
                    return subprocess.Popen(["flake8", "--max-line-length", "120", self._file])
                ret = subprocess.call(["flake8", "--max-line-length", "120", self._file])
            if ret and HAVE_AUTOPEP and self._fix:
                subprocess.call(["autopep8", "--max-line-length", "120", "--in-place", self._file])


optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-f", "--fix", action="store_true",
                     default=False, help="fix invalid svn properties, run astyle and autopep8")
optParser.add_option("-l", "--license", action="store_true",
                     default=False, help="fix license only")
optParser.add_option("-s", "--skip-pep", action="store_true",
                     default=False, help="skip autopep8 and flake8 tests")
optParser.add_option("-d", "--directory", help="check given subdirectory of sumo tree")
optParser.add_option("-x", "--exclude", default="contributed,foreign",
                     help="comma-separated list of (sub-)paths to exclude from pep and astyle checks")
(options, args) = optParser.parse_args()
seen = set()
sumoRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
if len(args) > 0:
    repoRoots = [os.path.abspath(a) for a in args]
elif options.directory:
    repoRoots = [os.path.join(sumoRoot, options.directory)]
else:
    repoRoots = [sumoRoot]
procs = []
for repoRoot in repoRoots:
    if options.verbose:
        print("checking", repoRoot)
    propRead = PropertyReader(options.fix, not options.skip_pep, options.license)
    if os.path.isfile(repoRoot):
        proc = propRead.checkFile(repoRoot)
        if proc is not None:
            procs.append(proc)
        continue
    try:
        oldDir = os.getcwd()
        os.chdir(repoRoot)
        exclude = options.exclude.split(",")
        for name in subprocess.check_output(["git", "ls-files"]).splitlines():
            ext = os.path.splitext(name)[1]
            if ext in _SOURCE_EXT:
                proc = propRead.checkFile(name, exclude)
                if proc is not None:
                    procs.append(proc)
                    if len(procs) == multiprocessing.cpu_count():
                        [p.wait() for p in procs]
                        procs = []
        os.chdir(oldDir)
        continue
    except (OSError, subprocess.CalledProcessError) as e:
        print("This seems to be no valid git repository", repoRoot, e)
        if options.verbose:
            print("trying svn at", repoRoot)
        output = subprocess.check_output(["svn", "pl", "-v", "-R", "--xml", repoRoot])
        xml.sax.parseString(output, propRead)
    if options.verbose:
        print("re-checking tree at", repoRoot)
    for root, dirs, files in os.walk(repoRoot):
        for name in files:
            ext = os.path.splitext(name)[1]
            if name not in _IGNORE:
                fullName = os.path.join(root, name)
                if ext in _SOURCE_EXT or ext in _TESTDATA_EXT or ext in _VS_EXT:
                    if fullName in seen or subprocess.call(["svn", "ls", fullName],
                                                           stdout=open(os.devnull, 'w'), stderr=subprocess.STDOUT):
                        continue
                    print(fullName, "svn:eol-style")
                    if options.fix:
                        if ext in _VS_EXT:
                            subprocess.call(
                                ["svn", "ps", "svn:eol-style", "CRLF", fullName])
                        else:
                            if os.name == "posix":
                                subprocess.call(["sed", "-i", 's/\r$//', fullName])
                            subprocess.call(
                                ["svn", "ps", "svn:eol-style", "LF", fullName])
                if ext in _SOURCE_EXT:
                    print(fullName, "svn:keywords")
                    if options.fix:
                        subprocess.call(
                            ["svn", "ps", "svn:keywords", _KEYWORDS, fullName])
        for ignoreDir in ['.svn', 'foreign', 'contributed', 'texttesttmp']:
            if ignoreDir in dirs:
                dirs.remove(ignoreDir)
[p.wait() for p in procs]
