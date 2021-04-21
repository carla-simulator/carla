# Copyright 2015 Johannes Grassler <johannes@btw23.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import re

# TODO: Implement working cross-references (for now they are simply replaced by
#       their link titles).

class XrefFilter(object):
    """Replaces mkdocs style cross-references by just their title"""

    def run(self, lines):
        """Filter method"""
        ret = []
        for line in lines:
            while True:
                match = re.search(r'[^!]\[([^\]]+?)\]\(([^http].*?)\)', line)
                if match != None:
                    title = match.group(1)
                    line = re.sub(r'[^!]\[[^\]]+?\]\([^http].*?\)', title, line, count=1)
                else:
                    break
            ret.append(line)

        return ret
