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

from __future__ import print_function
import os
import re


class ImageFilter(object):
    """Filter for adjusting image targets (absolute file names, optionally
    different extensions"""
    def __init__(self, **kwargs):
        self.filename = kwargs.get('filename', None)
        self.image_path = kwargs.get('image_path', None)
        self.adjust_path = kwargs.get('adjust_path', True)
        self.image_ext = kwargs.get('image_ext', None)

    def run(self, lines):
        """Filter method"""
        # Nothing to do in this case
        if (not self.adjust_path) and (not self.image_ext):
            return lines

        ret = []

        for line in lines:
            processed = {}
            while True:
                alt = ''
                img_name = ''

                match = re.search(r'!\[(.*?)\]\((.*?)\)', line)

                # Make sure there is in fact an image file name
                if match:
                    # Skip images we already processed
                    if match.group(0) in processed:
                        break
                    # Skip URLs
                    if re.match('\w+://', match.group(2)):
                        break
                    alt = match.group(1)
                    img_name = match.group(2)
                else:
                    break

                if self.image_ext:
                    img_name = re.sub(r'\.\w+$', '.' + self.image_ext, img_name)

                if self.adjust_path and (self.image_path or self.filename):
                    # explicitely specified image path takes precedence over
                    # path relative to chapter
                    if self.image_path and self.filename:
                        img_name = os.path.join(
                                os.path.abspath(self.image_path),
                                os.path.dirname(self.filename),
                                img_name)

                    # generate image path relative to file name
                    if self.filename and (not self.image_path):
                        img_name = os.path.join(
                                os.path.abspath(
                                    os.path.dirname(self.filename)),
                                img_name)
                    
                # handle Windows '\', although this adds a small amount of unnecessary work on Unix systems
                img_name = img_name.replace(os.path.sep, '/')
                             
                line = re.sub(r'!\[(.*?)\]\((.*?)\)',
                        '![%s](%s)' % (alt, img_name), line)

                # Mark this image as processed
                processed[match.group(0)] = True

            ret.append(line)

        return ret
