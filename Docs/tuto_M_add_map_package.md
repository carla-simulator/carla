# CARLA 패키지에서 맵 가져오기

이 섹션에서는 __CARLA의 패키지(바이너리) 버전__에 맵을 가져오는 과정을 설명합니다. 소스에서 빌드된 CARLA 버전을 사용하여 맵을 가져오려면 대신 [여기][source_ingest]의 가이드라인을 따르세요.

이 프로세스는 Linux 시스템에서만 사용할 수 있습니다. 가져오기 과정에서는 관련 파일들을 가져오고 독립 실행형 패키지로 내보내기 위해 언리얼 엔진의 Docker 이미지를 실행해야 하며, 이후 CARLA에서 사용할 수 있도록 구성할 수 있습니다. Docker 이미지를 처음 빌드할 때는 약 4시간과 600-700GB의 공간이 필요합니다.

- [__시작하기 전에__](#before-you-begin)
- [__CARLA 패키지에서 맵 가져오기__](#map-ingestion-in-a-carla-package)

---

## 시작하기 전에

- 다음과 같은 시스템 요구 사항을 충족해야 합니다:
    - Ubuntu 16.04+ 에서 64비트 버전의 [Docker](https://docs.docker.com/engine/install/)
    - 최소 8GB RAM
    - 컨테이너 이미지 빌드를 위한 최소 700GB의 여유 디스크 공간
    - [Git](https://git-scm.com/downloads) 버전 관리
- CARLA의 패키지(바이너리) 버전을 사용하고 있는지 확인하세요. 소스에서 빌드된 CARLA 버전을 사용하여 맵을 가져오려면 대신 [여기][source_ingest]의 가이드라인을 따르세요.
- RoadRunner와 같은 맵 편집기에서 [생성된][rr_generate_map] `<맵이름>.xodr`과 `<맵이름>.fbx` 파일이 최소한 두 개 있어야 합니다.
- 이 파일들은 동일한 맵으로 인식되기 위해 `<맵이름>` 값이 동일해야 합니다.

[source_ingest]: tuto_M_add_map_source.md
[import_map_package]: tuto_M_add_map_package.md
[rr_generate_map]: tuto_M_generate_map.md

---
## CARLA 패키지에서 맵 가져오기

__1.__ CARLA는 Docker 이미지에서 언리얼 엔진을 빌드하고 해당 이미지를 사용하여 CARLA를 컴파일하는 모든 유틸리티를 제공합니다. 이 도구들은 GitHub에서 제공되는 소스 코드에서 찾을 수 있습니다. 다음 명령어를 사용하여 저장소를 클론하세요:

```sh
    git clone https://github.com/carla-simulator/carla
```

__2.__ [이 지침](https://github.com/carla-simulator/carla/tree/master/Util/Docker)을 따라 언리얼 엔진의 Docker 이미지를 빌드하세요.

__3.__ `input_folder`를 생성하세요. 여기에 가져올 파일들을 넣게 됩니다. Docker는 패키지 폴더 구조를 설명하는 `.json` 파일을 자동으로 생성합니다. 이 파일이 성공적으로 생성되도록 `input_folder`의 권한을 변경하세요:

```sh
    #input 폴더가 포함된 상위 폴더로 이동
    chmod 777 input_folder
```

> !!! 참고
    패키지가 [수동으로 준비](tuto_M_manual_map_package.md)되어 있고 이미 `.json` 파일을 포함하고 있다면 이 과정은 필요하지 않습니다.

__4.__ `output_folder`를 생성하세요. Docker 이미지가 맵을 처리한 후 출력 파일을 작성할 위치입니다.

__5.__ `~/carla/Util/Docker`로 이동하세요. 여기에 가져오기 스크립트가 있습니다. 스크립트는 `input_folder`와 `output_folder`의 경로, 그리고 가져올 패키지의 이름이 필요합니다. `.json` 파일이 제공되면 해당 파일의 이름이 패키지 이름이 되고, `.json`이 제공되지 않으면 이름은 `map_package`여야 합니다:

```sh
    python3 docker_tools.py --input ~/path_to_input_folder --output ~/path_to_output_folder --packages map_package
```

> !!! 경고
    `--packages map_package` 인수가 제공되지 않으면 Docker 이미지는 CARLA의 패키지를 만들게 됩니다.

__6.__ 패키지는 `output_folder`에 `<map_package>.tar.gz`로 생성됩니다. 이제 이 독립 실행형 패키지를 CARLA로 가져올 준비가 되었습니다. 패키지를 CARLA 루트 디렉토리(맵을 사용할 패키지/바이너리 버전)의 `Import` 폴더로 이동하고, 루트 디렉토리에서 다음 스크립트를 실행하여 가져오세요:

```sh
        ./ImportAssets.sh
```

__7.__ 새 맵으로 시뮬레이션을 실행하려면 CARLA를 실행한 다음 `config.py` 파일을 사용하여 맵을 변경하세요:

```sh
    cd PythonAPI/util
    python3 config.py --map <맵이름>
```
<br>

---

이제 맵에서 CARLA 시뮬레이션을 실행할 준비가 되었습니다. 과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문하거나, 새 맵을 테스트하기 위해 [예제 스크립트](https://github.com/carla-simulator/carla/tree/master/PythonAPI/examples) 중 일부를 실행해볼 수 있습니다.