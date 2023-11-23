#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from setuptools import setup, Extension

import fnmatch
import os
import sys

def is_rss_variant_enabled():
    if 'BUILD_RSS_VARIANT' in os.environ and os.environ['BUILD_RSS_VARIANT'] == 'true':
        return True
    return False

def get_libcarla_extensions():
    include_dirs = ['dependencies/include']

    library_dirs = ['dependencies/lib']
    libraries = []


    sources = ['source/libcarla/libcarla.cpp']

    def walk(folder, file_filter='*'):
        for root, _, filenames in os.walk(folder):
            for filename in fnmatch.filter(filenames, file_filter):
                yield os.path.join(root, filename)

    if os.name == "posix":
        import distro

        linux_distro = distro.linux_distribution()[0]
        if linux_distro.lower() in ["ubuntu", "debian", "deepin"]:
            pwd = os.path.dirname(os.path.realpath(__file__))
            pylib = "libboost_python%d%d.a" % (sys.version_info.major,
                                               sys.version_info.minor)
            if is_rss_variant_enabled():
                print('Building AD RSS variant.')
                extra_link_args = [ os.path.join(pwd, 'dependencies/lib/libcarla_client_rss.a') ]
            else:
                extra_link_args = [ os.path.join(pwd, 'dependencies/lib/libcarla_client.a') ]

            extra_link_args += [
                os.path.join(pwd, 'dependencies/lib/librpc.a'),
                os.path.join(pwd, 'dependencies/lib/libboost_filesystem.a'),
                os.path.join(pwd, 'dependencies/lib/libRecast.a'),
                os.path.join(pwd, 'dependencies/lib/libDetour.a'),
                os.path.join(pwd, 'dependencies/lib/libDetourCrowd.a'),
                os.path.join(pwd, 'dependencies/lib/libosm2odr.a'),
                os.path.join(pwd, 'dependencies/lib/libxerces-c.a')]
            extra_link_args += ['-lz']
            extra_compile_args = [
                '-isystem', 'dependencies/include/system', '-fPIC', '-std=c++14',
                '-Werror', '-Wall', '-Wextra', '-Wpedantic', '-Wno-self-assign-overloaded',
                '-Wdeprecated', '-Wno-shadow', '-Wuninitialized', '-Wunreachable-code',
                '-Wpessimizing-move', '-Wold-style-cast', '-Wnull-dereference',
                '-Wduplicate-enum', '-Wnon-virtual-dtor', '-Wheader-hygiene',
                '-Wconversion', '-Wfloat-overflow-conversion',
                '-DBOOST_ERROR_CODE_HEADER_ONLY', '-DLIBCARLA_WITH_PYTHON_SUPPORT',
                '-stdlib=libstdc++'
            ]
            if is_rss_variant_enabled():
                extra_compile_args += ['-DLIBCARLA_RSS_ENABLED']
                extra_compile_args += ['-DLIBCARLA_PYTHON_MAJOR_' +  str(sys.version_info.major)]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_map_integration.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_access_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_access.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_rss.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_physics_python' +  str(sys.version_info.major) + str(sys.version_info.minor) + '.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_physics.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libad_map_opendrive_reader.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libboost_program_options.a')]
                extra_link_args += [os.path.join(pwd, 'dependencies/lib/libspdlog.a')]
                extra_link_args += ['-lrt']
                extra_link_args += ['-ltbb']

            # libproj, libsqlite and python libs are also required for rss_variant, therefore
            # place them after the rss_variant linked libraries
            extra_link_args += [os.path.join(pwd, 'dependencies/lib/libproj.a'),
                                os.path.join(pwd, 'dependencies/lib/libsqlite3.a'),
                                os.path.join(pwd, 'dependencies/lib', pylib)]

            if 'TRAVIS' in os.environ and os.environ['TRAVIS'] == 'true':
                print('Travis CI build detected: disabling PNG support.')
                extra_link_args += ['-ljpeg', '-ltiff']
                extra_compile_args += ['-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=false']
            else:
                extra_link_args += ['-lpng', '-ljpeg', '-ltiff']
                extra_compile_args += ['-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=true']
            # @todo Why would we need this?
            # include_dirs += ['/usr/lib/gcc/x86_64-linux-gnu/7/include']
            # library_dirs += ['/usr/lib/gcc/x86_64-linux-gnu/7']
            # extra_link_args += ['/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a']
            extra_link_args += ['-lstdc++']
        else:
            raise NotImplementedError
    elif os.name == "nt":
        pwd = os.path.dirname(os.path.realpath(__file__))
        pylib = 'libboost_python%d%d' % (
            sys.version_info.major,
            sys.version_info.minor)

        extra_link_args = ['shlwapi.lib', 'Advapi32.lib', 'ole32.lib', 'shell32.lib']

        required_libs = [
            pylib, 'libboost_filesystem',
            'rpc.lib', 'carla_client.lib',
            'libpng.lib', 'zlib.lib',
            'Recast.lib', 'Detour.lib', 'DetourCrowd.lib',
            'xerces-c_3.lib', 'sqlite3.lib',
            'proj.lib', 'osm2odr.lib']

        # Search for files in 'PythonAPI\carla\dependencies\lib' that contains
        # the names listed in required_libs in it's file name
        libs = [x for x in os.listdir('dependencies/lib') if any(d in x for d in required_libs)]

        for lib in libs:
            extra_link_args.append(os.path.join(pwd, 'dependencies/lib', lib))

        # https://docs.microsoft.com/es-es/cpp/porting/modifying-winver-and-win32-winnt
        extra_compile_args = [
            '/experimental:external', '/external:W0', '/external:I', 'dependencies/include/system',
            '/DBOOST_ALL_NO_LIB', '/DBOOST_PYTHON_STATIC_LIB',
            '/DBOOST_ERROR_CODE_HEADER_ONLY', '/D_WIN32_WINNT=0x0600', '/DHAVE_SNPRINTF',
            '/DLIBCARLA_WITH_PYTHON_SUPPORT', '-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=true', '/MD']
    else:
        raise NotImplementedError

    depends = [x for x in walk('source/libcarla')]
    depends += [x for x in walk('dependencies')]

    def make_extension(name, sources):

        return Extension(
            name,
            sources=sources,
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            libraries=libraries,
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
            language='c++14',
            depends=depends)

    print('compiling:\n  - %s' % '\n  - '.join(sources))

    return [make_extension('carla.libcarla', sources)]

def get_license():
    if is_rss_variant_enabled():
        return 'LGPL-v2.1-only License'
    return 'MIT License'

with open("README.md") as f:
    long_description = f.read()

setup(
    name='carla',
    version='0.9.15',
    package_dir={'': 'source'},
    packages=['carla'],
    ext_modules=get_libcarla_extensions(),
    license=get_license(),
    description='Python API for communicating with the CARLA server.',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/carla-simulator/carla',
    author='The CARLA team',
    author_email='carla.simulator@gmail.com',
    include_package_data=True)
