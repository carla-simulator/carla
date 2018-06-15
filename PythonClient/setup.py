from setuptools import setup

# @todo Dependencies are missing.

setup(
    name='carla_client',
    version='0.8.4',
    packages=['carla', 'carla.driving_benchmark', 'carla.agent',
              'carla.driving_benchmark.experiment_suites', 'carla.planner'],
    license='MIT License',
    description='Python API for communicating with the CARLA server.',
    url='https://github.com/carla-simulator/carla',
    author='The CARLA team',
    author_email='carla.simulator@gmail.com',
    include_package_data=True
)
