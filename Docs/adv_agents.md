# CARLA 에이전트

CARLA 에이전트 스크립트를 통해 차량이 무작위로 끝없는 경로를 따르거나 주어진 목적지까지 최단 경로를 택할 수 있습니다. 에이전트는 신호등을 준수하고 도로의 다른 장애물에 반응합니다. 세 가지 유형의 에이전트가 제공됩니다. 목표 속도, 제동 거리, 차량 추종 행동 등의 매개변수를 수정할 수 있습니다. 사용자의 요구에 맞게 액터 클래스를 수정하거나 기본 클래스로 사용하여 커스텀 에이전트를 만들 수 있습니다.

- [__에이전트 스크립트 개요__](#에이전트-스크립트-개요)
    - [계획 및 제어](#계획-및-제어)
    - [에이전트 동작](#에이전트-동작)
- [__에이전트 구현__](#에이전트-구현)
- [__동작 유형__](#동작-유형)
    - [자체 동작 유형 만들기](#자체-동작-유형-만들기)
- [__에이전트 만들기__](#에이전트-만들기)

---

## 에이전트 스크립트 개요

CARLA 에이전트와 관련된 주요 스크립트는 `PythonAPI/carla/agents/navigation`에 있습니다. 이들은 __계획 및 제어__와 __에이전트 동작__ 두 가지 범주로 나뉩니다.

### 계획 및 제어

- __`controller.py`:__ 종방향과 횡방향 PID 컨트롤러를 __VehiclePIDController__ 단일 클래스로 결합하여 CARLA 클라이언트 측에서 차량의 저수준 제어에 사용됩니다.
- __`global_route_planner.py`:__ CARLA 서버에서 상세한 토폴로지를 가져와 월드 맵의 그래프 표현을 구축하여 __Local Planner__에 웨이포인트와 도로 옵션 정보를 제공합니다.
- __`local_planner.py`:__ __VehiclePIDController__의 제어 입력에 기반하여 웨이포인트를 따릅니다. 웨이포인트는 __Global Route Planner__가 제공하거나 [Traffic Manager](adv_traffic_manager.md)와 유사하게 교차로에서 무작위 경로를 선택하여 동적으로 계산할 수 있습니다.

### 에이전트 동작

- __`basic_agent.py`:__ 맵을 돌아다니거나 최단 거리로 목표 목적지에 도달할 수 있는 __Basic Agent__를 구현하는 에이전트 기본 클래스를 포함합니다. 다른 차량을 피하고 신호등에 반응하지만 정지 표지판은 무시합니다.
- __`behavior_agent.py`:__ 신호등, 표지판, 속도 제한을 준수하면서 다른 차량을 따라가며 최단 거리로 목표 목적지에 도달할 수 있는 더 복잡한 __Behavior Agent__를 구현하는 클래스를 포함합니다. 에이전트의 행동을 조건화하는 세 가지 사전 정의된 유형이 있습니다.
- __`behavior_types.py`:__ __Behavior Agent__를 조건화하는 동작 유형(신중한, 보통, 공격적)의 매개변수를 포함합니다.

---

## 에이전트 구현

이 섹션에서는 자체 스크립트에서 예제 CARLA 에이전트 클래스를 사용하는 방법을 설명합니다. 섹션 끝에서 다양한 에이전트를 실제로 보여주는 예제 스크립트를 실행하는 방법을 알아볼 수 있습니다.

__1.__ 사용하고자 하는 에이전트 클래스를 임포트하세요:

```py
# 기본 에이전트를 임포트하려면
from agents.navigation.basic_agent import BasicAgent

# 동작 에이전트를 임포트하려면
from agents.navigation.behavior_agent import BehaviorAgent
```

__2.__ 어떤 차량도 에이전트로 만들 수 있습니다. [차량을 스폰](core_actors.md#spawning)하고 이를 에이전트 클래스에 인자로 전달하여 인스턴스화하세요:

```py
# 기본 에이전트를 시작하려면
agent = BasicAgent(vehicle)

# 공격적 프로필로 동작 에이전트를 시작하려면
agent = BehaviorAgent(vehicle, behavior='aggressive')
```

동작 유형과 자체 구성 방법에 대해 자세히 알아보려면 [__동작 유형__](#동작-유형) 섹션을 읽어보세요.

__3.__ 에이전트가 이동할 목적지를 설정할 수 있습니다. 에이전트의 목적지를 설정하지 않으면 맵을 무한히 돌아다닐 것입니다. 목적지를 설정하려면 에이전트에 [위치](python_api.md#carlalocation)를 제공하세요:

```py
destination = random.choice(spawn_points).location
agent.set_destination(destination)
```

__4.__ 내비게이션 단계에서 차량 제어와 동작이 적용됩니다. 각 단계에서 __Basic Agent__는 차량 제어를 적용하고 긴급 정지를 수행하여 차량이나 신호등에 반응할 것입니다. __Behavior Agent__는 적용한 동작 유형에 따라 신호등에 반응하고, 보행자를 피하고, 차량을 따라가며 교차로를 탐색할 것입니다:

```py
while True:
    vehicle.apply_control(agent.run_step())
```

__5.__ 에이전트가 궤적을 완료했는지 확인하고 그때 동작을 수행할 수 있습니다. 다음 스니펫은 차량이 목적지에 도달하면 시뮬레이션을 종료합니다:

```py
while True:
    if agent.done():
        print("목표에 도달했습니다. 시뮬레이션을 종료합니다")
        break

    vehicle.apply_control(agent.run_step())
```

__6.__ 에이전트가 목표 목적지에 도달했을 때 시뮬레이션을 종료하는 대신 에이전트가 따를 새로운 무작위 경로를 생성할 수 있습니다:

```py
while True:
    if agent.done():
        agent.set_destination(random.choice(spawn_points).location)
        print("목표에 도달했습니다. 다른 목표를 검색합니다")

    vehicle.apply_control(agent.run_step())
```

__Basic Agent__는 에이전트 동작을 조작하거나 따를 경로를 프로그래밍하기 위한 몇 가지 메서드를 제공합니다:

- __`set_target_speed(speed)`:__ 목표 속도를 km/h로 설정
- __`follow_speed_limits(value=True)`:__ 에이전트가 속도 제한을 따르도록 설정
- __`set_destination(end_location, start_location=None)`:__ 에이전트는 가능한 최단 경로를 통해 특정 시작 위치에서 끝 위치로 이동합니다. 시작 위치가 제공되지 않으면 현재 에이전트 위치를 사용합니다.
- __`set_global_plan(plan, stop_waypoint_creation=True, clean_queue=True)`:__ 에이전트가 따를 특정 계획을 추가합니다. plan 인자는 에이전트가 취해야 할 경로가 될 `[carla.Waypoint, RoadOption]` 목록으로 구성되어야 합니다. `stop_waypoint_creation`은 경로가 완료되면 웨이포인트가 자동으로 생성되는 것을 방지합니다. `clean_queue`는 에이전트의 현재 계획을 재설정합니다.
- __`trace_route(start_waypoint, end_waypoint)`:__ Global Route Planner에서 두 웨이포인트 사이의 최단 거리를 가져와 에이전트가 따를 수 있는 `[carla.Waypoint, RoadOption]` 목록으로 경로를 반환합니다.
- __`ignore_traffic_lights(active=True)`:__ 에이전트가 신호등을 무시하거나 준수하도록 설정
- __`ignore_stop_signs(active=True)`:__ 에이전트가 정지 표지판을 무시하거나 준수하도록 설정
- __`ignore_vehicles(active=True)`:__ 에이전트가 다른 차량을 무시하거나 반응하도록 설정

`PythonAPI/examples`에 있는 `automatic_control.py` 스크립트는 Basic과 Behavior 에이전트가 실제로 작동하는 예제입니다. 스크립트를 시도하려면 예제 디렉토리로 이동하여 다음 명령을 실행하세요:

```sh
# 기본 에이전트로 실행하려면
python3 automatic_control.py --agent=Basic

# 동작 에이전트로 실행하려면
python3 automatic_control.py --agent=Behavior --behavior=aggressive
```

---

## 동작 유형

동작 에이전트의 동작 유형은 `behavior_types.py`에 정의되어 있습니다. 사전 구성된 세 가지 프로필은 __'cautious'__(신중), __'normal'__(보통), __'aggressive'__(공격적)입니다. 설정된 프로필을 사용하거나, 수정하거나, 자체 프로필을 만들 수 있습니다. 다음 변수를 조정할 수 있습니다:

- __`max_speed`__: 차량이 도달할 수 있는 최대 속도(km/h)
- __`speed_lim_dist`__: 차량의 목표 속도가 현재 속도 제한에서 얼마나 멀어질지를 km/h로 정의하는 값(예: 속도 제한이 30km/h이고 `speed_lim_dist`가 10km/h이면 목표 속도는 20km/h가 됨)
- __`speed_decrease`__: 앞에 있는 더 느린 차량에 접근할 때 차량이 얼마나 빨리 감속할지(km/h)
- __`safety_time`__: 충돌 시까지의 시간; 앞 차량이 갑자기 제동할 경우 차량이 충돌하기까지 걸리는 대략적인 시간
- __`min_proximity_threshold`__: 차량이 회피나 추종과 같은 동작을 수행하기 전에 다른 차량이나 보행자와의 최소 거리(미터)
- __`braking_distance`__: 차량이 긴급 정지를 수행하는 보행자나 차량과의 거리
- __`tailgate_counter`__: 마지막 추종 후 너무 빨리 추종하는 것을 피하기 위한 카운터

## 자체 동작 유형 만들기

자체 동작 유형을 만들려면:

__1.__ `behavior_types.py`에서 동작 유형의 클래스를 만드세요:

```py
class ProfileName(object):
    # 값 정의 완료
```

__2.__ `behavior_agent.py` 스크립트에서 동작 유형을 정의하고 인스턴스화하세요:

```py
# 에이전트 동작을 위한 매개변수
if behavior == 'cautious':
    self._behavior = Cautious()

elif behavior == 'normal':
    self._behavior = Normal()

elif behavior == 'aggressive':
    self._behavior = Aggressive()

elif behavior == '<type_name>':
    self._behavior = <TypeName>()
```

---
## 에이전트 만들기

CARLA 에이전트는 사용자가 실행할 수 있는 에이전트의 예시일 뿐입니다. 사용자는 __Basic Agent__를 기반으로 자체 에이전트를 구축할 수 있습니다. 가능성은 무한합니다. 모든 에이전트에는 __초기화__와 __실행 단계__라는 두 가지 필수 요소만 있습니다.

아래에서 커스텀 에이전트의 최소 레이아웃 예제를 찾을 수 있습니다:

```py
import carla

from agents.navigation.basic_agent import BasicAgent

class CustomAgent(BasicAgent):
    def __init__(self, vehicle, target_speed=20, debug=False):
        """
        :param vehicle: 로컬 플래너 로직을 적용할 액터
        :param target_speed: 차량이 이동할 속도(km/h)
        """
        super().__init__(target_speed, debug)

    def run_step(self, debug=False):
        """
        내비게이션의 한 단계를 실행합니다.
        :return: carla.VehicleControl
        """
        # 각 시뮬레이션 단계에서 취할 동작
        control = carla.VehicleControl()
        return control
```

자체 에이전트를 만드는 방법에 대한 더 많은 아이디어를 얻으려면 구조와 기능을 살펴보기 위해 `basic_agent.py`와 `behavior_agent.py` 스크립트를 확인하세요.

---

제공된 에이전트 스크립트를 탐색하고 확장하거나 자체 에이전트를 만드는 기준선으로 사용할 수 있습니다. 에이전트에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions/)에 자유롭게 게시하세요.