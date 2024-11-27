# Linux 빌드

이 가이드는 Linux에서 CARLA를 소스로부터 빌드하는 방법을 설명합니다. 크게 두 부분으로 나뉘어 있습니다. 첫 번째 부분에서는 시스템 요구사항과 필요한 소프트웨어 설치에 대해 다루고, 두 번째 부분에서는 실제 CARLA를 빌드하고 실행하는 방법을 설명합니다.

빌드 과정은 길며(4시간 이상) 여러 종류의 소프트웨어가 필요합니다. 시작하기 전에 가이드 전체를 읽어보시기를 강력히 권장합니다.

오류나 어려움이 발생하면 가장 일반적인 문제에 대한 해결책을 제공하는 **[F.A.Q.](build_faq.md)** 페이지를 확인하시거나, [CARLA 포럼](https://github.com/carla-simulator/carla/discussions)에 질문을 올려주세요.

- [__Part One: 사전 준비__](#part-one-prerequisites)
    - [시스템 요구사항](#system-requirements)
    - [소프트웨어 요구사항](#software-requirements)
        - [언리얼 엔진](#unreal-engine)
- [__Part Two: CARLA 빌드__](#part-two-build-carla)
    - [CARLA 저장소 복제](#clone-the-carla-repository)
    - [에셋 다운로드](#get-assets)
    - [언리얼 엔진 환경 변수 설정](#set-unreal-engine-environment-variable)
    - [CARLA 빌드](#build-carla)
    - [기타 make 명령어](#other-make-commands)

---
## Part One: 사전 준비

### 시스템 요구사항

* __Ubuntu 18.04.__ CARLA는 16.04까지의 이전 Ubuntu 버전을 지원합니다. **하지만** 언리얼 엔진이 제대로 작동하려면 적절한 컴파일러가 필요합니다. Ubuntu 18.04와 이전 버전에 대한 의존성이 아래에 별도로 나열되어 있습니다. 시스템에 맞는 것을 설치하시기 바랍니다.
* __130GB 디스크 공간.__ Carla는 약 31GB를, 언리얼 엔진은 약 91GB를 차지하므로 이 둘과 기타 소프트웨어 설치를 위해 약 130GB의 여유 공간이 필요합니다.
* __적절한 GPU.__ CARLA는 현실적인 시뮬레이션을 목표로 하므로, 서버는 최소 6GB GPU가 필요하며 8GB를 권장합니다. 머신 러닝을 위해서는 전용 GPU를 강력히 권장합니다.
* __두 개의 TCP 포트와 좋은 인터넷 연결.__ 기본적으로 2000번과 2001번 포트를 사용합니다. 이 포트들이 방화벽이나 다른 애플리케이션에 의해 차단되지 않았는지 확인하세요.

!!! Warning
    __CARLA 0.9.12에서 0.9.13으로 업그레이드하는 경우__: 먼저 UE4 엔진의 CARLA 포크를 최신 버전으로 업그레이드해야 합니다. UE4 업그레이드에 대한 자세한 내용은 [__언리얼 엔진__](#unreal-engine) 섹션을 참조하세요.
### 소프트웨어 요구사항

CARLA를 실행하기 위해서는 여러 종류의 소프트웨어가 필요합니다. *Boost.Python*과 같은 일부는 CARLA 빌드 과정에서 함께 빌드되며, 그 외에는 빌드 시작 전에 설치해야 하는 바이너리(*cmake*, *clang*, 다양한 버전의 *Python* 등)들이 있습니다. 이러한 요구사항을 설치하려면 다음 명령어를 실행하세요:

```sh
sudo apt-get update &&
sudo apt-get install wget software-properties-common &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add
```

!!! Warning
    다음 명령어들은 Ubuntu 버전에 따라 다릅니다. 본인의 시스템에 맞는 것을 선택하세요.

언리얼 엔진과 CARLA 의존성 간의 호환성 문제를 피하기 위해, 모든 것을 컴파일할 때 동일한 컴파일러 버전과 C++ 런타임 라이브러리를 사용하세요. CARLA 팀은 clang-8(또는 Ubuntu 20.04에서는 clang-10)과 LLVM의 libc++를 사용합니다. 언리얼 엔진과 CARLA 의존성을 컴파일하기 위해 기본 clang 버전을 변경하세요.

__Ubuntu 22.04__.
```sh
sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe"
sudo apt-get update
sudo apt-get install build-essential clang-10 lld-10 g++-7 cmake ninja-build libvulkan1 python python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git git-lfs
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-10/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-10/bin/clang 180 &&
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 180
```

__Ubuntu 20.04__.
```sh
sudo apt-add-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal main"
sudo apt-get update
sudo apt-get install build-essential clang-10 lld-10 g++-7 cmake ninja-build libvulkan1 python python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-10/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-10/bin/clang 180
```

__Ubuntu 18.04__.
```sh
sudo apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic main"
sudo apt-get update
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180
```

__Ubuntu 16.04__.
```sh
sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" &&
sudo apt-get update
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180
```

__이전 Ubuntu 버전__.

CARLA 빌드를 위해서는 Ubuntu 18.04 이상을 강력히 권장합니다. 하지만 다음 명령어를 사용하여 이전 버전의 Ubuntu에서도 CARLA 빌드를 시도해볼 수 있습니다:

```sh
sudo apt-add-repository "deb http://apt.llvm.org/<code_name>/ llvm-toolchain-<code_name>-8 main" &&
sudo apt-get update
sudo apt-get install build-essential clang-8 lld-8 g++-7 cmake ninja-build libvulkan1 python python-pip python-dev python3-dev python3-pip libpng16-dev libtiff5-dev libjpeg-dev tzdata sed curl unzip autoconf libtool rsync libxml2-dev git
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 &&
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180
```

__모든 Ubuntu 시스템__.

CARLA 0.9.12부터는 사용자가 `pip` 또는 `pip3`를 사용하여 CARLA Python API를 설치할 수 있는 옵션이 제공됩니다. 20.3 이상의 버전이 필요합니다. 적절한 버전이 있는지 확인하려면 다음 명령어를 실행하세요:

```sh
# Python 3의 경우
pip3 -V

# Python 2의 경우
pip -V
```

업그레이드가 필요한 경우:

```sh
# Python 3의 경우
pip3 install --upgrade pip

# Python 2의 경우
pip install --upgrade pip
```

다음 Python 의존성을 설치해야 합니다:

```sh
pip install --user setuptools &&
pip3 install --user -Iv setuptools==47.3.1 &&
pip install --user distro &&
pip3 install --user distro &&
pip install --user wheel &&
pip3 install --user wheel auditwheel==4.0.0
```
## 언리얼 엔진

버전 0.9.12부터 CARLA는 수정된 언리얼 엔진 4.26 포크를 사용합니다. 이 포크에는 CARLA에 특화된 패치가 포함되어 있습니다.

이 언리얼 엔진 포크를 다운로드하려면 __GitHub 계정이 언리얼 엔진 계정과 연동되어 있어야 합니다__. 이 설정이 되어 있지 않다면, 계속 진행하기 전에 [이 가이드](https://www.unrealengine.com/en-US/ue4-on-github)를 따라 설정하세요.

__1.__ CARLA의 언리얼 엔진 4.26 포크 내용을 로컬 컴퓨터에 복제합니다:

```sh
git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26
```

__2.__ 저장소를 복제한 디렉토리로 이동합니다:
```sh
cd ~/UnrealEngine_4.26
```

__3.__ 빌드를 수행합니다. 시스템에 따라 한두 시간 정도 소요될 수 있습니다.
```sh
./Setup.sh && ./GenerateProjectFiles.sh && make
```

__4.__ 언리얼 엔진이 제대로 설치되었는지 확인하기 위해 에디터를 엽니다.
```sh
cd ~/UnrealEngine_4.26/Engine/Binaries/Linux && ./UE4Editor
```

## Part Two: CARLA 빌드

!!! Note
    `sudo apt-get install aria2`로 aria2를 설치하면 다음 명령어들의 속도가 향상됩니다.

### CARLA 저장소 복제

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla" target="_blank" class="btn btn-neutral" title="CARLA 저장소로 이동">
<span class="icon icon-github"></span> CARLA 저장소</a>
</p>
</div>

위 버튼을 클릭하면 프로젝트의 공식 저장소로 이동합니다. 저장소에서 다운로드하여 로컬에 압축을 풀거나, 다음 명령어를 사용하여 복제할 수 있습니다:

```sh
git clone https://github.com/carla-simulator/carla
```

!!! Note
    `master` 브랜치에는 최신 수정사항과 기능이 포함된 현재 CARLA 릴리스가 있습니다. 이전 CARLA 버전들은 버전 이름으로 태그가 지정되어 있습니다. git에서 `git branch` 명령어로 현재 브랜치를 항상 확인하세요.

### 에셋 다운로드

현재 버전의 CARLA로 작업하려면 __최신__ 에셋을 다운로드해야 합니다. 이 과정을 자동화하는 스크립트를 제공합니다. 스크립트를 사용하려면 CARLA 루트 폴더에서 다음 명령어를 실행하세요:

```sh
./Update.sh
```

에셋이 다운로드되어 적절한 위치에 압축이 풀립니다.

!!! Important
    개발 중인 에셋을 다운로드하려면 [CARLA 업데이트하기](build_update.md#get-development-assets)를 방문하여 __개발 에셋 받기__를 읽어보세요.

__특정 버전__의 CARLA 에셋을 다운로드하려면:

1. CARLA 루트 디렉토리에서 `/Util/ContentVersions.txt`로 이동하세요. 이 문서에는 모든 CARLA 릴리스의 에셋 링크가 포함되어 있습니다.
2. `Unreal\CarlaUE4\Content\Carla`에 에셋을 압축 해제하세요. 경로가 없다면 생성하세요.
3. 다음과 유사한 명령어로 파일을 압축 해제하세요:

```sh
tar -xvzf <assets_file_name>.tar.gz.tar -C /path/to/carla/Unreal/CarlaUE4/Content/Carla
```
### 언리얼 엔진 환경 변수 설정

CARLA가 올바른 언리얼 엔진 설치를 찾을 수 있도록 CARLA 환경 변수를 설정해야 합니다.

현재 세션에만 변수를 설정하려면:

```sh
export UE4_ROOT=~/UnrealEngine_4.26
```

세션 간에 변수가 유지되도록 설정하려면:

__1.__ `~/.bashrc` 또는 `./profile`을 엽니다.
```sh
gedit ~/.bashrc

# 또는 

gedit ~/.profile
```

__2.__ 파일 맨 아래에 다음 줄을 추가합니다:

```sh
export UE4_ROOT=~/UnrealEngine_4.26 
```

__3.__ 파일을 저장하고 터미널을 재시작합니다.

### CARLA 빌드
이 섹션에서는 CARLA를 빌드하는 명령어를 설명합니다. __모든 명령어는 CARLA 루트 폴더에서 실행해야 합니다.__

CARLA의 빌드 과정은 클라이언트 컴파일과 서버 컴파일, 두 부분으로 이루어져 있습니다.

!!! Warning
    클라이언트를 준비하기 위해 `make PythonAPI`를, 서버를 위해 `make launch`를 실행해야 합니다.
    또는 `make LibCarla`를 실행하면 어디서나 임포트할 수 있는 CARLA 라이브러리를 준비합니다.

__1.__ __Python API 클라이언트 컴파일__:

Python API 클라이언트는 시뮬레이션을 제어할 수 있게 해줍니다. Python API 클라이언트 컴파일은 CARLA를 처음 빌드할 때와 업데이트를 수행한 후에 필요합니다. 클라이언트가 컴파일되면 스크립트를 실행하여 시뮬레이션과 상호작용할 수 있습니다.

다음 명령어로 Python API 클라이언트를 컴파일합니다:

```sh
make PythonAPI
```

선택적으로, 특정 Python 버전을 위한 PythonAPI를 컴파일하려면 CARLA 루트 디렉토리에서 아래 명령어를 실행하세요.

```sh
# 필요한 버전만 선택하세요
make PythonAPI ARGS="--python-version=2.7, 3.6, 3.7, 3.8"
```

CARLA 클라이언트 라이브러리는 상호 배타적인 두 가지 형태로 빌드됩니다. 이를 통해 사용자가 선호하는 형태를 선택하여 CARLA 클라이언트 코드를 실행할 수 있습니다. 두 형태는 `.egg` 파일과 `.whl` 파일입니다. 클라이언트 라이브러리를 사용하기 위해 다음 옵션 중 __하나__를 선택하세요:

__A. `.egg` 파일__

>`.egg` 파일은 설치가 필요하지 않습니다. CARLA의 모든 예제 스크립트는 CARLA를 임포트할 때 자동으로 [이 파일을 찾습니다](build_system.md#versions-prior-to-0912).

>이전에 CARLA `.whl`을 설치했다면, `.whl`이 `.egg` 파일보다 우선순위가 높습니다.

__B. `.whl` 파일__

>`pip` 또는 `pip3`를 사용하여 `.whl` 파일을 설치해야 합니다:

```sh
# Python 3
pip3 install <path/to/wheel>.whl

# Python 2
pip install <path/to/wheel>.whl
```

>이 `.whl` 파일은 사용자의 OS에 맞게 특별히 빌드되었기 때문에 배포할 수 없습니다.

!!! Warning
    CARLA 클라이언트 라이브러리를 설치하는 다양한 방법을 사용하고 시스템에 여러 버전의 CARLA가 있을 경우 문제가 발생할 수 있습니다. `.whl`을 설치할 때는 가상 환경을 사용하고, 새로운 클라이언트 라이브러리를 설치하기 전에 이전에 설치된 클라이언트 라이브러리를 [제거](build_faq.md#how-do-i-uninstall-the-carla-client-library)하는 것을 권장합니다.
__2.__ __서버 컴파일__:

다음 명령어는 언리얼 엔진을 컴파일하고 실행합니다. 서버를 실행하거나 언리얼 엔진 에디터를 사용하고 싶을 때마다 이 명령어를 실행하세요:

```sh
make launch
```

처음 실행할 때 프로젝트가 `UE4Editor-Carla.dll` 등의 다른 인스턴스를 빌드하라고 요청할 수 있습니다. 프로젝트를 열기 위해 동의하세요. 첫 실행 시 에디터에서 셰이더와 메시 거리 필드에 관한 경고를 표시할 수 있습니다. 이들이 로드되는 데 시간이 걸리며, 로드가 완료될 때까지 맵이 제대로 표시되지 않습니다.

__3.__ __시뮬레이션 시작__:

**Play**를 눌러 서버 시뮬레이션을 시작하세요. 카메라는 `WASD` 키로 이동할 수 있으며, 마우스를 움직이면서 장면을 클릭하여 회전할 수 있습니다.

`PythonAPI\examples` 내의 예제 스크립트를 사용하여 시뮬레이터를 테스트하세요. 시뮬레이터가 실행 중인 상태에서 각 스크립트마다 새 터미널을 열고 다음 명령어를 실행하여 도시에 생명을 불어넣고 날씨 순환을 만드세요:

```sh
# 터미널 A 
cd PythonAPI/examples
python3 -m pip install -r requirements.txt
python3 generate_traffic.py  

# 터미널 B
cd PythonAPI/examples
python3 dynamic_weather.py 
```

!!! Important
    시뮬레이션이 매우 낮은 FPS로 실행된다면, 언리얼 엔진 에디터에서 `Edit -> Editor preferences -> Performance`로 이동하여 `Use less CPU when in background`를 비활성화하세요.

### 기타 make 명령어

유용할 수 있는 다른 `make` 명령어들이 있습니다. 아래 표에서 확인하세요:

| 명령어 | 설명 |
| ------- | ------- |
| `make help` | 사용 가능한 모든 명령어를 출력합니다. |
| `make launch` | 에디터 창에서 CARLA 서버를 실행합니다. |
| `make PythonAPI` | CARLA 클라이언트를 빌드합니다. |
| `make LibCarla` | 어디서나 임포트할 수 있도록 CARLA 라이브러리를 준비합니다. |
| `make package` | CARLA를 빌드하고 배포용 패키지 버전을 만듭니다. |
| `make clean` | 빌드 시스템이 생성한 모든 바이너리와 임시 파일을 삭제합니다. |
| `make rebuild` | `make clean`과 `make launch`를 하나의 명령어로 실행합니다. |

---

이 가이드에 대해 궁금한 점이 있다면 **[F.A.Q.](build_faq.md)** 페이지를 읽거나 [CARLA 포럼](https://github.com/carla-simulator/carla/discussions)에 글을 올려주세요.

다음으로, CARLA 빌드를 업데이트하는 방법을 배우거나 시뮬레이션에서 첫 걸음을 떼고 핵심 개념을 배워보세요.
<div class="build-buttons">

<p>
<a href="../build_update" target="_blank" class="btn btn-neutral" title="빌드 업데이트 방법 배우기">
CARLA 업데이트</a>
</p>

<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="CARLA 핵심 개념 배우기">
첫 걸음</a>
</p>

</div>