# CARLA 시작하기

CARLA 시뮬레이터는 자율주행(AD) 및 기타 로봇공학 애플리케이션을 위한 종합적인 학습 데이터 생성 솔루션입니다. CARLA는 실제 마을, 도시, 고속도로와 그 안의 차량 및 다른 객체들을 매우 사실적으로 구현한 환경을 시뮬레이션합니다.

또한 CARLA 시뮬레이터는 평가와 테스트 환경으로도 매우 유용합니다. 학습된 자율주행 에이전트를 시뮬레이션 환경에 배포하여 성능과 안전성을 테스트하고 평가할 수 있으며, 이 모든 것을 하드웨어나 다른 도로 사용자에게 어떤 위험도 없이 안전한 가상 환경에서 수행할 수 있습니다.

이 튜토리얼에서는 CARLA의 기본적인 작업 흐름을 다룹니다. 서버 실행과 클라이언트 연결부터 시작해서 차량 추가, 센서 설정, 머신러닝용 학습 데이터 생성까지 설명합니다. 세부 사항은 최소화하고 핵심 단계들을 최대한 효율적으로 안내하는 것을 목표로 합니다. 블루프린트 라이브러리의 다양한 차량이나 여러 종류의 센서와 같은 각 부분에 대한 자세한 내용은 본문의 링크나 왼쪽 메뉴를 참고해주세요.

* [__CARLA 실행하기__](#carla-실행과-클라이언트-연결)  
* [__맵 불러오기__](#맵-불러오기) 
* [__관찰자 시점 조작__](#관찰자-시점-조작)  
* [__NPC 추가하기__](#npc-추가하기)  
* [__센서 추가하기__](#센서-추가하기)  
* [__차량 움직이기__](#traffic-manager로-차량-움직이기)  
* [__주요 차량 지정하기__](#주요-차량-지정하기)
* [__맵 선택하기__](#맵-선택하기) 
* [__차량 선택하기__](#차량-선택하기) 

## CARLA 실행과 클라이언트 연결

CARLA는 Windows에서는 실행 파일을, Linux에서는 쉘 스크립트를 사용하여 명령줄에서 실행할 수 있습니다. [__Linux__](start_quickstart.md)와 [__Windows__](start_quickstart.md)의 설치 지침을 따른 후 명령줄에서 [__CARLA를 실행__](start_quickstart.md#running-carla)하세요:

```sh
cd /carla/root
./CarlaUE4.sh
```
Python API를 통해 CARLA를 제어하려면 열린 포트를 통해 Python 클라이언트를 서버에 연결해야 합니다. 클라이언트는 [__클라이언트와 월드 객체__](foundations.md#world-and-client)를 통해 시뮬레이터를 제어합니다. Python 노트북을 열거나 새 스크립트를 만든 후, 스크립트 시작 부분이나 main 함수에 다음 코드를 추가하세요:

```py
import carla
import random

# 클라이언트에 연결하고 월드 객체 가져오기
client = carla.Client('localhost', 2000)
world = client.get_world()
```

[__client__](python_api#carlaclient) 객체는 클라이언트의 서버 연결을 유지하고 명령을 실행하거나 데이터를 불러오고 내보내는 여러 기능을 제공합니다. 이를 통해 다른 맵을 불러오거나 현재 맵을 초기 상태로 다시 불러올 수 있습니다.

포트는 사용 가능한 아무 포트나 선택할 수 있으며, 기본값은 2000입니다. *localhost* 대신 컴퓨터의 IP 주소를 사용하여 다른 호스트를 선택할 수도 있습니다. 이렇게 하면 CARLA 서버는 네트워크 컴퓨터에서 실행하고 Python 클라이언트는 개인 컴퓨터에서 실행할 수 있습니다. 이는 CARLA 시뮬레이터 실행용 GPU와 신경망 학습용 GPU를 분리하는 데 특히 유용합니다. 두 작업 모두 그래픽 하드웨어에 많은 부하를 주기 때문입니다.

!!! 참고
    이후 내용은 CARLA가 기본 [__비동기__](adv_synchrony_timestep.md) 모드로 실행 중이라고 가정합니다. 동기 모드를 사용하는 경우 다음 섹션의 일부 코드가 예상대로 작동하지 않을 수 있습니다.

## 맵 불러오기 

CARLA API에서 [__world__](python_api.md#carla.World) 객체는 맵, 건물, 신호등, 차량, 보행자 등 시뮬레이션의 모든 요소에 접근할 수 있게 해줍니다. CARLA 서버는 보통 기본 맵(일반적으로 Town10)을 불러옵니다. 다른 맵으로 CARLA를 실행하고 싶다면 `config.py` 스크립트를 사용하세요:

```sh
./config.py --map Town05 
```

world 객체를 사용하여 클라이언트에서 맵을 불러올 수도 있습니다:

```py
client.load_world('Town05')
``` 

CARLA 맵에 대한 자세한 정보는 [__여기__](core_map.md)에서 확인할 수 있습니다.
## 관찰자 시점 조작

관찰자(Spectator)는 시뮬레이션을 보는 시점입니다. 화면이 있는 컴퓨터에서 CARLA 서버를 실행하면 `-RenderOffScreen` 옵션을 지정하지 않는 한 기본적으로 새 창에서 관찰자 화면이 열립니다.

관찰자 시점은 시뮬레이션을 시각화하는 데 매우 유용합니다. 이를 통해 불러온 맵을 둘러보고, 차량 추가, 날씨 변경, 맵의 여러 레이어 켜고 끄기 등 변경사항을 바로 확인할 수 있으며 디버깅 용도로도 활용할 수 있습니다.

마우스와 키보드를 사용해 관찰자 시점을 자유롭게 움직일 수 있습니다:

- Q - 위로 이동 (창의 위쪽으로)
- E - 아래로 이동 (창의 아래쪽으로)
- W - 앞으로 이동
- S - 뒤로 이동
- A - 왼쪽으로 이동
- D - 오른쪽으로 이동

관찰자 창에서 마우스 왼쪽 버튼을 누른 채 드래그하면 시점을 조절할 수 있습니다:
- 위아래로 드래그: 피치(상하 각도) 조절
- 좌우로 드래그: 요(좌우 각도) 조절

![flying_spectator](../img/tuto_G_getting_started/flying_spectator.gif)

Python API를 통해 관찰자와 그 속성에 접근하고 제어할 수 있습니다:

```py
# 관찰자 객체 가져오기
spectator = world.get_spectator()

# transform을 통해 관찰자의 위치와 회전값 가져오기
transform = spectator.get_transform()

location = transform.location
rotation = transform.rotation

# 빈 transform으로 관찰자 설정
spectator.set_transform(carla.Transform())
# 이렇게 하면 관찰자가 맵의 원점에 위치하며
# 피치, 요, 롤이 모두 0도로 설정됩니다.
# 맵에서 방향을 잡는 데 좋은 방법입니다.
```

## NPC 추가하기

이제 맵을 불러오고 서버가 실행되었으니, 실제 환경을 시뮬레이션하기 위해 교통량과 다른 도로 사용자들(NPC, Non-Player Characters)을 추가해야 합니다.

먼저 블루프린트 라이브러리에서 원하는 차량을 선택합니다:

```py
# 블루프린트 라이브러리에서 차량 블루프린트 필터링하기
vehicle_blueprints = world.get_blueprint_library().filter('*vehicle*')
```
블루프린트를 선택했으니 이제 차량을 생성할 적절한 위치를 맵에서 찾아야 합니다. 각 CARLA 맵은 도로 위에 고르게 분포된 미리 정의된 생성 지점들을 제공합니다.

```py
# 맵의 생성 지점 가져오기
spawn_points = world.get_map().get_spawn_points()

# 맵 전체에 무작위로 50대의 차량 생성
# 각 생성 지점마다 블루프린트 라이브러리에서 무작위로 차량 선택
for i in range(0,50):
    world.try_spawn_actor(random.choice(vehicle_blueprints), random.choice(spawn_points))
```

다음으로 시뮬레이션의 중심이 될 차량을 추가해야 합니다. 자율주행 에이전트를 학습시키려면 에이전트가 제어할 차량이 필요합니다. CARLA에서는 이 차량을 "Ego Vehicle(주요 차량)"이라고 부릅니다.

```py
ego_vehicle = world.spawn_actor(random.choice(vehicle_blueprints), random.choice(spawn_points))
```

차량 외에도 CARLA는 현실적인 주행 시나리오를 시뮬레이션하기 위한 보행자도 제공합니다. CARLA에서는 차량과 보행자를 __액터(actors)__ 라고 부릅니다. 자세한 내용은 [__여기__](core_actors.md)에서 확인할 수 있습니다.

## 센서 추가하기

현대의 자율주행 차량은 여러 센서를 통해 주변 환경을 이해하고 해석합니다. 이러한 센서에는 광학 비디오 카메라, 광학 흐름 카메라, 라이다(LIDAR), 레이더(RADAR), 가속도계 등이 있습니다. CARLA에는 머신러닝 학습 데이터를 생성하기 위한 다양한 종류의 센서 모델이 내장되어 있습니다. 이 센서들은 차량에 부착하거나 CCTV 카메라처럼 고정된 지점에 설치할 수 있습니다.

여기서는 주요 차량에 기본 카메라 센서를 부착하여 영상 데이터를 녹화해보겠습니다:

```py
# 카메라를 차량 위에 설치하기 위한 transform 생성
camera_init_trans = carla.Transform(carla.Location(z=1.5))

# 속성이 정의된 블루프린트로 카메라 생성
camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')

# 카메라를 생성하고 주요 차량에 부착
camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)
```
카메라를 생성한 후에는 `listen()` 메서드를 통해 녹화를 시작해야 합니다. listen 메서드는 데이터를 어떻게 처리할지 정의하는 콜백 함수를 인자로 받습니다. 데이터를 다른 프로그램으로 스트리밍하거나 디스크에 저장할 수 있습니다.

다음과 같이 람다 함수를 콜백으로 사용하여 데이터를 디스크에 저장하겠습니다:

```py
# PyGame 콜백으로 카메라 시작
camera.listen(lambda image: image.save_to_disk('out/%06d.png' % image.frame))
```

이렇게 하면 데이터가 `out/` 폴더에 시뮬레이션 프레임 번호에 따라 이름이 지정된 PNG 이미지 파일들로 저장됩니다.

다양한 종류의 센서들과 그 사용 방법에 대해 더 자세히 알아보려면 [__여기__](core_sensors.md)를 참고하세요.

## Traffic Manager로 차량 움직이기

이제 교통 차량과 주요 차량을 시뮬레이션에 추가하고 카메라 데이터 녹화를 시작했으니, [__Traffic Manager__](adv_traffic_manager.md)를 사용하여 차량들을 움직여야 합니다. Traffic Manager는 CARLA의 핵심 구성 요소로, 차량들이 도로 규칙을 준수하고 실제 도로 사용자처럼 행동하면서 시뮬레이션 맵의 도로를 자율적으로 주행하도록 제어합니다.

`world.get_actors()` 메서드로 시뮬레이션의 모든 차량을 찾은 다음, `set_autopilot()` 메서드를 사용하여 차량의 제어권을 Traffic Manager에 넘깁니다:

```py
for vehicle in world.get_actors().filter('*vehicle*'):
    vehicle.set_autopilot(True)
```

이제 시뮬레이션이 실행되면서 여러 차량이 맵을 돌아다니고, 그 중 한 차량에서 카메라가 데이터를 녹화하고 있습니다. 이 데이터는 자율주행 에이전트를 학습시키는 머신러닝 알고리즘의 입력으로 사용할 수 있습니다. Traffic Manager는 교통 패턴을 커스터마이즈하기 위한 다양한 기능을 제공합니다. 자세한 내용은 [__여기__](tuto_G_traffic_manager.md)에서 확인할 수 있습니다.

지금까지 설명한 것이 시뮬레이션의 가장 기본적인 설정입니다. 이제 데이터 생성에 사용할 수 있는 추가 센서들과 시뮬레이션을 더 상세하고 현실적으로 만들 수 있는 CARLA의 다른 기능들에 대해 문서를 더 자세히 살펴보시기 바랍니다.
## 주요 차량 지정하기

__주요 차량(Ego Vehicle)__ 은 CARLA를 사용할 때 반드시 이해해야 할 중요한 개념입니다. 주요 차량은 시뮬레이션의 중심이 되는 차량을 의미합니다. 대부분의 CARLA 사용 사례에서 이 차량은 센서를 부착하거나 자율주행 머신러닝 시스템이 제어할 차량이 됩니다. 이는 시뮬레이션의 효율성을 높이는 여러 작업의 기준점이 되기 때문에 매우 중요합니다. 예를 들면:

* __대형 맵의 타일 로딩__: 대형 맵(예: Town 12)은 CARLA의 성능을 최적화하기 위해 필요한 부분만 로드되는 타일 구조로 되어 있습니다. 주요 차량의 위치에 따라 어떤 타일을 사용할지 결정되며, 주요 차량 근처의 타일만 로드됩니다.

* __하이브리드 물리 모드__: Traffic Manager가 제어하는 차량이 많은 경우, 모든 차량의 물리 계산은 컴퓨팅 자원을 많이 소모합니다. [하이브리드 물리 모드](adv_traffic_manager.md#hybrid-physics-mode)를 사용하면 주요 차량 주변의 차량들에 대해서만 물리 계산을 수행하므로 컴퓨팅 자원을 절약할 수 있습니다.

주요 차량을 지정하려면 차량을 생성할 때 [carla.Actor](python_api.md#carlaactor) 객체의 [블루프린트](python_api.md#carlaactorblueprint)에서 `role_name` 속성을 설정하면 됩니다:

```py
ego_bp = world.get_blueprint_library().find('vehicle.lincoln.mkz_2020')
ego_bp.set_attribute('role_name', 'hero')
ego_vehicle = world.spawn_actor(ego_bp, random.choice(spawn_points))
```

## 맵 선택하기

![maps_montage](../img/catalogue/maps/maps_montage.webp)

CARLA에는 다양한 특징을 제공하는 여러 사전 제작 맵이 포함되어 있습니다. 도시, 시골, 주거 지역 등 다양한 환경과 함께 여러 건축 스타일, 비포장 시골길부터 다차선 고속도로까지 다양한 도로 구조를 제공합니다. [카탈로그](catalogue.md)나 아래 표에서 맵 가이드를 확인하세요.

| 마을 | 설명 |
|------|------|
| [__Town01__](map_town01.md) | 강과 여러 다리가 있는 작고 단순한 마을 |
| [__Town02__](map_town02.md) | 주거와 상업 건물이 혼합된 작고 단순한 마을 |
| [__Town03__](map_town03.md) | 로터리와 큰 교차로가 있는 더 큰 도시 맵 |
| [__Town04__](map_town04.md) | 특별한 "8자형" *무한* 고속도로가 있는 산속의 작은 마을 |
| [__Town05__](map_town05.md) | 교차로와 다리가 있는 바둑판식 도시. 각 방향에 여러 차선이 있어 차선 변경 연습에 유용 |
| [__Town06__](map_town06.md) | 많은 진입로와 출구가 있는 긴 다차선 고속도로. [**미시간 좌회전**](<https://en.wikipedia.org/wiki/Michigan_left>) 구간도 포함 |
| [__Town07__](map_town07.md) | 좁은 도로, 옥수수 밭, 헛간이 있고 신호등이 거의 없는 시골 환경 |
| **Town08** | [Leaderboard](https://leaderboard.carla.org/) 챌린지용 비공개 "미공개" 마을 |
| **Town09** | [Leaderboard](https://leaderboard.carla.org/) 챌린지용 비공개 "미공개" 마을 |
| [__Town10__](map_town10.md) | 고층 빌딩, 주거 건물, 해변 산책로가 있는 도심 환경 |
| __Town11__ | 장식이 없는 대형 맵 |
| [__Town12__](map_town12.md) | 고층 건물, 주거 지역, 시골 환경을 포함한 여러 구역으로 구성된 대형 맵 |
CARLA 설치에서 사용 가능한 맵들을 다음과 같이 확인할 수 있습니다:

```py
client.get_available_maps()
```

이 명령어를 실행하면 직접 제작하거나 가져온 맵들도 포함하여 모든 사용 가능한 맵이 표시됩니다.

맵을 선택했다면 다음과 같이 불러올 수 있습니다:

```py
client.load_world('Town03_Opt')
```

## 차량 선택하기

![vehicles_overview](../img/catalogue/vehicles/vehicle_montage.webp)

CARLA는 시뮬레이션에서 교통 상황을 구현하기 위한 다양한 차량 라이브러리를 제공합니다. [CARLA 차량 카탈로그](catalogue_vehicles.md)에서 차량 모델들을 살펴보세요.

블루프린트 라이브러리를 필터링하여 사용 가능한 모든 차량 블루프린트를 확인할 수 있습니다:

```py
for bp in world.get_blueprint_library().filter('vehicle'):
    print(bp.id)
```

이것으로 CARLA 시작하기 가이드가 완성되었습니다. 이 기본적인 설정을 바탕으로 더 많은 센서를 추가하고, 다양한 CARLA의 기능을 활용하여 더욱 상세하고 현실적인 시뮬레이션을 구현해보세요.