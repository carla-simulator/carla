# 맵 커스터마이즈: 절차적 건물

- [__절차적 건물__](#add-serial-meshes)
	- [건물 구조](#building-structure)
	- [구조 수정](#structure-modifications)
- [__다음 단계__](#next-steps)

---

## 절차적 건물

절차적 건물 도구를 사용하면 여러 층으로 구성된 직사각형 건물을 만들 수 있습니다. 각 층은 구성 가능한 메시 배열이나 단일 블루프린트를 사용하여 만들어집니다. 메시 배열을 사용하는 경우, 각 메시는 다양성을 제공하기 위해 층을 따라 무작위로 반복됩니다. 메시는 한 번 생성되고 각 반복은 해당 메시의 인스턴스가 됩니다. 이는 맵의 성능을 향상시킵니다.

### 건물 구조

건물 작업을 시작하려면:

1. 언리얼 엔진 에디터의 _콘텐츠 브라우저_에서 `Content/Carla/Blueprints/LevelDesign`으로 이동합니다.
2. `BP_Procedural_Building`을 장면으로 드래그합니다.

_Details_ 패널에서 건물을 커스터마이즈할 수 있는 모든 옵션을 볼 수 있습니다. 여기서 변경이 이루어질 때마다 핵심 메시가 업데이트되면서 건물이 장면 뷰에서 사라집니다. 새로운 결과를 보려면 _Create Building_을 클릭하거나, 이 단계를 반복하지 않도록 _Create automatically_를 활성화하세요.

핵심 메시는 건물 구조의 부분들입니다. 이는 네 가지 카테고리로 나뉩니다:

- __Base:__ 1층
- __Body:__ 중간층
- __Top:__ 최상층
- __Roof:__ 최상층을 덮는 지붕

**Roof**를 제외한 각 카테고리에는 층의 중앙을 채우는 메시와 층의 측면에 배치될 __Corner__ 메시가 있습니다. 다음 그림은 전체 구조를 나타냅니다.

![bp_procedural_building_visual](../img/map_customization/BP_Procedural_Building_Visual.jpg)
<div style="text-align: right"><i>건물 구조의 시각화.</i></div>

**Base parameters**는 크기를 설정합니다.

- __Num Floors:__ 건물의 층수. __Body__ 메시의 반복 횟수.
- __Length X and Length Y:__ 건물의 길이와 너비. 건물 각 면의 중앙 메시 반복 횟수.

![bp_procedural_building_full](../img/map_customization/BP_Procedural_Building_Full.jpg)
<div style="text-align: right"><i>BP_Procedural_Building 예시.</i></div>

### 구조 수정

건물의 일반 구조를 수정하기 위한 추가 옵션들이 있습니다.

- __Disable corners:__ 선택하면 모서리 메시가 사용되지 않습니다.
- __Use full blocks:__ 선택하면 건물 구조는 층당 하나의 메시만 사용합니다. 각 층에 모서리나 반복이 나타나지 않습니다.
- __Doors:__ 1층의 중앙 메시 바로 앞에 나타나는 메시. 문의 수량과 위치를 설정할 수 있습니다. `0`은 초기 위치, `1`은 다음 기본 반복 등입니다.
- __Walls:__ 건물의 한 면 이상을 대체하는 메시. 예를 들어, 평면 메시를 사용하여 건물의 한 면을 칠할 수 있습니다.

![bp_procedural_building_extras](../img/map_customization/BP_Procedural_Building_Extras.jpg)
<div style="text-align: right"><i>왼쪽은 모서리가 없고 문이 하나인 건물. <br> 오른쪽은 건물의 한 면에 벽이 적용된 건물. 벽은 비상 탈출구가 없는 텍스처입니다.</i></div>

---

## 다음 단계

아래의 도구와 가이드를 사용하여 맵 커스터마이즈를 계속하세요:

- [맵에 서브레벨 구현하기](tuto_M_custom_layers.md)
- [신호등과 표지판 추가 및 설정하기](tuto_M_custom_add_tl.md)
- [도로 페인터 도구로 도로 커스터마이즈하기](tuto_M_custom_road_painter.md)
- [날씨 커스터마이즈하기](tuto_M_custom_weather_landscape.md#weather-customization)
- [연속 메시로 지형 커스터마이즈하기](tuto_M_custom_weather_landscape.md#add-serial-meshes)

커스터마이즈가 완료되면 [보행자 내비게이션 정보를 생성](tuto_M_generate_pedestrian_navigation.md)할 수 있습니다.

---

과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>