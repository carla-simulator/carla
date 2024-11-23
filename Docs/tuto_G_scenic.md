# Scenic

이 가이드는 단일 시나리오 정의로 다양한 다중 시나리오를 생성하기 위해 CARLA에서 Scenic을 사용하는 방법을 개괄적으로 설명합니다. Scenic 문법에 대한 사전 지식이 있다고 가정합니다. Scenic에 대해 더 자세히 알아보려면 ["Scenic 시작하기"](https://scenic-lang.readthedocs.io/en/latest/quickstart.html) 가이드를 읽고 [정적](https://scenic-lang.readthedocs.io/en/latest/tutorials/tutorial.html) 및 [동적](https://scenic-lang.readthedocs.io/en/latest/tutorials/dynamics.html) 시나리오 생성 튜토리얼을 살펴보세요.

이 가이드를 마치면 다음을 알게 됩니다:

- CARLA에서 Scenic 스크립트를 실행하는 데 필요한 최소 요구사항
- 다양한 시나리오 시뮬레이션을 생성하기 위한 간단한 시나리오 정의 작성 방법
- CARLA에서 Scenic 스크립트를 실행하는 방법
- CARLA에서 Scenic 시뮬레이션을 구성하는 데 사용되는 매개변수

---

- [__시작하기 전에__](#시작하기-전에)
- [__Scenic 도메인__](#scenic-도메인)
- [__CARLA와 함께 사용할 Scenic 시나리오 생성__](#carla와-함께-사용할-scenic-시나리오-생성)
- [__시나리오 실행__](#시나리오-실행)
- [__추가 매개변수__](#추가-매개변수)

---

## 시작하기 전에

CARLA에서 Scenic을 사용하기 전에 다음 요구사항을 충족해야 합니다:

- [Python 3.8](https://www.python.org/downloads/) 이상 설치 
- [Scenic](https://scenic-lang.readthedocs.io/en/latest/quickstart.html#installation) 설치

---

## Scenic 도메인

Scenic에는 사용자가 모든 주행 시뮬레이터에서 실행할 수 있는 시나리오를 정의할 수 있는 일반 주행 도메인이 있습니다. 또한 각 시뮬레이터에 특화된 다른 도메인도 있습니다. Scenic 도메인에 대한 자세한 정보는 [여기](https://scenic-lang.readthedocs.io/en/latest/libraries.html)를 확인하세요.

각 도메인에서 특히 중요한 것은 동작(behaviour)과 액션(action) 정의입니다. Scenic 주행 도메인과 CARLA 도메인의 동작과 액션에 대한 참조 자료는 아래 링크를 확인하세요:

- [Scenic 주행 도메인의 동작](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.behaviors.html)
- [CARLA 도메인의 동작](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.behaviors.html)
- [Scenic 주행 도메인의 액션](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.actions.html)
- [CARLA 도메인의 액션](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.actions.html#module-scenic.simulators.carla.actions)

---

## CARLA와 함께 사용할 Scenic 시나리오 생성

이 섹션에서는 도로의 장애물로 인해 선행 차량이 갑자기 감속하는 기본적인 Scenic 스크립트 작성 방법을 설명합니다. 그러면 자아 차량은 선행 차량과의 충돌을 피하기 위해 갑자기 제동해야 합니다. [전체 스크립트](https://github.com/BerkeleyLearnVerify/Scenic/blob/master/examples/carla/Carla_Challenge/carlaChallenge2.scenic)는 더 복잡한 도로 네트워크를 포함하는 다른 예제와 함께 Scenic 저장소에서 찾을 수 있습니다.

__1.__ 맵 매개변수를 설정하고 시나리오에서 사용할 모델을 선언합니다:

- [`map`][scenic_map] 매개변수의 값으로 `.xodr` 파일을 설정해야 합니다. 이는 나중에 도로 네트워크 정보를 생성하는 데 사용됩니다.
- `carla_map` 매개변수는 시뮬레이션에서 사용하고 싶은 CARLA 맵의 이름을 나타냅니다. 이것이 정의되면 Scenic은 맵의 모든 에셋(건물, 나무 등)을 로드하고, 정의되지 않으면 [OpenDRIVE 단독 모드](adv_opendrive.md)가 사용됩니다.
- 모델은 CARLA에서 시나리오를 실행하는 데 특화된 모든 유틸리티를 포함합니다. 이는 CARLA에서 실행하려는 모든 스크립트에서 정의되어야 합니다.

```scenic
## 맵과 모델 설정
param map = localPath('../../../tests/formats/opendrive/maps/CARLA/Town01.xodr')
param carla_map = 'Town01'
model scenic.simulators.carla.model
```

[scenic_map]: https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.model.html?highlight=map#module-scenic.domains.driving.model

__2.__ 시나리오에서 사용할 상수를 정의합니다:

시나리오에는 선행 차량과 자아 차량, 두 대의 차량이 포함됩니다. 자아 차량 모델, 두 차량의 속도, 제동을 위한 거리 임계값, 적용할 제동량을 정의할 것입니다.

```scenic
## 상수
EGO_MODEL = "vehicle.lincoln.mkz_2017"
EGO_SPEED = 10
EGO_BRAKING_THRESHOLD = 12

LEAD_CAR_SPEED = 10
LEADCAR_BRAKING_THRESHOLD = 10

BRAKE_ACTION = 1.0
```

__3__. 시나리오 동작을 정의합니다:

이 시나리오에서는 Scenic [동작 라이브러리](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.behaviors.html)를 사용하여 자아 차량이 미리 정의된 속도로 차선을 따라가다가 다른 차량과의 거리가 특정 거리 이내로 가까워지면 급제동하도록 지시할 것입니다. 선행 차량도 미리 정의된 속도로 차선을 따라가다가 어떤 물체와의 거리가 일정 거리 이내가 되면 급제동할 것입니다:

```scenic
## 동작 정의
# 자아 차량 동작: 차선을 따라가고, 선행 차량과의 거리가 임계값을 넘으면 제동
behavior EgoBehavior(speed=10):
    try:
        do FollowLaneBehavior(speed)

    interrupt when withinDistanceToAnyCars(self, EGO_BRAKING_THRESHOLD):
        take SetBrakeAction(BRAKE_ACTION)

# 선행 차량 동작: 차선을 따라가고, 장애물과의 거리가 임계값을 넘으면 제동
behavior LeadingCarBehavior(speed=10):
    try: 
        do FollowLaneBehavior(speed)

    interrupt when withinDistanceToAnyObjs(self, LEADCAR_BRAKING_THRESHOLD):
        take SetBrakeAction(BRAKE_ACTION)
```

__4.__ 도로 네트워크를 생성합니다:

Scenic [도로 라이브러리](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.roads.html)를 사용하여 도로 네트워크 기하학과 교통 정보를 생성합니다. 도로 네트워크는 [`Network`](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.roads.html#scenic.domains.driving.roads.Network) 클래스의 인스턴스로 표현되며 스크립트 시작 부분에서 정의한 `.xodr` 파일에서 생성됩니다.

```scenic
## 공간 관계 정의
# 'lanes' 리스트의 모든 요소에서 균일하게 무작위 선택하기 위해 '*'를 사용해야 합니다
lane = Uniform(*network.lanes)
```

__5.__ 장면을 설정합니다:

이제 차량의 시작 위치와 물체의 배치를 정의할 것입니다.

- 차선 중앙에 쓰레기통을 배치합니다:

```scenic
obstacle = Trash on lane.centerline
```

- 장애물 뒤 50~30미터 사이의 거리에서 미리 정의된 속도로 도로를 따라 주행하는 선행 차량을 배치합니다:

```scenic
leadCar = Car following roadDirection from obstacle for Range(-50, -30),
        with behavior LeadingCarBehavior(LEAD_CAR_SPEED)
```

- 선행 차량 뒤 15~10미터 사이의 거리에서 미리 정의된 속도로 도로를 따라 주행하는 자아 차량을 배치합니다:

```scenic
ego = Car following roadDirection from leadCar for Range(-15, -10),
        with blueprint EGO_MODEL,
        with behavior EgoBehavior(EGO_SPEED)
```

- 장면이 교차로에서 80미터 이상 떨어진 곳에서 일어나도록 요구합니다:

```scenic
require (distance to intersection) > 80
```

__6.__ 장면이 언제 끝나는지 알 수 있도록 종료 지점을 설정합니다:

시나리오는 자아 차량의 속도가 초당 0.1미터 미만이 되고 장애물과의 거리가 30미터 미만일 때 종료됩니다.

```scenic
terminate when ego.speed < 0.1 and (distance to obstacle) < 30
```

---

### 시나리오 실행

Scenic 시나리오를 실행하려면:

__1.__ CARLA 서버를 시작합니다.

__2.__ 다음 명령을 실행합니다:

```scenic
scenic path/to/scenic/script.scenic --simulate
```

pygame 창이 나타나고 시나리오가 반복적으로 실행되며, 매번 스크립트에 설정된 제한 범위 내에서 고유한 시나리오가 생성됩니다. 시나리오 생성을 중지하려면 터미널에서 `ctrl + C`를 누르세요.

---

### 추가 매개변수

CARLA 모델은 [`param` 문](https://scenic-lang.readthedocs.io/en/latest/syntax_details.html#param-identifier-value)을 사용하여 시나리오에서 재정의하거나 [`--param` 옵션](https://scenic-lang.readthedocs.io/en/latest/options.html#cmdoption-p)을 사용하여 명령줄에서 재정의할 수 있는 여러 전역 매개변수를 제공합니다.

아래는 CARLA 모델에서 구성 가능한 매개변수 표입니다:

| 이름 | 값 | 설명 |
|------|-------|-------------|
| `carla_map` | `str` | 사용할 CARLA 맵의 이름(예: 'Town01'). `None`으로 설정하면 CARLA는 [`map`][scenic_map] 매개변수에 정의된 `.xodr` 파일을 사용하여 OpenDRIVE 단독 모드에서 월드를 생성하려고 시도합니다. |
| `timestep` | `float` | 시뮬레이션에 사용할 타임스텝(Scenic이 CARLA를 중단하여 동작 실행, 요구사항 확인 등을 수행하는 빈도)을 초 단위로 지정. 기본값은 0.1초. |
| `weather` | `str` 또는 `dict` | 시뮬레이션에 사용할 날씨. CARLA 날씨 프리셋 중 하나를 식별하는 문자열(예: 'ClearSunset') 또는 모든 [날씨 매개변수](python_api.md#carla.WeatherParameters))를 지정하는 딕셔너리일 수 있습니다. 기본값은 모든 날씨 프리셋에 대한 균일 분포입니다. |
| `address` | `str` | CARLA에 연결할 IP 주소. 기본값은 localhost(127.0.0.1)입니다.|
| `port` | `int` | CARLA에 연결할 포트. 기본값은 2000입니다. |
| `timeout` | `float` | CARLA 연결 시도 시 대기할 최대 시간(초). 기본값은 10입니다.|
| `render` | `int` | CARLA가 자아 객체의 시점에서 시뮬레이션을 보여주는 창을 생성할지 여부: 예는 `1`, 아니오는 `0`. 기본값 `1`. |
| `record` | `str` | 비어있지 않은 경우, 시뮬레이션 재생을 위한 CARLA 기록 파일을 저장할 폴더. |

<br>

이로써 CARLA에서 Scenic을 사용하여 다양한 시나리오를 생성하고 실행하는 방법에 대한 설명을 마칩니다. 이 가이드를 통해 Scenic의 기본 구성 요소를 이해하고 CARLA와 함께 사용하여 자율주행 테스트 시나리오를 만들 수 있게 되었을 것입니다. 더 복잡한 시나리오를 만들기 위해서는 Scenic의 공식 문서와 예제를 참조하시기 바랍니다.