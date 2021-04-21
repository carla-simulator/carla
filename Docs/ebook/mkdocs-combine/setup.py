#!/usr/bin/env/python

from os import path

from setuptools import find_packages, setup

long_description = (
    "mkdocs_combine is a library that combines a MkDocs-style Markdown site "
    "(multiple files, with the document structure defined in the MkDocs "
    "configuration file mkdocs.yml) into a single Markdown document. "
    "The resulting document can be processed by pandoc or other Markdown tools."
    "The command line frontend tool mkdocscombine is the primary user interface."
    "Derived from https://github.com/jgrassler/mkdocs-pandoc/"
)

setup(
    name='mkdocs-combine',

    # Versions should comply with PEP440.
    version='0.4.0.0',

    description='Combines a MkDocs Markdown site into a single Markdown file',

    long_description=long_description,

    # The project's main homepage.
    url='https://github.com/twardoch/mkdocs-combine/',
    download_url='https://github.com/twardoch/mkdocs-combine/archive/master.zip',

    # Author details
    author='Johannes Grassler',
    author_email='johannes@btw23.de',
    maintainer='Adam Twardoch',
    maintainer_email='adam+github@twardoch.com',

    # Choose your license
    license='Apache',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        'Environment :: MacOS X',
        "Environment :: Console",
        'Operating System :: MacOS :: MacOS X',
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 3 - Alpha',
        # Indicate who your project is intended for
        'Intended Audience :: End Users/Desktop',
        'Intended Audience :: Developers',
        'Intended Audience :: Information Technology',
        'Intended Audience :: System Administrators',
        'Topic :: Documentation',
        'Topic :: Text Processing',
        'Topic :: Text Processing :: Filters',
        'Topic :: Text Processing :: Markup',
        'Topic :: Text Processing :: Markup :: HTML',
        'Topic :: Software Development :: Documentation',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Natural Language :: English',
        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: Apache Software License',
        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python :: 2.7',
    ],

    # What does your project relate to?
    keywords='mkdocs markdown pandoc print inline combine flatten',
    # You can just specify the packages manually here if your project is
    # simple. Or you can use find_packages().
    packages=find_packages(),

    # List run-time dependencies here.  These will be installed by pip when
    # your project is installed. For an analysis of "install_requires" vs pip's
    # requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires=['mkdocs>=1.0.4',
                      'Markdown>=3.0.1',
                      'markdown-include>=0.5.1'
                      ],

    entry_points={
        'console_scripts': [
            'mkdocscombine=mkdocs_combine.cli.mkdocscombine:main',
        ],
    },
)
