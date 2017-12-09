from setuptools import setup

setup(
    name='PythonClient',
    version='0.7',
    packages=['carla','carla.planner','carla.benchmarks','carla.planner'],
    license='',
    long_description=open('README.txt').read(),
    include_package_data=True,
    package_data={'carla.planner' : ['Town01.txt']}
)

