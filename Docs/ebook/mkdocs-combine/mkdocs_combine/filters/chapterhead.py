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

class ChapterheadFilter(object):
    """Filter for adding chapter titles from mkdocs.yml to chapter files"""

    def __init__(self, **kwargs):
        self.headlevel = kwargs.get('headlevel', 1)
        self.title = kwargs.get('title', None)
        if self.title == None:
            raise ValueError(
                    'Mandatory keyword argument `title` missing.')

    def run(self, lines):
        """Filter method"""

        head = [('#' * self.headlevel) + ' ' + self.title, '']

        head.extend(lines)

        return head
