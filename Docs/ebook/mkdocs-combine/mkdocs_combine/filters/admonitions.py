#!/usr/bin/python
#
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
# mdtableconv.py - converts pipe tables to Pandoc's grid tables

import markdown.extensions.admonition as adm
import markdown.blockparser
from markdown.util import etree

class AdmonitionFilter(adm.AdmonitionProcessor):

    def __init__(self, encoding='utf-8', tab_length = 4):
        self.encoding = encoding
        self.tab_length = tab_length

    def blocks(self, lines):
        """Groups lines into markdown blocks"""
        state = markdown.blockparser.State()
        blocks = []

        # We use three states: start, ``` and '\n'
        state.set('start')

        # index of current block
        currblock = 0

        for line in lines:
            line += '\n'
            if state.isstate('start'):
                if line[:3] == '```':
                    state.set('```')
                else:
                    state.set('\n')
                blocks.append('')
                currblock = len(blocks) - 1
            else:
                marker = line[:3]  # Will capture either '\n' or '```'
                if state.isstate(marker):
                    state.reset()
            blocks[currblock] += line

        return blocks

    def run(self, lines):
        """Filter method: Passes all blocks through convert_admonition() and returns a list of lines."""
        ret = []

        blocks = self.blocks(lines)
        for block in blocks:
            blacklist = [  ]
            if ("!!!" in block) and (not (any(ch in blacklist for ch in block))) :
                block = block.replace("_", " ")
                block = block.replace("#", " ")
                block = block.replace("**", " ")
                block = block.replace("<br>", " ")
                block = block.replace("\\", "\\\\\\\\")
                block = block.replace("!!! Note", " \\notebox{ ")
                block = block.replace("!!! note", " \\notebox{ ")
                block = block.replace("!!! Important", " \\importantbox{ ")
                block = block.replace("!!! important", " \\importantbox{ ")
                block = block.replace("!!! Warning", " \\warningbox{ ")
                block = block.replace("!!! warning", " \\warningbox{ ")
                block = block.replace("\n", " ")
                block = block + ' }\n'
            ret.append(block)

        return ret

    def convert_admonition(self, block):
        lines = block.split('\n')

        if self.RE.search(block):

            m = self.RE.search(lines.pop(0))
            klass, title = self.get_class_and_title(m)
            
            lines = list(map(lambda x:self.detab(x)[0], lines))
            lines = '\n'.join(lines[:-1])
            
            div = etree.Element('div')
            div.set('class', '%s %s' % (self.CLASSNAME, klass))
            if title:
                p = etree.SubElement(div, 'p')
                p.set('class', self.CLASSNAME_TITLE)
                p.text = title

            content = etree.SubElement(div, 'p')
            content.text = lines

            string = etree.tostring(div).decode(self.encoding)
            lines = [string]
            lines.append('')

        return lines
