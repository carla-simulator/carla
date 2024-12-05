# 빌드 시스템

* [__설정__](#setup)  
* [__LibCarla__](#libcarla)  
* [__CarlaUE4와 Carla 플러그인__](#carlaue4-and-carla-plugin)  
* [__PythonAPI__](#pythonapi)
    - [0.9.12+ 버전](#versions-0912)
    - [0.9.12 이전 버전](#versions-prior-to-0912)

> _이 문서는 작성 중이며, 현재는 Linux 빌드 시스템만을 다루고 있습니다._

설정에서 가장 어려운 부분은 모든 의존성과 모듈을 a) 서버 측의 언리얼 엔진과 b) 클라이언트 측의 Python과 호환되도록 컴파일하는 것입니다.

목표는 별도의 Python 프로세스에서 언리얼 엔진의 함수를 호출할 수 있도록 하는 것입니다.

![modules](img/build_modules.jpg)

Linux에서는 CARLA와 모든 의존성을 clang-8.0과 C++14 표준으로 컴파일합니다. 하지만 코드가 사용될 위치에 따라 다른 런타임 C++ 라이브러리와 링크합니다. 언리얼 엔진과 링크될 모든 코드는 `libc++`를 사용하여 컴파일해야 하기 때문입니다.

---
## 설정

명령어

```sh
make setup
```

의존성 가져오기 및 컴파일

  * llvm-8 (libc++ 및 libc++abi)
  * rpclib-2.2.1 (libstdc++와 libc++로 두 번)
  * boost-1.72.0 (libstdc++용 헤더와 boost_python)
  * googletest-1.8.1 (libc++로)

---
## LibCarla

CMake로 컴파일됩니다(최소 요구 버전 CMake 3.9).

명령어

```sh
make LibCarla
```

두 가지 구성:

|  | 서버 | 클라이언트 |
| ---------- | ---------- | ---------- |
| **단위 테스트**        | 예                    | 아니오                 |
| **요구사항**           | rpclib, gtest, boost  | rpclib, boost         |
| **std 런타임**         | LLVM의 `libc++`       | 기본 `libstdc++`      |
| **출력**               | 헤더와 테스트 실행 파일 | `libcarla_client.a`   |
| **다음에서 필요**      | Carla 플러그인         | PythonAPI            |

---
## CarlaUE4와 Carla 플러그인

둘 다 언리얼 엔진 빌드 도구로 같은 단계에서 컴파일됩니다. `UE4_ROOT` 환경 변수 설정이 필요합니다.

명령어

```sh
make CarlaUE4Editor
```

언리얼 엔진의 에디터를 실행하려면

```sh
make launch
```

---
## PythonAPI
### 0.9.12+ 버전

Python의 `setuptools`("setup.py")를 사용하여 컴파일됩니다. 현재 시스템에 다음이 설치되어 있어야 합니다: Python, libpython-dev, libboost-python-dev, pip>=20.3, wheel, auditwheel.

명령어:

```sh
make PythonAPI
```

시스템에서 지원하는 Python 버전에 해당하는 클라이언트 라이브러리를 포함하는 두 개의 파일을 생성합니다. 하나는 `.whl` 파일이고 다른 하나는 `.egg` 파일입니다. 이를 통해 클라이언트 라이브러리를 사용하는 두 가지 상호 배타적인 방법 중 하나를 선택할 수 있습니다.

>__A. .whl 파일__

>>`.whl`은 다음 명령어로 설치됩니다:

>>      pip install <wheel_file>.whl

>>이전 버전이나 `.egg` 파일에서 필요했던 것처럼([__0.9.12 이전 버전__](#versions-prior-to-0912) 섹션 참조) 스크립트에서 라이브러리 경로를 직접 임포트할 필요가 없습니다; `import carla`만으로 충분합니다.

>__B. .egg 파일__

>>[__0.9.12 이전 버전__](#versions-prior-to-0912) 섹션에서 자세한 정보를 확인하세요.

### 0.9.12 이전 버전

Python의 `setuptools`("setup.py")를 사용하여 컴파일됩니다. 현재 시스템에 다음이 설치되어 있어야 합니다: Python, libpython-dev, libboost-python-dev.

명령어

```sh
make PythonAPI
```

두 개의 "egg" 패키지를 생성합니다

  * `PythonAPI/dist/carla-X.X.X-py2.7-linux-x86_64.egg`
  * `PythonAPI/dist/carla-X.X.X-py3.7-linux-x86_64.egg`

이 패키지는 시스템 경로에 추가하여 Python 스크립트에서 직접 임포트할 수 있습니다.

```python
#!/usr/bin/env python

import sys

sys.path.append(
    'PythonAPI/dist/carla-X.X.X-py%d.%d-linux-x86_64.egg' % (sys.version_info.major,
                                                             sys.version_info.minor))

import carla

# ...
```

또는 `easy_install`로 설치할 수 있습니다

```sh
easy_install2 --user --no-deps PythonAPI/dist/carla-X.X.X-py2.7-linux-x86_64.egg
easy_install3 --user --no-deps PythonAPI/dist/carla-X.X.X-py3.7-linux-x86_64.egg
```