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

class ExcludeFilter(object):
    """Removes selected mkdown_include include statements (useful for excluding
    a macros include pulled in by every chapter)"""

    def __init__(self, **kwargs):
        self.exclude = kwargs.get('exclude', [])

    def run(self, lines):
        """Filter method"""
        ret = []
        for line in lines:
            for exclude in self.exclude:
                line = re.sub(r'\{!%s!\}' % exclude, '', line)
            ret.append(line)

        return ret
