# Docker에서의 CARLA

사용자는 Docker 컨테이너에서 실행할 CARLA 릴리스 기반 이미지를 가져올 수 있습니다. 이는 다음과 같은 사용자에게 유용합니다:

- 모든 의존성을 설치할 필요 없이 CARLA를 실행하고 싶은 경우
- 여러 CARLA 서버를 실행하고 GPU 매핑을 수행하고 싶은 경우
- 디스플레이 없이 CARLA 서버를 실행하고 싶은 경우

이 튜토리얼에서는 CARLA 이미지를 실행하기 위한 요구사항과 OpenGL 및 Vulkan 그래픽 API로 이미지를 실행하는 방법을 설명합니다.

- [__시작하기 전에__](#before-you-begin)
- [__컨테이너에서 CARLA 실행하기__](#running-carla-in-a-container)
- [__오프스크린 모드__](#off-screen-mode)

---
## 시작하기 전에

다음 항목들이 설치되어 있어야 합니다:

- __Docker:__ [여기](https://docs.docker.com/engine/install/)의 설치 지침을 따르세요.
- __NVIDIA Container Toolkit:__ NVIDIA Container Toolkit은 Linux 컨테이너에 NVIDIA 그래픽 장치를 노출하는 라이브러리와 도구 세트입니다. Linux 호스트 시스템 또는 Windows Subsystem for Linux 버전 2에서 실행되는 Linux 배포판에서 실행되는 Linux 컨테이너용으로 특별히 설계되었습니다. [여기](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#installation-guide)의 지침을 따라 `nvidia-docker2` 패키지를 설치하세요.

!!! note
    Docker는 실행을 위해 sudo가 필요합니다. docker sudo 그룹에 사용자를 추가하려면 [이 가이드](https://docs.docker.com/install/linux/linux-postinstall/)를 따르세요.
---
## 컨테이너에서 CARLA 실행하기

__1. CARLA 이미지 가져오기__

최신 CARLA 이미지나 특정 릴리스 버전을 가져올 수 있습니다. 최신 이미지는 가장 [최근 패키지 릴리스](https://github.com/carla-simulator/carla/releases)를 참조합니다. 이미지를 가져오려면 다음 명령 중 하나를 실행하세요:

```sh
# 최신 이미지 가져오기
docker pull carlasim/carla:latest

# 특정 버전 가져오기
docker pull carlasim/carla:0.9.12
```

__2. CARLA 컨테이너 실행하기__

CARLA의 다른 버전들은 서로 다른 그래픽 API를 지원하며, 이는 Docker 이미지가 실행될 수 있는 조건에 영향을 미칠 수 있습니다:

- 0.9.12는 Vulkan만 지원합니다
- 0.9.7 이상은 Vulkan과 OpenGL 모두를 지원합니다.

__CARLA 0.9.12__

디스플레이와 함께 CARLA를 실행하려면:

```
sudo docker run --privileged --gpus all --net=host -e DISPLAY=$DISPLAY carlasim/carla:0.9.12 /bin/bash ./CarlaUE4.sh
```

오프스크린 모드로 CARLA를 실행하려면:

```
sudo docker run --privileged --gpus all --net=host -v /tmp/.X11-unix:/tmp/.X11-unix:rw carlasim/carla:0.9.12 /bin/bash ./CarlaUE4.sh -RenderOffScreen
```

__CARLA 0.9.7부터 0.9.11까지__

Vulkan을 사용하여 CARLA를 실행하려면:

```sh
sudo docker run --privileged --gpus all --net=host -e DISPLAY=$DISPLAY -e SDL_VIDEODRIVER=x11 -v /tmp/.X11-unix:/tmp/.X11-unix:rw carlasim/carla:0.9.11 /bin/bash ./CarlaUE4.sh -vulkan <-additonal-carla-flags>
```

!!! Note
    이 명령을 사용하면 머신에 디스플레이가 있는 한 Vulkan으로 CARLA 이미지를 실행할 수 있습니다. 오프스크린 모드에서 Vulkan으로 실행하는 방법에 대한 정보는 [렌더링 문서](adv_rendering_options.md#off-screen-mode)를 참조하세요.

OpenGL을 사용하여 CARLA를 실행하려면:

```sh
docker run -e DISPLAY=$DISPLAY --net=host --gpus all --runtime=nvidia carlasim/carla:<version> /bin/bash CarlaUE4.sh -opengl <-additonal-carla-flags>
```
__3. (선택사항) Docker 플래그 구성하기__

위의 명령들은 사용자의 필요에 따라 구성할 수 있는 몇 가지 Docker 플래그를 사용합니다:

- __네트워킹:__ [`--net=host`](https://docs.docker.com/engine/reference/run/#network-settings) 인자는 컨테이너가 호스트의 전체 네트워크를 공유할 수 있게 합니다. 호스트 머신의 특정 포트를 컨테이너 포트에 [매핑](https://docs.docker.com/engine/reference/run/#expose-incoming-ports)하려면, `-p <host-ports>:<container-ports>` 플래그를 사용하세요.
- __GPU:__ 모든 GPU를 `--gpus all`로 사용하거나, `--gpus '"device=<gpu_01>,<gpu_02>"'`로 특정 GPU를 지정할 수 있습니다. 자세한 정보는 [여기](https://docs.docker.com/config/containers/resource_constraints/#gpu)를 참조하세요.

---

## 오프스크린 모드

디스플레이가 없는 머신에서 CARLA를 실행하는 경우 OpenGL은 설정이 필요하지 않지만, CARLA 0.9.12 이전 버전에서 Vulkan을 사용하려면 추가 단계를 수행해야 합니다. 자세한 정보는 [렌더링 문서](adv_rendering_options.md#off-screen-mode)를 참조하세요.

---

이 주제와 관련된 모든 문제나 의문사항은 CARLA 포럼에 게시할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>