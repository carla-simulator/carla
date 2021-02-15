#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    setup-traci.py
# @author  Dominik Buse
# @author  Michael Behrisch
# @date    2017-01-26


from setuptools import setup
import os
import version

SUMO_VERSION = version.get_pep440_version()
package_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

setup(
    name='traci',
    version=SUMO_VERSION,
    url='https://sumo.dlr.de/docs/TraCI/Interfacing_TraCI_from_Python.html',
    download_url='https://sumo.dlr.de/download',
    author='DLR and contributors',
    author_email='sumo@dlr.de',
    license='EPL-2.0',
    description="The pure python version of the TraCI API to communicate with the traffic simulation SUMO",

    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: Eclipse Public License 2.0 (EPL-2.0)',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
    ],
    keywords='traffic simulation traci sumo',

    packages=["traci"],
    package_dir={'': package_dir},

    install_requires=['sumolib>='+SUMO_VERSION],
)
