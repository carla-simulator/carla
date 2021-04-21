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

import markdown.extensions.tables as tbl
import markdown.blockparser
import re
import string
import textwrap

class TableFilter(tbl.TableProcessor):
    def __init__(self, width=100, encoding='utf-8'):
        self.width = width
        self.width_default = 20   # Default column width for rogue rows with more cells than the first row.


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


    def convert_table(self, block):
        """"Converts a table to grid table format"""
        lines_orig = block.split('\n')
        lines_orig.pop() # Remove extra newline at end of block
        widest_cell = [] # Will hold the width of the widest cell for each column
        widest_word = [] # Will hold the width of the widest word for each column
        widths = []      # Will hold the computed widths of grid table columns

        rows = []   # Will hold table cells during processing
        lines = []  # Will hold the finished table

        has_border = False  # Will be set to True if this is a bordered table

        width_unit = 0.0 # This number is used to divide up self.width according
                         # to the following formula:
                         #
                         # self.width = width_unit * maxwidth
                         #
                         # Where maxwidth is the sum over all elements of
                         # widest_cell.

        # Only process tables, leave everything else untouched

        if not self.test(None, block):
            return lines_orig

        if lines_orig[0].startswith('|'):
            has_border = True

        # Initialize width arrays

        for i in range(0, len(self._split_row(lines_orig[0], has_border))):
            widest_cell.append(0)
            widest_word.append(0)
            widths.append(0)

        # Parse lines into array of cells and record width of widest cell/word

        for line in lines_orig:
            row = self._split_row(line, has_border)
            # pad widest_cell to account for under length first row
            for i in range(0, len(row) - len(widest_cell)):
                widest_cell.append(0)
                widest_word.append(0)
                widths.append(0)
            for i in range(0, len(row)):
                # Record cell width
                if len(row[i]) > widest_cell[i]:
                    widest_cell[i] = len(row[i])
                # Record longest word
                words = row[i].split()
                for word in words:
                    # Keep URLs from throwing the word length count off too badly.
                    match = re.match(r'\[(.*?)\]\(.*?\)', word)
                    if match:
                       word = match.group(1)

                    if len(word) > widest_word[i]:
                        widest_word[i] = len(word)
            rows.append(row)

        # Remove table header divider line from rows
        rows.pop(1)

        # Compute first approximation of column widths based on maximum cell width

        for width in widest_cell:
            width_unit += float(width)

        width_unit = self.width / width_unit

        for i in range(0, len(widest_cell)):
            widths[i] = int(widest_cell[i] * width_unit)

        # Add rounding errors to narrowest column
        if sum(widths) < self.width:
            widths[widths.index(min(widths))] += self.width - sum(widths)

        # Attempt to correct first approximation of column widths based on
        # words that fail to fit their cell's width (if this fails textwrap
        # will break up long words but since it does not add hyphens this
        # should be avoided)

        for i in range(0, len(widths)):
            if widths[i] < widest_word[i]:
                offset = widest_word[i] - widths[i]
                for j in range(0, len(widths)):
                    if widths[j] - widest_word[j] >= offset:
                        widths[j] -= offset
                        widths[i] += offset
                        offset = 0

        lines.append(self.ruler_line(widths, linetype='-'))

        # Only add header row if it contains more than just whitespace
        if ''.join(rows[0]).strip() != '':
                lines.extend(self.wrap_row(widths, rows[0]))
                lines.append(self.ruler_line(widths, linetype='='))

        for row in rows[1:]:
            # Skip empty rows
            if ''.join(row).strip() == '':
                continue
            lines.extend(self.wrap_row(widths, row))
            lines.append(self.ruler_line(widths, linetype='-'))

        # Append empty line after table
        lines.append('')

        return lines


    def run(self, lines):
        """Filter method: Passes all blocks through convert_table() and returns a list of lines."""
        ret = []

        for block in self.blocks(lines):
            ret.extend(self.convert_table(block))

        return ret


    def ruler_line(self, widths, linetype='-'):
        """Generates a ruler line for separating rows from each other"""
        cells = []
        for w in widths:
            cells.append(linetype * (w+2))
        return '+' + '+'.join(cells) + '+'


    def wrap_row(self, widths, row, width_default=None):
        """Wraps a single line table row into a fixed width, multi-line table."""
        lines = []
        longest = 0 # longest wrapped column in row

        if not width_default:
            width_default = self.width_default

        # Wrap column contents
        for i in range(0, len(row)):
            w=width_default # column width

            # Only set column width dynamicaly for non-rogue rows
            if i < len(widths):
              w = widths[i]

            tw = textwrap.TextWrapper(width=w, break_on_hyphens=False)
            # Wrap and left-justify
            row[i] = tw.wrap(textwrap.dedent(row[i]))
            # Pad with spaces up to to fixed column width
            for l in range(0, len(row[i])):
                    row[i][l] += (w - len(row[i][l])) * ' '
            if len(row[i]) > longest:
                longest = len(row[i])

        # Pad all columns to have the same number of lines
        for i in range(0, len(row)):
            w=width_default # column width

            # Only set column width dynamicaly for non-rogue rows
            if i < len(widths):
                w = widths[i]

            if len(row[i]) < longest:
                for j in range(len(row[i]), longest):
                    row[i].append(w * ' ')

        for l in range(0,longest):
            line = []
            for c in range(len(row)):
                line.append(row[c][l])
            line = '| ' + ' | '.join(line) + ' |'
            lines.append(line)

        return lines
