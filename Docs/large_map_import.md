# 대형 맵 가져오기/패키징하기

RoadRunner에서 생성된 대형 맵은 CARLA 소스 빌드에 가져와 CARLA 독립 실행형 패키지에서 사용할 수 있도록 배포 및 패키징할 수 있습니다. 이 과정은 일반 맵과 비슷하지만, 타일에 대한 특정 명명 규칙과 배치 가져오기가 추가됩니다.

- [__파일 및 폴더__](#파일-및-폴더)
- [__JSON 설명 파일 생성 (선택 사항)__](#json-설명-파일-생성-선택-사항)
- [__맵 가져오기__](#맵-가져오기)
- [__대형 맵 패키징__](#대형-맵-패키징)

---

## 파일 및 폴더

모든 가져올 파일은 CARLA 루트 디렉터리의 `Import` 폴더에 배치해야 합니다. 이 파일들은 다음을 포함해야 합니다:

- 맵의 메시(mesh)를 나타내는 여러 개의 `.fbx` 파일 (맵의 서로 다른 타일을 나타냄).
- 단일 `.xodr` 파일로 된 OpenDRIVE 정의.

!!! 경고
    대형 맵과 일반 맵을 동시에 가져올 수 없습니다.

맵 타일의 이름 지정 규칙이 매우 중요합니다. 각 맵 타일은 다음 규칙에 따라 이름을 지정해야 합니다:

```
<mapName>_Tile_<x-좌표>_<y-좌표>.fbx
```

__y 좌표__ 값이 클수록 y축 아래쪽에 있는 타일을 의미합니다. 예를 들어, `Map01_Tile_0_1`은 `Map01_Tile_0_0` 바로 아래에 위치합니다.

>>>>>>>>![맵 타일](../img/map_tiles.png)

다음은 네 개의 타일로 구성된 대형 맵 패키지가 포함된 `Import` 폴더의 예제 구조입니다:

```sh
Import
│
└── Package01
  ├── Package01.json
  ├── Map01_Tile_0_0.fbx
  ├── Map01_Tile_0_1.fbx
  ├── Map01_Tile_1_0.fbx
  ├── Map01_Tile_1_1.fbx
  └── Map01.xodr
```

!!! 참고
    `package.json` 파일은 필수는 아닙니다. 파일이 없을 경우, 자동 가져오기 프로세스에서 생성됩니다. 직접 `package.json` 파일을 작성하는 방법에 대한 자세한 내용은 다음 섹션을 참조하십시오.

---

## JSON 설명 파일 생성 (선택 사항)

`.json` 설명 파일은 가져오기 프로세스 중 자동으로 생성되지만, 직접 생성할 수도 있습니다. 기존 `.json` 설명 파일은 가져오기 프로세스에서 전달된 모든 값을 덮어씁니다.

`.json` 파일은 패키지의 루트 폴더에 생성되어야 합니다. 파일 이름은 패키지 배포 이름이 됩니다. 파일의 내용은 __maps__ 및 __props__의 JSON 배열을 기본 정보와 함께 설명합니다.

__Maps__에는 다음 매개변수가 필요합니다:

- __name:__ 맵 이름. 이는 `.fbx` 및 `.xodr` 파일과 동일해야 합니다.
- __xodr:__ `.xodr` 파일 경로.
- __use_carla_materials:__ __True__일 경우 CARLA 소재를 사용하며, 그렇지 않으면 RoadRunner 소재를 사용합니다.
- __tile_size:__ 타일의 크기. 기본값은 2000(2km x 2km)입니다.
- __tiles:__ 전체 맵을 구성하는 `.fbx` 타일 파일 목록.

__Props__는 이 튜토리얼의 범위에 포함되지 않습니다. 새로운 소품을 추가하는 방법은 [이 튜토리얼](tuto_A_add_props.md)을 참조하십시오.

다음은 결과적으로 생성되는 `.json` 파일의 예입니다:

```json
{
  "maps": [
      {
        "name": "Map01",
        "xodr": "./Map01.xodr",
        "use_carla_materials": true,
        "tile_size": 2000,
        "tiles": [ 
        "./Map01_Tile_0_0.fbx",
        "./Map01_Tile_0_1.fbx",
        "./Map01_Tile_1_0.fbx",
        "./Map01_Tile_1_1.fbx"
        ]
      }
  ],
  "props": []
}
```

---

## 맵 가져오기

모든 파일이 `Import` 폴더에 배치된 후, CARLA 루트 폴더에서 다음 명령을 실행하십시오:

```sh
make import
```

시스템에 따라 Unreal Engine이 모든 파일을 한 번에 가져오기에는 메모리를 너무 많이 사용할 수 있습니다. MB 단위로 파일을 배치 가져오기를 선택하려면 다음 명령을 실행하십시오:

```sh
make import ARGS="--batch-size=200"
```

`make import` 명령에는 추가로 두 개의 플래그가 있습니다:

- `--package=<package_name>`: 패키지 이름을 지정합니다. 기본값은 `map_package`입니다. 두 패키지가 동일한 이름을 가질 수 없으므로, 기본값을 사용하면 이후 가져오기에 오류가 발생할 수 있습니다. __패키지 이름을 변경하는 것이 권장됩니다.__ 명령은 다음과 같이 실행하십시오:

```sh
make import ARGS="--package=<package_name>"
```

- `--no-carla-materials`: 기본 CARLA 소재(도로 텍스처 등)를 사용하지 않고 RoadRunner 소재를 사용하려는 경우 지정합니다. 이 플래그는 __사용자가 자체 [`.json` 파일](tuto_M_manual_map_package.md)을 제공하지 않을 때만__ 필요합니다. `.json` 파일의 값은 이 플래그를 덮어씁니다. 명령은 다음과 같이 실행하십시오:

```sh
make import ARGS="--no-carla-materials"
```

모든 파일이 가져와져 Unreal Editor에서 사용할 준비가 됩니다. 맵 패키지는 `Unreal/CarlaUE4/Content`에 생성됩니다. `<mapName>` 기본 맵 타일은 모든 타일의 스트리밍 레벨로 생성되며, 하늘, 날씨, 대형 맵 액터를 포함하고 시뮬레이션에서 사용할 준비가 됩니다.

!!! 참고
    현재로서는 Unreal Editor에서 제공하는 도로 페인터, 절차적 건물 등 일반 맵 사용자 지정 도구를 사용하는 것이 권장되지 않습니다.

---

## 대형 맵 패키징

대형 맵을 패키징하여 CARLA 독립 실행형 패키지에서 사용할 수 있도록 하려면 다음 명령을 실행하십시오:

```sh
make package ARGS="--packages=<mapPackage>"
```

이 명령은 `.tar.gz` 파일로 압축된 독립 실행형 패키지를 생성합니다. 파일은 Linux에서는 `Dist` 폴더, Windows에서는 `/Build/UE4Carla/`에 저장됩니다. 이후 배포 및 독립 실행형 CARLA 패키지에서 사용할 수 있도록 패키징됩니다.

---

대형 맵 가져오기 및 패키징 프로세스에 대한 질문이 있으면 [포럼](https://github.com/carla-simulator/carla/discussions)에 문의하십시오.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>