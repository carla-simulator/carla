# 시뮬레이션 데이터 검색

CARLA에서 시뮬레이션 데이터를 검색하는 효율적인 방법을 배우는 것은 필수적입니다. 이 종합적인 튜토리얼은 초보자와 경험이 많은 사용자 모두에게 권장됩니다. 매우 기초적인 내용부터 시작하여 점차 CARLA에서 사용할 수 있는 다양한 옵션들을 살펴봅니다.

먼저, 시뮬레이션은 사용자 정의 설정과 교통 상황으로 초기화됩니다. 자아(ego) 차량이 도시를 돌아다니도록 설정되며, 선택적으로 기본 센서들이 장착됩니다. 시뮬레이션이 기록되어 나중에 주요 장면을 찾을 수 있습니다. 그 후, 원본 시뮬레이션이 재생되고 최대한 활용됩니다. 일관된 데이터를 검색하기 위해 새로운 센서를 추가할 수 있고, 날씨 조건을 변경할 수 있으며, 레코더를 사용하여 특정 시나리오를 다른 결과로 테스트할 수도 있습니다.

*   [__개요__](#개요)  
*   [__시뮬레이션 설정__](#시뮬레이션-설정)
    *   [맵 설정](#맵-설정)
    *   [날씨 설정](#날씨-설정)
*   [__교통 설정__](#교통-설정)
    *   [CARLA 교통과 보행자](#carla-교통과-보행자)
    *   [SUMO 공동 시뮬레이션 교통](#sumo-공동-시뮬레이션-교통)
*   [__자아 차량 설정__](#자아-차량-설정)
    *   [자아 차량 생성](#자아-차량-생성)
    *   [관전자 위치 지정](#관전자-위치-지정)
*   [__기본 센서 설정__](#기본-센서-설정)
    *   [RGB 카메라](#rgb-카메라)
    *   [감지기](#감지기)
    *   [기타 센서](#기타-센서)
*   [__고급 센서 설정__](#고급-센서-설정)
    *   [깊이 카메라](#깊이-카메라)
    *   [의미론적 분할 카메라](#의미론적-분할-카메라)
    *   [LIDAR 레이캐스트 센서](#lidar-레이캐스트-센서)
    *   [레이더 센서](#레이더-센서)
*   [__렌더링 없는 모드__](#렌더링-없는-모드)
    *   [빠른 속도로 시뮬레이션하기](#빠른-속도로-시뮬레이션하기)
    *   [렌더링 없는 수동 제어](#렌더링-없는-수동-제어)
*   [__데이터 기록 및 검색__](#데이터-기록-및-검색)
    *   [기록 시작](#기록-시작)
    *   [캡처 및 기록](#캡처-및-기록)
    *   [기록 중지](#기록-중지)
*   [__기록 활용__](#기록-활용)
    *   [이벤트 조회](#이벤트-조회)
    *   [단편 선택](#단편-선택)
    *   [더 많은 데이터 검색](#더-많은-데이터-검색)
    *   [날씨 변경](#날씨-변경)
    *   [새로운 결과 시도](#새로운-결과-시도)
*   [__튜토리얼 스크립트__](#튜토리얼-스크립트)

---
## 개요

시뮬레이션 데이터를 검색하는 과정에서 일반적인 실수들이 있습니다. 시뮬레이터에 너무 많은 센서를 추가하거나, 불필요한 데이터를 저장하거나, 특정 이벤트를 찾는 데 어려움을 겪는 것이 그 예입니다. 하지만 이 과정에 대한 몇 가지 지침을 제공할 수 있습니다. 목표는 데이터를 검색하고 복제할 수 있으며, 시뮬레이션을 원하는 대로 검사하고 수정할 수 있도록 하는 것입니다.

!!! 참고
    이 튜토리얼은 [__CARLA 0.9.8 deb 패키지__](start_quickstart.md)를 사용합니다. CARLA 버전과 설치 방법에 따라, 특히 경로와 관련하여 약간의 변경사항이 있을 수 있습니다.

이 튜토리얼은 다양한 단계에 대한 광범위한 옵션들을 제시합니다. 튜토리얼 전반에 걸쳐 여러 스크립트가 언급될 것입니다. 모든 스크립트가 사용되는 것은 아니며, 특정 사용 사례에 따라 달라집니다. 대부분은 일반적인 용도로 CARLA에서 이미 제공됩니다.

* __config.py__ 시뮬레이션 설정을 변경합니다. 맵, 렌더링 옵션, 고정 시간 단계 설정 등...
    * `carla/PythonAPI/util/config.py`
* __dynamic_weather.py__ 흥미로운 날씨 조건을 생성합니다.
    * `carla/PythonAPI/examples/dynamic_weather.py`
* __spawn_npc.py__ AI가 제어하는 차량과 보행자를 생성합니다.
    * `carla/PythonAPI/examples/spawn_npc.py`
* __manual_control.py__ 자아 차량을 생성하고 이에 대한 제어를 제공합니다.
    * `carla/PythonAPI/examples/manual_control.py`
그러나 튜토리얼에서 언급되는 두 개의 스크립트는 CARLA에서 찾을 수 없습니다. 이 스크립트들은 인용된 코드 조각들을 포함하고 있습니다. 이는 두 가지 목적을 가집니다. 첫째, 사용자가 자신만의 스크립트를 구축하도록 장려하는 것입니다. 코드가 무엇을 하는지 완전히 이해하는 것이 중요합니다. 둘째, 이 튜토리얼은 사용자 선호도에 따라 크게 달라질 수 있고, 또 그래야 하는 개요일 뿐입니다. 이 두 스크립트는 단지 예시일 뿐입니다.

* __tutorial_ego.py__ 기본 센서가 있는 자아 차량을 생성하고 자동 조종을 활성화합니다. 관전자는 생성 위치에 배치됩니다. 레코더는 처음부터 시작하여 스크립트가 종료될 때 중지됩니다.
* __tutorial_replay.py__ **tutorial_ego.py**가 기록한 시뮬레이션을 재현합니다. 기록을 조회하고, 고급 센서를 생성하고, 날씨 조건을 변경하고, 기록의 단편들을 재현하는 다양한 코드 단편이 있습니다.

전체 코드는 튜토리얼의 마지막 섹션에서 찾을 수 있습니다. 이것들은 엄격한 것이 아니라 사용자 정의되도록 의도되었음을 기억하세요. CARLA에서 데이터를 검색하는 것은 사용자가 원하는 만큼 강력합니다.

!!! 중요
    이 튜토리얼은 Python에 대한 기본 지식이 필요합니다.

---
## 시뮬레이션 설정

가장 먼저 해야 할 일은 원하는 환경에서 시뮬레이션을 준비하는 것입니다.

CARLA를 실행합니다.

```sh
cd /opt/carla/bin
./CarlaUE4.sh
```

### 맵 설정

시뮬레이션을 실행할 맵을 선택합니다. [맵 문서](core_map.md#carla-maps)를 참조하여 각 맵의 특정 속성에 대해 자세히 알아보세요. 이 튜토리얼에서는 *Town07*이 선택되었습니다.

새 터미널을 열고 __config.py__ 스크립트를 사용하여 맵을 변경합니다.

```
cd /opt/carla/PythonAPI/utils
python3 config.py --map Town01
```

이 스크립트는 다양한 설정을 활성화할 수 있습니다. 일부는 튜토리얼에서 언급될 것이고, 일부는 그렇지 않습니다. 다음은 간단한 요약입니다.

<details>
<summary> <b>config.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help            도움말 메시지 표시 및 종료
  --host H              CARLA 시뮬레이터의 IP (기본값: localhost)
  -p P, --port P        CARLA 시뮬레이터의 TCP 포트 (기본값: 2000)
  -d, --default         기본 설정으로 설정
  -m MAP, --map MAP     새 맵 로드, --list를 사용하여 사용 가능한 맵 확인
  -r, --reload-map      현재 맵 다시 로드
  --delta-seconds S     고정 델타 초 설정, 가변 프레임 속도의 경우 0
  --fps N               고정 FPS 설정, 가변 FPS의 경우 0 (--delta-seconds와 유사)
  --rendering           렌더링 활성화
  --no-rendering        렌더링 비활성화
  --no-sync            동기 모드 비활성화
  --weather WEATHER     날씨 프리셋 설정, --list를 사용하여 사용 가능한 프리셋 확인
  -i, --inspect         시뮬레이션 검사
  -l, --list           사용 가능한 옵션 나열
  -b FILTER, --list-blueprints FILTER
                        FILTER와 일치하는 사용 가능한 블루프린트 나열 ('*'를 사용하여 모두 나열)
  -x XODR_FILE_PATH, --xodr-path XODR_FILE_PATH
                        제공된 OpenDRIVE의 최소 물리적 도로 표현이 있는 새 맵 로드
```
</details>
<br>

![tuto_map](img/tuto_map.jpg)
<div style="text-align: right"><i>Town07의 항공 뷰</i></div>

### 날씨 설정

각 도시는 해당 도시에 맞는 특정 날씨로 로드되지만, 이는 원하는 대로 설정할 수 있습니다. 이 문제에 대한 다른 접근 방식을 제공하는 두 개의 스크립트가 있습니다. 첫 번째는 시간이 지남에 따라 조건이 변하는 동적 날씨를 설정합니다. 다른 하나는 사용자 정의 날씨 조건을 설정합니다. 날씨 조건을 코딩하는 것도 가능합니다. 이는 나중에 [날씨 조건 변경](#날씨-조건-변경)에서 다룰 것입니다.

* __동적 날씨를 설정하려면__ 새 터미널을 열고 **dynamic_weather.py**를 실행합니다. 이 스크립트는 날씨가 변하는 비율을 설정할 수 있으며, `1.0`이 기본 설정입니다.

```sh
cd /opt/carla/PythonAPI/examples

python3 dynamic_weather.py --speed 1.0
```

* __사용자 정의 조건을 설정하려면__ __environment.py__ 스크립트를 사용합니다. 가능한 설정이 꽤 많습니다. 선택적 인수와 [carla.WeatherParameters](python_api.md#carla.WeatherParameters)에 대한 문서를 살펴보세요.

```sh
cd /opt/carla/PythonAPI/util
python3 environment.py --clouds 100 --rain 80 --wetness 100 --puddles 60 --wind 80 --fog 50
```

<details>
<summary> <b>environment.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help            도움말 메시지 표시 및 종료
  --host H              호스트 서버의 IP (기본값: 127.0.0.1)
  -p P, --port P        수신할 TCP 포트 (기본값: 2000)
  --sun SUN             태양 위치 프리셋 [sunset | day | night]
  --weather WEATHER     날씨 조건 프리셋 [clear | overcast | rain]
  --altitude A, -alt A  태양 고도 [-90.0, 90.0]
  --azimuth A, -azm A   태양 방위각 [0.0, 360.0]
  --clouds C, -c C      구름량 [0.0, 100.0]
  --rain R, -r R        강우량 [0.0, 100.0]
  --puddles Pd, -pd Pd  물웅덩이량 [0.0, 100.0]
  --wind W, -w W        바람 강도 [0.0, 100.0]
  --fog F, -f F         안개 강도 [0.0, 100.0]
  --fogdist Fd, -fd Fd  안개 거리 [0.0, inf)
  --wetness Wet, -wet Wet
                        습도 강도 [0.0, 100.0]
```
</details>
<br>

![tuto_weather](img/tuto_weather.jpg)
<div style="text-align: right"><i>적용된 날씨 변경</i></div>
---
## 교통 설정

교통을 시뮬레이션하는 것은 맵에 생명력을 불어넣는 가장 좋은 방법 중 하나입니다. 또한 도시 환경에 대한 데이터를 검색하는 데도 필요합니다. CARLA에서 이를 수행하는 방법에는 여러 가지가 있습니다.

### CARLA 교통과 보행자

CARLA 교통은 [Traffic Manager](adv_traffic_manager.md) 모듈에 의해 관리됩니다. 보행자의 경우, 각각이 자신만의 [carla.WalkerAIController](python_api.md#carla.WalkerAIController)를 가집니다.

새 터미널을 열고 **spawn_npc.py**를 실행하여 차량과 보행자를 생성합니다. 50대의 차량과 동일한 수의 보행자를 생성해 보겠습니다.

```sh
cd /opt/carla/PythonAPI/examples
python3 spawn_npc.py -n 50 -w 50 --safe
```
<details>
<summary> <b>spawn_npc.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help            도움말 메시지 표시 및 종료
  --host H              호스트 서버의 IP (기본값: 127.0.0.1)
  -p P, --port P        수신할 TCP 포트 (기본값: 2000)
  -n N, --number-of-vehicles N
                        차량 수 (기본값: 10)
  -w W, --number-of-walkers W
                        보행자 수 (기본값: 50)
  --safe               사고가 발생하기 쉬운 차량 생성 방지
  --filterv PATTERN    차량 필터 (기본값: "vehicle.*")
  --filterw PATTERN    보행자 필터 (기본값: "walker.pedestrian.*")
  -tm_p P, --tm-port P  TM과 통신할 포트 (기본값: 8000)
  --async              비동기 모드 실행
```
</details>
<br>
![tuto_spawning](img/tuto_spawning.jpg)
<div style="text-align: right"><i>교통 시뮬레이션을 위해 생성된 차량들.</i></div>

### SUMO 공동 시뮬레이션 교통

CARLA는 SUMO와 함께 공동 시뮬레이션을 실행할 수 있습니다. 이를 통해 SUMO에서 생성된 교통이 CARLA로 전파될 수 있습니다. 이 공동 시뮬레이션은 양방향입니다. CARLA에서 차량을 생성하면 SUMO에서도 생성됩니다. 이 기능에 대한 자세한 문서는 [여기](adv_sumo.md)에서 찾을 수 있습니다.

이 기능은 CARLA 0.9.8 이상 버전의 __Town01__, __Town04__, **Town05**에서 사용할 수 있습니다. 첫 번째가 가장 안정적입니다.

!!! 참고
    공동 시뮬레이션은 CARLA에서 동기 모드를 활성화합니다. 이에 대해 자세히 알아보려면 [문서](adv_synchrony_timestep.md)를 읽어보세요.

* 먼저, SUMO를 설치합니다.
```sh
sudo add-apt-repository ppa:sumo/stable
sudo apt-get update
sudo apt-get install sumo sumo-tools sumo-doc
```
* 환경 변수 SUMO_HOME을 설정합니다.
```sh
echo "export SUMO_HOME=/usr/share/sumo" >> ~/.bashrc && source ~/.bashrc
```
* CARLA 서버가 실행 중인 상태에서 [SUMO-CARLA 동기화 스크립트](https://github.com/carla-simulator/carla/blob/master/Co-Simulation/Sumo/run_synchronization.py)를 실행합니다.
```sh
cd ~/carla/Co-Simulation/Sumo
python3 run_synchronization.py examples/Town01.sumocfg --sumo-gui
```
* SUMO 창이 열려야 합니다. 두 시뮬레이션에서 교통을 시작하기 위해 __Play__를 누릅니다.
```
> SUMO 창에서 "Play"를 누릅니다.
```

이 스크립트로 생성되는 교통은 CARLA 팀이 만든 예시입니다. 기본적으로 동일한 차량이 동일한 경로를 따라 생성됩니다. 이는 SUMO에서 사용자가 변경할 수 있습니다.

![tuto_sumo](img/tuto_sumo.jpg)
<div style="text-align: right"><i>SUMO와 CARLA의 교통 공동 시뮬레이션.</i></div>

!!! 경고
    현재 SUMO 공동 시뮬레이션은 베타 기능입니다. 차량들은 물리적 특성이 없으며 CARLA 신호등을 고려하지 않습니다.
---
## 자아 차량 설정

이제부터 레코더가 중지될 때까지 __tutorial_ego.py__에 속하는 코드 조각들이 있을 것입니다. 이 스크립트는 자아 차량을 생성하고, 선택적으로 일부 센서를 추가하며, 사용자가 스크립트를 종료할 때까지 시뮬레이션을 기록합니다.

### 자아 차량 생성

CARLA에서는 일반적으로 `role_name` 속성을 `ego`로 설정하여 사용자가 제어하는 차량을 구분합니다. 다른 속성들도 설정할 수 있으며, 일부는 권장 값이 있습니다.

아래에서 테슬라 모델을 [블루프린트 라이브러리](bp_library.md)에서 검색하고, 권장되는 색상 중 무작위로 선택된 색상으로 생성합니다. 맵에서 권장하는 생성 지점 중 하나가 자아 차량을 배치하기 위해 선택됩니다.

```py        
# --------------
# 자아 차량 생성
# --------------
ego_bp = world.get_blueprint_library().find('vehicle.tesla.model3')
ego_bp.set_attribute('role_name','ego')
print('\n자아 role_name이 설정되었습니다')
ego_color = random.choice(ego_bp.get_attribute('color').recommended_values)
ego_bp.set_attribute('color',ego_color)
print('\n자아 색상이 설정되었습니다')

spawn_points = world.get_map().get_spawn_points()
number_of_spawn_points = len(spawn_points)

if 0 < number_of_spawn_points:
    random.shuffle(spawn_points)
    ego_transform = spawn_points[0]
    ego_vehicle = world.spawn_actor(ego_bp,ego_transform)
    print('\n자아가 생성되었습니다')
else: 
    logging.warning('생성 지점을 찾을 수 없습니다')
```

### 관전자 위치 지정

관전자 액터는 시뮬레이션 뷰를 제어합니다. 스크립트를 통해 이를 이동시키는 것은 선택사항이지만, 자아 차량을 찾는 것을 쉽게 만들 수 있습니다.

```py
# --------------
# 자아 위치에 관전자 설정
# --------------
spectator = world.get_spectator()
world_snapshot = world.wait_for_tick() 
spectator.set_transform(ego_vehicle.get_transform())
```

---
## 기본 센서 설정

모든 센서를 생성하는 과정은 매우 유사합니다.

__1.__ 라이브러리를 사용하여 센서 블루프린트를 찾습니다.  
__2.__ 센서에 대한 특정 속성을 설정합니다. 이는 매우 중요합니다. 속성들이 검색되는 데이터의 형태를 결정합니다.  
__3.__ 센서를 자아 차량에 부착합니다. **변환(transform)은 부모를 기준으로 상대적**입니다. [carla.AttachmentType](python_api.md#carlaattachmenttype)은 센서의 위치가 어떻게 업데이트되는지 결정합니다.  
__4.__ `listen()` 메서드를 추가합니다. 이것이 핵심 요소입니다. 센서가 데이터를 수신할 때마다 호출될 [__lambda__](https://www.w3schools.com/python/python_lambda.asp) 메서드입니다. 인자는 검색된 센서 데이터입니다.

이 기본 지침을 염두에 두고, 자아 차량에 몇 가지 기본 센서를 설정해 보겠습니다.

### RGB 카메라

[RGB 카메라](ref_sensors.md#rgb-camera)는 장면의 사실적인 이미지를 생성합니다. 이는 모든 센서 중에서 설정 가능한 속성이 가장 많지만, 또한 가장 기본적인 센서입니다. 이는 `focal_distance`, `shutter_speed` 또는 `gamma`와 같은 속성으로 내부적으로 작동 방식을 결정하는 실제 카메라로 이해해야 합니다. 렌즈 왜곡을 정의하는 특정 속성 세트도 있고, 많은 고급 속성도 있습니다. 예를 들어, `lens_circle_multiplier`를 사용하여 어안 렌즈와 유사한 효과를 얻을 수 있습니다. 이에 대해 자세히 알아보려면 [문서](ref_sensors.md#rgb-camera)를 참조하세요.

단순화를 위해, 스크립트는 이 센서의 가장 일반적으로 사용되는 속성만 설정합니다.

* __`image_size_x`와 `image_size_y`__ 는 출력 이미지의 해상도를 변경합니다.
* __`fov`__ 는 카메라의 수평 시야각입니다.

속성을 설정한 후, 센서를 생성할 차례입니다. 스크립트는 카메라를 차량의 보닛에 위치시키고 전방을 향하게 합니다. 이는 차량의 전방 시야를 캡처할 것입니다.

데이터는 매 단계마다 [carla.Image](python_api.md#carla.Image)로 검색됩니다. listen 메서드는 이를 디스크에 저장합니다. 경로는 원하는 대로 변경할 수 있습니다. 각 이미지의 이름은 촬영된 시뮬레이션 프레임을 기반으로 코딩됩니다.

```py
# --------------
# RGB 카메라를 자아 차량에 부착
# --------------
cam_bp = None
cam_bp = world.get_blueprint_library().find('sensor.camera.rgb')
cam_bp.set_attribute("image_size_x",str(1920))
cam_bp.set_attribute("image_size_y",str(1080))
cam_bp.set_attribute("fov",str(105))
cam_location = carla.Location(2,0,1)
cam_rotation = carla.Rotation(0,180,0)
cam_transform = carla.Transform(cam_location,cam_rotation)
ego_cam = world.spawn_actor(cam_bp,cam_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
ego_cam.listen(lambda image: image.save_to_disk('tutorial/output/%.6d.jpg' % image.frame))
```
![tuto_rgb](img/tuto_rgb.jpg)
<div style="text-align: right"><i>RGB 카메라 출력</i></div>
### 감지기

이 센서들은 부착된 객체가 특정 이벤트를 등록할 때 데이터를 검색합니다. 세 가지 유형의 감지기 센서가 있으며, 각각 한 가지 유형의 이벤트를 설명합니다.

* [__충돌 감지기__](ref_sensors.md#collision-detector) 부모와 다른 액터 사이의 충돌을 검색합니다.
* [__차선 침범 감지기__](ref_sensors.md#lane-invasion-detector) 부모가 차선 표시를 건널 때 등록합니다.
* [__장애물 감지기__](ref_sensors.md#obstacle-detector) 부모의 전방에 있는 잠재적 장애물을 감지합니다.

이들이 검색하는 데이터는 나중에 시뮬레이션의 어느 부분을 재연할지 결정할 때 도움이 될 것입니다. 실제로 충돌은 레코더를 사용하여 명시적으로 조회할 수 있습니다. 이는 출력되도록 준비되어 있습니다.

장애물 감지기 블루프린트만이 설정해야 할 속성을 가집니다. 다음은 중요한 속성들입니다.

* __`sensor_tick`__ 센서가 `x`초가 지난 후에만 데이터를 검색하도록 설정합니다. 이는 매 단계마다 데이터를 검색하는 센서의 일반적인 속성입니다.
* __`distance`와 `hit-radius`__ 전방의 장애물을 감지하는 데 사용되는 디버그 라인의 형태를 결정합니다.
* __`only_dynamics`__ 정적 객체를 고려해야 하는지 여부를 결정합니다. 기본적으로 모든 객체가 고려됩니다.

스크립트는 장애물 감지기가 동적 객체만 고려하도록 설정합니다. 차량이 정적 객체와 충돌하면 충돌 센서에 의해 감지될 것입니다.

```py
# --------------
# 자아 차량에 충돌 센서 추가
# --------------

col_bp = world.get_blueprint_library().find('sensor.other.collision')
col_location = carla.Location(0,0,0)
col_rotation = carla.Rotation(0,0,0)
col_transform = carla.Transform(col_location,col_rotation)
ego_col = world.spawn_actor(col_bp,col_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def col_callback(colli):
    print("충돌 감지됨:\n"+str(colli)+'\n')
ego_col.listen(lambda colli: col_callback(colli))

# --------------
# 자아 차량에 차선 침범 센서 추가
# --------------

lane_bp = world.get_blueprint_library().find('sensor.other.lane_invasion')
lane_location = carla.Location(0,0,0)
lane_rotation = carla.Rotation(0,0,0)
lane_transform = carla.Transform(lane_location,lane_rotation)
ego_lane = world.spawn_actor(lane_bp,lane_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def lane_callback(lane):
    print("차선 침범 감지됨:\n"+str(lane)+'\n')
ego_lane.listen(lambda lane: lane_callback(lane))

# --------------
# 자아 차량에 장애물 센서 추가
# --------------

obs_bp = world.get_blueprint_library().find('sensor.other.obstacle')
obs_bp.set_attribute("only_dynamics",str(True))
obs_location = carla.Location(0,0,0)
obs_rotation = carla.Rotation(0,0,0)
obs_transform = carla.Transform(obs_location,obs_rotation)
ego_obs = world.spawn_actor(obs_bp,obs_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def obs_callback(obs):
    print("장애물 감지됨:\n"+str(obs)+'\n')
ego_obs.listen(lambda obs: obs_callback(obs))
```
![tuto_detectors](img/tuto_detectors.jpg)
<div style="text-align: right"><i>감지기 센서 출력</i></div>

### 기타 센서

현재는 이 카테고리의 두 가지 센서만 고려하겠습니다.

* [__GNSS 센서__](ref_sensors.md#gnss-sensor) 센서의 지리적 위치를 검색합니다.
* [__IMU 센서__](ref_sensors.md#imu-sensor) 가속도계, 자이로스코프, 나침반을 포함합니다.

차량 객체의 일반적인 측정값을 얻기 위해, 이 두 센서는 차량의 중앙에 생성됩니다.

이 센서들에 사용할 수 있는 속성들은 대부분 측정의 노이즈 모델에서 평균이나 표준 편차 매개변수를 설정합니다. 이는 더 현실적인 측정값을 얻는 데 유용합니다. 하지만 **tutorial_ego.py**에서는 하나의 속성만 설정합니다.

* __`sensor_tick`__ 이러한 측정값은 단계 사이에 크게 변하지 않을 것으로 예상되므로, 데이터를 가끔씩만 검색해도 괜찮습니다. 이 경우 3초마다 출력되도록 설정됩니다.

```py
# --------------
# 자아 차량에 GNSS 센서 추가
# --------------

gnss_bp = world.get_blueprint_library().find('sensor.other.gnss')
gnss_location = carla.Location(0,0,0)
gnss_rotation = carla.Rotation(0,0,0)
gnss_transform = carla.Transform(gnss_location,gnss_rotation)
gnss_bp.set_attribute("sensor_tick",str(3.0))
ego_gnss = world.spawn_actor(gnss_bp,gnss_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def gnss_callback(gnss):
    print("GNSS 측정:\n"+str(gnss)+'\n')
ego_gnss.listen(lambda gnss: gnss_callback(gnss))

# --------------
# 자아 차량에 IMU 센서 추가
# --------------

imu_bp = world.get_blueprint_library().find('sensor.other.imu')
imu_location = carla.Location(0,0,0)
imu_rotation = carla.Rotation(0,0,0)
imu_transform = carla.Transform(imu_location,imu_rotation)
imu_bp.set_attribute("sensor_tick",str(3.0))
ego_imu = world.spawn_actor(imu_bp,imu_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def imu_callback(imu):
    print("IMU 측정:\n"+str(imu)+'\n')
ego_imu.listen(lambda imu: imu_callback(imu))
```

![tuto_other](img/tuto_other.jpg)
<div style="text-align: right"><i>GNSS와 IMU 센서 출력</i></div>
---
## 고급 센서 설정

스크립트 __tutorial_replay.py__는 다른 것들 중에서도 더 많은 센서에 대한 정의를 포함하고 있습니다. 이들은 기본 센서와 같은 방식으로 작동하지만, 이해하기가 조금 더 어려울 수 있습니다.

### 깊이 카메라

[깊이 카메라](ref_sensors.md#depth-camera)는 모든 픽셀을 회색조 깊이 맵에 매핑하는 장면의 이미지를 생성합니다. 하지만 출력은 직관적이지 않습니다. 카메라의 깊이 버퍼는 RGB 색상 공간을 사용하여 매핑됩니다. 이는 이해할 수 있도록 회색조로 변환되어야 합니다.

이를 위해서는 RGB 카메라와 마찬가지로 이미지를 저장하되, [carla.ColorConverter](python_api.md#carla.ColorConverter)를 적용하면 됩니다. 깊이 카메라에 사용할 수 있는 두 가지 변환이 있습니다.

* __carla.ColorConverter.Depth__ 원래의 깊이를 밀리미터 단위의 정밀도로 변환합니다.
* __carla.ColorConverter.LogarithmicDepth__ 또한 밀리미터 단위의 정밀도를 가지지만, 가까운 거리에서는 더 좋은 결과를 제공하고 먼 요소에 대해서는 약간 더 나쁜 결과를 제공합니다.

깊이 카메라의 속성은 이전에 RGB 카메라에서 설명한 요소들만 설정합니다: `fov`, `image_size_x`, `image_size_y`, `sensor_tick`. 스크립트는 이 센서가 이전의 RGB 카메라와 일치하도록 설정합니다.

```py
# --------------
# 자아 차량에 깊이 카메라 추가
# --------------
depth_cam = None
depth_bp = world.get_blueprint_library().find('sensor.camera.depth')
depth_location = carla.Location(2,0,1)
depth_rotation = carla.Rotation(0,180,0)
depth_transform = carla.Transform(depth_location,depth_rotation)
depth_cam = world.spawn_actor(depth_bp,depth_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
# 이번에는 이미지에 색상 변환기가 적용되어 의미론적 분할 뷰를 얻습니다
depth_cam.listen(lambda image: image.save_to_disk('tutorial/new_depth_output/%.6d.jpg' % image.frame,carla.ColorConverter.LogarithmicDepth))
```

![tuto_depths](img/tuto_depths.jpg)
<div style="text-align: right"><i>깊이 카메라 출력. 왼쪽은 단순 변환, 오른쪽은 로그 변환.</i></div>

### 의미론적 분할 카메라

[의미론적 분할 카메라](ref_sensors.md#semantic-segmentation-camera)는 장면의 요소들을 태그 방식에 따라 다른 색상으로 렌더링합니다. 태그는 생성에 사용된 에셋의 경로에 따라 시뮬레이터에 의해 생성됩니다. 예를 들어, `Pedestrians`로 태그된 메시는 `Unreal/CarlaUE4/Content/Static/Pedestrians`에 저장된 콘텐츠로 생성됩니다.

출력은 모든 카메라와 마찬가지로 이미지이지만, 각 픽셀은 빨간색 채널에 태그를 인코딩하고 있습니다. 이 원본 이미지는 **ColorConverter.CityScapesPalette**를 사용하여 변환해야 합니다. 새로운 태그를 생성할 수 있으며, [문서](ref_sensors.md#semantic-segmentation-camera)에서 자세히 알아보세요.

이 카메라에 사용할 수 있는 속성은 깊이 카메라와 정확히 동일합니다. 스크립트는 이것도 원본 RGB 카메라와 일치하도록 설정합니다.

```py
# --------------
# 자아에 새로운 의미론적 분할 카메라 추가
# --------------
sem_cam = None
sem_bp = world.get_blueprint_library().find('sensor.camera.semantic_segmentation')
sem_bp.set_attribute("image_size_x",str(1920))
sem_bp.set_attribute("image_size_y",str(1080))
sem_bp.set_attribute("fov",str(105))
sem_location = carla.Location(2,0,1)
sem_rotation = carla.Rotation(0,180,0)
sem_transform = carla.Transform(sem_location,sem_rotation)
sem_cam = world.spawn_actor(sem_bp,sem_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
# 이번에는 이미지에 색상 변환기가 적용되어 의미론적 분할 뷰를 얻습니다
sem_cam.listen(lambda image: image.save_to_disk('tutorial/new_sem_output/%.6d.jpg' % image.frame,carla.ColorConverter.CityScapesPalette))
```

![tuto_sem](img/tuto_sem.jpg)
<div style="text-align: right"><i>의미론적 분할 카메라 출력</i></div>
### LIDAR 레이캐스트 센서

[LIDAR 센서](ref_sensors.md#lidar-raycast-sensor)는 회전하는 LIDAR를 시뮬레이션합니다. 이는 장면을 3D로 매핑하는 포인트 클라우드를 생성합니다. LIDAR는 특정 주파수로 회전하는 레이저 세트를 포함합니다. 레이저는 충돌까지의 거리를 레이캐스트하고, 모든 샷을 단일 포인트로 저장합니다.

레이저 배열의 배치 방식은 다양한 센서 속성을 사용하여 설정할 수 있습니다.

* __`upper_fov`와 `lower_fov`__ 각각 가장 높은 레이저와 가장 낮은 레이저의 각도입니다.
* __`channels`__ 사용할 레이저의 수를 설정합니다. 이들은 원하는 _fov_ 를 따라 분포됩니다.

다른 속성들은 이 포인트들이 계산되는 방식을 설정합니다. 이들은 각 레이저가 매 단계마다 계산하는 포인트의 수를 결정합니다: `points_per_second / (FPS * channels)`.

* __`range`__ 캡처할 수 있는 최대 거리입니다.
* __`points_per_second`__ 매 초마다 얻을 포인트의 수입니다. 이 수량은 `channels` 수로 나누어집니다.
* __`rotation_frequency`__ LIDAR가 매 초마다 회전하는 횟수입니다.

포인트 클라우드 출력은 [carla.LidarMeasurement]로 설명됩니다. 이는 [carla.Location] 목록으로 반복되거나 _.ply_ 표준 파일 형식으로 저장될 수 있습니다.

```py
# --------------
# 자아에 새로운 LIDAR 센서 추가
# --------------
lidar_cam = None
lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
lidar_bp.set_attribute('channels',str(32))
lidar_bp.set_attribute('points_per_second',str(90000))
lidar_bp.set_attribute('rotation_frequency',str(40))
lidar_bp.set_attribute('range',str(20))
lidar_location = carla.Location(0,0,2)
lidar_rotation = carla.Rotation(0,0,0)
lidar_transform = carla.Transform(lidar_location,lidar_rotation)
lidar_sen = world.spawn_actor(lidar_bp,lidar_transform,attach_to=ego_vehicle)
lidar_sen.listen(lambda point_cloud: point_cloud.save_to_disk('tutorial/new_lidar_output/%.6d.ply' % point_cloud.frame))
```

_.ply_ 출력은 __Meshlab__을 사용하여 시각화할 수 있습니다.

__1.__ [Meshlab](http://www.meshlab.net/#download) 설치.
```sh
sudo apt-get update -y
sudo apt-get install -y meshlab
```
__2.__ Meshlab 실행.
```sh
meshlab
```
__3.__ _.ply_ 파일 중 하나를 엽니다. `File > Import mesh...` 

![tuto_lidar](img/tuto_lidar.jpg)
<div style="text-align: right"><i>Meshlab에서 처리된 후의 LIDAR 출력.</i></div>

### 레이더 센서

[레이더 센서](ref_sensors.md#radar-sensor)는 LIDAR와 유사합니다. 원뿔형 뷰를 생성하고 그 안에서 레이저를 발사하여 충돌을 레이캐스트합니다. 출력은 [carla.RadarMeasurement](python_api.md#carlaradarmeasurement)입니다. 이는 레이저가 검색한 [carla.RadarDetection](python_api.md#carlaradardetection) 목록을 포함합니다. 이들은 공간상의 점이 아니라 센서에 관한 데이터를 가진 감지입니다: `azimuth`, `altitude`, `sensor`, `velocity`.

이 센서의 속성들은 주로 레이저가 위치하는 방식을 설정합니다.

* __`horizontal_fov`와 `vertical_fov`__ 원뿔형 뷰의 진폭을 결정합니다.
* __`channels`__ 사용할 레이저의 수를 설정합니다. 이들은 원하는 `fov`를 따라 분포됩니다.
* __`range`__ 레이저가 레이캐스트할 최대 거리입니다.
* __`points_per_second`__ 캡처할 포인트의 수를 설정하며, 이는 지정된 채널들 사이에 분배됩니다.

스크립트는 센서를 차량의 보닛에 위치시키고 약간 위쪽으로 회전시킵니다. 이렇게 하면 출력이 차량의 전방을 매핑할 것입니다. `horizontal_fov`는 증가하고 `vertical_fov`는 감소합니다. 관심 영역은 특히 차량과 보행자가 일반적으로 이동하는 높이입니다. `range`도 100m에서 10m로 변경되어 차량 바로 앞의 데이터만 검색합니다.

콜백은 이번에는 조금 더 복잡하여 더 많은 기능을 보여줍니다. 이는 레이더가 캡처한 포인트들을 실시간으로 그릴 것입니다. 포인트들은 자아 차량에 대한 속도에 따라 색상이 지정됩니다.

* __파란색__ 차량에 접근하는 포인트.
* __빨간색__ 차량에서 멀어지는 포인트.
* __흰색__ 자아 차량에 대해 정지해 있는 포인트.

```py
# --------------
# 자아에 새로운 레이더 센서 추가
# --------------
rad_cam = None
rad_bp = world.get_blueprint_library().find('sensor.other.radar')
rad_bp.set_attribute('horizontal_fov', str(35))
rad_bp.set_attribute('vertical_fov', str(20))
rad_bp.set_attribute('range', str(20))
rad_location = carla.Location(x=2.0, z=1.0)
rad_rotation = carla.Rotation(pitch=5)
rad_transform = carla.Transform(rad_location,rad_rotation)
rad_ego = world.spawn_actor(rad_bp,rad_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
def rad_callback(radar_data):
    velocity_range = 7.5 # m/s
    current_rot = radar_data.transform.rotation
    for detect in radar_data:
        azi = math.degrees(detect.azimuth)
        alt = math.degrees(detect.altitude)
        # 0.25는 점들이 제대로 보일 수 있도록
        # 거리를 약간 조정합니다
        fw_vec = carla.Vector3D(x=detect.depth - 0.25)
        carla.Transform(
            carla.Location(),
            carla.Rotation(
                pitch=current_rot.pitch + alt,
                yaw=current_rot.yaw + azi,
                roll=current_rot.roll)).transform(fw_vec)

        def clamp(min_v, max_v, value):
            return max(min_v, min(value, max_v))

        norm_velocity = detect.velocity / velocity_range # 범위 [-1, 1]
        r = int(clamp(0.0, 1.0, 1.0 - norm_velocity) * 255.0)
        g = int(clamp(0.0, 1.0, 1.0 - abs(norm_velocity)) * 255.0)
        b = int(abs(clamp(- 1.0, 0.0, - 1.0 - norm_velocity)) * 255.0)
        world.debug.draw_point(
            radar_data.transform.location + fw_vec,
            size=0.075,
            life_time=0.06,
            persistent_lines=False,
            color=carla.Color(r, g, b))
rad_ego.listen(lambda radar_data: rad_callback(radar_data))
```

![tuto_radar](img/tuto_radar.jpg)
<div style="text-align: right"><i>레이더 출력. 차량이 신호등에서 정지해 있어 앞에 있는 정적 요소들이 흰색으로 나타납니다.</i></div>
---
## 렌더링 없는 모드

[렌더링 없는 모드](adv_rendering_options.md)는 나중에 데이터를 검색하기 위해 재생할 초기 시뮬레이션을 실행하는 데 유용할 수 있습니다. 특히 밀집된 교통과 같은 극단적인 조건이 있는 경우에 더욱 그렇습니다.

### 빠른 속도로 시뮬레이션하기

렌더링을 비활성화하면 시뮬레이션에서 많은 작업이 절약됩니다. GPU가 사용되지 않기 때문에 서버는 최대 속도로 작동할 수 있습니다. 이는 복잡한 조건을 빠른 속도로 시뮬레이션하는 데 유용할 수 있습니다. 이를 수행하는 가장 좋은 방법은 고정 시간 단계를 설정하는 것입니다. 고정 시간 단계와 렌더링이 없는 비동기 서버를 실행하면, 시뮬레이션의 유일한 제한은 서버의 내부 로직이 됩니다.

[맵 설정](#맵-설정)에 사용된 동일한 `config.py`로 렌더링을 비활성화하고 고정 시간 단계를 설정할 수 있습니다.

```
cd /opt/carla/PythonAPI/utils
python3 config.py --no-rendering --delta-seconds 0.05 # 절대 0.1초보다 크지 않게 설정
```

!!! 경고
    동기화와 시간 단계에 대해 변경하기 전에 [문서](adv_synchrony_timestep.md)를 읽어보세요.

### 렌더링 없는 수동 제어

스크립트 `PythonAPI/examples/no_rendering_mode.py`는 시뮬레이션의 개요를 제공합니다. 이는 Pygame을 사용하여 자아 차량을 따라다니는 최소한의 항공 뷰를 생성합니다. 이는 __manual_control.py__와 함께 사용하여 거의 비용 없이 경로를 생성하고 기록한 다음, 나중에 재생하고 활용할 수 있습니다.

```
cd /opt/carla/PythonAPI/examples
python3 manual_control.py
```

```
cd /opt/carla/PythonAPI/examples
python3 no_rendering_mode.py --no-rendering
```

<details>
<summary> <b>no_rendering_mode.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help           도움말 메시지 표시
  -v, --verbose        디버그 정보 출력
  --host H             호스트 서버의 IP (기본값: 127.0.0.1)
  -p P, --port P       수신할 TCP 포트 (기본값: 2000)
  --res WIDTHxHEIGHT   창 해상도 (기본값: 1280x720)
  --filter PATTERN     액터 필터 (기본값: "vehicle.*")
  --map TOWN          주어진 TOWN에서 새 에피소드 시작
  --no-rendering       서버 렌더링 끄기
  --show-triggers      트래픽 표지판의 트리거 박스 표시
  --show-connections   웨이포인트 연결 표시
  --show-spawn-points  권장 생성 지점 표시
```
</details>
<br>

![tuto_no_rendering](img/tuto_no_rendering.jpg)
<div style="text-align: right"><i>Town07에서 작동하는 no_rendering_mode.py</i></div>

!!! 참고
    이 모드에서는 GPU 기반 센서가 빈 데이터를 검색합니다. 카메라는 사용할 수 없지만, 감지기와 같은 다른 센서는 정상적으로 작동합니다.
---
## 데이터 기록 및 검색

### 기록 시작

[__레코더__](adv_recorder.md)는 언제든지 시작할 수 있습니다. 스크립트는 처음 액터들의 생성을 포함하여 모든 것을 캡처하기 위해 맨 처음에 이를 실행합니다. 경로가 지정되지 않으면 로그는 `CarlaUE4/Saved`에 저장됩니다.

```py
# --------------
# 기록 시작
# --------------
client.start_recorder('~/tutorial/recorder/recording01.log')
```

### 캡처 및 기록

이를 수행하는 방법에는 여러 가지가 있습니다. 주로 자동으로 돌아다니게 하거나 수동으로 제어하는 방식으로 나뉩니다. 생성된 센서의 데이터는 실시간으로 검색됩니다. 모든 것이 제대로 설정되었는지 확인하기 위해 기록하는 동안 이를 확인하세요.

* __자동 조종 활성화.__ 이는 차량을 [Traffic Manager](adv_traffic_manager.md)에 등록합니다. 차량은 도시를 끝없이 돌아다닐 것입니다. 스크립트는 이를 수행하고 스크립트가 끝나는 것을 방지하기 위한 루프를 만듭니다. 기록은 사용자가 스크립트를 종료할 때까지 계속됩니다. 또는 특정 시간 후에 스크립트를 종료하도록 타이머를 설정할 수도 있습니다.

```py
# --------------
# 데이터 캡처
# --------------
ego_vehicle.set_autopilot(True)
print('\n자아 자동 조종이 활성화되었습니다')

while True:
    world_snapshot = world.wait_for_tick()
```

* __수동 제어.__ 한 클라이언트에서는 `PythonAPI/examples/manual_control.py` 스크립트를 실행하고, 다른 클라이언트에서는 레코더를 실행합니다. 원하는 경로를 만들기 위해 자아 차량을 운전하고, 완료되면 레코더를 중지합니다. __tutorial_ego.py__ 스크립트를 사용하여 레코더를 관리할 수 있지만, 다른 코드 조각들은 주석 처리해야 합니다.

```
cd /opt/carla/PythonAPI/examples
python3 manual_control.py
```

!!! 참고
    렌더링을 피하고 컴퓨터 자원을 절약하려면 [__렌더링 없는 모드__](adv_rendering_options.md#no-rendering-mode)를 활성화하세요. 스크립트 `/PythonAPI/examples/no_rendering_mode.py`는 간단한 항공 뷰를 생성하면서 이를 수행합니다.

### 기록 중지

중지 호출은 시작 호출보다도 더 간단합니다. 레코더가 완료되면 기록이 이전에 지정된 경로에 저장됩니다.

```py
# --------------
# 기록 중지
# --------------
client.stop_recorder()
```

---
## 기록 활용

지금까지 시뮬레이션이 기록되었습니다. 이제 기록을 검토하고, 가장 주목할 만한 순간들을 찾아, 그것들로 작업할 시간입니다. 이러한 단계들은 스크립트 **tutorial_replay.py**에 모여 있습니다. 개요는 주석 처리된 다른 코드 조각들로 구성되어 있습니다.

새 시뮬레이션을 실행할 시간입니다.

```sh
./CarlaUE4.sh
```
시뮬레이션을 재현하려면 [단편을 선택](#단편-선택)하고 재생을 위한 코드가 포함된 스크립트를 실행하세요.

```sh
python3 tuto_replay.py
```
### 이벤트 조회

다양한 조회 방법이 [__레코더 문서__](adv_recorder.md)에 자세히 설명되어 있습니다. 요약하면, 이들은 특정 이벤트나 프레임에 대한 데이터를 검색합니다. 조회를 사용하여 기록을 연구하세요. 주목할 만한 순간들을 찾고, 관심있을 수 있는 것들을 추적하세요.

```py
# --------------
# 기록 조회
# --------------
# 기록에서 가장 중요한 이벤트만 표시
print(client.show_recorder_file_info("~/tutorial/recorder/recording01.log",False))
# 10초 동안 1미터를 이동하지 않은 액터 표시
print(client.show_recorder_actors_blocked("~/tutorial/recorder/recording01.log",10,1))
# 차량 'v'와 다른 유형의 액터 'a' 사이의 충돌 필터링
print(client.show_recorder_collisions("~/tutorial/recorder/recording01.log",'v','a'))
```

!!! 참고
    조회를 하기 위해 레코더가 켜져 있을 필요는 없습니다.

![tuto_query_frames](img/tuto_query_frames.jpg)
<div style="text-align: right"><i>중요한 이벤트를 보여주는 조회. 이는 자아 차량이 생성된 프레임입니다.</i></div>

![tuto_query_blocked](img/tuto_query_blocked.jpg)
<div style="text-align: right"><i>멈춰있는 액터를 보여주는 조회. 이 시뮬레이션에서 자아 차량은 100초 동안 정지해있었습니다.</i></div>

![tuto_query_collisions](img/tuto_query_collisions.jpg)
<div style="text-align: right"><i>자아 차량과 "기타" 유형의 객체 사이의 충돌을 보여주는 조회.</i></div>

!!! 참고
    모든 프레임에 대한 자세한 파일 정보를 얻는 것은 부담이 될 수 있습니다. 다른 조회를 통해 어디를 봐야 할지 알게 된 후에 사용하세요.

### 단편 선택

조회 후에는 시뮬레이션의 일부 순간들을 다시 재생해보는 것이 좋은 아이디어일 수 있습니다. 이는 매우 간단하며 매우 도움이 될 수 있습니다. 시뮬레이션에 대해 더 잘 알아보세요. 이는 나중에 시간을 절약하는 가장 좋은 방법입니다.

이 메서드를 사용하면 재생의 시작점과 종료점, 그리고 따라갈 액터를 선택할 수 있습니다.

```py
# --------------
# 기록의 단편 재현
# --------------
client.replay_file("~/tutorial/recorder/recording01.log",45,10,0)
```

다음은 지금 할 수 있는 일들의 목록입니다.

* __조회의 정보를 활용하세요.__ 이벤트의 순간과 관련된 액터들을 찾아내고, 그것을 다시 재생하세요. 이벤트 몇 초 전에 레코더를 시작하세요.
* __다른 액터들을 따라가세요.__ 다른 관점들은 조회에 포함되지 않은 새로운 이벤트들을 보여줄 것입니다.
* __자유로운 관전자 시점으로 돌아다니세요.__ `actor_id`를 `0`으로 설정하고 시뮬레이션의 전체적인 뷰를 얻으세요. 기록 덕분에 원하는 시간과 장소 어디든 있을 수 있습니다.

!!! 참고
    로그가 끝나거나 재생이 지정된 종료점에 도달하면 시뮬레이션이 중지되지 않습니다. 보행자들은 가만히 서 있고, 차량들은 계속 돌아다닐 것입니다.
### 더 많은 데이터 검색

레코더는 이 시뮬레이션에서 원본과 정확히 동일한 조건을 재현할 것입니다. 이는 서로 다른 재생 사이에서 일관된 데이터를 보장합니다.

중요한 순간들, 액터들, 이벤트들의 목록을 모으세요. 필요할 때마다 센서를 추가하고 시뮬레이션을 재생하세요. 이 과정은 이전과 정확히 동일합니다. 스크립트 __tutorial_replay.py__는 [__고급 센서 설정__](#고급-센서-설정) 섹션에서 자세히 설명된 다양한 예시를 제공합니다. 다른 예시들은 [__기본 센서 설정__](#기본-센서-설정) 섹션에서 설명되었습니다.

필요한 만큼 많은 센서를 추가하고, 필요한 곳에 추가하세요. 원하는 만큼 자주 시뮬레이션을 재생하고 원하는 만큼 많은 데이터를 검색하세요.

### 날씨 변경

레코딩은 원본의 날씨 조건을 재현할 것입니다. 하지만 이는 원하는 대로 변경할 수 있습니다. 이는 다른 모든 이벤트는 동일하게 유지하면서 날씨가 센서에 어떤 영향을 미치는지 비교하는 데 흥미로울 수 있습니다.

현재 날씨를 가져오고 자유롭게 수정하세요. [carla.WeatherParameters](python_api.md#carla.WeatherParameters)에 일부 사전 설정이 있다는 것을 기억하세요. 스크립트는 환경을 안개 낀 일몰로 변경할 것입니다.

```py
# --------------
# 재생을 위한 날씨 변경
# --------------
weather = world.get_weather()
weather.sun_altitude_angle = -30
weather.fog_density = 65
weather.fog_distance = 10
world.set_weather(weather)
```

### 새로운 결과 시도

새로운 시뮬레이션은 레코딩에 엄격하게 연결되어 있지 않습니다. 이는 언제든지 수정될 수 있으며, 레코더가 중지된 후에도 시뮬레이션은 계속됩니다.

이는 사용자에게 유용할 수 있습니다. 예를 들어, 시뮬레이션을 몇 초 전에 재생하고 액터를 생성하거나 제거함으로써 충돌을 강제하거나 피할 수 있습니다. 특정 순간에 레코딩을 종료하는 것도 유용할 수 있습니다. 이렇게 하면 차량들이 다른 경로를 택할 수 있습니다.

조건을 변경하고 시뮬레이션을 실험해 보세요. 잃을 것은 없습니다. 레코더 덕분에 초기 시뮬레이션은 항상 재현될 수 있습니다. 이것이 CARLA의 모든 잠재력을 활용하는 핵심입니다.

---
## 튜토리얼 스크립트

아래는 이 튜토리얼의 코드 조각들을 모은 두 스크립트입니다. 대부분의 코드는 주석 처리되어 있는데, 이는 특정 목적에 맞게 수정되도록 의도된 것입니다.

[튜토리얼 스크립트 전체 내용은 원문 참조]

이것으로 시뮬레이션에서 데이터를 적절히 검색하는 방법에 대한 설명을 마칩니다. 시뮬레이터의 조건을 변경하고, 센서 설정을 실험하고, 직접 해보세요. 가능성은 무한합니다.

이 읽기 중에 떠오른 의문점이나 제안사항은 포럼에서 게시하세요.

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA 포럼</a>
</p>
</div>
</div>