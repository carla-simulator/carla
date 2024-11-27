!!! warning
    이것은 진행 중인 작업입니다!! 이 CARLA 버전은 안정 릴리스로 간주되지 않습니다. 앞으로 몇 달 동안 이 브랜치에 많은 중요한 변경사항이 있을 수 있으며, 이로 인해 사용자가 만든 수정사항이 작동하지 않을 수 있습니다. 이 브랜치를 실험적인 것으로 취급하시기를 권장합니다.

# 언리얼 엔진 5.5로 Linux에서 CARLA 빌드하기

!!! note
    이 빌드 과정은 Ubuntu 22.04에서 구현되고 테스트되었습니다. 이 Ubuntu 버전을 사용하는 것을 권장합니다.

## 환경 설정

이 가이드는 언리얼 엔진 5.5로 Linux에서 CARLA를 소스로부터 빌드하는 방법을 설명합니다.

로컬 머신에 CARLA의 `ue5-dev` 브랜치를 복제하세요:

```sh
git clone -b ue5-dev https://github.com/carla-simulator/carla.git CarlaUE5
```

설정 스크립트를 실행하세요:

```sh
cd CarlaUE5
bash -x CarlaSetup.sh
```

Setup.sh 스크립트는 Cmake, debian 패키지, Python 패키지, 언리얼 엔진 5.5를 포함한 모든 필요한 패키지를 설치합니다. 또한 CARLA 콘텐츠를 다운로드하고 CARLA를 빌드합니다. 따라서 이 스크립트는 완료하는 데 오랜 시간이 걸릴 수 있습니다.

!!! note
    * 이 버전의 CARLA는 **언리얼 엔진 5.5의 CARLA 포크**가 필요합니다. UE 저장소를 복제할 권한을 얻기 위해 GitHub 계정을 Epic Games에 연결해야 합니다. 아직 계정을 연결하지 않았다면 [이 가이드](https://www.unrealengine.com/en-US/ue4-on-github)를 따르세요.
    * 이전 CARLA 언리얼 엔진 5 빌드를 사용하는 경우, **CARLA_UNREAL_ENGINE_PATH 환경 변수가 정의되어 있고** CARLA 언리얼 엔진 5.5의 절대 경로를 가리키는지 확인하세요. 이 변수가 정의되어 있지 않으면 Setup.sh 스크립트가 CARLA 언리얼 엔진 5를 다운로드하고 빌드하며, **이는 1시간 이상의 빌드 시간과 225Gb의 디스크 공간이 추가로 필요합니다**.
    * Setup.sh 스크립트는 PATH 변수 맨 앞에 설치된 Python이 있는지 확인하고, 없으면 Python을 설치합니다. **자신의 Python 버전을 사용하려면 스크립트를 실행하기 전에 PATH 변수가 Python에 대해 적절히 설정되어 있는지 확인하세요**.
    * CARLA는 외부 디스크에서 빌드할 수 없습니다. Ubuntu가 빌드에 필요한 읽기/쓰기/실행 권한을 제공하지 않기 때문입니다.

## CARLA UE5 빌드 및 실행

설정 스크립트는 다음 명령어들을 자동으로 실행합니다. 코드를 수정하고 다시 실행하고 싶을 때 다음 명령어들을 사용해야 합니다:

* 구성:

```sh
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake \
-DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON \
-DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
```

* CARLA 빌드:

```sh
cmake --build Build
```

* Python API 빌드 및 설치:

```sh
cmake --build Build --target carla-python-api-install
```

* 에디터 실행:

```sh
cmake --build Build --target launch
```

## CARLA UE5로 패키지 빌드하기

```sh
cmake --build Build --target package
```

패키지는 `$CARLA_PATH/Build/Package` 디렉토리에 생성됩니다.

## 패키지 실행하기

다음 명령어로 패키지를 실행하세요.

```sh
./CarlaUnreal.sh
```

기본 ROS2 인터페이스를 실행하고 싶다면 `--ros2` 인수를 추가하세요.

```sh
./CarlaUnreal.sh --ros2
```

빌드한 패키지에 해당하는 Python API를 설치하고 싶다면:

```sh
pip3 install PythonAPI/carla/dist/carla-*.whl
```

## 추가 빌드 대상

위에서 설명한 절차는 CARLA를 빌드하는 데 필요한 모든 구성 요소를 다운로드합니다. 모든 것을 다운로드하고 싶지 않을 수 있습니다...