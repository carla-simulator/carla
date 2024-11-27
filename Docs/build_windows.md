# Windows 빌드

이 가이드는 Windows에서 CARLA를 소스로부터 빌드하는 방법을 설명합니다. 크게 두 부분으로 나뉘어 있습니다. 첫 번째 부분에서는 시스템 요구사항과 필요한 소프트웨어 설치에 대해 다루고, 두 번째 부분에서는 실제 CARLA를 빌드하고 실행하는 방법을 설명합니다.

빌드 과정은 길며(4시간 이상) 여러 종류의 소프트웨어가 필요합니다. 시작하기 전에 가이드 전체를 읽어보시기를 강력히 권장합니다.

오류나 어려움이 발생하면 가장 일반적인 문제에 대한 해결책을 제공하는 **[F.A.Q.](build_faq.md)** 페이지를 확인하시거나, [CARLA 포럼](https://github.com/carla-simulator/carla/discussions)에 질문을 올려주세요.

- [__Part One: 사전 준비__](#part-one-prerequisites)
    - [시스템 요구사항](#system-requirements)
    - [소프트웨어 요구사항](#software-requirements)
        - [기본 설치](#minor-installations)
        - [Python 의존성](#python-dependencies)
        - [주요 설치](#major-installations)
            - [Visual Studio 2019](#visual-studio-2019)
            - [언리얼 엔진](#unreal-engine)
- [__Part Two: CARLA 빌드__](#part-two-build-carla)
    - [CARLA 저장소 복제](#clone-the-carla-repository)
    - [에셋 가져오기](#get-assets)
    - [언리얼 엔진 환경 변수 설정](#set-unreal-engine-environment-variable)
    - [CARLA 빌드](#build-carla)
    - [기타 make 명령어](#other-make-commands)

---
## Part One: 사전 준비

이 섹션에서는 CARLA 빌드를 시작하기 전에 필요한 시스템 요구사항, 기본 및 주요 소프트웨어 설치, Python 의존성에 대한 세부 사항을 확인할 수 있습니다.

### 시스템 요구사항

* __x64 시스템.__ 시뮬레이터는 모든 64비트 Windows 시스템에서 실행되어야 합니다.
* __165GB 디스크 공간.__ CARLA 자체는 약 32GB를 차지하며, 관련된 주요 소프트웨어 설치(언리얼 엔진 포함)는 약 133GB를 차지합니다.
* __적절한 GPU.__ CARLA는 현실적인 시뮬레이션을 목표로 하므로, 서버는 최소 6GB GPU가 필요하며 8GB를 권장합니다. 머신 러닝을 위해서는 전용 GPU를 강력히 권장합니다.
* __두 개의 TCP 포트와 좋은 인터넷 연결.__ 기본적으로 2000번과 2001번 포트를 사용합니다. 이 포트들이 방화벽이나 다른 애플리케이션에 의해 차단되지 않았는지 확인하세요.

!!! Warning
    __CARLA 0.9.12에서 0.9.13으로 업그레이드하는 경우__: 먼저 UE4 엔진의 CARLA 포크를 최신 버전으로 업그레이드해야 합니다. UE4 업그레이드에 대한 자세한 내용은 [__언리얼 엔진__](#unreal-engine) 섹션을 참조하세요.

### 소프트웨어 요구사항

#### 기본 설치

* [__CMake__](https://cmake.org/download/) - 간단한 구성 파일에서 표준 빌드 파일을 생성합니다. __버전 3.15 이상을 권장합니다__.
* [__Git__](https://git-scm.com/downloads) - CARLA 저장소를 관리하는 버전 관리 시스템입니다.
* [__Make__](http://gnuwin32.sourceforge.net/packages/make.htm) - 실행 파일을 생성합니다. __Make 버전 3.81__을 사용해야 하며, 그렇지 않으면 빌드가 실패할 수 있습니다. 여러 버전의 Make가 설치되어 있다면, CARLA를 빌드할 때 PATH에서 버전 3.81을 사용하고 있는지 확인하세요. `make --version` 명령어로 기본 Make 버전을 확인할 수 있습니다.
* [__7Zip__](https://www.7-zip.org/) - 파일 압축 소프트웨어입니다. 에셋 파일의 자동 압축 해제에 필요하며, 대용량 파일이 잘못되거나 부분적으로 추출되어 빌드 중에 오류가 발생하는 것을 방지합니다.
* [__Python3 x64__](https://www.python.org/downloads/) - CARLA의 주요 스크립트 언어입니다. x32 버전이 설치되어 있으면 충돌이 발생할 수 있으므로, 제거하는 것이 좋습니다.

!!! Important
    위의 프로그램들이 [환경 경로](https://www.java.com/en/download/help/path.xml)에 추가되었는지 확인하세요. 추가된 경로는 프로그램의 `bin` 디렉토리에 해당해야 한다는 것을 잊지 마세요.
#### Python 의존성

CARLA 0.9.12부터는 사용자가 `pip3`를 사용하여 CARLA Python API를 설치할 수 있는 옵션이 제공됩니다. 20.3 이상의 버전이 필요합니다. 적절한 버전이 있는지 확인하려면 다음 명령어를 실행하세요:

```sh
pip3 -V
```

업그레이드가 필요한 경우:

```sh
pip3 install --upgrade pip
```

다음 Python 의존성을 설치해야 합니다:

```sh
pip3 install --user setuptools
pip3 install --user wheel
```

#### 주요 설치
##### Visual Studio 2019

[여기](https://developerinsider.co/download-visual-studio-2019-web-installer-iso-community-professional-enterprise/)에서 Visual Studio 2019 버전을 다운로드하세요. 무료 버전인 __Community__를 선택하세요. _Visual Studio Installer_를 사용하여 세 가지 추가 요소를 설치하세요:

* __Windows 8.1 SDK.__ 오른쪽의 _Installation details_ 섹션에서 선택하거나 _Individual Components_ 탭에서 _SDKs, libraries, and frameworks_ 제목 아래를 확인하세요.
* __x64 Visual C++ 도구셋.__ _Workloads_ 섹션에서 __Desktop development with C++__를 선택하세요. 이를 통해 빌드에 사용될 x64 명령 프롬프트가 활성화됩니다. `Windows` 버튼을 누르고 `x64`를 검색하여 제대로 설치되었는지 확인하세요. __`x86_x64` 프롬프트를 열지 않도록__ 주의하세요.
* __.NET framework 4.6.2.__ _Workloads_ 섹션에서 __.NET desktop development__를 선택하고, 오른쪽의 _Installation details_ 패널에서 `.NET Framework 4.6.2 development tools`를 선택하세요. 이는 언리얼 엔진을 빌드하는 데 필요합니다.

!!! Important
    다른 Visual Studio 버전은 충돌을 일으킬 수 있습니다. 제거했더라도 일부 레지스트리가 남아있을 수 있습니다. 컴퓨터에서 Visual Studio를 완전히 제거하려면 `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout`로 이동하여 `.\InstallCleanup.exe -full`을 실행하세요.

!!! Note
    위의 단계를 따르고 Windows 8.1 SDK 대신 Windows 11/10 SDK를 사용하여 Visual Studio 2022를 사용할 수도 있습니다. CMake의 기본 Visual Studio 2019 Generator를 재정의하려면 makefile 명령어를 사용할 때 GENERATOR="Visual Studio 17 2022"를 지정하세요([표](build_windows.md#other-make-commands) 참조). 빌드 스크립트에 지정된 대로 빌드 명령어와 함께 작동하는 모든 generator를 지정할 수 있습니다. 전체 목록을 보려면 `cmake -G`를 실행하세요(지금까지 LibCarla 빌드에는 Ninja가 잘 작동하는 것으로 테스트되었습니다).

##### 언리얼 엔진

버전 0.9.12부터 CARLA는 수정된 언리얼 엔진 4.26 포크를 사용합니다. 이 포크에는 CARLA에 특화된 패치가 포함되어 있습니다.

이 언리얼 엔진 포크를 다운로드하려면 __GitHub 계정이 언리얼 엔진 계정과 연결되어 있어야 합니다__. 이 설정이 되어 있지 않다면, 계속 진행하기 전에 [이 가이드](https://www.unrealengine.com/en-US/ue4-on-github)를 따르세요.

수정된 버전의 언리얼 엔진을 빌드하려면:

__1.__ 터미널에서 언리얼 엔진을 저장하고 싶은 위치로 이동하여 _carla_ 브랜치를 복제하세요:

```sh
    git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git .
```

!!! Note 
    언리얼 엔진 폴더를 가능한 한 `C:\\`에 가깝게 유지하세요. 경로가 일정 길이를 초과하면 3단계에서 `Setup.bat`가 오류를 반환할 수 있습니다.

__2.__ 구성 스크립트를 실행하세요:

```sh
    Setup.bat
    GenerateProjectFiles.bat
```

__3.__ 수정된 엔진을 컴파일하세요:

>1. Visual Studio 2019로 소스 폴더 내의 `UE4.sln` 파일을 여세요.

>2. 빌드 바에서 'Development Editor', 'Win64', 'UnrealBuildTool' 옵션이 선택되어 있는지 확인하세요. 도움이 필요하다면 [이 가이드](https://docs.unrealengine.com/en-US/ProductionPipelines/DevelopmentSetup/BuildingUnrealEngine/index.html)를 참조하세요.
        
>3. 솔루션 탐색기에서 `UE4`를 마우스 오른쪽 버튼으로 클릭하고 `Build`를 선택하세요.

__4.__ 솔루션이 컴파일되면 `Engine\Binaries\Win64\UE4Editor.exe` 실행 파일을 실행하여 모든 것이 올바르게 설치되었는지 확인할 수 있습니다.

!!! Note
    설치가 성공적이었다면 언리얼 엔진의 버전 선택기에서 인식될 것입니다. `.uproject` 파일을 마우스 오른쪽 버튼으로 클릭하고 `Switch Unreal Engine version`을 선택하여 확인할 수 있습니다. 선택한 설치 경로가 있는 `Source Build at PATH`라는 팝업이 표시되어야 합니다. 이 선택기나 `.uproject` 파일을 마우스 오른쪽 버튼으로 클릭했을 때 `Generate Visual Studio project files`가 보이지 않는다면, 언리얼 엔진 설치에 문제가 있었을 것이며 다시 설치해야 할 가능성이 높습니다.

!!! Important
    지금까지 많은 일이 있었습니다. 계속하기 전에 컴퓨터를 재시작하는 것이 매우 권장됩니다.

---
## Part Two: CARLA 빌드

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

### 에셋 가져오기

CARLA 루트 폴더에서 다음 명령어를 실행하여 현재 버전의 CARLA에서 작업할 __최신__ 에셋을 다운로드하세요:

```sh
    Update.bat
```

7zip가 설치되어 있다면 에셋이 다운로드되어 적절한 위치에 압축이 풀립니다. 이 소프트웨어가 설치되어 있지 않다면, 파일 내용을 수동으로 `Unreal\CarlaUE4\Content\Carla`에 압축 해제해야 합니다.

__특정 버전__의 CARLA 에셋을 다운로드하려면:

1. CARLA 루트 디렉토리에서 `\Util\ContentVersions.txt`로 이동하세요. 이 문서에는 모든 CARLA 릴리스의 에셋 링크가 포함되어 있습니다.
2. `Unreal\CarlaUE4\Content\Carla`에 에셋을 압축 해제하세요. 경로가 없다면 생성하세요.
3. 다음과 유사한 명령어로 파일을 압축 해제하세요:

```sh
    tar -xvzf <assets_file_name>.tar.gz.tar -C C:\path\to\carla\Unreal\CarlaUE4\Content\Carla
```

### 언리얼 엔진 환경 변수 설정

CARLA가 언리얼 엔진 설치 폴더를 찾을 수 있도록 환경 변수를 설정해야 합니다. 이를 통해 사용자는 사용할 특정 버전의 언리얼 엔진을 선택할 수 있습니다. 환경 변수가 지정되지 않으면 CARLA는 windows 레지스트리에서 언리얼 엔진을 검색하고 찾은 첫 번째 버전을 사용합니다.

환경 변수를 설정하려면:

1. Windows 제어판을 열고 `고급 시스템 설정`으로 이동하거나 Windows 검색 창에서 `고급 시스템 설정`을 검색하세요.
2. `고급` 패널에서 `환경 변수...`를 엽니다.
3. `새로 만들기...`를 클릭하여 변수를 생성합니다.
4. 변수 이름을 `UE4_ROOT`로 지정하고 원하는 언리얼 엔진 설치의 설치 폴더 경로를 선택하세요.

### CARLA 빌드

이 섹션에서는 CARLA를 빌드하는 명령어를 설명합니다.

- 모든 명령어는 CARLA 루트 폴더에서 실행해야 합니다.
- 명령어는 __x64 Native Tools Command Prompt for VS 2019__를 통해 실행해야 합니다. `Windows` 키를 누르고 `x64`를 검색하여 이를 열 수 있습니다.

CARLA의 빌드 과정은 클라이언트 컴파일과 서버 컴파일, 두 부분으로 이루어져 있습니다.

__1.__ __Python API 클라이언트 컴파일__:

Python API 클라이언트는 시뮬레이션을 제어할 수 있게 해줍니다. Python API 클라이언트 컴파일은 CARLA를 처음 빌드할 때와 업데이트를 수행한 후에 필요합니다. 클라이언트가 컴파일되면 스크립트를 실행하여 시뮬레이션과 상호작용할 수 있습니다.

다음 명령어로 Python API 클라이언트를 컴파일합니다:

```sh
    make PythonAPI
```

CARLA 클라이언트 라이브러리는 상호 배타적인 두 가지 형태로 빌드됩니다. 이를 통해 사용자가 선호하는 형태로 CARLA 클라이언트 코드를 실행할 수 있습니다. 두 형태는 `.egg` 파일과 `.whl` 파일입니다. 클라이언트 라이브러리를 사용하기 위해 다음 옵션 중 __하나__를 선택하세요:

__A. `.egg` 파일__

>`.egg` 파일은 설치가 필요하지 않습니다. CARLA의 모든 예제 스크립트는 CARLA를 임포트할 때 자동으로 [이 파일을 찾습니다](build_system.md#versions-prior-to-0912).

>이전에 CARLA `.whl`을 설치했다면, `.whl`이 `.egg` 파일보다 우선순위가 높습니다.

__B. `.whl` 파일__

>`pip3`를 사용하여 `.whl` 파일을 설치해야 합니다:

```sh
pip3 install <path/to/wheel>.whl
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
        cd PythonAPI\examples
        pip3 install -r requirements.txt
        python3 generate_traffic.py  

        # 터미널 B
        cd PythonAPI\examples
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

이제 CARLA를 빌드했으니, CARLA 빌드를 업데이트하는 방법을 배우거나 시뮬레이션에서 첫 걸음을 떼고 핵심 개념을 배워보세요.

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