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

# @file    status.py
# @author  Michael Behrisch
# @author  Laura Bieker
# @date    2007-03-13

from __future__ import absolute_import
from __future__ import print_function

import sys
import smtplib
import re
from os.path import basename, commonprefix
from datetime import datetime


def printLog(msg, log):
    print(u"%s: %s" % (datetime.now(), msg), file=log)
    log.flush()


def findErrors(line, warnings, errors, failed):
    if re.search("[Ww]arn[ui]ng[: ]", line) or "[WARNING]" in line:
        if " test-case " not in line:
            warnings += 1
    if re.search("[Ee]rror[: ]", line) or re.search("[Ff]ehler:", line) or "[ERROR]" in line:
        if " test-case " not in line:
            errors += 1
            failed += line
    return warnings, errors, failed


def printStatus(makeLog, makeAllLog, smtpServer="localhost", out=sys.stdout, toAddr="sumo-tests@dlr.de"):
    failed = ""
    build = commonprefix([basename(makeLog), basename(makeAllLog)])
    print(build, end=' ', file=out)
    print(datetime.now().ctime(), file=out)
    print("--", file=out)
    print(basename(makeLog), file=out)
    warnings = 0
    errors = 0
    svnLocked = False
    for l in open(makeLog):
        if ("svn: Working copy" in l and "locked" in l) or "svn: Failed" in l:
            svnLocked = True
            failed += l
        warnings, errors, failed = findErrors(l, warnings, errors, failed)
    if svnLocked:
        failed += "svn up failed\n\n"
    print(warnings, "warnings", file=out)
    if errors:
        print(errors, "errors", file=out)
        failed += "make failed\n\n"
    print("--\nbatchreport\n--", file=out)
    print(basename(makeAllLog), file=out)
    warnings = 0
    errors = 0
    for l in open(makeAllLog):
        warnings, errors, failed = findErrors(l, warnings, errors, failed)
    print(warnings, "warnings", file=out)
    if errors:
        print(errors, "errors", file=out)
        failed += "make debug failed\n\n"
    print("--", file=out)
    if failed:
        fromAddr = "sumo-tests@dlr.de"
        message = """From: "%s" <%s>
To: %s
Subject: Error occurred while building

%s""" % (build, fromAddr, toAddr, failed)
        try:
            server = smtplib.SMTP(smtpServer)
            server.sendmail(fromAddr, toAddr, message)
            server.quit()
        except Exception:
            print("Could not send mail.")


if __name__ == "__main__":
    printStatus(sys.argv[1], sys.argv[2], sys.argv[3], sys.stdout, sys.argv[4])
