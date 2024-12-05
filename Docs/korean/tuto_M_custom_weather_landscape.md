# 맵 커스터마이즈: 날씨와 지형

CARLA는 맵의 기본 날씨 설정을 만들고 가로등, 전선 등의 연속 메시로 지형을 채우는 것을 돕는 여러 블루프린트를 제공합니다.

이 가이드는 이러한 블루프린트들의 위치와 사용 및 구성 방법을 설명합니다.

- [__날씨 커스터마이즈__](#weather-customization)
	- [BP_Sky](#bp_sky)
	- [BP_Weather](#bp_weather)
- [__연속 메시__](#add-serial-meshes)
	- [BP_RepSpline](#bp_repspline)
	- [BP_Spline](#bp_spline)
	- [BP_Wall](#bp_wall)
	- [BP_SplinePoweLine](#bp_splinepoweline)
- [__다음 단계__](#next-steps)

!!! 중요
    이 튜토리얼은 소스에서 빌드하여 언리얼 에디터에 접근할 수 있는 사용자에게만 적용됩니다.

---

## 날씨 커스터마이즈

이 섹션에서는 맵의 기본 날씨를 설정하기 전에 다양한 날씨 매개변수를 실험하는 방법과, 설정에 만족한 후 맵의 기본 날씨 매개변수를 구성하는 방법을 설명합니다.

### BP_Sky

`BP_Sky` 블루프린트는 맵에 빛과 날씨를 가져오는 데 필요합니다. 또한 기본 날씨 매개변수를 결정하기 전에 다양한 날씨 구성을 테스트하는 데 사용할 수 있습니다.

`BP_Sky` 블루프린트는 이미 맵에 로드되어 있을 것입니다. 그렇지 않다면 `Content/Carla/Blueprints/Weather`에서 장면으로 드래그하여 추가할 수 있습니다.

다양한 날씨 매개변수를 시도해보려면 `BP_Sky` 액터의 _Details_ 패널로 이동하여 _Parameters_ 섹션의 값들을 조정해보세요.

!!! 중요
    장면에 둘 이상의 `BP_Sky` 블루프린트가 로드되면 두 개의 태양이 생기는 등 원하지 않는 결과가 발생할 수 있습니다.

### BP_Weather

맵의 기본 날씨는 `BP_Weather` 블루프린트에서 정의됩니다. 이 블루프린트는 Python API를 통해 사용할 수 있는 것과 동일한 매개변수를 설정할 수 있게 해줍니다. 이러한 매개변수들은 [여기](https://carla.readthedocs.io/en/latest/python_api/#carlaweatherparameters)에 설명되어 있습니다.

맵의 기본 날씨를 설정하려면:

__1. `BP_Weather` 블루프린트 열기__

_콘텐츠 브라우저_에서 `Content/Carla/Blueprints/Weather`로 이동하여 `BP_Weather`를 더블클릭합니다.

__2. 도시 추가하기__

`BP_Weather` 창의 _Details_ 패널에서 _Weather_ 섹션으로 이동하여 _Default Weathers_ 배열에 도시를 추가합니다.

__3. 기본 날씨 매개변수 구성하기__

각 날씨 매개변수에 대해 원하는 값을 설정합니다. 완료되면 _Compile_을 누른 다음 _Save_를 누르고 닫습니다.

>>>>>![bp_weather_pic](../img/map_customization/BP_Weather.jpg)<br>
<div style="text-align: right">
<i>모든 CARLA 맵의 기본 날씨 매개변수를 포함하는 배열. Town01이 열려있음.</i></div>
<br>

---

## 연속 메시 추가하기

벽, 전선, 가로등과 같이 한 방향으로 정렬된 프롭을 추가하는 데 사용할 수 있는 네 가지 블루프린트가 있습니다. 이 블루프린트들은 베지어 곡선을 따라 분포된 일련의 메시를 사용합니다. 각각은 동일한 방식으로 초기화됩니다:

__1. 시리즈 초기화하기__

블루프린트를 장면으로 드래그합니다. 시작과 끝을 표시하는 두 노드가 있는 베지어 곡선의 시작점에 하나의 요소가 있는 것을 볼 수 있습니다.

__2. 경로 정의하기__

요소의 방향 화살표를 선택하고 원하는 방향으로 요소를 드래그하는 동안 __Alt__ 키를 누릅니다. 이렇게 하면 곡선을 정의하는 데 사용할 수 있는 새로운 요소가 생성됩니다. 드래그하면 블루프린트에 따라 곡선의 각 노드마다 또는 Alt를 누르면서 드래그할 때마다 새로운 메시가 나타납니다.

__3. 패턴 커스터마이즈하기__

다음 섹션에서는 각 블루프린트에서 사용할 수 있는 다양한 커스터마이즈 매개변수들을 설명합니다.

### BP_RepSpline

`BP_RepSpline` 블루프린트는 `Carla/Blueprints/LevelDesign`에 있습니다. 베지어 곡선으로 정의된 경로를 따라 __개별__ 요소를 추가하는 데 사용됩니다.

연속화는 다음 값들로 커스터마이즈됩니다:

- _Distance between_ — 요소 사이의 거리 설정
- _Offset rotation_ — 다양한 축에 대해 고정 회전 설정
- _Random rotation_ — 다양한 축에 대해 무작위 회전 범위 설정
- _Offset translation_ — 다양한 축을 따라 무작위 위치의 범위 설정
- _Max Number of Meshes_ — 곡선의 노드 사이에 배치될 최대 요소 수 설정
- _World aligned ZY_ — 선택하면 요소들이 월드 축을 기준으로 수직 정렬됨
- _EndPoint_ — 선택하면 곡선의 끝 노드에 요소가 추가됨
- _Collision enabled_ — 메시에 대해 활성화된 충돌 유형 설정

![bp_repspline_pic](../img/map_customization/BP_Repspline.jpg)
<div style="text-align: right"><i>BP_RepSpline 예시.</i></div>

### BP_Spline

`BP_Spline` 블루프린트는 `Carla/Blueprints/LevelDesign`에 있습니다. 베지어 곡선으로 정의된 경로를 __엄격하게__ 따르는 __연결된__ 요소들을 추가합니다. 메시는 생성된 경로에 맞게 변형됩니다.

블루프린트는 다음 값으로 커스터마이즈할 수 있습니다:

- _Gap distance_ — 요소 사이에 간격 추가

![bp_spline_pic](../img/map_customization/BP_Spline.jpg)
<div style="text-align: right"><i>BP_Spline 예시.</i></div>

### BP_Wall

`BP_Wall` 블루프린트는 `Carla/Blueprints/LevelDesign`에 있습니다. 베지어 곡선으로 정의된 경로를 따라 __연결된__ 요소들을 추가합니다. 메시는 곡선에 맞게 변형되지 않지만 노드는 유지됩니다.

- _Distance between_ — 요소 사이의 거리 설정
- _Vertically aligned_ — 선택하면 요소들이 월드 축을 기준으로 수직 정렬됨
- _Scale offset_ — 요소 사이의 연결을 둥글게 하기 위해 메시의 길이를 조정

![bp_wall_pic](../img/map_customization/BP_Wall.jpg)
<div style="text-align: right"><i>BP_Wall 예시.</i></div>

### BP_SplinePoweLine

__BP_SplinePoweLine__ 블루프린트는 `Carla/Static/Pole/PoweLine`에 있습니다. 베지어 곡선으로 정의된 경로를 따라 __전신주__를 추가하고 __전선으로 연결__합니다.

다양성을 제공하기 위해 경로를 채울 전선 메시 배열을 블루프린트에 제공할 수 있습니다. 방법은 다음과 같습니다:

1. _콘텐츠 브라우저_에서 __BP_SplinePoweLine__ 블루프린트를 더블클릭합니다.
2. _Details_ 패널에서 _Default_ 섹션으로 이동합니다.
3. _Array Meshes_를 확장하고 필요에 따라 추가하거나 변경합니다.
4. _Compile_을 누른 다음 저장하고 창을 닫습니다.

![bp_splinepowerline_pic](../img/map_customization/BP_Splinepowerline.jpg)
<div style="text-align: right"><i>BP_SplinePowerLine 예시.</i></div>

전선의 장력을 변경하려면:

1. 에디터 장면에서 블루프린트 액터를 선택하고 _Details_ 패널로 이동합니다.
2. _Default_ 섹션으로 이동합니다.
3. _Tension_의 값을 조정합니다. `0`은 선이 곧게 된다는 것을 나타냅니다.

전선의 수를 늘리려면:

1. _콘텐츠 브라우저_에서 전신주 메시 중 하나를 더블클릭합니다.
2. _Socket Manager_ 패널로 이동합니다.
3. 기존 소켓을 구성하거나 _Create Socket_을 클릭하여 새 소켓을 추가합니다. 소켓은 전선의 연결 지점을 나타내는 빈 메시입니다. 전신주 사이에서 소켓에서 소켓으로 전선이 생성됩니다.

![bp_powerline_socket_pic](../img/map_customization/BP_Splinepowerline_Sockets.jpg)
<div style="text-align: right"><i>BP_SplinePowerLine의 소켓 시각화.</i></div>
<br>

!!! 중요
    소켓의 수와 이름은 전신주 사이에서 일관성이 있어야 합니다. 그렇지 않으면 시각화 문제가 발생할 수 있습니다.

---

## 다음 단계

아래의 도구와 가이드를 사용하여 맵 커스터마이즈를 계속하세요:

- [맵에 서브레벨 구현하기](tuto_M_custom_layers.md)
- [신호등과 표지판 추가 및 설정하기](tuto_M_custom_add_tl.md)
- [절차적 건물 도구로 건물 추가하기](tuto_M_custom_buildings.md)
- [도로 페인터 도구로 도로 커스터마이즈하기](tuto_M_custom_road_painter.md)
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