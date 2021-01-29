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

# @file    configTemplateToWiki.py
# @author  Michael Behrisch
# @date    2012-01-26

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from xml.sax import parse, handler


class ConfigReader(handler.ContentHandler):

    def __init__(self, mergeWikiTxt, out=None):
        self._level = 0
        self._mergeWiki = mergeWikiTxt
        self._intro = {}
        self._end = len(mergeWikiTxt)
        self._file = open(out, "w") if out else sys.stdout
        active = 0
        currSect = ""
        for idx, line in enumerate(mergeWikiTxt):
            line = line.strip('\n\r')
            if line == "# Options" or line == "## Options":
                active = 1
            elif active:
                if line[:3] == "###":
                    start = idx
                    currSect = line
                elif line.startswith("| Option"):
                    self._intro[currSect] = (start, idx)
                elif line[:4] == "----" or (len(line) > 2 and line[0] == "#" and line[1] != "#"):
                    self._end = idx
                    break
            if currSect == "":
                print(line, file=self._file)

    def startElement(self, name, attrs):
        if self._level == 1:
            # subtopic
            title = "### " + name.replace("_", " ").title()
            if title in self._intro:
                begin, end = self._intro[title]
                title = ("".join(self._mergeWiki[begin:end]))
            else:
                title += "\n"
            print("%s| Option | Description |\n|%s|%s|" % (title, 8 * "-", 13 * "-"), file=self._file)
        if self._level == 2:
            # entry
            a = ""
            for s in attrs.get('synonymes', '').split():
                if len(s) == 1:
                    a = s
            print('|', end=' ', file=self._file)
            if a != "":
                print('**-%s** {{DT_%s}}<br>' % (a, attrs['type']), end=' ', file=self._file)
            print('**--%s** {{DT_%s}}' % (name, attrs['type']), end=' ', file=self._file)
            helpStr = attrs['help']
            if attrs['value']:
                if helpStr:
                    helpStr += "; "
                helpStr += "*default:* **%s**" % attrs['value']
            print('| %s |' % helpStr, file=self._file)
        self._level += 1

    def endElement(self, name):
        self._level -= 1
        if self._level == 1:
            # subtopic end
            print(file=self._file)

    def endDocument(self):
        print(("".join(self._mergeWiki[self._end:])).strip(), file=self._file)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        for app in ("activitygen", "dfrouter", "duarouter", "jtrrouter", "marouter",
                    "od2trips", "polyconvert", "netgenerate", "netconvert", "sumo"):
            if app == "netgenerate":
                cfg = os.path.join(os.path.dirname(__file__), "..", "..",
                                   "tests", "netgen", "meta", "write_template_full", "cfg.netgen")
            else:
                cfg = os.path.join(os.path.dirname(__file__), "..", "..",
                                   "tests", app, "meta", "write_template_full", "cfg." + app)
            docs = os.path.join(os.path.dirname(__file__), "..", "..",
                                "docs", "web", "docs", app + ".md")
            parse(cfg, ConfigReader(open(docs).readlines(), docs))
    elif len(sys.argv) == 2:
        app = sys.argv[1].lower()
        if app == "netgenerate":
            app = "netgen"
        cfg = os.path.join(os.path.dirname(__file__), "..", "..",
                           "tests", app, "meta", "write_template_full", "cfg." + app)
        docs = os.path.join(os.path.dirname(__file__), "..", "..",
                            "docs", "web", "docs", sys.argv[1] + ".md")
        parse(cfg, ConfigReader(open(docs).readlines()))
    elif len(sys.argv) == 3:
        parse(sys.argv[1], ConfigReader(open(sys.argv[2]).readlines()))
    else:
        print("Usage: %s <template> <wikisrc>\n   or: %s <app>" % (
            os.path.basename(__file__), os.path.basename(__file__)), file=sys.stderr)
