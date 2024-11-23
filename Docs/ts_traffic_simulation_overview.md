# CARLA의 교통 시뮬레이션

교통 시뮬레이션은 자율주행 시스템의 정확하고 효율적인 학습 및 테스트에 필수적입니다. CARLA는 교통 및 특정 교통 시나리오를 시뮬레이션하기 위한 다양한 옵션을 제공합니다. 이 섹션에서는 사용 사례에 가장 적합한 옵션을 선택하는 데 도움이 되는 사용 가능한 옵션들을 개괄적으로 설명합니다.

- [__Traffic Manager__](#traffic-manager)
- [__Scenario Runner와 OpenScenario__](#scenario-runner와-openscenario)
- [__Scenic__](#scenic)
- [__SUMO__](#sumo)

---

## Traffic Manager

[__Traffic Manager__](adv_traffic_manager.md)는 클라이언트 측에서 시뮬레이션의 특정 차량들을 제어하는 CARLA 내의 모듈입니다. 차량은 [`carla.Vehicle.set_autopilot`](https://carla.readthedocs.io/en/latest/python_api/#carla.Vehicle.set_autopilot) 메서드나 [`command.SetAutopilot`](https://carla.readthedocs.io/en/latest/python_api/#commandsetautopilot) 클래스를 통해 Traffic Manager에 등록됩니다. 각 차량의 제어는 서로 다른 스레드에서 실행되는 [구별된 단계](adv_traffic_manager.md#stages)의 사이클을 통해 관리됩니다.

__다음과 같은 경우에 유용합니다:__

- 시뮬레이션에 현실적인 도시 교통 상황을 구현할 때
- 특정 학습 환경을 설정하기 위해 [교통 행동을 커스터마이즈](tuto_G_traffic_manager.md)할 때
- 계산 효율성을 개선하면서 단계 관련 기능과 데이터 구조를 개발할 때

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/docs-preview/adv_traffic_manager/" target="_blank" class="btn btn-neutral" title="Traffic Manager로 이동">
Traffic Manager로 이동</a>
</p>
</div>

---

## Scenario Runner와 OpenScenario

Scenario Runner는 기본적으로 [미리 정의된 교통 시나리오](https://carla-scenariorunner.readthedocs.io/en/latest/list_of_scenarios/)를 제공하며, 사용자가 Python이나 [OpenSCENARIO 1.0 표준](https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_foreword)을 사용하여 [자신만의 시나리오를 정의](https://carla-scenariorunner.readthedocs.io/en/latest/creating_new_scenario/)할 수 있게 합니다.

OpenSCENARIO의 주요 용도는 여러 차량이 관련된 복잡한 기동을 설명하는 것입니다. 사용자는 [여기](https://carla-scenariorunner.readthedocs.io/en/latest/openscenario_support/)에서 Scenario Runner가 지원하는 OpenSCENARIO 기능을 확인할 수 있습니다. 여기에는 기동(Maneuvers), 동작(Actions), 조건(Conditions), 스토리(Stories), 스토리보드(Storyboard)가 포함됩니다.

Scenario Runner는 CARLA 메인 패키지와는 [별도로](https://github.com/carla-simulator/scenario_runner) 설치해야 합니다.

__다음과 같은 경우에 유용합니다:__

- [CARLA 리더보드](https://leaderboard.carla.org/)에서 평가할 AD 에이전트를 준비하기 위한 복잡한 교통 시나리오와 경로를 만들 때
- 시나리오 시뮬레이션 기록에 대해 실행할 수 있는 맞춤형 [메트릭](https://carla-scenariorunner.readthedocs.io/en/latest/metrics_module/)을 정의할 때 (시뮬레이션을 반복적으로 실행할 필요가 없음)

<div class="build-buttons">
<p>
<a href="https://carla-scenariorunner.readthedocs.io" target="_blank" class="btn btn-neutral" title="Scenario Runner로 이동">
Scenario Runner로 이동</a>
</p>
</div>

---

## Scenic

[Scenic](https://scenic-lang.readthedocs.io)은 로봇과 자율주행 자동차와 같은 사이버-물리 시스템의 환경을 모델링하기 위한 도메인 특화 확률적 프로그래밍 언어입니다. Scenic은 CARLA 시뮬레이터에서 Scenic 스크립트를 실행하기 위한 [특수 도메인](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.html)을 제공합니다.

Scenic 시나리오 정의는 읽고 구성하기 쉽습니다. 간단한 시나리오를 만드는 튜토리얼은 [여기](tuto_G_scenic.md)에서 제공됩니다.

__다음과 같은 경우에 유용합니다:__

- 하나의 시나리오 정의로 여러 가지 다양한 시나리오를 생성할 때
- 월드의 상태에 반응하여 동적 에이전트가 시간에 따라 취할 확률적 정책을 정의할 때

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/tuto_G_scenic/" target="_blank" class="btn btn-neutral" title="Scenic 튜토리얼로 이동">
Scenic 튜토리얼로 이동</a>
</p>
</div>

---

## SUMO

[SUMO](https://sumo.dlr.de/docs/SUMO_at_a_Glance.html)는 오픈소스, 미시적, 다중 모달 교통 시뮬레이션입니다. SUMO에서는 각 차량이 명시적으로 모델링되며, 자체 경로를 가지고 네트워크를 통해 개별적으로 이동합니다. 시뮬레이션은 기본적으로 결정론적이지만 무작위성을 도입하기 위한 다양한 옵션이 있습니다.

CARLA는 SUMO와의 코시뮬레이션 기능을 제공하여 두 시뮬레이터 간에 작업을 분산할 수 있게 합니다. 차량은 SUMO를 통해 CARLA에서 스폰될 수 있으며, Traffic Manager처럼 SUMO에 의해 관리됩니다.

__다음과 같은 경우에 유용합니다:__

- CARLA와 SUMO의 기능을 하나의 패키지로 활용하고자 할 때

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/adv_sumo/" target="_blank" class="btn btn-neutral" title="SUMO 코시뮬레이션으로 이동">
SUMO 코시뮬레이션으로 이동</a>
</p>
</div>

---

CARLA에서 교통을 시뮬레이션하는 다양한 옵션에 대해 궁금한 점이 있으시다면 포럼이나 [Discord](https://discord.gg/8kqACuC)에 자유롭게 글을 올려주세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>