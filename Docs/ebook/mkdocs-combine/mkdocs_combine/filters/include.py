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
"""Wrapper for using markdown.markdown_include as simple preprocessor (just
pulls in includes without running the HTML generator)"""

from __future__ import print_function
import markdown_include.include as incl


### This class is merely a wrapper for providing markdown_include.include
class IncludeFilter(incl.IncludePreprocessor):
    def __init__(self, **kwargs):
        self.base_path = kwargs.get('base_path', '.')
        self.encoding = kwargs.get('encoding', 'utf-8')
