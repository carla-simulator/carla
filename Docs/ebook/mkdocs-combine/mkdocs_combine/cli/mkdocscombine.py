#!/usr/bin/python
#
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
# mkdocscombine - combines an MkDocs source site into a single Markdown document

from __future__ import print_function

import argparse
import codecs
import sys

import mkdocs_combine
from mkdocs_combine.exceptions import FatalError

from pkg_resources import get_distribution
__version__ = get_distribution('mkdocs-combine').version

def stdout_file(encoding):
    # Python 2 and Python 3 have mutually incompatible approaches to writing
    # encoded data to sys.stdout, so we'll have to pick the appropriate one.

    if sys.version_info.major == 2:
        return codecs.getwriter(encoding)(sys.stdout)
    elif sys.version_info.major >= 3:
        return open(sys.stdout.fileno(), mode='w', encoding=encoding, buffering=1)


def parse_args():
    args = argparse.ArgumentParser(
        description="mkdocscombine.py " +
        "- combines an MkDocs source site into a single Markdown document")

    args.add_argument('-V', '--version', action='version',
                      version='%(prog)s {version}'.format(version=__version__))
    args.add_argument('-v', '--verbose', dest='verbose', action='store_true',
                      help="print additional info during execution")

    args_files = args.add_argument_group('files')
    args_files.add_argument('-o', '--outfile', dest='outfile', default=None,
                            help="write combined Markdown to path ('-' for stdout)")
    args_files.add_argument('-f', '--config-file', dest='config_file', default='mkdocs.yml',
                            help="MkDocs config file (default: mkdocs.yml)")
    args_files.add_argument('-e', '--encoding', dest='encoding', default='utf-8',
                            help="set encoding for input files (default: utf-8)")
    args_files.add_argument('-x', '--exclude', dest='exclude', default=None, action='append',
                            help="exclude Markdown files from processing (default: none)")
    args_files.add_argument('-H', '--outhtml', dest='outhtml', default=None,
                            help="write simple HTML to path ('-' for stdout)")

    args_struct = args.add_argument_group('structure')
    args_strip_metadata = args_struct.add_mutually_exclusive_group(required=False)
    args_strip_metadata.add_argument('-y', '--meta', dest='strip_metadata', action='store_false',
                                     help='keep YAML metadata (default)')
    args_strip_metadata.add_argument('-Y', '--no-meta', dest='strip_metadata', action='store_true',
                                     help='strip YAML metadata')
    args.set_defaults(strip_metadata=False)

    args_add_chapter_heads = args_struct.add_mutually_exclusive_group(required=False)
    args_add_chapter_heads.add_argument('-c', '--titles', dest='add_chapter_heads', action='store_true',
                                        help='add titles from mkdocs.yml to Markdown files (default)')
    args_add_chapter_heads.add_argument('-C', '--no-titles', dest='add_chapter_heads', action='store_false',
                                        help='do not add titles to Markdown files')
    args.set_defaults(add_chapter_heads=True)

    args_increase_heads = args_struct.add_mutually_exclusive_group(required=False)
    args_increase_heads.add_argument('-u', '--up-levels', dest='increase_heads', action='store_true',
                                     help='increase ATX header levels in Markdown files (default)')
    args_increase_heads.add_argument('-k', '--keep-levels', dest='increase_heads', action='store_false',
                                     help='do not increase ATX header levels in Markdown files')
    args.set_defaults(increase_heads=True)

    args_add_page_break = args_struct.add_mutually_exclusive_group(required=False)
    args_add_page_break.add_argument('-B', '--no-page-break', dest='add_page_break', action='store_false',
                                     help='do not add page break between pages (default)')
    args_add_page_break.add_argument('-b', '--page-break', dest='add_page_break', action='store_true',
                                     help='add page break between pages')
    args.set_defaults(add_page_break=False)

    args_tables = args.add_argument_group('tables')
    args_filter_tables = args_tables.add_mutually_exclusive_group(required=False)
    args_filter_tables.add_argument('-t', '--tables', dest='filter_tables', action='store_false',
                                    help='keep original Markdown tables (default)')
    args_filter_tables.add_argument('-g', '--grid-tables', dest='filter_tables', action='store_true',
                                    help='combine Markdown tables to Pandoc-style grid tables')
    args.set_defaults(filter_tables=False)

    args_tables.add_argument('-G', '--grid-width', dest='width', default=100,
                             help="char width of converted grid tables (default: 100)")

    args_links = args.add_argument_group('links')
    args_filter_xrefs = args_links.add_mutually_exclusive_group(required=False)
    args_filter_xrefs.add_argument('-r', '--refs', dest='filter_xrefs', action='store_false',
                                   help='keep MkDocs-style cross-references')
    args_filter_xrefs.add_argument('-R', '--no-refs', dest='filter_xrefs', action='store_true',
                                   help='replace MkDocs-style cross-references by just their title (default)')
    args.set_defaults(filter_xrefs=True)

    args_strip_anchors = args_links.add_mutually_exclusive_group(required=False)
    args_strip_anchors.add_argument('-a', '--anchors', dest='strip_anchors', action='store_false',
                                    help='keep HTML anchor tags')
    args_strip_anchors.add_argument('-A', '--no-anchors', dest='strip_anchors', action='store_true',
                                    help='strip out HTML anchor tags (default)')
    args.set_defaults(strip_anchors=True)

    args_extras = args.add_argument_group('extras')
    args_convert_math = args_extras.add_mutually_exclusive_group(required=False)
    args_convert_math.add_argument('-m', '--math', dest='convert_math', action='store_false',
                                   help='keep \( \) Markdown math notation as is (default)')
    args_convert_math.add_argument('-l', '--latex', dest='convert_math', action='store_true',
                                   help='combine the \( \) Markdown math into LaTeX $$ inlines')
    args.set_defaults(convert_math=False)

    args_extras.add_argument('-i', '--image-ext', dest='image_ext', default=None,
                             help="replace image extensions by (default: no replacement)")
    args_extras.add_argument('-d', '--admonitions-md', dest='convert_admonition_md', action='store_true',
                             help='convert admonitions to HTML already in the Markdown')

    return args.parse_args()


def main():
    args = parse_args()

    try:
        mkdocs_combiner = mkdocs_combine.MkDocsCombiner(
            config_file=args.config_file,
            exclude=args.exclude,
            image_ext=args.image_ext,
            width=args.width,
            encoding=args.encoding,
            filter_tables=args.filter_tables,
            filter_xrefs=args.filter_xrefs,
            strip_anchors=args.strip_anchors,
            strip_metadata=args.strip_metadata,
            convert_math=args.convert_math,
            add_chapter_heads=args.add_chapter_heads,
            increase_heads=args.increase_heads,
            add_page_break=args.add_page_break,
            verbose=args.verbose,
            convert_admonition_md=args.convert_admonition_md
        )
    except FatalError as e:
        print(e.message, file=sys.stderr)
        return e.status

    mkdocs_combiner.combine()

    combined_md_file = None
    if args.outfile == '-':
        combined_md_file = stdout_file(args.encoding)
    elif args.outfile:
        try:
            combined_md_file = codecs.open(args.outfile, 'w', encoding=args.encoding)
        except IOError as e:
            print("Couldn't open %s for writing: %s" % (args.outfile, e.strerror), file=sys.stderr)
    if combined_md_file:
        combined_md_file.write('\n'.join(mkdocs_combiner.combined_md_lines))
        combined_md_file.close()

    html_file = None
    if args.outhtml == '-':
        html_file = stdout_file(args.encoding)
    elif args.outhtml:
        try:
            html_file = codecs.open(args.outhtml, 'w', encoding=args.encoding)
        except IOError as e:
            print("Couldn't open %s for writing: %s" % (args.htmlfile, e.strerror), file=sys.stderr)
    if html_file:
        html_file.write(mkdocs_combiner.to_html())
        html_file.close()
