#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import yaml

class MarkdownFile:
    def __init__(self):
        self._data = ""
        self._list_depth = 0
        self.endl = '  \n'

    def data(self):
        return str(self._data)

    def push_list(self):
        self._list_depth = (self._list_depth + 1)

    def pop_list(self):
        self._list_depth = max(self._list_depth - 1, 0)

    def list_depth(self):
        if self._list_depth == 0:
            return ''
        return ''.join(['  ' * (self._list_depth + 1)])

    def list_item(self, buf):
        self._data = ''.join([self._data, self.list_depth()[:-2], '* ', buf])

    def numeric_list_item(self, buf):
        self._data = ''.join([self._data, self.list_depth(), '1. ', buf])

    def text(self, buf):
        self._data = ''.join([self._data, self.list_depth(), buf])

    def title(self, strongness, buf):
        self._data = ''.join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

    @classmethod
    def italic(cls, buf):
        return ''.join(['_', buf, '_'])

    @classmethod
    def bold(cls, buf):
        return ''.join(['**', buf, '**'])

    @classmethod
    def code(cls, buf):
        return ''.join(['`', buf, '`'])

    def code_block(self, buf, language=''):
        return ''.join(['```', language, '\n', self.list_depth(), buf, '\n', self.list_depth(), '```\n'])


class YamlIO:
    """YAML loader and writer"""

    def __init__(self, file_path):
        """Constructor that loads the yaml file"""
        with open(file_path) as yaml_file:
            self.data = yaml.safe_load(yaml_file)

    def gen_markdown(self):
        """Generates the markdown file"""
        md_data = ''
        for module in self.data:
            # print('Generating module: ' + module['module_name'])
            md = MarkdownFile()
            if module['classes']:
                for c in module['classes']:
                    class_name = c['class_name']
                    doc = c['doc']
                    variables = c['instance_variables']
                    methods = c['methods']

                    md.title(2, ''.join([md.italic('class'), ' ', md.code('carla.' + class_name)]))
                    md.text(doc)

                    if variables:
                        md.title(4, 'Instance Variables')
                        md.push_list()
                        for var in variables:
                            md.list_item(''.join([md.code(var['var_name']), md.endl]))
                            md.text(var['doc'])
                        md.pop_list()

                    if methods:
                        md.title(4, 'Methods')
                        md.push_list()
                        for m in methods:
                            param = ''
                            for p in m['params']:
                                p_type = ''.join([': ', str(p['type'])]) if 'type' in p else ''
                                default = ''.join([' = ', str(p['default'])]) if 'default' in p else ''
                                param = ''.join([param, p['param_name'], p_type, default, ', '])
                            param = param[:-2] # delete the last ', '
                            return_type = ''.join([' -> ', m['return']]) if 'return' in m else ''
                            md.list_item(''.join([md.code(''.join([m['def_name'], '(', param, ')', return_type])), md.endl]))
                            if 'doc' in m and m['doc'] is not '':
                                doc = m['doc'].strip()
                                md.text(''.join([doc, md.endl]))
                        md.pop_list()
            md_data = ''.join([md_data, md.data().strip()])
        return md_data


def main():
    """Main function"""
    script_path = os.path.dirname(os.path.abspath(__file__))
    yml_files = [f for f in os.listdir(script_path) if f.endswith('.yml')]

    data = ''

    for yml_file in yml_files:
        yml = YamlIO(os.path.join(script_path, yml_file))
        data = '\n\n'.join([data, yml.gen_markdown()])
    data = data.strip()
    with open(os.path.join(script_path, 'Output.md'), 'w') as md_file:
        md_file.write(data)

if __name__ == "__main__":
    main()
