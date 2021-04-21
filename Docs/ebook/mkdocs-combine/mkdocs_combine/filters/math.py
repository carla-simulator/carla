# Copyright 2015 Johannes Grassler <johannes@btw23.de>
# Copyright 2016 Kergonath <kergonath@me.com>
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

class MathFilter(object):
    """Turn the \( \) Markdown math notation into LaTex $$ inlines"""

    def run(self, lines):
        """Filter method"""
        ret = []
        for line in lines:
            ret.append(re.sub(r'\\\((.*)\\\)', r'$\1$', line))

        return ret
