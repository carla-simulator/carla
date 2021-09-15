#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from setuptools import setup

CARLA_VERSION = '0.9.12'

with open("README.md") as f:
    LONG_DESCRIPTION = f.read()

setup(
    name='carla-agents',
    version=CARLA_VERSION,
    description='Baseline for the development of autonomous agents in CARLA',
    long_description=LONG_DESCRIPTION,
    long_description_content_type='text/markdown',
    url='https://github.com/carla-simulator/carla',
    author='The CARLA team',
    author_email='carla.simulator@gmail.com',
    license='MIT',
    package_dir={'': 'source'},
    packages=['agents', 'agents.navigation', 'agents.tools'],
    install_requires=[
        'carla=={}'.format(CARLA_VERSION),
        'networkx',
        'numpy; python_version < "3.0"',
        'numpy==1.18.4; python_version >= "3.0"',
        'Shapely==1.7.1'
    ]
)
