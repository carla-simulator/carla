# OpenADS specific changes

- Added a GitLab CI pipeline for UE5 CARLA builds, including builder image creation, Unreal Engine setup, CARLA package build, release/client Docker image builds, image publishing, and cleanup.
- Integrated additional OpenADS content via the new `Unreal/CarlaUnreal/content-openads` submodule and configured the campus map for cooking and packaging.
- Updated Docker packaging and added dedicated CARLA PythonAPI client image.
- Added Python 3.12 support to the build environment and PythonAPI CMake configuration.
- Enabled ROS 2 actor control for vehicles using `role_name=ego_vehicle`.
