from setuptools import setup

# @todo Dependencies are missing.

setup(
    name='carla_client',
    version='0.7.1',
    packages=['carla', 'carla.benchmarks', 'carla.planner'],
    license='MIT License',
    description='Python API for communicating with the CARLA server.',
    url='https://github.com/carla-simulator/carla',
    author='The CARLA team',
    author_email='carla.simulator@gmail.com',
    include_package_data=True
)
