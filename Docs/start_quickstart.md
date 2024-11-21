# CARLA 빠른 설치 가이드

이 가이드에서는 CARLA 패키지 버전을 다운로드하고 설치하는 방법을 설명합니다. 패키지에는 CARLA 서버와 두 가지 클라이언트 라이브러리 옵션이 포함되어 있습니다. 추가 에셋도 다운로드하여 패키지에 가져올 수 있습니다. 언리얼 엔진 에디터를 사용해야 하는 고급 커스터마이징과 개발 기능은 패키지 버전에서 사용할 수 없습니다. 이러한 기능을 사용하시려면 [Windows](build_windows.md)나 [Linux](build_linux.md)용 빌드 버전을 이용해주세요.

* __[시작하기 전에](#시작하기-전에)__  
* __[CARLA 설치하기](#carla-설치하기)__  
  * [A. 데비안 패키지로 설치하기](#a-데비안-패키지로-설치하기)  
  * [B. GitHub 패키지로 설치하기](#b-github-패키지로-설치하기)  
* __[추가 에셋 가져오기](#추가-에셋-가져오기)__  
* __[클라이언트 라이브러리 설치](#클라이언트-라이브러리-설치)__
  * [CARLA 0.9.12 이전 버전](#carla-0912-이전-버전)
  * [CARLA 0.9.12 이상 버전](#carla-0912-이상-버전)
* __[CARLA 실행하기](#carla-실행하기)__  
  * [명령줄 옵션](#명령줄-옵션)  
* __[CARLA 업데이트](#carla-업데이트)__    
* __[다음 단계](#다음-단계)__ 

---
## 시작하기 전에

CARLA를 설치하기 전에 다음 요구사항들을 확인해주세요:

* __시스템 요구사항:__ CARLA는 Windows와 Linux 시스템에서 작동합니다.

* __그래픽 카드:__ CARLA는 현실적인 시뮬레이션을 목표로 하기 때문에, 서버에 최소 6GB GPU가 필요하며 8GB를 권장합니다. 머신러닝을 위해서는 전용 GPU를 강력히 추천합니다.

* __저장 공간:__ CARLA는 약 20GB의 저장 공간을 사용합니다.

* __Python:__ CARLA의 주요 스크립트 언어는 [Python](https://www.python.org/downloads/)입니다. Linux에서는 Python 2.7과 3을 모두 지원하며, Windows에서는 Python 3만 지원합니다.

* __Pip:__ CARLA 클라이언트 라이브러리 설치 방법 중 일부는 __pip__ 또는 __pip3__(Python 버전에 따라) 버전 20.3 이상이 필요합니다. 현재 버전 확인 방법:

>>      # Python 3 사용시
>>      pip3 -V

>>      # Python 2 사용시
>>      pip -V

>버전 업그레이드가 필요한 경우:

>>      # Python 3 사용시
>>      pip3 install --upgrade pip

>>      # Python 2 사용시
>>      pip install --upgrade pip

* __TCP 포트:__ 기본적으로 2000번과 2001번 포트를 사용합니다. 방화벽이나 다른 프로그램에서 이 포트들을 차단하고 있지 않은지 확인해주세요. 또한 안정적인 인터넷 연결이 필요합니다.

* __Python 패키지:__ CARLA는 몇 가지 Python 패키지를 필요로 합니다. 운영체제에 따라 다음 명령어로 설치해주세요:

### Windows의 경우

```sh
pip3 install --user pygame numpy
```

### Linux의 경우

```sh
pip install --user pygame numpy &&
pip3 install --user pygame numpy
```

---
## CARLA 설치하기

CARLA를 패키지로 다운로드하고 설치하는 방법에는 두 가지가 있습니다:

__A)__ [데비안 패키지로 설치하기](#a-데비안-패키지로-설치하기)

__B)__ [GitHub에서 패키지 다운로드하여 설치하기](#b-github-패키지로-설치하기)

### A. 데비안 패키지로 설치하기

데비안 패키지는 Ubuntu 18.04와 20.04 모두에서 사용할 수 있지만, __공식 지원 플랫폼은 Ubuntu 18.04__ 입니다.

__1.__ 시스템에 데비안 저장소를 설정합니다:
```sh
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
    sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
```

__2.__ CARLA를 설치하고 `/opt/` 폴더에 설치되었는지 확인합니다:
```sh
    sudo apt-get update # 데비안 패키지 인덱스 업데이트
    sudo apt-get install carla-simulator # 최신 CARLA 버전 설치 또는 현재 설치된 버전 업데이트
    cd /opt/carla-simulator # CARLA가 설치된 폴더로 이동
```

이 저장소에는 CARLA 0.9.10 이상의 버전이 포함되어 있습니다. 특정 버전을 설치하려면 설치 명령어에 버전 태그를 추가하세요:
```sh
    apt-cache madison carla-simulator # 사용 가능한 CARLA 버전 목록 확인
    sudo apt-get install carla-simulator=0.9.10-1 # 여기서 "0.9.10"은 CARLA 버전을, "1"은 데비안 리비전을 의미합니다
```

!!! 중요
    CARLA 0.9.10 이전 버전을 설치하려면, 창 오른쪽 하단의 패널을 사용하여 이전 버전의 문서로 이동한 후 이전 설치 방법을 따라주세요.

### B. GitHub 패키지로 설치하기

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/blob/master/Docs/download.md" target="_blank" class="btn btn-neutral" title="최신 CARLA 릴리스로 이동">
<span class="icon icon-github"></span> CARLA 저장소</a>
</p>
</div>

이 저장소에는 다양한 버전의 CARLA가 있습니다. __최신 릴리스__(모든 최신 수정사항과 기능 포함), __이전 릴리스__, 그리고 개발 중인 모든 수정사항과 기능이 포함된 __나이틀리 빌드__(가장 불안정한 버전의 CARLA)를 다운로드할 수 있습니다.

패키지는 __CARLA_버전.번호__ 형식으로 압축되어 있습니다. 릴리스 파일을 다운로드하고 압축을 풀면 시뮬레이터의 미리 컴파일된 버전, Python API 모듈, 그리고 예제로 사용할 수 있는 스크립트들이 포함되어 있습니다.

---
## 추가 에셋 가져오기

각 릴리스에는 추가 에셋과 맵이 포함된 별도의 패키지가 있습니다. 이 추가 패키지에는 __Town06__과 __Town07__ 맵이 포함되어 있습니다. 빌드 크기를 줄이기 위해 이러한 에셋들은 별도로 저장되어 있어서, 메인 패키지 설치 후에만 가져올 수 있습니다.

__1.__ 사용하고 있는 CARLA 버전에 맞는 패키지를 [다운로드](https://github.com/carla-simulator/carla/blob/master/Docs/download.md)합니다.

__2.__ 패키지 압축을 풉니다:

- __Linux의 경우__:
    - 패키지를 _Import_ 폴더로 이동하고 다음 스크립트를 실행하여 압축을 풉니다:

```sh
        cd path/to/carla/root

        ./ImportAssets.sh
```

- __Windows의 경우__:
    - 내용물을 루트 폴더에 직접 압축 해제합니다.

---

## 클라이언트 라이브러리 설치

### CARLA 0.9.12 이전 버전

이전 버전의 CARLA는 Python 라이브러리 설치가 필요하지 않았으며, 바로 사용할 수 있는 `.egg` 파일이 포함되어 있었습니다. __CARLA 0.9.12 이상 버전에서는 이 방식이 크게 변경되어 클라이언트 라이브러리를 설치하는 여러 옵션이 제공됩니다.__ CARLA 0.9.12 이전 버전을 사용하고 계신다면, 화면 우측 하단에서 해당 버전을 선택하여 관련 문서를 확인해주세요. CARLA 0.9.12 이상 버전을 사용하시는 경우에는 아래에서 사용 가능한 옵션들을 확인해주세요.

### CARLA 0.9.12 이상 버전

CARLA 클라이언트 라이브러리를 설치하고 사용하는 방법에는 세 가지가 있습니다:

- __.egg 파일__
- __.whl 파일__
- __다운로드 가능한 Python 패키지__

각 방법의 요구사항과 제한사항을 확인하신 후 선택해주세요. 여러 방법을 혼용하면 호환성 문제가 발생할 수 있으므로, 가능하면 가상 환경을 사용하시거나 새로운 방법을 사용하기 전에 이전에 설치한 라이브러리를 [제거](build_faq.md#how-do-i-uninstall-the-carla-client-library)해주세요.

>__A. .egg 파일 사용하기__

>>`PythonAPI/carla/dist/` 폴더에 다양한 Python 버전용 `.egg` 파일이 제공됩니다. 이 파일들은 바로 사용할 수 있으며, `PythonAPI/examples`의 모든 예제 스크립트에는 이 파일을 자동으로 찾는 [코드](build_system.md#versions-prior-to-0912)가 포함되어 있습니다. Linux에서는 이 파일을 `PYTHONPATH`에 추가해야 할 수 있습니다. CARLA에서 `.egg` 파일에 대해 자세히 알아보려면 [여기](build_faq.md#importerror-no-module-named-carla)를 참고하세요.

>>__이전에 `pip`로 클라이언트 라이브러리를 설치했다면, 이는 `.egg` 파일보다 우선적으로 적용됩니다.__ 새로운 라이브러리를 사용하기 전에 이전 라이브러리를 먼저 [제거](build_faq.md#how-do-i-uninstall-the-carla-client-library)해야 합니다.

>__B. .whl 파일 사용하기__

>>다양한 Python 버전용 `.whl` 파일이 제공됩니다. `PythonAPI/carla/dist/` 폴더에서 `.whl` 파일을 찾을 수 있습니다. 파일 이름에 지원하는 Python 버전이 표시되어 있습니다(예: carla-0.9.12-__cp36__-cp36m-manylinux_2_27_x86_64.whl은 Python 3.6용입니다).

>>__여러 버전을 사용할 때 충돌을 피하기 위해 가상 환경에서 CARLA 클라이언트 라이브러리를 설치하는 것을 권장합니다.__

>>CARLA 클라이언트 라이브러리를 설치하려면 원하는 Python 버전에 맞는 파일을 선택하고 다음 명령어를 실행하세요. __pip/pip3__ 버전 20.3 이상이 필요합니다. 버전 확인과 업그레이드 방법은 [시작하기 전에](#시작하기-전에) 섹션을 참고하세요:

>>      # Python 3 사용시
>>      pip3 install <wheel-파일-이름>.whl 

>>      # Python 2 사용시
>>      pip install <wheel-파일-이름>.whl

>>이전에 클라이언트 라이브러리를 설치했다면, 새로운 버전을 설치하기 전에 이전 버전을 [제거](build_faq.md#how-do-i-uninstall-the-carla-client-library)해주세요.

>__C. PyPi에서 Python 패키지 다운로드하기__

>>[PyPi](https://pypi.org/project/carla/)에서 CARLA 클라이언트 라이브러리를 다운로드할 수 있습니다. 이 라이브러리는 Python 2.7, 3.6, 3.7, 3.8 버전과 호환됩니다. 설치하려면 __pip/pip3__ 버전 20.3 이상이 필요합니다. 버전 확인과 업그레이드 방법은 [시작하기 전에](#시작하기-전에) 섹션을 참고하세요.

>>__여러 버전을 사용할 때 충돌을 피하기 위해 가상 환경에서 CARLA 클라이언트 라이브러리를 설치하는 것을 권장합니다.__

>>PyPi에서 클라이언트 라이브러리를 설치하려면 다음 명령어를 실행하세요:

>>      # Python 3 사용시
>>      pip3 install carla

>>      # Python 2 사용시
>>      pip install carla

>>PyPi 다운로드는 CARLA 패키지 버전에만 사용할 수 있습니다(소스에서 빌드한 버전에는 사용할 수 없음). PyPi 다운로드는 __클라이언트 라이브러리만 포함__ 하므로, 전체 CARLA 패키지를 다운로드할 필요 없이 원격 CARLA 서버와 통신하는 상황에서 가장 유용합니다.

---
## CARLA 실행하기

CARLA 서버를 시작하는 방법은 설치 방법과 운영체제에 따라 다릅니다:

- 데비안 설치의 경우:

```sh
    cd /opt/carla-simulator/bin/

    ./CarlaUE4.sh
```

- Linux 패키지 설치의 경우:

```sh
    cd path/to/carla/root

    ./CarlaUE4.sh
```

- Windows 패키지 설치의 경우:

```sh
    cd path/to/carla/root

    CarlaUE4.exe
```

도시를 조망할 수 있는 창이 나타날 것입니다. 이것이 _관찰자 뷰_ 입니다. 마우스 오른쪽 버튼을 누른 채로 마우스와 `WASD` 키를 사용하여 도시를 자유롭게 돌아다닐 수 있습니다.

이제 서버 시뮬레이터가 실행되었고 클라이언트의 연결을 기다리고 있습니다. 도시에 생명을 불어넣고 차량을 운전해보려면 다음과 같은 예제 스크립트를 실행해보세요:

```sh
        # 터미널 A 
        cd PythonAPI\examples

        python3 -m pip install -r requirements.txt # CARLA 릴리스 패키지에서는 Python2도 지원됩니다

        python3 generate_traffic.py  

        # 터미널 B
        cd PythonAPI\examples

        python3 manual_control.py 
```

#### 명령줄 옵션

CARLA를 실행할 때 사용할 수 있는 설정 옵션들이 있습니다. 다음과 같이 사용할 수 있습니다:

```sh
    ./CarlaUE4.sh -carla-rpc-port=3000
```

* `-carla-rpc-port=N` `N`번 포트에서 클라이언트 연결을 대기합니다. 스트리밍 포트는 기본적으로 `N+1`로 설정됩니다.  
* `-carla-streaming-port=N` 센서 데이터 스트리밍용 포트를 지정합니다. 0을 입력하면 사용하지 않는 임의의 포트가 할당됩니다. 두 번째 포트는 자동으로 `N+1`로 설정됩니다.  
* `-quality-level={Low,Epic}` 그래픽 품질 수준을 변경합니다. 자세한 내용은 [렌더링 옵션](adv_rendering_options.md)을 참고하세요.  
* __[언리얼 엔진 4 명령줄 인수 목록][ue4clilink].__ 언리얼 엔진에서 제공하는 많은 옵션이 있지만, CARLA에서는 일부만 사용할 수 있습니다.  

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments

서버가 시작된 후에는 [`PythonAPI/util/config.py`][config] 스크립트를 통해 더 많은 설정 옵션을 사용할 수 있습니다:

[config]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/config.py

```sh
    ./config.py --no-rendering      # 렌더링 비활성화
    ./config.py --map Town05        # 맵 변경
    ./config.py --weather ClearNoon # 날씨 변경

    ./config.py --help # 사용 가능한 모든 설정 옵션 확인
```

---
## CARLA 업데이트

패키지 버전의 CARLA는 직접 업데이트할 수 없습니다. 새 버전이 출시되면 저장소가 업데이트되므로, 이전 버전을 삭제하고 새 버전을 설치해야 합니다.

__pip/pip3__ 로 클라이언트 라이브러리를 설치한 경우, 다음 명령어로 제거할 수 있습니다:

```sh
# Python 3 사용시
pip3 uninstall carla

# Python 2 사용시
pip uninstall carla
```

---
## 다음 단계

이제 CARLA 패키지 버전을 설치하고 실행하는 방법을 알게 되었습니다. 설치 과정에서 어려움이 있으셨다면 [CARLA 포럼](https://github.com/carla-simulator/carla/discussions/)이나 [Discord](https://discord.gg/8kqACuC) 채널에 문의해주세요.

다음 단계는 CARLA의 핵심 개념들을 배우는 것입니다. __시작하기__ 섹션을 읽고 학습을 시작해보세요. 또한 [Python API 레퍼런스](python_api.md)에서 Python API 클래스와 메서드에 대한 모든 정보를 찾아볼 수 있습니다.

<div class="build-buttons">
<p>
<a href="../core_concepts" target="_blank" class="btn btn-neutral" title="시작하기로 이동">
다음: 시작하기</a>
</p>
</div>