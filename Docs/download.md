# Download

Where to get CARLA, and which build matches which engine version. The [quick start](start_quickstart.md) covers installing what you download.

- [__Latest release__](#latest-release)
- [__Nightly build__](#nightly-build)
- [__Docker__](#docker)

---
## Latest release

| Release | Engine | Documentation |
| --- | --- | --- |
| [CARLA 1.0](https://github.com/carla-simulator/carla/releases/tag/1.0) | Unreal Engine 5.8 | This documentation |
| [CARLA 0.10.0](https://github.com/carla-simulator/carla/releases/tag/0.10.0) | Unreal Engine 5.5 | [`Docs/` at the 0.10.0 tag](https://github.com/carla-simulator/carla/tree/0.10.0/Docs) |
| [CARLA 0.9.16](https://github.com/carla-simulator/carla/releases/tag/0.9.16) and earlier | Unreal Engine 4.26 | [carla.readthedocs.io](https://carla.readthedocs.io/en/latest/) |

---
## Nightly build

An automated build of the development branch. It carries the latest fixes and features, and also the experimental ones. Use it at your own risk.

| Build | Branch | Engine |
| --- | --- | --- |
| [CARLA Nightly Build (Linux)](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Latest.tar.gz) | `ue58-dev` | Unreal Engine 5.8 |
| [CARLA Nightly Build (Windows)](https://downloads.carlasim.com/Windows/Dev/CARLA_UE5_Latest.zip) | `ue58-dev` | Unreal Engine 5.8 |

<p><a id="last-run-link" href='https://github.com/carla-simulator/carla/actions'>Last successful build</a>: <span id="last-run-time" class="loading">Loading...</span></p>

!!! Note
    For the Unreal Engine 4.26 nightly, see the [downloads page](https://carla.readthedocs.io/en/latest/download/) of the UE4 documentation.

---
## Weekly build

The development branch built every Sunday, with the [AdditionalMaps](map_content_packs.md) pack built against it and Python API wheels for Python 3.10 to 3.15. The nightly build has no pack, so use the weekly build to get the additional maps of the development branch.

| Build | Additional maps | Branch | Engine |
| --- | --- | --- | --- |
| [CARLA Weekly Build (Linux)](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Weekly.tar.gz) | [AdditionalMaps (Linux)](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Weekly_AdditionalMaps.tar.gz) | `ue58-dev` | Unreal Engine 5.8 |
| [CARLA Weekly Build (Windows)](https://downloads.carlasim.com/Windows/Dev/CARLA_UE5_Weekly.zip) | [AdditionalMaps (Windows)](https://downloads.carlasim.com/Windows/Dev/CARLA_UE5_Weekly_AdditionalMaps.tar.gz) | `ue58-dev` | Unreal Engine 5.8 |

---
## Docker

Every release is on DockerHub. Always pull an explicit tag; `latest` is an old 0.9.x image. The CARLA 1.0 image has the tag `1.0` (see the [CARLA 1.0 release](https://github.com/carla-simulator/carla/releases/tag/1.0)).

```sh
docker pull carlasim/carla:1.0
```

See [CARLA in Docker](build_docker.md) for how to run the image.
