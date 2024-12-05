# 맵 커스터마이즈: 신호등과 표지판

이 가이드는 사용자 정의 맵에 신호등과 표지판을 추가하고, 각각의 영향 범위를 설정하며, 교차로에서 신호등을 그룹으로 구성하는 방법을 설명합니다. 이 기능은 언리얼 엔진 에디터에 접근할 수 있는 사용자만 사용할 수 있습니다.

- [__신호등__](#traffic-lights)
- [__교통 표지판__](#traffic-signs)
- [__다음 단계__](#next-steps)

---

## 신호등

새 맵에 신호등을 추가하려면:

__1.__ _콘텐츠 브라우저_에서 `Content/Carla/Static/TrafficLight/StreetLights_01`로 이동합니다. 여기서 선택할 수 있는 여러 신호등 블루프린트를 찾을 수 있습니다.

__2.__ 신호등을 장면으로 드래그하여 원하는 위치에 배치합니다. 키보드의 스페이스바를 눌러 위치 지정, 회전, 크기 조절 도구를 전환할 수 있습니다.

__3.__ _Details_ 패널에서 _BoxTrigger_ 컴포넌트를 선택하고 _Transform_ 섹션의 값을 조정하여 각 신호등의 [`트리거 볼륨`][triggerlink]을 조정합니다. 이것은 신호등의 영향 범위를 결정합니다.

>>![ue_trafficlight](../img/ue_trafficlight.jpg)

__4.__ 교차로의 경우, `BP_TrafficLightGroup` 액터를 레벨로 드래그합니다. _Details_ 패널의 _Traffic Lights_ 배열에 교차로의 모든 신호등을 추가하여 신호등 그룹에 할당합니다.

>>![ue_tl_group](../img/ue_tl_group.jpg)

__5.__ 신호등 타이밍은 Python API를 통해서만 구성할 수 있습니다. 자세한 정보는 [여기](core_actors.md#traffic-signs-and-traffic-lights)의 문서를 참조하세요.

>>![ue_tlsigns_example](../img/ue_tlsigns_example.jpg)

> _예시: 교통 표지판, 신호등 및 회전 기반 정지._

[triggerlink]: python_api.md#carla.TrafficSign.trigger_volume

---

## 교통 표지판

새 맵에 교통 표지판을 추가하려면:

__1.__ _콘텐츠 브라우저_에서 `Content/Carla/Static/TrafficSign`으로 이동합니다. 여기서 선택할 수 있는 여러 교통 표지판 블루프린트를 찾을 수 있습니다.

__2.__ 교통 표지판을 장면으로 드래그하여 원하는 위치에 배치합니다. 키보드의 스페이스바를 눌러 위치 지정, 회전, 크기 조절 도구를 전환할 수 있습니다.

__3.__ _Details_ 패널에서 _BoxTrigger_ 컴포넌트를 선택하고 _Transform_ 섹션의 값을 조정하여 각 교통 표지판의 [`트리거 볼륨`][triggerlink]을 조정합니다. 이것은 교통 표지판의 영향 범위를 결정합니다. 모든 교통 표지판에 트리거 볼륨이 있는 것은 아닙니다. 양보, 정지, 속도 제한 표지판에는 트리거 볼륨이 포함됩니다.

---

## 다음 단계

아래의 도구와 가이드를 사용하여 맵 커스터마이즈를 계속하세요:

- [맵에 서브레벨 구현하기](tuto_M_custom_layers.md)
- [절차적 건물 도구로 건물 추가하기](tuto_M_custom_buildings.md)
- [도로 페인터 도구로 도로 커스터마이즈하기](tuto_M_custom_road_painter.md)
- [날씨 커스터마이즈하기](tuto_M_custom_weather_landscape.md#weather-customization)
- [연속 메시로 지형 커스터마이즈하기](tuto_M_custom_weather_landscape.md#add-serial-meshes)

커스터마이즈가 완료되면 [보행자 내비게이션 정보를 생성](tuto_M_generate_pedestrian_navigation.md)할 수 있습니다.

---

과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문할 수 있습니다.