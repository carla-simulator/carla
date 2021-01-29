#!/usr/bin/env python
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

# @file    checkAuthors.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2011-11-07

from __future__ import absolute_import
from __future__ import print_function

import os
import subprocess
import xml.sax
from optparse import OptionParser

_SOURCE_EXT = [".h", ".cpp", ".py", ".pl", ".java"]


class PropertyReader(xml.sax.handler.ContentHandler):

    """Reads the svn properties of files as written by svn log --xml"""

    def __init__(self, outfile):
        self._out = outfile
        self._authors = set()
        self._currAuthor = None
        self._value = ""
        self._revision = None

    def startElement(self, name, attrs):
        self._value = ""
        if name == 'logentry':
            self._revision = attrs['revision']

    def characters(self, content):
        self._value += content

    def endElement(self, name):
        if name == 'author':
            self._currAuthor = realNames.get(self._value, self._value)
        if name == "msg":
            msg = self._value.lower()
            if self._revision in ignoreRevisions:
                return
            keep = True
            ticket = msg.find("#")
            while ticket >= 0:
                keep = False
                e = ticket + 1
                while e < len(msg) and msg[e].isdigit():
                    e += 1
                if msg[ticket + 1:e] not in ignoreTickets:
                    keep = True
                    break
                ticket = msg.find("#", e)
            if not keep:
                return
            if self._currAuthor not in self._authors:
                self._authors.add(self._currAuthor)
                print("@author", self._currAuthor, file=self._out)
                try:
                    print(msg, file=self._out)
                except UnicodeEncodeError:
                    pass
                if self._currAuthor not in authorFiles:
                    authorFiles[self._currAuthor] = set()
                authorFiles[self._currAuthor].add(self._out.name)
            if "thank" in msg:
                try:
                    print("THANKS", " ".join(msg.splitlines()), file=self._out)
                    print("thank %s %s" % (msg, self._out.name), file=log)
                except UnicodeEncodeError:
                    pass
                authorFiles["thank"].add(self._out.name)


def checkAuthors(fullName, pattern):
    authors = set()
    found = False
    for line in open(fullName):
        if line.startswith(pattern):
            for item in line[len(pattern):].split(","):
                a = item.strip()
                found = True
                if a in realNames.values():
                    authors.add(a)
                else:
                    print("unknown author", a, fullName, file=log)
    if not found:
        print("no author", fullName, file=log)
    return authors


def setAuthors(fullName, removal, add, pattern):
    if options.fix:
        out = open(fullName + ".tmp", "w")
    authors = []
    for line in open(fullName):
        if line.startswith(pattern):
            for item in line[len(pattern):].split(","):
                a = item.strip()
                if a in removal:
                    print("author %s not in svn log for %s" % (
                        a, fullName), file=log)
                authors.append(a)
        elif authors:
            if options.fix:
                for a in authors:
                    print("%s  %s" % (pattern, a), file=out)
                for a in add:
                    print("%s  %s" % (pattern, a), file=out)
                out.write(line)
            elif add:
                print("need to add author %s to %s" %
                      (add, fullName), file=log)
            authors = []
        elif options.fix:
            out.write(line)
    if options.fix:
        out.close()
        os.rename(out.name, fullName)


ignoreRevisions = set(["12129", "12128", "11445", "10974", "9705", "9477", "9429", "9348", "8566",
                       "8439", "8000", "7728", "7533", "6958", "6589", "6537",
                       "6399", "6069", "5922", "5048", "4669", "4389", "4257", "4166",
                       "4165", "4084", "4076", "4015", "3966", "3486"])
ignoreTickets = set(["2", "22", "409"])
sumoRoot = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true",
                     default=False, help="tell me what you are doing")
optParser.add_option("-f", "--fix", action="store_true",
                     default=False, help="fix invalid svn properties")
optParser.add_option("-a", "--authors", action="store_true",
                     default=False, help="print author files")
optParser.add_option(
    "-r", "--root", default=sumoRoot, help="root to start parsing at")
(options, args) = optParser.parse_args()
authorFiles = {"thank": set()}
realNames = {}
for line in open(os.path.join(sumoRoot, 'AUTHORS')):
    entries = line.split()
    author = ""
    authorDone = False
    getAccounts = False
    for e in line.split():
        if e[0] == "<":
            author = author[:-1]
            authorDone = True
        if not authorDone:
            author += e + " "
        if e[-1] == ">":
            getAccounts = True
        if getAccounts:
            realNames[e] = author
    if author and author not in realNames.values():
        realNames[author] = author
log = open(os.path.join(sumoRoot, 'author.log'), "w")
for root, dirs, files in os.walk(options.root):
    for name in files:
        ext = os.path.splitext(name)[1]
        if ext in _SOURCE_EXT:
            fullName = os.path.join(root, name)
            print("checking authors for", fullName)
            if ext in _SOURCE_EXT[:2]:
                pattern = "/// @author"
            elif ext == ".py":
                pattern = "@author"
            else:
                print("cannot parse for authors", fullName, file=log)
                continue
            authors = checkAuthors(fullName, pattern)
            p = subprocess.Popen(
                ["svn", "log", "--xml", fullName], stdout=subprocess.PIPE)
            output = p.communicate()[0]
            if p.returncode == 0:
                if options.authors:
                    out = open(fullName + ".authors", "w")
                else:
                    out = open(os.devnull, "w")
                pr = PropertyReader(out)
                xml.sax.parseString(output, pr)
                out.close()
                setAuthors(
                    fullName, authors - pr._authors, pr._authors - authors, pattern)
    for ignoreDir in ['.svn', 'foreign', 'contributed', 'foxtools']:
        if ignoreDir in dirs:
            dirs.remove(ignoreDir)
print(authorFiles, file=log)
log.close()
