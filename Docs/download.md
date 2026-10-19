# Download

Where to get CARLA, and which build matches which engine version. The [quick start](start_quickstart.md) covers installing what you download.

---
## Latest release

| Release | Engine | Documentation |
| --- | --- | --- |
| [CARLA 0.10.0](https://github.com/carla-simulator/carla/releases/tag/0.10.0) | Unreal Engine 5.5 | [carla-ue5.readthedocs.io](https://carla-ue5.readthedocs.io/en/latest/) |
| [CARLA 0.9.15](https://github.com/carla-simulator/carla/releases) and earlier | Unreal Engine 4.26 | [carla.readthedocs.io](https://carla.readthedocs.io/en/latest/) |

---
## Nightly build

An automated build of the development branch. It carries the latest fixes and features, and also the experimental ones — use it at your own risk.

| Build | Branch | Engine |
| --- | --- | --- |
| [CARLA Nightly Build (Linux)](https://downloads.carlasim.com/Linux/Dev/CARLA_UE5_Latest.tar.gz) | `ue5-dev` | Unreal Engine 5.5 |

<p><a id="last-run-link" href='https://github.com/carla-simulator/carla/actions'>Last successful build</a>: <span id="last-run-time" class="loading">Loading...</span></p>

!!! Note
    There is no packaged nightly of the Unreal Engine 5.8 line yet; build `ue58-dev` [from source](build_linux_ue5.md) to use it. For the Unreal Engine 4.26 nightly, see the [downloads page](https://carla.readthedocs.io/en/latest/download/) of the UE4 documentation.

---
## Docker

Every release is on DockerHub. Use the explicit tag; `latest` tracks the most recent release of either line.

```sh
docker pull carlasim/carla:0.10.0
```

See [CARLA in Docker](build_docker.md) for how to run the image.
