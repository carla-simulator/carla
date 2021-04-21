# Copyright 2015 Johannes Grassler <johannes@btw23.de>
# Copyright 2017 Adam Twardoch <adam+github@twardoch.com>
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
import codecs
import os
import sys

import markdown
import mkdocs.config
import mkdocs.utils

import mkdocs_combine.filters.anchors
import mkdocs_combine.filters.chapterhead
import mkdocs_combine.filters.exclude
import mkdocs_combine.filters.headlevels
import mkdocs_combine.filters.images
import mkdocs_combine.filters.include
import mkdocs_combine.filters.math
import mkdocs_combine.filters.metadata
import mkdocs_combine.filters.tables
import mkdocs_combine.filters.toc
import mkdocs_combine.filters.xref
import mkdocs_combine.filters.admonitions
from mkdocs_combine.exceptions import FatalError


class MkDocsCombiner:
    """Top level converter class. Instantiate separately for each mkdocs.yml."""

    def __init__(self, **kwargs):
        self.config_file = kwargs.get('config_file', 'mkdocs.yml')
        self.encoding = kwargs.get('encoding', 'utf-8')
        self.exclude = kwargs.get('exclude', None)
        self.filter_tables = kwargs.get('filter_tables', True)
        self.filter_xrefs = kwargs.get('filter_xrefs', True)
        self.image_ext = kwargs.get('image_ext', None)
        self.strip_anchors = kwargs.get('strip_anchors', True)
        self.strip_metadata = kwargs.get('strip_metadata', True)
        self.convert_math = kwargs.get('convert_math', True)
        self.width = kwargs.get('width', 100)
        self.add_chapter_heads = kwargs.get('add_chapter_heads', True)
        self.add_page_break = kwargs.get('add_page_break', False)
        self.increase_heads = kwargs.get('increase_heads', True)
        self.convert_admonition_md = kwargs.get('convert_admonition_md', False)
        self.verbose = kwargs.get('verbose', False)
        self.combined_md_lines = []
        self.html_bare = u''
        self.html = u''

        self.log('Arguments: ' + str(kwargs))
        
        try:
            cfg = codecs.open(self.config_file, 'r', self.encoding)
        except IOError as e:
            raise FatalError("Couldn't open %s for reading: %s" % (self.config_file,
                                                                   e.strerror), 1)

        self.config = mkdocs.config.load_config(config_file=self.config_file)

        if not u'docs_dir' in self.config:
            self.config[u'docs_dir'] = u'docs'

        if not u'site_dir' in self.config:
            self.config[u'site_dir'] = u'site'

        # Set filters depending on markdown extensions from config
        # Defaults first...
        self.filter_include = False
        self.filter_toc = False

        # ...then override defaults based on config, if any:

        if u'markdown_extensions' in self.config:
            for ext in self.config[u'markdown_extensions']:
                extname = u''
                # extension entries may be dicts (for passing extension parameters)
                if type(ext) is dict:
                    extname = list(ext.keys())[0].split(u'(')[0]
                if type(ext) is str or type(ext) is self.encoding:
                    extname = ext

                if extname == u'markdown_include.include':
                    self.filter_include = True
                if extname == u'toc':
                    self.filter_toc = True

        cfg.close()

    def log(self, message):
        """Print messages if verbose mode is activated"""
        if(self.verbose):
            print('[mkdocscombine] ' + message)

    def flatten_pages(self, pages, level=1):
        """Recursively flattens pages data structure into a one-dimensional data structure"""
        flattened = []

        if sys.version_info.major < 3:
            str_type = (str, self.encoding, unicode)
        else:
            str_type = (str, self.encoding)

        for page in pages:
            if type(page) in str_type:
                flattened.append(
                    {
                        u'file' : page,
                        u'title': u'%s {: .page-title}' % mkdocs.utils.filename_to_title(page),
                        u'level': level,
                    })
            if type(page) is list:
                flattened.append(
                    {
                        u'file' : page[0],
                        u'title': u'%s {: .page-title}' % page[1],
                        u'level': level,
                    })
            if type(page) is dict:
                if type(list(page.values())[0]) in (str, self.encoding):
                    flattened.append(
                        {
                            u'file' : list(page.values())[0],
                            u'title': u'%s {: .page-title} TOREMOVE ' % list(page.keys())[0],
                            u'level': level,
                        })
                if type(list(page.values())[0]) is list:
                    # Add the parent section
                    flattened.append(
                        {
                            u'file' : None,
                            u'title': u'%s' % list(page.keys())[0],
                            u'level': level,
                        })
                    # Add children sections
                    flattened.extend(
                        self.flatten_pages(
                            list(page.values())[0],
                            level + 1)
                    )
        return flattened
        
    def combine(self):
        """User-facing conversion method. Returns combined document as a list of lines."""
        lines = []

        if(self.verbose):
            self.log('Running mkdocs-combine in verbose mode')
        
        self.log(u'Configuration: {0}'.format(self.config))

        pages = []
        if u'pages' in self.config and self.config[u'pages'] is not None:
            pages = self.flatten_pages(self.config[u'pages'])
            self.log('Pages: ')
        else:
            if u'nav' in self.config and self.config[u'nav'] is not None:
                pages = self.flatten_pages(self.config[u'nav'])
                self.log('Pages (using "nav" property): ')
        
        f_exclude = mkdocs_combine.filters.exclude.ExcludeFilter(
            exclude=self.exclude)

        f_include = mkdocs_combine.filters.include.IncludeFilter(
            base_path=self.config[u'docs_dir'],
            encoding=self.encoding)

        # First, do the processing that must be done on a per-file basis:
        # Adjust header levels, insert chapter headings and adjust image paths.

        f_headlevel = mkdocs_combine.filters.headlevels.HeadlevelFilter(pages)

        for page in pages:
            lines_tmp = []
            if page[u'file']:
                fname = os.path.join(self.config[u'docs_dir'], page[u'file'])
                try:
                    with codecs.open(fname, 'r', self.encoding) as p:
                        for line in p.readlines():
                            lines_tmp.append(line.rstrip())
                except IOError as e:
                    raise FatalError("Couldn't open %s for reading: %s" % (fname,
                                                                           e.strerror), 1)

            f_chapterhead = mkdocs_combine.filters.chapterhead.ChapterheadFilter(
                headlevel=page[u'level'],
                title=page[u'title']
            )

            f_image = mkdocs_combine.filters.images.ImageFilter(
                filename=page[u'file'],
                image_path=self.config[u'site_dir'],
                image_ext=self.image_ext)

            if self.exclude:
                lines_tmp = f_exclude.run(lines_tmp)

            if self.filter_include:
                lines_tmp = f_include.run(lines_tmp)

            lines_tmp = mkdocs_combine.filters.metadata.MetadataFilter().run(lines_tmp)
            if self.increase_heads:
                lines_tmp = f_headlevel.run(lines_tmp)
            if self.add_chapter_heads:
                lines_tmp = f_chapterhead.run(lines_tmp)
            lines_tmp = f_image.run(lines_tmp)
            lines.extend(lines_tmp)
            # Add an empty line between pages to prevent text from a previous
            # file from butting up against headers in a subsequent file.
            lines.append('')
            if self.add_page_break:
                lines.append('\\newpage')
                lines.append('')

        # Strip anchor tags
        if self.strip_anchors:
            self.log('Stripping anchor tags')
            lines = mkdocs_combine.filters.anchors.AnchorFilter().run(lines)

        # Convert math expressions
        if self.convert_math:
            self.log('Converting math expressions')
            lines = mkdocs_combine.filters.math.MathFilter().run(lines)

        # Fix cross references
        if self.filter_xrefs:
            self.log('Fixing cross references')
            lines = mkdocs_combine.filters.xref.XrefFilter().run(lines)

        # Convert admonitions already for Markdown output
        if self.convert_admonition_md:
            self.log('Converting admonitions to HTML in Markdown output')
            lines = mkdocs_combine.filters.admonitions.AdmonitionFilter().run(lines)

        if self.filter_toc:
            self.log('Creating TOC')
            lines = mkdocs_combine.filters.toc.TocFilter().run(lines)

        if self.filter_tables:
            self.log('Filtering tables')
            lines = mkdocs_combine.filters.tables.TableFilter().run(lines)

        self.combined_md_lines = lines
        return (self.combined_md_lines)

    def to_html(self):
        md = u"\n".join(self.combined_md_lines)
        mkdocs_extensions = self.config.get(u'markdown_extensions', [])
        extensions = ['markdown.extensions.attr_list']
        extension_configs = self.config.get(u'mdx_configs', [])
        for ext in mkdocs_extensions:
            if type(ext) is str or type(ext) is self.encoding:
                extname = str(ext)
                extensions.append(extname)
            elif type(ext) is dict:
                extname = str(ext.keys()[0])
                extensions.append(extname)
                extension_configs[extname] = ext[extname]
        self.html_bare = markdown.markdown(md, extensions=extensions,
                                           extension_configs=extension_configs,
                                           output_format='html5')
        self.html = u"""<!DOCTYPE html>
        <html lang="en">
        <head>
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        </head>
        <body>
        {}
        </body>
        </html>
        """.format(self.html_bare)
        return (self.html)
