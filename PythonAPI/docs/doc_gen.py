#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import yaml

COLOR_METHOD = '#7fb800'
COLOR_PARAM = '#00a6ed'
COLOR_INSTANCE_VAR = '#f8805a'

def join(elem, separator = ''):
    return separator.join(elem)

class MarkdownFile:
    def __init__(self):
        self._data = ""
        self._list_depth = 0
        self.endl = '  \n'

    def data(self):
        return self._data

    def list_push(self, buf=''):
        if buf:
            self.text(join([
                '    ' * self._list_depth if self._list_depth != 0 else '', '- ', buf]))
        self._list_depth = (self._list_depth + 1)

    def list_pushn(self, buf):
        self.list_push(join([buf, self.endl]))

    def list_pop(self):
        self._list_depth = max(self._list_depth - 1, 0)

    def list_popn(self):
        self.list_pop()
        self._data = ''.join([self._data, '\n'])

    def list_depth(self):
        if self._data.strip()[-1:] != '\n' or self._list_depth == 0:
            return ''
        return ''.join(['    ' * self._list_depth])

    def separator(self):
        self._data = ''.join([self._data, '\n---\n'])

    def new_line(self):
        self._data = ''.join([self._data, self.endl])

    def text(self, buf):
        self._data = ''.join([self._data, buf])

    def textn(self, buf):
        self._data = ''.join([self._data, self.list_depth(), buf, self.endl])

    def title(self, strongness, buf):
        self._data = ''.join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

    def code_block(self, buf, language=''):
        return ''.join(['```', language, '\n', self.list_depth(), buf, '\n', self.list_depth(), '```\n'])

    def prettify_doc(self, doc):
        doc = doc.strip()
        doc += '' if doc[-1:] == '.' else '.'
        # doc = doc.replace('\n', '\n' + ('    ' * self._list_depth) if self._list_depth != 0 else '\n')
        return doc

def italic(buf):
    return ''.join(['_', buf, '_'])

def bold(buf):
    return ''.join(['**', buf, '**'])

def code(buf):
    return ''.join(['`', buf, '`'])

def brackets(buf):
    return ''.join(['[', buf, ']'])

def parentheses(buf):
    return ''.join(['(', buf, ')'])

def small(buf):
    return ''.join(['<sub><sup>', buf, '</sup></sub>'])

def sub(buf):
    return ''.join(['<sub>', buf, '</sub>'])

def html_key(buf):
    return ''.join(['<a name="', buf, '"></a>'])

def color(col, buf):
    return ''.join(['<font color="', col, '">', buf, '</font>'])

def valid_dic_val(dic, value):
    return value in dic and dic[value]

class YamlFile:
    """Yaml file class"""
    def __init__(self, path):
        self._path = path
        with open(path) as yaml_file:
            self.data = yaml.safe_load(yaml_file)
        self.validate()

    def validate(self):
        # print('Validating ' + str(self._path.replace('\\', '/').split('/')[-1:][0]))
        if self.data is None:
            print('\n[ERROR] File: ' + self._path)
            print("This file has no data:")
            exit(0)
        for module in self.data:
            if 'module_name' in module and module['module_name'] is None:
                print('\n[ERROR] File: ' + self._path)
                print("'module_name' is empty in:")
                exit(0)
            if 'classes' in module:
                if not module['classes']:
                    print('\n[ERROR] File: ' + self._path)
                    print("'classes' is empty in:")
                    exit(0)
                for cl in module['classes']:
                    if 'class_name' in cl and cl['class_name'] is None:
                        print('\n[ERROR] File: ' + self._path)
                        print("'class_name' is empty in:")
                        exit(0)
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for iv in cl['instance_variables']:
                            if 'var_name' in iv and iv['var_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'var_name' is empty in:")
                                exit(0)
                    if 'methods' in cl and cl['methods']:
                        for met in cl['methods']:
                            if 'def_name' in met and met['def_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'def_name' is empty in:")
                                exit(0)

    def get_modules(self):
        return [module for module in self.data]


def gen_stub_method_def(method):
    """Return python def as it should be written in stub files"""
    param = ''
    method_name = method['def_name']
    for p in method['params']:
        p_type = ''.join([': ', str(p['type'])]) if 'type' in p else ''
        default = ''.join([' = ', str(p['default'])]) if 'default' in p else ''
        param = ''.join([param, p['param_name'], p_type, default, ', '])
    param = param[:-2] # delete the last ', '
    return_type = ''.join([' -> ', method['return']]) if 'return' in method else ''
    return ''.join([method_name, parentheses(param), return_type])


def gen_doc_method_def(method, indx=False):
    """Return python def as it should be written in docs"""
    param = ''
    method_name = method['def_name']
    # to correclty render methods like __init__ in md
    if method_name[0] == '_':
        method_name = '\\' + method_name
    if indx:
        method_name = bold(method_name)
    else:
        method_name = bold(color(COLOR_METHOD, method_name))
    for p in method['params']:
        default = ''.join(['=', str(p['default'])]) if 'default' in p else ''
        if indx:
            param = ''.join([param, bold(p['param_name']), default, ', '])
        else:
            param = ''.join([param, '<font color="', COLOR_PARAM, '">', bold(p['param_name']), default, '</font>', ', '])
    param = param[:-2] # delete the last ', '
    return ''.join([method_name, parentheses(param)])


def gen_inst_var_indx(inst_var, class_key):
    inst_var_name = inst_var['var_name']
    inst_var_key = '.'.join([class_key, inst_var_name])
    return ''.join([
        brackets(bold(inst_var_name)),
        parentheses(inst_var_key), ' ',
        sub(italic('Instance variable'))])


def gen_method_indx(method, class_key):
    method_name = method['def_name']
    method_key = '.'.join([class_key, method_name])
    method_def = gen_doc_method_def(method, True)
    return ''.join([
        brackets(method_def),
        parentheses(method_key), ' ',
        sub(italic('Method'))])


def add_doc_method_param(md, param):
    param_name = param['param_name']
    param_type = ''
    param_doc = ''
    if valid_dic_val(param, 'type'):
        param_type = param['type']
    if valid_dic_val(param, 'doc'):
        param_doc = md.prettify_doc(param['doc'])
    param_type = '' if not param_type else parentheses(italic(param_type))
    md.list_push(code(param_name))
    if param_type:
        md.text(' ' + param_type)
    if param_doc:
        md.textn(' – ' + param_doc)
    else:
        md.new_line()
    md.list_pop()


def add_doc_method(md, method, class_key):
    method_name = method['def_name']
    method_key = '.'.join([class_key, method_name])
    method_def = gen_doc_method_def(method)
    md.list_pushn(''.join([html_key(method_key), method_def]))
    # method doc
    if 'doc' in method and method['doc'] is not '':
        md.textn(md.prettify_doc(method['doc']))
    # ignore if only have the parameter self
    if valid_dic_val(method, 'params') and \
        len(method['params']) != 1 and \
        method['params'][0] != 'self':
        md.list_push(bold('Parameters:') + '\n')
        for param in method['params']:
            if param['param_name'] == 'self':
                continue
            add_doc_method_param(md, param)
        md.list_pop()
    # raises error doc
    if valid_dic_val(method, 'raises'):
        md.list_pushn(bold('Raises:') + ' ' + method['raises'])
        md.list_pop()
    md.list_pop()


def add_doc_inst_var(md, inst_var, class_key):
    var_name = inst_var['var_name']
    var_key = '.'.join([class_key, var_name])
    md.list_pushn(html_key(var_key) + bold(color(COLOR_INSTANCE_VAR, var_name)))
    if valid_dic_val(inst_var, 'doc'):
        md.textn(md.prettify_doc(inst_var['doc']))
    md.list_pop()

def dump_keys(d, lvl=0):
    for k, v in d.iteritems():
        print '%s%s' % (lvl * ' ', k)
        if type(v) == dict:
            dump_keys(v, lvl+1)


class Documentation:
    """Main documentation class"""

    def __init__(self, path):
        self._path = path
        self._files = [f for f in os.listdir(path) if f.endswith('.yml')]
        self._yamls = list()
        for yaml_file in self._files:
            self._yamls.append(YamlFile(os.path.join(path, yaml_file)))
        # merge modules
        self.master_dict = dict()
        for yaml_file in self._yamls:
            for module in yaml_file.get_modules():
                module_name = module['module_name']
                if module_name not in self.master_dict:
                    self.master_dict[module_name] = module
                elif valid_dic_val(module, 'classes'):
                    for new_module in module['classes']:
                        # Create the 'classes' key if does not exist already
                        if not valid_dic_val(self.master_dict[module_name], 'classes'):
                            self.master_dict[module_name]['classes'] = []
                        self.master_dict[module_name]['classes'].append(new_module)

    def gen_overview(self):
        """Generates a referenced index for markdown file"""
        md = MarkdownFile()
        md.title(3, 'Overview')
        for module_name in sorted(self.master_dict):
            module = self.master_dict[module_name]
            module_key = '#' + module_name
            md.list_pushn(
                brackets(bold(module_key[1:])) +
                parentheses(module_key) + ' ' +
                sub(italic('Module')))
            # Generate class overview (if any)
            if 'classes' in module and module['classes']:
                for cl in sorted(module['classes']):
                    class_name = cl['class_name']
                    class_key = '.'.join([module_key, class_name])
                    md.list_pushn(
                        brackets(bold(class_name)) +
                        parentheses(class_key) + ' ' +
                        sub(italic('Class')))
                    # Generate class instance variables overview (if any)
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for inst_var in cl['instance_variables']:
                            md.list_push(gen_inst_var_indx(inst_var, class_key))
                            md.list_popn()
                    # Generate class methods overview (if any)
                    if 'methods' in cl and cl['methods']:
                        for method in cl['methods']:
                            md.list_push(gen_method_indx(method, class_key))
                            md.list_popn()
                    md.list_pop()
            md.list_pop()
        return md.data()

    def gen_body(self):
        """Generates the documentaion body"""
        md = MarkdownFile()
        for module_name in sorted(self.master_dict):
            module = self.master_dict[module_name]
            module_key = module_name
            # Generate class doc (if any)
            if valid_dic_val(module, 'classes'):
                for cl in sorted(module['classes']):
                    class_name = cl['class_name']
                    class_key = '.'.join([module_key, class_name])
                    md.title(2,
                        html_key(class_key) +
                        class_name + ' ' +
                        small(italic('Class')))
                    # Class main doc
                    if valid_dic_val(cl, 'doc'):
                        md.textn(cl['doc'])
                    # Generate instance variable doc (if any)
                    if valid_dic_val(cl, 'instance_variables'):
                        md.title(3, 'Instance Variables')
                        for inst_var in cl['instance_variables']:
                            add_doc_inst_var(md, inst_var, class_key)
                    # Generate method doc (if any)
                    if valid_dic_val(cl, 'methods'):
                        md.title(3, 'Methods')
                        for method in cl['methods']:
                            add_doc_method(md, method, class_key)
                    md.separator()
        return md.data().strip()

    def gen_markdown(self):
        """Generates the whole markdown file"""
        return '\n'.join([self.gen_overview(), self.gen_body()]).strip()

def main():
    """Main function"""
    print("Generating PythonAPI documentation...")
    script_path = os.path.dirname(os.path.abspath(__file__))
    docs = Documentation(script_path)
    with open(os.path.join(script_path, '../../Docs/python_api.md'), 'w') as md_file:
        md_file.write(docs.gen_markdown())
    print("Done!")

if __name__ == "__main__":
    main()
