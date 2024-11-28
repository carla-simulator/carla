# 소스에서 빌드된 CARLA에서 맵 가져오기

이 섹션에서는 **소스에서 빌드된 CARLA**에 맵을 가져오는 과정을 설명합니다. 맵을 가져오기 위해 패키지(바이너리) 버전의 CARLA를 사용하고 있다면 대신 [여기][package_ingest]의 가이드라인을 따르세요.

가져오기 과정에는 관련 맵 파일들을 패키지로 컴파일하여 가져오는 작업이 포함됩니다. 이후 이 패키지는 언리얼 엔진 에디터에서 열어 커스터마이즈할 수 있으며, 보행자 내비게이션 파일을 생성한 후 최종적으로 패키지에 추가할 수 있습니다.

[package_ingest]: tuto_M_add_map_package.md

- [__시작하기 전에__](#before-you-begin)
- [__맵 가져오기__](#map-ingestion)
- [__다음 단계__](#next-steps)

---

## 시작하기 전에

- 소스에서 빌드된 CARLA 버전을 사용하고 있는지 확인하세요. 패키지(바이너리) 버전의 CARLA를 사용하고 있다면 [여기][import_map_package]의 튜토리얼을 따르세요.
- RoadRunner와 같은 맵 편집기에서 [생성된][rr_generate_map] `<맵이름>.xodr`과 `<맵이름>.fbx` 파일이 최소한 두 개 있어야 합니다.
- 이 파일들은 동일한 맵으로 인식되기 위해 `<맵이름>` 값이 동일해야 합니다.
- 동일한 패키지에 여러 맵을 가져올 수 있습니다. 각 맵은 고유한 이름을 가져야 합니다.

[import_map_package]: tuto_M_add_map_package.md
[rr_generate_map]: tuto_M_generate_map.md

---
## 맵 가져오기

__1.__ CARLA 루트 디렉토리의 `Import` 폴더에 가져올 맵 파일들을 배치합니다.

__2.__ 아래 명령어를 실행하여 파일들을 가져옵니다:

```sh
make import
```

__설정할 수 있는 두 가지 선택적 매개변수 플래그가 있습니다__:

- `--package=<package_name>`은 패키지의 이름을 지정합니다. 기본값은 `map_package`입니다. 두 패키지는 동일한 이름을 가질 수 없으므로, 기본값을 사용하면 후속 가져오기 시 오류가 발생합니다. __패키지 이름을 변경하는 것을 강력히 권장합니다__. 다음 명령어로 이 플래그를 사용하세요:

```sh
make import  ARGS="--package=<package_name>"
```

- `--no-carla-materials`는 기본 CARLA 재질(도로 텍스처 등)을 사용하지 않겠다는 것을 지정합니다. 대신 RoadRunner 재질을 사용하게 됩니다. 이 플래그는 자체 [`.json` 파일](tuto_M_manual_map_package.md)을 제공하지 __않는 경우에만__ 필요합니다. `.json` 파일의 값은 이 플래그를 덮어씁니다. 다음 명령어로 이 플래그를 사용하세요:

```sh
make import  ARGS="--no-carla-materials"
```

`Unreal/CarlaUE4/Content`에 맵 패키지의 이름으로 폴더가 생성됩니다. 이 폴더는 구성 파일, 오버드라이브 정보, 정적 에셋 정보 및 내비게이션 정보를 포함합니다.

---

## 다음 단계

이제 언리얼 에디터에서 맵을 열고 시뮬레이션을 실행할 수 있습니다. 여기서 맵을 커스터마이즈하고 보행자 내비게이션 데이터를 생성할 수 있습니다. 보행자 경로가 장애물에 의해 차단될 가능성을 없애기 위해 모든 커스터마이즈가 완료된 후에 보행자 내비게이션을 생성하는 것을 권장합니다.

CARLA는 맵 커스터마이즈를 위한 여러 도구와 가이드를 제공합니다:

- [맵에 서브레벨 구현하기](tuto_M_custom_layers.md)
- [신호등과 표지판 추가 및 설정하기](tuto_M_custom_add_tl.md)
- [절차적 건물 도구로 건물 추가하기](tuto_M_custom_buildings.md)
- [도로 페인터 도구로 도로 커스터마이즈하기](tuto_M_custom_road_painter.md)
- [날씨 커스터마이즈하기](tuto_M_custom_weather_landscape.md#weather-customization)
- [연속 메시로 지형 커스터마이즈하기](tuto_M_custom_weather_landscape.md#add-serial-meshes)

커스터마이즈가 완료되면 [보행자 내비게이션 정보를 생성](tuto_M_generate_pedestrian_navigation.md)할 수 있습니다.

---

과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문할 수 있습니다.