"""
Template setup.py, to be called from Bazel.
"""

import glob
import os
import shutil
import sys

from setuptools import setup, Distribution, find_packages


def find_package_data():
    py_version = '{}.{}'.format(sys.version_info.major, sys.version_info.minor)

    if sys.version_info.major < 3:
        if os.name == 'nt':
            raise NotImplementedError('Windows not yet implemented')
        lib_folder = 'build/lib.linux-x86_64-{}'.format(py_version)
    else:
        lib_folder = 'build/lib'

    os.makedirs(lib_folder)

    package_data = []

    for src in %{data}:
        dst = os.path.join(lib_folder, os.path.basename(src))
        dst = dst.replace('-py{}'.format(py_version), '')
        shutil.copyfile(src, dst)
        package_data.append(dst)

    return package_data


class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True


setup(
    name='%{name}',
    version='%{version}',
    license='%{license}',
    description='%{description}',
    url='%{url}',
    author='%{author}',
    author_email='%{author_email}',
    packages=find_packages(),
    include_package_data=True,
    package_data={'': find_package_data()},
    distclass=BinaryDistribution,
    zip_safe=True,
)

os.system('tree')

shutil.move('dist/%{wheel}', '%{outpath}/%{wheel}')
shutil.move('dist/%{egg}', '%{outpath}/%{egg}')
