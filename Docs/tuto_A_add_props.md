# 새로운 프롭 추가하기

프롭은 맵과 차량 외에도 장면을 구성하는 자산들입니다. 여기에는 가로등, 건물, 나무 등이 포함됩니다. 시뮬레이터는 간단한 과정을 통해 언제든지 새로운 프롭을 불러올 수 있습니다. 이는 맵에서 사용자 정의 환경을 만드는 데 매우 유용합니다.

* [__패키지 준비하기__](#패키지-준비하기)
  * [폴더 구조 생성하기](#폴더-구조-생성하기)
  * [JSON 설명 파일 생성하기](#json-설명-파일-생성하기)
* [__CARLA 패키지에 불러오기__](#carla-패키지에-불러오기)
* [__소스에서 빌드하여 불러오기__](#소스에서-빌드하여-불러오기)

---
## 패키지 준비하기

### 폴더 구조 생성하기

__1. `carla/Import` 내부에 폴더를 생성합니다.__ 폴더 이름은 중요하지 않습니다.

__2. 하위 폴더를 생성합니다.__ 모든 프롭을 위한 하나의 일반 하위 폴더가 있어야 하며, 그 안에 불러올 프롭 수만큼의 하위 폴더가 있어야 합니다.

__3. 각 프롭의 파일을 해당 하위 폴더로 이동합니다.__ 프롭 하위 폴더에는 `.fbx` 메시와 선택적으로 이에 필요한 텍스처가 포함됩니다.

예를 들어, 두 개의 별도 패키지가 있는 `Import` 폴더는 아래와 같은 구조를 가져야 합니다.

```sh
Import
│
├── Package01
│   ├── Package01.json
│   └── Props
│       ├── Prop01
│       │   ├── Prop01_Diff.png
│       │   ├── Prop01_Norm.png
│       │   ├── Prop01_Spec.png
│       │   └── Prop01.fbx
│       └── Prop02
│           └── Prop02.fbx
└── Package02
    ├── Package02.json
    └── Props
        └── Prop03
            └── Prop03.fbx
```

### JSON 설명 파일 생성하기

패키지의 루트 폴더에 `.json` 파일을 생성합니다. 패키지 이름으로 파일 이름을 지정하세요. 이 이름이 배포 이름이 됩니다. 파일의 내용은 각 __맵__과 __프롭__에 대한 기본 정보를 포함하는 JSON 배열을 설명합니다.

__맵__은 이 튜토리얼의 일부가 아니므로 이 정의는 비어 있을 것입니다. [__새 맵 추가하기__](tuto_M_custom_map_overview.md)에 대한 특정 튜토리얼이 있습니다.

__프롭__에는 다음 매개변수가 필요합니다.

* __name__: 프롭의 이름. `.fbx`와 동일해야 합니다.
* __source__: `.fbx`까지의 경로.
* __size__: 프롭의 크기 추정치. 가능한 값은 다음과 같습니다.
  * `tiny`
  * `small`
  * `medium`
  * `big`
  * `huge`
* __tag__: 의미론적 분할을 위한 값. 태그가 잘못 입력되면 `Unlabeled`로 읽힙니다.
  * `Bridge`
  * `Building`
  * `Dynamic`
  * `Fence`
  * `Ground`
  * `GuardRail`
  * `Other`
  * `Pedestrian`
  * `Pole`
  * `RailTrack`
  * `Road`
  * `RoadLine`
  * `SideWalk`
  * `Sky`
  * `Static`
  * `Terrain`
  * `TrafficLight`
  * `TrafficSign`
  * `Unlabeled`
  * `Vegetation`
  * `Vehicles`
  * `Wall`
  * `Water`

최종적으로 `.json`은 아래와 비슷해야 합니다.

```json
{
  "maps": [
  ],
  "props": [
    {
      "name": "MyProp01",
      "size": "medium",
      "source": "./Props/Prop01/Prop01.fbx",
      "tag": "SemanticSegmentationTag01"
    },
    {
      "name": "MyProp02",
      "size": "small",
      "source": "./Props/Prop02/Prop02.fbx",
      "tag": "SemanticSegmentationTag02"
    }
  ]
}
```

!!! 경고
    동일한 이름의 패키지는 오류를 발생시킵니다.

---
## CARLA 패키지에 불러오기

이는 CARLA 0.9.8과 같은 CARLA 패키지에 프롭을 불러오는 방법입니다.

언리얼 엔진의 도커 이미지가 생성됩니다. 이는 패키지를 CARLA 이미지로 자동으로 가져오고 배포 패키지를 생성하는 블랙박스 역할을 합니다. 도커 이미지는 구축하는 데 4시간과 400GB가 필요합니다. 하지만 이는 처음에만 필요합니다.

__1. 언리얼 엔진의 도커 이미지를 빌드합니다.__ 이미지를 빌드하려면 [이 지침](https://github.com/carla-simulator/carla/tree/master/Util/Docker)을 따르세요.

__2. 프롭을 요리하는 스크립트를 실행합니다.__ `~/carla/Util/Docker` 폴더에는 이전에 생성된 도커 이미지와 연결하여 자동으로 불러오기를 수행하는 스크립트가 있습니다. 입력 및 출력 파일의 경로와 불러올 패키지의 이름만 필요합니다.

```sh
python3 docker_tools.py --input ~/path_to_package --output ~/path_for_output_assets  --packages Package01
```

__3. 패키지를 찾습니다.__ 도커는 출력 경로에 `Package01.tar.gz` 패키지를 생성했어야 합니다. 이것이 자산의 독립 실행형 패키지입니다.

__4. CARLA에 패키지를 가져옵니다.__

* __Windows에서__ `WindowsNoEditor` 폴더에 패키지를 추출합니다.

* __Linux에서__ `Import` 폴더로 패키지를 이동하고 가져오기 스크립트를 실행합니다.

```sh
cd Util
./ImportAssets.sh
```

!!! 참고
    Linux에서는 대안이 있습니다. 패키지를 `Import` 폴더로 이동하고 `Util/ImportAssets.sh` 스크립트를 실행하여 패키지를 추출합니다.

---
## 소스에서 빌드하여 불러오기

이는 소스에서 빌드한 CARLA에 프롭을 가져오는 방법입니다.

JSON 파일을 읽어 언리얼 엔진의 `Content` 안에 프롭을 배치합니다. 또한 패키지의 `Config` 폴더 안에 `Package1.Package.json` 파일을 생성합니다. 이는 블루프린트 라이브러리에서 프롭을 정의하고 Python API에서 노출시키는 데 사용됩니다. 패키지가 [독립 실행형 패키지](tuto_A_create_standalone.md)로 내보내질 때도 사용됩니다.

모든 준비가 완료되면 명령을 실행합니다.

```sh
make import
```

!!! 경고
    패키지가 CARLA의 `Import` 폴더 안에 있는지 확인하세요.

---

이것이 CARLA에 새로운 프롭을 가져오는 다양한 방법에 대해 알아야 할 전부입니다. 의문사항이 있다면 포럼에 자유롭게 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>