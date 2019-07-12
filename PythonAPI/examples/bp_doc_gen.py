

import glob
import os
import sys
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import re

COLOR_LIST = '#737373'

def color(col, buf):
    return join(['<font color="', col, '">', buf, '</font>'])

def join(elem, separator = ''):
    return separator.join(elem)

def valid_dic_val(dic, value):
    return value in dic and dic[value]

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
        self._data = join([self._data, '\n'])

    def list_depth(self):
        if self._data.strip()[-1:] != '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    def text(self, buf):
        self._data = join([self._data, buf])

    def textn(self, buf):
        self._data = join([self._data, self.list_depth(), buf, self.endl])

    def not_title(self, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '<h1>', buf, '</h1>', '\n'])
    def title(self, strongness, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

def main():
    """Generates markdown file"""
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(2.0)

    world = client.get_world()
    bp_dict = {}
    blueprints = [bp.id for bp in world.get_blueprint_library().filter('*')]
    for bp in sorted(blueprints):
        bp_type = bp.split('.')[0]
        if bp_type in bp_dict:
            bp_dict[bp_type].append(bp)
        else:  
            bp_dict[bp_type] = [bp]

    md = MarkdownFile()
    md.not_title('Blueprint Library')
    for key, value in bp_dict.items():
        md.title(3, key)
        for x in value:
            md.list_pushn(color(COLOR_LIST, x))
            md.list_popn()
        md.list_pop()
    
    return md.data()

if __name__ == '__main__':

    try:

        script_path = os.path.dirname(os.path.abspath(__file__))
        with open(os.path.join(script_path, '../../Docs/bp_library.md'), 'w') as md_file:
            md_file.write(main())
        print("Done!")

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
