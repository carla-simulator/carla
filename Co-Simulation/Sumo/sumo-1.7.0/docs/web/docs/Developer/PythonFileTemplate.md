---
title: Developer/PythonFileTemplate
permalink: /Developer/PythonFileTemplate/
---

```py
#!/usr/bin/env python # Leave this one out for non executable python files
# -*- coding: utf-8 -*- # optional encoding line, use if non-ASCII characters are in the code
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) <YEAR OF CREATION>-<CURRENT YEAR> German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    <FILENAME>
# @author  <AUTHOR'S NAME, ONE SEPARATE LINE FOR EACH AUTHOR>
# @author  <AUTHOR'S NAME 2>
# @date    <FILE CREATION DATE>

"""
<A BRIEF DESCRIPTION OF THE FILE'S CONTENTS>

<more documentation including examples>
"""

from __future__ import print_function
from __future__ import absolute_import

import os  # imports one per line
import sys
import ...

# use the following when you depend on sumolib or traci
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
import sumolib  # noqa
# the noqa is needed to tell the style checker that it is OK to have an import which is not at the top of the file
```