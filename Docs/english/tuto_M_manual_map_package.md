# 수동 패키지 준비

맵 패키지는 특정 폴더 구조를 따르며 해당 구조를 설명하는 `.json` 파일을 포함해야 합니다. 자동 맵 가져오기 프로세스는 이 `.json` 파일을 자동으로 생성하지만, 직접 준비할 수도 있습니다. 자체 `.json` 파일을 포함하면 `make import` 명령어에 전달된 모든 인수를 덮어씁니다.

- [__일반 맵__](#standard-maps)
    - [일반 맵의 폴더 구조 생성하기](#create-the-folder-structure-for-the-standard-maps)
    - [일반 맵의 JSON 설명 생성하기](#create-the-json-description-for-the-standard-maps)
- [__대형 맵__](#large-maps)
    - [대형 맵의 폴더 구조 생성하기](#create-the-folder-structure-for-the-large-maps)
    - [대형 맵의 JSON 설명 생성하기](#create-the-json-description-for-the-large-maps)

---

## 일반 맵
### 일반 맵의 폴더 구조 생성하기

__1. `carla/Import` 내부에 폴더를 생성합니다.__ 폴더 이름은 중요하지 않습니다.

__2. 가져올 각 맵에 대해 다른 하위 폴더를__ 생성합니다.

__3. 각 맵의 파일을 해당 하위 폴더로 이동합니다.__ 하위 폴더는 특정 요소 세트를 포함합니다:

-   `.fbx` 파일의 맵 메시
-   `.xodr` 파일의 OpenDRIVE 정의
-   선택적으로, 에셋에 필요한 텍스처

예를 들어, 두 개의 맵을 포함하는 하나의 패키지가 있는 `Import` 폴더는 아래와 같은 구조를 가져야 합니다.

```sh
Import
│
└── Package01
  ├── Package01.json
  ├── Map01
  │   ├── Asphalt1_Diff.jpg
  │   ├── Asphalt1_Norm.jpg
  │   ├── Asphalt1_Spec.jpg
  │   ├── Grass1_Diff.jpg
  │   ├── Grass1_Norm.jpg
  │   ├── Grass1_Spec.jpg
  │   ├── LaneMarking1_Diff.jpg
  │   ├── LaneMarking1_Norm.jpg
  │   ├── LaneMarking1_Spec.jpg
  │   ├── Map01.fbx
  │   └── Map01.xodr
  └── Map02
      └── Map02.fbx
```

---

### 일반 맵의 JSON 설명 생성하기

패키지의 루트 폴더에 `.json` 파일을 생성합니다. 패키지의 이름을 따서 파일 이름을 지정하세요. 이것이 배포 이름이 될 것입니다. 파일의 내용은 각각에 대한 기본 정보가 있는 __maps__와 __props__의 JSON 배열을 설명합니다.

__Maps__에는 다음 매개변수가 필요합니다:

- __name__: 맵의 이름. `.fbx`와 `.xodr` 파일과 동일해야 합니다.
- __source__: `.fbx` 파일의 경로.
- __use_carla_materials__: __True__이면 맵이 CARLA 재질을 사용하고, 그렇지 않으면 RoadRunner 재질을 사용합니다.
- __xodr__: `.xodr` 파일의 경로.

__Props__는 이 튜토리얼의 일부가 아닙니다. 필드는 비어있게 됩니다. [새로운 프롭 추가하기](tuto_A_add_props.md)에 대한 다른 튜토리얼이 있습니다.

결과 `.json` 파일은 다음과 유사해야 합니다:

```json
{
  "maps": [
    {
      "name": "Map01",
      "source": "./Map01/Map01.fbx",
      "use_carla_materials": true,
      "xodr": "./Map01/Map01.xodr"
    },
    {
      "name": "Map02",
      "source": "./Map02/Map02.fbx",
      "use_carla_materials": false,
      "xodr": "./Map02/Map02.xodr"
    }
  ],
  "props": [
  ]
}
```
</details>
<br>

---

과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>