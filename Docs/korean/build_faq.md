# 자주 묻는 질문

CARLA 설치 및 빌드와 관련된 가장 일반적인 문제들을 여기에 정리했습니다. 프로젝트의 [GitHub issues](https://github.com/carla-simulator/carla/issues?utf8=%E2%9C%93&q=label%3Aquestion+)에서 더 많은 내용을 찾을 수 있습니다. 여기에서 궁금한 점을 찾지 못했다면, 포럼을 확인하고 자유롭게 질문해 주세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>

---

## 시스템 요구사항

* [CARLA 빌드에 필요한 디스크 공간](#expected-disk-space-to-build-carla)
* [CARLA 실행에 권장되는 하드웨어](#recommended-hardware-to-run-carla)

---

## Linux 빌드

* ["CarlaUE4.sh" 스크립트가 GitHub에서 다운로드할 때 보이지 않음](#carlaue4sh-script-does-not-appear-when-downloading-from-github)
* ["make launch"가 Linux에서 작동하지 않음](#make-launch-is-not-working-on-linux)
* [언리얼 엔진 저장소 복제 시 오류 발생](#cloning-the-unreal-engine-repository-shows-an-error)
* [스크립트 실행 시 AttributeError: module 'carla' has no attribute 'Client' 오류 발생](#attributeerror-module-carla-has-no-attribute-client-when-running-a-script)
* [예제 스크립트를 실행할 수 없거나 "RuntimeError: rpc::rpc_error during call in function version" 오류 발생](#cannot-run-example-scripts-or-runtimeerror-rpcrpc_error-during-call-in-function-version)

---

## Windows 빌드

* ["CarlaUE4.exe"가 GitHub에서 다운로드할 때 보이지 않음](#carlaue4exe-does-not-appear-when-downloading-from-github)
* [CarlaUE4를 컴파일할 수 없음. 소스에서 수동으로 재빌드 시도 필요](#carlaue4-could-not-be-compiled-try-rebuilding-it-from-source-manually)
* [CMake가 제대로 설치되어 있는데도 CMake 오류 발생](#cmake-error-shows-even-though-cmake-is-properly-installed)
* [Error C2440, C2672: 컴파일러 버전 문제](#error-c2440-c2672-compiler-version)
* ["make launch"가 Windows에서 작동하지 않음](#make-launch-is-not-working-on-windows)
* [Make에서 libintl3.dll 또는/및 libiconv2.dll 누락](#make-is-missing-libintl3dll-orand-libiconv2dll)
* [모듈이 누락되었거나 다른 엔진 버전으로 빌드됨](#modules-are-missing-or-built-with-a-different-engine-version)
* [성공 메시지에도 불구하고 `PythonAPI/carla`에 `dist` 폴더가 없음](#there-is-no-dist-folder-in-pythonapicarla-despite-a-successful-output-message)

---

## CARLA 실행

* [언리얼 에디터에서 서버 실행 시 낮은 FPS](#low-fps-rate-when-running-the-server-in-unreal-editor)
* [스크립트를 실행할 수 없음](#cant-run-a-script)
* [언리얼 에디터 내에서 실행 중일 때 시뮬레이터에 연결](#connect-to-the-simulator-while-running-within-unreal-editor)
* [바이너리와 소스 빌드 모두 CARLA를 실행할 수 없음](#cant-run-carla-neither-binary-nor-source-build)
* [ImportError: DLL load failed: 지정된 모듈을 찾을 수 없음](#importerror-dll-load-failed-the-specified-module-could-not-be-found)
* [ImportError: DLL load failed while importing libcarla: %1이 유효한 Win32 앱이 아님](#importerror-dll-load-failed-while-importing-libcarla-1-is-not-a-valid-win32-app)
* [ImportError: 'carla' 모듈이 없음](#importerror-no-module-named-carla)

---

## 기타

* [Fatal error: 'version.h'가 미리 컴파일된 헤더 이후로 수정됨](#fatal-error-versionh-has-been-modified-since-the-precompiled-header)
* [CARLA의 바이너리 버전 생성](#create-a-binary-version-of-carla)
* [Linux 머신에서 Windows용 CARLA를 패키징하거나 그 반대로 할 수 있나요?](#can-i-package-carla-for-windows-on-a-linux-machine-and-vice-versa)
* [CARLA 클라이언트 라이브러리를 어떻게 제거하나요?](#how-do-i-uninstall-the-carla-client-library)

---

###### CARLA 빌드에 필요한 디스크 공간

> 최소 170GB의 여유 공간이 권장됩니다. CARLA 빌드에는 약 35GB의 디스크 공간이 필요하며, 언리얼 엔진은 약 95-135GB가 필요합니다.

###### CARLA 실행에 권장되는 하드웨어

> CARLA는 높은 성능을 요구하는 소프트웨어입니다. 최소한 6GB GPU가 필요하며, 가능하면 언리얼 엔진을 실행할 수 있는 전용 GPU를 사용하는 것이 좋습니다.
>
> [언리얼 엔진의 권장 하드웨어](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine)를 참고하세요.

###### "CarlaUE4.sh" 스크립트가 GitHub에서 다운로드할 때 보이지 않음

> CARLA 소스 버전에는 `CarlaUE4.sh` 스크립트가 포함되어 있지 않습니다. 소스에서 CARLA를 빌드하려면 [빌드 지침](build_linux.md)을 따르세요.
> 
> `CarlaUE4.sh`를 사용하여 CARLA를 실행하려면 [빠른 시작 설치](start_quickstart.md)를 따르세요.

###### Linux에서 "make launch"가 작동하지 않음

> 빌드 설치 과정에서 여러 가지 문제가 발생할 수 있으며, 이런 방식으로 나타날 수 있습니다. 가장 가능성이 높은 원인들은 다음과 같습니다:
> 
> * __언리얼 엔진 4.26 실행.__ 언리얼 엔진 빌드 중에 문제가 발생했을 수 있습니다. UE 에디터를 단독으로 실행해보고 4.26 버전인지 확인하세요.
> * __에셋 다운로드.__ 시각적 콘텐츠 없이는 서버가 실행될 수 없습니다. 이 단계는 필수입니다.
> * __UE4_ROOT가 정의되지 않음.__ 환경 변수가 설정되지 않았습니다. 세션 전반에 걸쳐 지속되도록 `~/.bashrc` 또는 `~/.profile`에 추가하는 것을 잊지 마세요. 그렇지 않으면 새로운 셸마다 설정해야 합니다. `export UE4_ROOT=<path_to_unreal_4-26>`를 실행하여 이번에 변수를 설정하세요.
> * __의존성 확인.__ 모든 것이 제대로 설치되었는지 확인하세요. 명령어를 건너뛰었거나, 실패했거나, 시스템에 맞지 않는 의존성이 있을 수 있습니다.
> * __CARLA 삭제 후 재복제.__ 문제가 있을 경우를 대비하여, CARLA를 삭제하고 다시 복제하거나 다운로드하세요.
> * __시스템 요구사항 충족.__ Ubuntu 버전은 16.04 이상이어야 합니다. CARLA는 실행을 위해 약 170GB의 디스크 공간과 전용 GPU(또는 최소 6GB)가 필요합니다.
> 
> 시스템별로 CARLA와 충돌을 일으키는 다른 특정 원인이 있을 수 있습니다. 이러한 문제들을 [포럼](https://github.com/carla-simulator/carla/discussions/)에 게시해 주시면 팀이 더 많이 알 수 있습니다.

###### 언리얼 엔진 저장소 복제 시 오류 발생

> __1. 언리얼 엔진 계정이 활성화되어 있나요?__ UE 저장소는 비공개입니다. 복제하려면 [UE](https://www.unrealengine.com/en-US/) 계정을 만들고, 활성화하고(확인 메일 확인), [GitHub 계정을 연결](https://www.unrealengine.com/en-US/blog/updated-authentication-process-for-connecting-epic-github-accounts)해야 합니다.
> 
> __2. git이 제대로 설치되어 있나요?__ 때로는 `https` 프로토콜과의 비호환성을 보여주는 오류가 발생합니다. git을 제거하고 다시 설치하면 쉽게 해결할 수 있습니다. 터미널을 열고 다음 명령어를 실행하세요:
>
>      sudo apt-get remove git #git 제거
>      sudo apt install git-all #git 설치

###### 스크립트 실행 시 AttributeError: module 'carla' has no attribute 'Client' 오류 발생

> 다음 명령어를 실행하세요.
> 
>       pip3 install -Iv setuptools==47.3.1
>  
> 그리고 PythonAPI를 다시 빌드하세요.
> 
>      make PythonAPI
>
> 모든 것이 제대로 실행되고 있는지 테스트하기 위해 문서를 빌드해보세요. 성공 메시지가 표시되어야 합니다.
>
>      make PythonAPI.docs

###### 예제 스크립트를 실행할 수 없거나 "RuntimeError: rpc::rpc_error during call in function version" 오류 발생

> ![faq_rpc_error](img/faq_rpc_error.jpg)
>
> 스크립트 실행 시 이와 유사한 출력이 나타난다면, PythonAPI의 `.egg` 파일에 문제가 있는 것입니다.

!!! Important
    0.9.12+ 버전을 사용하는 경우, 클라이언트 라이브러리를 사용/설치하는 여러 가지 방법이 있습니다. 클라이언트 라이브러리의 새로운 방법(`.whl` 또는 PyPi 다운로드)을 사용하고 있다면 이 섹션의 정보는 해당되지 않습니다.
> 
> 먼저 `<root_carla>/PythonAPI/carla/dist`를 여세요. 사용 중인 CARLA와 Python 버전에 맞는 `.egg` 파일(`carla-0.X.X-pyX.X-linux-x86_64.egg`와 유사)이 있어야 합니다. 파일이 사용 중인 Python 버전과 일치하는지 확인하세요. Python 버전을 확인하려면 다음 명령어를 사용하세요.
> 
>       python3 --version
>       # 또는 Python 2의 경우
>       python --version
> 
> 파일이 누락되었거나 손상되었다고 생각되면 다시 빌드해보세요.
>
>      make clean
>      make PythonAPI
>      make launch
>
> 이제 예제 스크립트를 다시 시도해보세요.
>
>      cd PythonAPI/examples
>      python3 dynamic_weather.py
>
> 오류가 계속되면 문제가 PythonPATH와 관련이 있을 수 있습니다. 이 스크립트들은 빌드와 관련된 `.egg` 파일을 자동으로 찾기 때문에, PythonPATH에 있는 다른 `.egg` 파일이 프로세스를 방해하고 있을 수 있습니다. 다음 명령어로 PythonPATH의 내용을 확인하세요.

>      echo $PYTHONPATH
>
> 출력에서 `PythonAPI/carla/dist`와 유사한 경로에 있는 다른 `.egg` 파일 인스턴스를 찾아 제거하세요. 이들은 아마도 다른 CARLA 설치에 속할 것입니다. 예를 들어, *apt-get*을 통해 CARLA를 설치한 경우 다음 명령어로 제거할 수 있으며, PythonPATH도 정리될 것입니다.
>
>      sudo apt-get purge carla-simulator
>
> 최종적으로 빌드의 `.egg` 파일을 `~/.bashrc`를 사용하여 PythonPATH에 추가하는 옵션이 있습니다. 이는 권장되는 방법이 아닙니다. PythonPATH를 깨끗하게 유지하고 스크립트에서 필요한 `.egg` 파일의 경로만 추가하는 것이 더 좋습니다.
>
> 먼저 `~/.bashrc`를 엽니다.
>
>      gedit ~/.bashrc
>
> `~/.bashrc`에 다음 줄을 추가하세요. 이는 Python이 자동으로 찾을 수 있도록 빌드 `.egg` 파일의 경로를 저장합니다. 파일을 저장하고 변경사항을 적용하기 위해 터미널을 재시작하세요.
>
> ```
> export PYTHONPATH=$PYTHONPATH:"${CARLA_ROOT}/PythonAPI/carla/dist/$(ls ${CARLA_ROOT}/PythonAPI/carla/dist | grep py3.)"
> export PYTHONPATH=$PYTHONPATH:${CARLA_ROOT}/PythonAPI/carla
> ```
> 
> PythonPATH를 정리하거나 빌드 `.egg` 파일의 경로를 추가한 후에는 모든 예제 스크립트가 제대로 작동해야 합니다.

###### "CarlaUE4.exe"가 GitHub에서 다운로드할 때 보이지 않음

> CARLA 소스 버전에는 `CarlaUE4.exe` 실행 파일이 없습니다. 소스에서 CARLA를 빌드하려면 [빌드 지침](build_windows.md)을 따르세요. `CarlaUE4.exe`를 직접 얻으려면 [빠른 시작 지침](start_quickstart.md)을 따르세요.

###### CarlaUE4를 컴파일할 수 없음. 소스에서 수동으로 재빌드 시도 필요

> CARLA를 빌드하려고 할 때 문제가 발생했습니다. Visual Studio를 사용하여 재빌드하면 무엇이 잘못되었는지 알 수 있습니다.
>
> __1.__ `carla/Unreal/CarlaUE4`로 이동하여 `CarlaUE4.uproject`를 마우스 오른쪽 버튼으로 클릭하세요.
> __2.__ __Generate Visual Studio project files__를 클릭하세요.
> __3.__ 생성된 파일을 Visual Studio 2019로 여세요.
> __4.__ Visual Studio에서 프로젝트를 컴파일하세요. 단축키는 F7입니다. 빌드가 실패하겠지만, 아래에 발견된 문제들이 표시될 것입니다.
>
> 이 특정 오류 메시지가 나타날 수 있는 다양한 문제가 있습니다. 사용자 [@tamakoji](https://github.com/tamakoji)가 소스 코드가 제대로 복제되지 않아 CARLA 버전을 설정할 수 없는 반복적인 케이스를 해결했습니다(git에서 .zip으로 다운로드할 때).
>
> * __`Build/CMakeLists.txt.in`을 확인하세요.__ `set(CARLA_VERSION )`으로 표시되면 다음을 수행하세요:
>
> __1.__ `Setup.bat` 198번 줄로 이동하세요.
>
> __2.__ 다음 줄을: 
> 
> ```
> for /f %%i in ('git describe --tags --dirty --always') do set carla_version=%%i
> ```
> 
> 다음과 같이 업데이트하세요:
> 
> ```
> for /f %%i in ('git describe --tags --dirty --always') do set carla_version="0.9.9"
> ```

###### CMake가 제대로 설치되어 있는데도 CMake 오류 발생

> 이 문제는 서버나 클라이언트를 빌드하기 위해 _make_ 명령어를 사용할 때 발생합니다. CMake가 설치되어 있고, 업데이트되어 있고, 환경 경로에 추가되어 있더라도 발생할 수 있습니다. Visual Studio 버전 간의 충돌이 있을 수 있습니다.
>
> VS2019만 남기고 나머지는 완전히 제거하세요.

###### Error C2440, C2672: 컴파일러 버전

> 다른 Visual Studio 또는 Microsoft 컴파일러 버전과의 충돌로 인해 빌드가 2019 컴파일러를 사용하지 않습니다. 이들을 제거하고 다시 빌드하세요.
>
> Visual Studio는 자체 제거가 잘 되지 않습니다. 컴퓨터에서 Visual Studio를 완전히 제거하려면 `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout`로 이동하여 `.\InstallCleanup.exe -full`을 실행하세요. 관리자 권한이 필요할 수 있습니다.
>
> 다른 Visual Studio 버전을 유지하려면 ```%appdata%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml```을 편집하여 다음 줄을 추가하세요:
>
> ```
>  <VCProjectFileGenerator>
>    <Version>VisualStudio2019</Version>
> </VCProjectFileGenerator>
> ```
> 
> ``` 
>  <WindowsPlatform>
>    <Compiler>VisualStudio2019</Compiler>
> </WindowsPlatform>
> ```  

###### Windows에서 "make launch"가 작동하지 않음

> 빌드 설치 중에 여러 가지 문제가 발생할 수 있으며, 이런 방식으로 나타날 수 있습니다. 가장 가능성이 높은 원인들은 다음과 같습니다:
> 
> * __컴퓨터 재시작.__ Windows 빌드 중에는 많은 일이 일어납니다. 재시작하고 모든 것이 제대로 업데이트되었는지 확인하세요.
> * __언리얼 엔진 4.26 실행.__ 언리얼 엔진 빌드 중에 문제가 발생했을 수 있습니다. 에디터를 실행하고 4.26 버전이 사용되고 있는지 확인하세요.
> * __에셋 다운로드.__ 시각적 콘텐츠 없이는 서버가 실행될 수 없습니다. 이 단계는 필수입니다.
> * __Visual Studio 2019.__ 다른 Visual Studio 버전이 설치되어 있거나 최근에 제거된 경우 충돌이 발생할 수 있습니다. 컴퓨터에서 Visual Studio를 완전히 제거하려면 `Program Files (x86)\Microsoft Visual Studio\Installer\resources\app\layout`로 이동하여 `.\InstallCleanup.exe -full`을 실행하세요.
> * __CARLA 삭제 후 재복제.__ 문제가 있을 경우를 대비하여, CARLA를 삭제하고 다시 복제하거나 다운로드하세요.
> * __시스템 요구사항 충족.__ CARLA는 실행을 위해 약 170GB의 디스크 공간과 전용 GPU(또는 최소 6GB)가 필요합니다.
>
> 시스템별로 CARLA와 충돌을 일으키는 다른 특정 원인이 있을 수 있습니다. 이러한 문제들을 [포럼](https://github.com/carla-simulator/carla/discussions/)에 게시해 주시면 팀이 더 많이 알 수 있습니다.

###### Make에서 libintl3.dll 또는/및 libiconv2.dll 누락

> [의존성](http://gnuwin32.sourceforge.net/downlinks/make-dep-zip.php)을 다운로드하고 `bin` 내용을 `make` 설치 경로에 압축 해제하세요.

###### 모듈이 누락되었거나 다른 엔진 버전으로 빌드됨

> __Accept__를 클릭하여 재빌드하세요.

###### 성공 메시지에도 불구하고 `PythonAPI/carla`에 `dist` 폴더가 없음

> Windows에서 `make PythonAPI` 명령은 실제로는 설치되지 않았을 때도 Python API가 성공적으로 설치되었다는 메시지를 반환할 수 있습니다. `PythonAPI/carla` 디렉토리에 이 출력 후 `dist` 폴더가 생성되지 않았다면, 명령 출력의 위쪽을 살펴보세요. 오류가 발생했을 가능성이 높으며, 오류를 수정하고 `make clean`을 실행한 후 빌드를 다시 시도해야 합니다.

###### 언리얼 에디터에서 서버 실행 시 낮은 FPS

> UE4 에디터는 포커스를 벗어나면 낮은 성능 모드로 전환됩니다.
>
> 에디터 설정의 `Edit/Editor Preferences/Performance`로 이동하여 "Use Less CPU When in Background" 옵션을 비활성화하세요.

###### 스크립트를 실행할 수 없음

> 일부 스크립트에는 요구사항이 있습니다. 이러한 요구사항은 스크립트 자체와 동일한 경로에 있는 __Requirements.txt__ 파일에 나열되어 있습니다. 스크립트를 실행하기 위해 이를 확인하세요. 대부분은 간단한 `pip` 명령으로 설치할 수 있습니다.
>
> Windows에서는 때때로 스크립트가 `> script_name.py`만으로 실행되지 않을 수 있습니다. `> python3 script_name.py`를 추가해보고, 올바른 디렉토리에 있는지 확인하세요.

###### 언리얼 에디터 내에서 실행 중일 때 시뮬레이터에 연결

> __Play__를 클릭하고 장면이 로드될 때까지 기다리세요. 그 시점에서 Python 클라이언트는 독립 실행형 시뮬레이터와 마찬가지로 시뮬레이터에 연결할 수 있습니다.

###### 바이너리와 소스 빌드 모두 CARLA를 실행할 수 없음

> NVIDIA 드라이버가 오래되었을 수 있습니다. 이것이 문제가 아닌지 확인하세요. 문제가 여전히 해결되지 않으면 [포럼](https://github.com/carla-simulator/carla/discussions/)을 확인하고 특정 문제를 게시하세요.

###### ImportError: DLL load failed: 지정된 모듈을 찾을 수 없음

> 필요한 라이브러리 중 하나가 제대로 설치되지 않았습니다. 해결 방법으로, `carla\Build\zlib-source\build`로 이동하여 `zlib.dll`이라는 파일을 스크립트의 디렉토리에 복사하세요.

###### ImportError: DLL load failed while importing libcarla: %1이 유효한 Win32 앱이 아님

> 32비트 Python 버전이 스크립트를 실행하려고 할 때 충돌을 일으키고 있습니다. 이를 제거하고 필요한 Python3 x64만 남겨두세요.

###### ImportError: 'carla' 모듈이 없음

> 이 오류는 Python이 CARLA 라이브러리를 찾을 수 없을 때 발생합니다. CARLA 라이브러리는 `PythonAPI/carla/dist` 디렉토리에 있는 `.egg` 파일에 포함되어 있으며, 모든 예제 스크립트는 이 디렉토리에서 이를 찾습니다. `.egg` 파일은 `carla-<carla-version>-py<python-version>-<operating-system>.egg` 형식을 따릅니다.

!!! Important
    CARLA는 0.9.12 이전 버전에서만 클라이언트 라이브러리에 `.egg` 파일을 사용했습니다. 0.9.12+ 버전을 사용하는 경우, 클라이언트 라이브러리를 사용/설치하는 여러 가지 방법이 있습니다. 클라이언트 라이브러리의 새로운 방법(`.whl` 또는 PyPi 다운로드)을 사용하고 있다면 이 섹션의 정보는 해당되지 않습니다.

    클라이언트 라이브러리를 사용/설치하는 새로운 방법에 대해 [__빠른 시작 튜토리얼__](start_quickstart.md#carla-0912)에서 자세히 알아보세요.

> CARLA의 패키지 버전을 사용하는 경우, CARLA 버전에 따라 Python의 다양한 버전에 해당하는 여러 `.egg` 파일이 있을 것입니다. 이러한 Python 버전 중 하나로 스크립트를 실행하고 있는지 확인하세요. 기본 Python 버전을 확인하려면 명령줄에 다음을 입력하세요:
>
>       python3 --version
>       # 또는
>       python --version 

> 소스에서 Python을 빌드한 경우, `.egg` 파일은 시스템의 기본 Python 버전에 따라 빌드됩니다. Linux에서는 다음 명령어의 기본 Python 버전이 됩니다:

>       /usr/bin/env python3 --version
>       # 또는 ARGS="--python-version=2"를 지정한 경우
>       /usr/bin/env python2 --version

> Windows에서는 다음의 기본 Python 버전이 됩니다:

>       python --version

> `.egg` 파일에 해당하는 Python 버전으로 스크립트를 실행하고 있는지 확인하세요.
> Linux에서는 CARLA `.egg`를 가리키도록 Python 경로를 설정해야 할 수도 있습니다. 이를 위해 다음 명령어를 실행하세요:

>       export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/carla/dist/<your_egg_file>
>       # CARLA를 찾을 수 있는지 확인
>       python3 -c 'import carla;print("Success")'

> 가상 환경이나 Conda와 같은 다른 Python 환경이 CARLA 설치를 복잡하게 만들 수 있다는 점에 유의하세요. Python 기본값과 경로를 적절히 설정했는지 확인하세요.

###### Fatal error: 'version.h'가 미리 컴파일된 헤더 이후로 수정됨

> 이는 Linux 업데이트로 인해 때때로 발생합니다. Makefile에 이 문제를 위한 특별한 대상이 있습니다. 시간이 오래 걸리지만 문제를 해결합니다:
>
>      make hard-clean
>      make CarlaUE4Editor

###### CARLA의 바이너리 버전 생성

> Linux에서는 프로젝트 폴더에서 `make package`를 실행하세요. 패키지에는 프로젝트와 Python API 모듈이 포함됩니다.
>
> 또는 언리얼 에디터 내에서 CARLA의 바이너리 버전을 컴파일할 수 있습니다. CarlaUE4 프로젝트를 열고, `File/Package Project` 메뉴로 이동한 다음 플랫폼을 선택하세요. 시간이 좀 걸릴 수 있습니다.

###### Linux 머신에서 Windows용 CARLA를 패키징하거나 그 반대로 할 수 있나요?

> 이 기능은 언리얼 엔진에서는 사용할 수 있지만 CARLA에서는 사용할 수 없습니다. 크로스 컴파일을 지원하지 않는 여러 의존성이 있습니다.

###### CARLA 클라이언트 라이브러리를 어떻게 제거하나요?

> __pip/pip3__를 사용하여 클라이언트 라이브러리를 설치한 경우 다음을 실행하여 제거해야 합니다:

```sh
# Python 3
pip3 uninstall carla

# Python 2
pip uninstall carla
```

---