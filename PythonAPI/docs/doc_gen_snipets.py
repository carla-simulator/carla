#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import yaml
import re

COLOR_METHOD = '#7fb800'
COLOR_PARAM = '#00a6ed'
COLOR_INSTANCE_VAR = '#f8805a'
COLOR_NOTE = '#8E8E8E'
COLOR_WARNING = '#ED2F2F'

QUERY = re.compile(r'([cC]arla(\.[a-zA-Z0-9_]+)+)')


def create_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)

def create_getter_setter_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)

def join(elem, separator=''):
    return separator.join(elem)


class MarkdownFile:
    def __init__(self):
        self._data = ""
        self._list_depth = 0
        self.endl = '  \n'

    def data(self):
        return self._data

    def list_depth(self):
        if self._data.strip()[-1:] != '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    def textn(self, buf):
        self._data = join([self._data, self.list_depth(), buf, self.endl])



class Documentation:
    """Main documentation class"""

    def __init__(self, path, images_path):
        self._snipets_path = os.path.join(os.path.dirname(path), 'snipets')
        self._files = [f for f in os.listdir(self._snipets_path) if f.endswith('.py')]
        self._snipets = list()
        for snipet_file in self._files:
            current_snipet_path = os.path.join(self._snipets_path, snipet_file)
            self._snipets.append(current_snipet_path)
        # Gather snipet images
        self._snipets_images_path = images_path
        self._files_images = [f for f in os.listdir(self._snipets_images_path)]
        self._snipets_images = list()
        for snipet_image in self._files_images:
            current_image_path = os.path.join(self._snipets_images_path, snipet_image)
            self._snipets_images.append(current_image_path)


    def gen_body(self):
        """Generates the documentation body"""
        md = MarkdownFile()
        # Create header for snipets (div container and script to copy)
        md.textn(
        "[comment]: <> (=========================)\n"+
        "[comment]: <> (PYTHON API SCRIPT SNIPETS)\n"+
        "[comment]: <> (=========================)\n"+
        "<div id=\"snipets-container\" class=\"Container\" onmouseover='this.style[\"overflowX\"]=\"scroll\";' onmouseout='this.style[\"overflowX\"]=\"visible\";'></div>\n"+
        "<script>\n"+
        "function CopyToClipboard(containerid) {\n"+
        "if (document.selection) {\n"+
        "var range = document.body.createTextRange();\n"+
        "range.moveToElementText(document.getElementById(containerid));\n"+
        "range.select().createTextRange();\n"+
        "document.execCommand(\"copy\");\n"+
        "} \n"+
        "else if (window.getSelection) {\n"+
        "var range = document.createRange();\n"+
        "range.selectNode(document.getElementById(containerid));\n"+
        "window.getSelection().addRange(range);\n"+
        "document.execCommand(\"copy\");\n"+
        "}\n"+
        "}\n</script>\n"+
        "<script>\n"+
        "function CloseSnipet() {\n"+
        "document.getElementById(\"snipets-container\").innerHTML = null;\n"+
        "}\n"+
        "</script>\n")
        # Create content for every snipet
        for snipet_path in self._snipets:
            current_snipet = open(snipet_path, 'r')
            snipet_name = os.path.basename(current_snipet.name) # Remove path
            snipet_name = os.path.splitext(snipet_name)[0] # Remove extension
            # Header for a snipet
            md.textn("<div id =\""+snipet_name+"-snipet\" style=\"display: none;\">\n"+
            "<p class=\"SnipetFont\">\n"+
            "Snippet for "+snipet_name+"\n"+
            "</p>\n"+
            "<div id=\""+snipet_name+"-code\" class=\"SnipetContent\">\n\n```py\n")
            # The snipet code
            md.textn(current_snipet.read())
            # Closing for a snipet
            md.textn("\n```\n<button id=\"button1\" class=\"CopyScript\" onclick=\"CopyToClipboard('"+snipet_name+"-code')\">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id=\"button1\" class=\"CloseSnipet\" onclick=\"CloseSnipet()\">Close snippet</button><br><br>\n")
            # Check if snipet image exists, and add it
            for snipet_path_to_image in self._snipets_images:
                snipet_image_name = os.path.splitext(os.path.basename(snipet_path_to_image))[0]
                if snipet_name == snipet_image_name:
                    md.textn("\n<img src=\"/img/snipets_images/"+os.path.basename(snipet_path_to_image)+"\">\n")
            md.textn("</div>\n")
        # Closing div
        md.textn("\n</div>\n")
        return md.data().strip()


    def gen_markdown(self):
        """Generates the whole markdown file"""
        return join([self.gen_body()], '\n').strip()


def main():
    """Main function"""
    print("Generating PythonAPI snipets...")
    script_path = os.path.dirname(os.path.abspath(__file__)+'/snipets')
    snipets_images_path = os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__)))) + '/Docs/img/snipets_images'
    docs = Documentation(script_path, snipets_images_path)
    snipets_md_path = os.path.join(os.path.dirname(os.path.dirname(
        os.path.dirname(script_path))), 'Docs/python_api_snipets.md')
    with open(snipets_md_path, 'w') as md_file:
        md_file.write(docs.gen_markdown())
    print("Done snipets!")


if __name__ == "__main__":
    main()
