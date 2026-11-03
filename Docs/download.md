# Download

Where to get CARLA, and which build matches which engine version. The [quick start](start_quickstart.md) covers installing what you download.

- [__Latest release__](#latest-release)
- [__Nightly build__](#nightly-build)
- [__Docker__](#docker)

---
## Latest release

| Release | Engine | Documentation |
| --- | --- | --- |
| [CARLA 1.0](https://github.com/carla-simulator/carla/releases/tag/1.0) ([Linux](https://downloads.carlasim.com/Linux/Carla-1.0-Linux-Shipping.tar.gz), [Windows](https://downloads.carlasim.com/Windows/Carla-1.0-Windows-Shipping.zip)) | Unreal Engine 5.8 | This documentation |
| [CARLA 0.10.0](https://github.com/carla-simulator/carla/releases/tag/0.10.0) | Unreal Engine 5.5 | [carla-ue5.readthedocs.io](https://carla-ue5.readthedocs.io/en/latest/) |
| [CARLA 0.9.16](https://github.com/carla-simulator/carla/releases/tag/0.9.16) and earlier | Unreal Engine 4.26 | [carla.readthedocs.io](https://carla.readthedocs.io/en/latest/) |

---
## Nightly build

An automated build of the development branch. It carries the latest fixes and features, and also the experimental ones. Use it at your own risk.

| Build | Branch | Engine |
| --- | --- | --- |
| [CARLA Nightly Build (Linux)](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Latest.tar.gz) | `ue58-dev` | Unreal Engine 5.8 |

<p><a id="last-run-link" href='https://github.com/carla-simulator/carla/actions'>Last successful build</a>: <span id="last-run-time" class="loading">Loading...</span></p>

!!! Note
    For the Unreal Engine 4.26 nightly, see the [downloads page](https://carla.readthedocs.io/en/latest/download/) of the UE4 documentation.

---
## Docker

Every release is on DockerHub. Always pull an explicit tag; `latest` is an old 0.9.x image. The CARLA 1.0 image has the tag `1.0` (see the [CARLA 1.0 release](https://github.com/carla-simulator/carla/releases/tag/1.0)).

```sh
docker pull carlasim/carla:1.0
```

See [CARLA in Docker](build_docker.md) for how to run the image.
