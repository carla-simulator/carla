# Docker에서 Unreal Engine과 CARLA 빌드하기

이 가이드는 Docker를 사용하여 Unreal Engine과 CARLA를 처음부터 빌드하는 방법을 설명합니다. 생성된 이미지는 CARLA 패키지를 만들거나 CARLA 패키지에서 사용할 에셋을 준비하는 데 사용할 수 있습니다. 이 프로세스는 여러 서버에서 또는 디스플레이 없이 CARLA를 실행하는 데 사용되는 사전 빌드된 CARLA Docker 이미지와 혼동해서는 안 됩니다. 해당 문서는 [여기](build_docker.md)에서 찾을 수 있습니다.

- [__시작하기 전에__](#before-you-begin)
    - [__시스템 요구사항__](#system-requirements)
    - [__소프트웨어 요구사항__](#software-requirements)
- [__이미지 빌드하기__](#building-the-images)
- [__다음 단계: 패키지__](#next-steps-packages)

---

## 시작하기 전에

##### 시스템 요구사항

다음 시스템 요구사항을 충족해야 합니다:

- Ubuntu 16.04+ 용 64비트 Docker
- 최소 8GB RAM
- 초기 컨테이너 빌드 프로세스를 위한 최소 600GB 사용 가능한 디스크 공간

##### 소프트웨어 요구사항

__Docker:__

[여기](https://docs.docker.com/engine/install/)의 설치 지침에 따라 Docker를 설치하세요.

__Python:__

Python 3.6 이상이 설치되어 있고 시스템 Path에 제대로 설정되어 있어야 합니다. 설치 지침과 Python 문서는 [여기](https://www.python.org/downloads/)에서 확인하세요.

__Unreal Engine GitHub 접근:__

버전 0.9.12부터 CARLA는 수정된 Unreal Engine 4.26 포크를 사용합니다. 이 포크에는 CARLA에 특화된 패치가 포함되어 있습니다. 이는 Docker 빌드 프로세스 중에 다운로드됩니다. 이 다운로드를 위해서는 __Unreal Engine 계정에 연결된 GitHub 계정이 필요합니다__. 이것이 설정되어 있지 않다면, 계속하기 전에 [이 가이드](https://www.unrealengine.com/en-US/ue4-on-github)를 따르세요. 빌드 프로세스 중에 계정에 로그인해야 합니다.
__CARLA:__

CARLA용 Unreal Engine과 CARLA 자체를 빌드하는 데 필요한 Dockerfile과 도구들은 CARLA 소스 저장소의 `Util/Docker` 디렉토리에 있습니다.

아직 저장소가 없다면 다음 명령을 사용하여 다운로드하세요:

```sh
git clone https://github.com/carla-simulator/carla
```

---

## 이미지 빌드하기

다음 단계들은 각각 오랜 시간이 걸립니다.

__1. CARLA 사전 요구사항 이미지 빌드하기.__

다음 명령은 `Prerequisites.Dockerfile`을 사용하여 `carla-prerequisites`라는 이미지를 빌드합니다. 이 빌드에서는 컴파일러와 필요한 도구들을 설치하고, Unreal Engine 4.26 포크를 다운로드하여 컴파일합니다. Unreal Engine 다운로드가 성공하려면 로그인 정보(비밀번호 대신 Github Personal Access Token 사용)를 빌드 인자로 제공해야 합니다:

```sh
docker build --build-arg EPIC_USER=<GitHubUserName> --build-arg EPIC_PASS=<GitHubAccessToken> -t carla-prerequisites -f Prerequisites.Dockerfile .
```

__2. 최종 CARLA 이미지 빌드하기.__

다음 명령은 이전 단계에서 생성된 이미지를 사용하여 CARLA 저장소의 현재 master 브랜치(최신 릴리스)를 기반으로 최종 CARLA 이미지를 빌드합니다:

```sh
docker build -t carla -f Carla.Dockerfile .
```

CARLA 저장소의 특정 브랜치나 태그를 빌드하고 싶다면, 다음 명령을 실행하세요:

```sh
docker build -t carla -f Carla.Dockerfile . --build-arg GIT_BRANCH=<branch_or_tag_name>
```

---

## 다음 단계: 패키지

이 가이드에서 생성된 CARLA 이미지는 독립 실행형 CARLA 패키지를 만들거나 맵이나 메시와 같은 에셋을 패키징하여 CARLA 패키지에서 사용할 수 있도록 하는 데 사용됩니다. 이는 `Util/Docker`에 있는 `docker_tools.py` 스크립트를 통해 이루어집니다. 이 스크립트는 Docker 이미지를 다루기 위해 [`docker-py`](https://github.com/docker/docker-py)를 사용합니다.

`docker_tools.py` 스크립트는 다음과 같은 용도로 사용할 수 있습니다:

- __CARLA 패키지 생성하기:__ 튜토리얼은 [여기](tuto_A_create_standalone.md#export-a-package-using-docker)에서 찾을 수 있습니다
- __CARLA 패키지에서 사용할 에셋 쿠킹하기:__ 튜토리얼은 [여기](tuto_A_add_props.md#ingestion-in-a-carla-package)에서 찾을 수 있습니다
- __맵을 CARLA 패키지에서 사용할 수 있도록 준비하기:__ 튜토리얼은 [여기](tuto_M_add_map_package.md)에서 찾을 수 있습니다

---

이 주제와 관련된 모든 문제나 의문사항은 CARLA 포럼에 게시할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>