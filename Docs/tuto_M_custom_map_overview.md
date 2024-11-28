# 새로운 맵 추가하기

CARLA 사용자는 사용자 정의 맵을 만들어 시뮬레이션을 실행할 수 있습니다. CARLA에 사용자 정의 맵을 가져오는 방법은 여러 가지가 있습니다. 사용할 방법은 패키지 버전의 CARLA를 사용하는지 또는 소스에서 빌드한 버전을 사용하는지에 따라 달라집니다. 이 섹션에서는 프로세스를 시작하는 데 필요한 것들, 가져오기에 사용할 수 있는 다양한 옵션, 그리고 사용 가능한 커스터마이즈 및 보행자 내비게이션 도구에 대한 개요를 제공합니다.

- [__개요__](#overview)
- [__생성__](#generation)
- [__가져오기__](#importation)
- [__커스터마이즈__](#customization)
- [__보행자 내비게이션 생성__](#generate-pedestrian-navigation)

---

## 개요

CARLA에서 사용자 정의 맵을 사용하는 것은 크게 네 가지 주요 프로세스로 이루어집니다:

1. 생성
2. 가져오기
3. 커스터마이즈
4. 보행자 내비게이션

각 프로세스에 대한 추가적인 일반 정보는 계속해서 읽어보세요.

---

## 생성

CARLA는 `.fbx` 형식의 맵 지오메트리 정보와 `.xodr` 형식의 OpenDRIVE 정보가 필요합니다. 현재 이러한 파일들을 생성하는 데 권장되는 소프트웨어는 RoadRunner입니다.

__[이 가이드](tuto_M_generate_map.md)는 RoadRunner를 사용하여 맵 정보를 생성하는 방법을 설명합니다.__

---

## 가져오기

CARLA에 맵을 가져오는 방법은 여러 가지가 있습니다.

__패키지 버전__의 CARLA를 사용하는 경우, Docker를 사용하여 맵을 가져오게 됩니다. 이 옵션은 Linux에서만 사용할 수 있으며, 언리얼 에디터를 사용하여 맵을 커스터마이즈할 수 없습니다. __가이드는 [여기](tuto_M_add_map_package.md)에서 찾을 수 있습니다.__

__소스 빌드__ 버전의 CARLA를 사용하는 경우, 맵을 가져오는 데 사용할 수 있는 세 가지 방법이 있습니다:

1. 자동 `make import` 프로세스 사용(권장). __가이드는 [여기](tuto_M_add_map_source.md)에서 찾을 수 있습니다.__
2. RoadRunner 플러그인 사용. __가이드는 [여기](tuto_M_add_map_alternative.md#roadrunner-plugin-import)에서 찾을 수 있습니다.__
3. 언리얼 엔진에 수동으로 맵 가져오기. __가이드는 [여기](tuto_M_add_map_alternative.md#manual-import)에서 찾을 수 있습니다.__

다음 비디오는 CARLA에 맵을 가져오는 데 사용할 수 있는 일부 방법을 설명합니다:

<iframe width="560" height="315" src="https://www.youtube.com/embed/mHiUUZ4xC9o" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

<br>

---

## 커스터마이즈

지형에 추가할 수 있는 수백 개의 정적 메시 외에도, CARLA는 맵을 커스터마이즈하는 데 도움이 되는 여러 도구와 가이드를 제공합니다:

- __서브레벨 추가:__ 서브레벨을 사용하면 여러 사람이 동시에 같은 맵에서 작업할 수 있습니다. 또한 CARLA 레이어드 맵처럼 Python API로 맵의 레이어를 토글할 수 있습니다. __가이드는 [여기](tuto_M_custom_layers.md)에서 찾을 수 있습니다.__
- __기본 날씨 설정:__ 다양한 날씨 프리셋을 실험하고, 적절한 조합을 찾으면 맵의 기본 날씨로 설정하세요. __가이드는 [여기](tuto_M_custom_weather_landscape.md#weather-customization)에서 찾을 수 있습니다.__
- __지형 채우기:__ 블루프린트를 사용하여 가로등, 전선, 벽 등과 같은 반복되는 메시로 지형을 채우세요. __가이드는 [여기](tuto_M_custom_weather_landscape.md#add-serial-meshes)에서 찾을 수 있습니다.__
- __도로 페인팅:__ 다양한 텍스처를 혼합하는 마스터 재질로 도로를 페인팅하세요. 떨어진 잎, 균열, 맨홀 등의 데칼과 메시를 추가하세요. __가이드는 [여기](tuto_M_custom_road_painter.md)에서 찾을 수 있습니다.__
- __절차적 건물 추가:__ 절차적 건물 블루프린트를 사용하여 사용자 정의 크기, 층수, 다양한 메시 조합을 가진 건물을 추가하세요. __가이드는 [여기](tuto_M_custom_buildings.md)에서 찾을 수 있습니다.__
- __신호등과 표지판 추가:__ 신호등과 표지판을 추가하고 영향 범위를 구성하세요. 교차로의 신호등을 그룹화하세요. __가이드는 [여기](tuto_M_custom_add_tl.md)에서 찾을 수 있습니다.__

---

## 보행자 내비게이션 생성

보행자가 맵에서 생성되고 이동할 수 있도록 하려면 CARLA에서 제공하는 도구를 사용하여 보행자 내비게이션 정보를 생성해야 합니다. 내비게이션 경로 위에 장애물이 생성되지 않도록 맵 커스터마이즈를 완료한 후에 보행자 내비게이션을 생성해야 합니다. __가이드는 [여기](tuto_M_generate_pedestrian_navigation.md)에서 찾을 수 있습니다.__

---

위 과정에 대해 질문이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에 자유롭게 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>