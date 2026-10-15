# Building CARLA from source

Build CARLA from source to add features of your own, or to use the Unreal Editor to create assets and edit maps. A packaged release is enough for everything else — see the [quick start](start_quickstart.md).

!!! Note
    These instructions are for the Unreal Engine 5 line: branch `ue58-dev`, engine `ue58-dev-carla` (Unreal Engine 5.8), content branch `ue58-dev-carla`. The Unreal Engine 4.26 line builds differently and has [its own documentation](https://carla.readthedocs.io/en/latest/).

---
## Pick a route

| Route | Host | What you get | Cost |
| --- | --- | --- | --- |
| [__Linux build__](build_linux_ue5.md) | Ubuntu 22.04 or 24.04 | The editor, the server and the Python API, built from source | ~225 GB, two to three hours for the engine alone |
| [__Windows build__](build_windows_ue5.md) | Windows 11 | The same, through Visual Studio 2022 | ~225 GB, two to three hours for the engine alone |
| [__Docker development environment__](build_devcontainer.md) | any Linux with Docker | The same build, with the toolchain in a container and the source on the host | As above, plus the images |
| [__Running CARLA in Docker__](build_docker.md) | any Linux with Docker | A released server, prebuilt. No build at all | One image pull |

Every build route needs a GitHub account [linked to Epic Games](https://www.unrealengine.com/en-US/ue-on-github), to clone the CARLA fork of Unreal Engine, and the NVIDIA DLSS SDK, which the setup script fetches.

Clone the repository from [__CARLA on GitHub__](https://github.com/carla-simulator/carla) to begin.
