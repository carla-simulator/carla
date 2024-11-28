# 1. 월드와 클라이언트 

클라이언트와 월드는 CARLA의 기본 요소 중 두 가지로, 시뮬레이션과 액터들을 조작하는 데 필요한 추상화입니다.

이 튜토리얼에서는 이러한 요소들의 기본 개념과 생성 방법부터 시작하여 그들의 기능들을 설명합니다. 읽는 동안 의문점이나 문제가 생기면 [CARLA 포럼](https://github.com/carla-simulator/carla/discussions/)에서 해결할 수 있습니다.

* [__클라이언트__](#the-client)
  * [클라이언트 생성](#client-creation)
  * [월드 연결](#world-connection)
  * [기타 클라이언트 유틸리티](#other-client-utilities)
* [__월드__](#the-world)
  * [액터](#actors)
  * [날씨](#weather)
  * [조명](#lights)
  * [디버깅](#debugging)
  * [월드 스냅샷](#world-snapshots)
  * [월드 설정](#world-settings)

---
## 클라이언트

클라이언트는 CARLA 아키텍처의 주요 요소 중 하나입니다. 서버에 연결하여 정보를 검색하고 변경을 명령합니다. 이는 스크립트를 통해 수행됩니다. 클라이언트는 자신을 식별하고 월드에 연결하여 시뮬레이션을 조작합니다.

이외에도 클라이언트는 고급 CARLA 모듈, 기능에 접근하고 명령 배치를 적용할 수 있습니다. 이 섹션에서는 명령 배치만 다룹니다. 이는 많은 액터를 한 번에 생성하는 것과 같은 기본적인 작업에 유용합니다. 나머지 기능들은 더 복잡하며, **고급 단계**의 각 페이지에서 다룰 것입니다.

클래스의 구체적인 메서드와 변수에 대해 알아보려면 Python API 레퍼런스의 [__carla.Client__](python_api.md#carla.Client)를 참조하세요.
### 클라이언트 생성

두 가지가 필요합니다. 클라이언트를 식별하는 __IP__ 주소와 서버와 통신하기 위한 **두 개의 TCP 포트**입니다. 세 번째 선택적 매개변수는 작업 스레드의 수를 설정합니다. 기본값은 전체(`0`)입니다. Python API 레퍼런스의 [carla.Client](python_api.md#carla.Client.__init__)에는 스크립트 실행 시 이러한 매개변수를 인수로 파싱하는 방법을 보여주는 예제가 포함되어 있습니다.

```py
client = carla.Client('localhost', 2000)
```
기본적으로 CARLA는 로컬호스트 IP와 2000번 포트를 사용하여 연결하지만 이는 필요에 따라 변경할 수 있습니다. 두 번째 포트는 항상 `n+1`, 즉 이 경우 2001이 됩니다.

클라이언트가 생성되면, __시간 제한__을 설정합니다. 이는 모든 네트워킹 작업이 클라이언트를 영원히 차단하지 않도록 제한합니다. 연결이 실패하면 오류가 반환됩니다.

```py
client.set_timeout(10.0) # 초 단위
```

동시에 여러 스크립트를 실행하는 것이 일반적이므로 여러 클라이언트가 연결될 수 있습니다. 트래픽 매니저와 같은 고급 CARLA 기능을 사용하여 멀티클라이언트 방식으로 작업하면 통신이 더 복잡해질 수 있습니다.

!!! 참고
    클라이언트와 서버는 서로 다른 `libcarla` 모듈을 가지고 있습니다. 버전이 다르면 문제가 발생할 수 있습니다. 이는 `get_client_version()`과 `get_server_version()` 메서드를 사용하여 확인할 수 있습니다.

### 월드 연결

클라이언트는 현재 월드에 쉽게 연결하고 정보를 검색할 수 있습니다.

```py
world = client.get_world()
```

클라이언트는 또한 현재 맵을 변경하기 위해 사용 가능한 맵 목록을 얻을 수 있습니다. 이는 현재 월드를 파괴하고 새로운 월드를 생성합니다.
```py
print(client.get_available_maps())
...
world = client.load_world('Town01')
# client.reload_world()는 동일한 맵으로 월드의 새 인스턴스를 생성합니다.
```

각 월드 객체는 `id` 또는 에피소드를 가집니다. 클라이언트가 `load_world()` 또는 `reload_world()`를 호출할 때마다 이전 월드는 파괴됩니다. 새로운 에피소드로 처음부터 새 월드가 생성됩니다. 이 과정에서 언리얼 엔진은 재시작되지 않습니다.
### 명령 사용하기

__명령들__은 가장 일반적인 CARLA 메서드들을 배치로 적용할 수 있도록 조정한 것입니다. 예를 들어, [command.SetAutopilot](python_api.md#command.SetAutopilot)은 [Vehicle.set_autopilot()](python_api.md#carla.Vehicle.set_autopilot)과 동일하며, 차량의 자율주행을 활성화합니다. 하지만 [Client.apply_batch](python_api.md#carla.Client.apply_batch) 또는 [Client.apply_batch_sync()](python_api.md#carla.Client.apply_batch_sync) 메서드를 사용하면 명령 목록을 하나의 시뮬레이션 스텝에서 적용할 수 있습니다. 이는 보통 수백 개의 요소에 적용해야 하는 메서드에 매우 유용합니다.

다음 예제는 배치를 사용하여 차량 목록을 한 번에 모두 파괴합니다.

```py
client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])
```

사용 가능한 모든 명령은 Python API 레퍼런스의 [마지막 섹션](python_api.md#command.ApplyAngularVelocity)에 나열되어 있습니다.

### 기타 클라이언트 유틸리티

클라이언트 객체의 주요 목적은 월드를 가져오거나 변경하고 명령을 적용하는 것입니다. 하지만 몇 가지 추가 기능에도 접근할 수 있습니다.

* __트래픽 매니저.__ 이 모듈은 도시 교통을 재현하기 위해 자율주행으로 설정된 모든 차량을 담당합니다.
* __[레코더](adv_recorder.md).__ 이전 시뮬레이션을 재연할 수 있게 합니다. 프레임별 시뮬레이션 상태를 요약하는 [스냅샷](core_world.md#world-snapshots)을 사용합니다.

---
## 월드

시뮬레이션의 주요 관리자입니다. 클라이언트가 인스턴스를 검색해야 합니다. 월드 자체의 모델은 포함하지 않으며, 이는 [Map](core_map.md) 클래스의 일부입니다. 대신 대부분의 정보와 일반 설정은 이 클래스에서 접근할 수 있습니다.

* 시뮬레이션의 액터와 관찰자
* 블루프린트 라이브러리
* 맵
* 시뮬레이션 설정
* 스냅샷
* 날씨와 조명 관리자

이에 대해 더 자세히 알아보려면 [carla.World](python_api.md#carla.World)를 참조하세요.
### 액터

월드는 액터와 관련된 여러 메서드를 가지고 있어 다양한 기능을 제공합니다.

* 액터 생성 (단, 파괴는 불가)
* 장면의 모든 액터를 가져오거나 특정 액터를 찾기
* 블루프린트 라이브러리 접근
* 시뮬레이션의 관점인 관찰자 액터 접근
* 액터를 생성하기에 적합한 무작위 위치 검색

생성에 대해서는 [2장. 액터와 블루프린트](core_actors.md)에서 설명됩니다. 블루프린트 라이브러리, 속성 등에 대한 이해가 필요합니다.

### 날씨

날씨는 독립된 클래스가 아니라 월드에서 접근할 수 있는 매개변수 집합입니다. 매개변수화에는 태양 방향, 구름량, 바람, 안개 등이 포함됩니다. 도우미 클래스 [carla.WeatherParameters](python_api.md#carla.WeatherParameters)를 사용하여 사용자 지정 날씨를 정의합니다.

```py
weather = carla.WeatherParameters(
    cloudiness=80.0,
    precipitation=30.0,
    sun_altitude_angle=70.0)

world.set_weather(weather)

print(world.get_weather())
```

월드에 직접 적용할 수 있는 몇 가지 날씨 프리셋이 있습니다. 이들은 [carla.WeatherParameters](python_api.md#carla.WeatherParameters)에 나열되어 있고 열거형으로 접근할 수 있습니다.

```py
world.set_weather(carla.WeatherParameters.WetCloudySunset)
```

날씨는 CARLA가 제공하는 두 가지 스크립트를 사용하여 사용자 지정할 수도 있습니다.

* __`environment.py`__ *(PythonAPI/util에 위치)* — 날씨와 조명 매개변수에 접근하여 실시간으로 변경할 수 있게 합니다.
<details>
<summary> <b>environment.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help            도움말 메시지 표시
  --host H              호스트 서버의 IP (기본값: 127.0.0.1)
  -p P, --port P        수신할 TCP 포트 (기본값: 2000)
  --sun SUN             태양 위치 프리셋 [sunset | day | night]
  --weather WEATHER     날씨 조건 프리셋 [clear | overcast | rain]
  --altitude A, -alt A  태양 고도 [-90.0, 90.0]
  --azimuth A, -azm A   태양 방위각 [0.0, 360.0]
  --clouds C, -c C      구름량 [0.0, 100.0]
  --rain R, -r R        강수량 [0.0, 100.0]
  --puddles Pd, -pd Pd  물웅덩이 양 [0.0, 100.0]
  --wind W, -w W        바람 강도 [0.0, 100.0]
  --fog F, -f F         안개 강도 [0.0, 100.0]
  --fogdist Fd, -fd Fd  안개 거리 [0.0, inf)
  --wetness Wet, -wet Wet
                       습도 강도 [0.0, 100.0]
```
</details><br>

* __`dynamic_weather.py`__ *(PythonAPI/examples에 위치)* — 각 CARLA 맵을 위해 개발자가 준비한 특정 날씨 순환을 활성화합니다.

<details>
<summary> <b>dynamic_weather.py</b>의 선택적 인수들 </summary>

```sh
  -h, --help            도움말 메시지 표시
  --host H              호스트 서버의 IP (기본값: 127.0.0.1)
  -p P, --port P        수신할 TCP 포트 (기본값: 2000)
  -s FACTOR, --speed FACTOR
                        날씨가 변화하는 속도 (기본값: 1.0)
```
</details><br>

!!! 참고
    날씨 변화는 물리적 효과에 영향을 주지 않습니다. 카메라 센서로 포착할 수 있는 시각적 효과일 뿐입니다.

**sun_altitude_angle < 0**일 때 일몰로 간주되어 야간 모드가 시작됩니다. 이때 조명이 특히 중요해집니다.
### 조명

* **가로등**은 시뮬레이션이 야간 모드에 들어가면 자동으로 켜집니다. 조명은 맵 개발자가 배치하며 [__carla.Light__](python_api.md#carla.Light) 객체로 접근할 수 있습니다. 색상과 강도 같은 속성을 자유롭게 변경할 수 있습니다. [__carla.LightState__](python_api.md#carla.LightState) 타입의 변수 __light_state__를 사용하면 이러한 속성을 한 번에 설정할 수 있습니다.
가로등은 [__carla.LightGroup__](python_api.md#carla.LightGroup) 타입의 __light_group__ 속성을 사용하여 분류됩니다. 이를 통해 가로등, 건물 조명 등으로 분류할 수 있습니다. [__carla.LightManager__](python_api.md#carla.LightManager)의 인스턴스를 가져와서 여러 조명을 한 번에 제어할 수 있습니다.

```py
# 조명 관리자와 조명 가져오기
lmanager = world.get_lightmanager()
mylights = lmanager.get_all_lights()

# 특정 조명 사용자 지정
light01 = mylights[0]
light01.turn_on()
light01.set_intensity(100.0)
state01 = carla.LightState(200.0,red,carla.LightGroup.Building,True)
light01.set_light_state(state01)

# 조명 그룹 사용자 지정
my_lights = lmanager.get_light_group(carla.LightGroup.Building)
lmanager.turn_on(my_lights)
lmanager.set_color(my_lights,carla.Color(255,0,0))
lmanager.set_intensities(my_lights,list_of_intensities)
```
* __차량 조명__은 사용자가 직접 켜고 끄게 되어 있습니다. 각 차량은 [__carla.VehicleLightState__](python_api.md#carla.VehicleLightState)에 나열된 조명 세트를 가지고 있습니다. 현재까지 모든 차량에 조명이 통합되어 있지는 않습니다. 다음은 현재 사용 가능한 차량 목록입니다.
  * __자전거.__ 모든 자전거는 전방과 후방 위치등을 가지고 있습니다.
  * __오토바이.__ Yamaha와 Harley Davidson 모델.
  * __자동차.__ Audi TT, Chevrolet, Dodge(경찰차), Etron, Lincoln, Mustang, Tesla 3S, Wolkswagen T2 및 CARLA에 새로 추가되는 차량들.

차량의 조명은 [carla.Vehicle.get_light_state](python_api.md#carla.Vehicle.get_light_state)와 [carla.Vehicle.set_light_state](#python_api.md#carla.Vehicle.set_light_state) 메서드를 사용하여 언제든지 가져오고 업데이트할 수 있습니다. 이들은 이진 연산을 사용하여 조명 설정을 사용자 지정합니다.

```py
# 위치등 켜기
current_lights = carla.VehicleLightState.NONE
current_lights |= carla.VehicleLightState.Position
vehicle.set_light_state(current_lights)
```

!!! 참고
    조명은 [날씨](#weather) 섹션에서 설명한 `environment.py`를 사용하여 실시간으로 설정할 수도 있습니다.

### 디버깅

월드 객체는 공개 속성으로 [carla.DebugHelper](python_api.md#carla.DebugHelper) 객체를 가지고 있습니다. 이를 통해 시뮬레이션 중에 다양한 형태를 그릴 수 있습니다. 이는 발생하는 이벤트를 추적하는 데 사용됩니다. 다음 예제는 액터의 위치와 회전에 빨간색 상자를 그립니다.

```py
debug = world.debug
debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
```

이 예제는 [carla.DebugHelper](python_api.md#carla.DebugHelper.draw_box)의 스니펫에서 확장되어 월드 스냅샷의 모든 액터에 대해 상자를 그리는 방법을 보여줍니다.
### 월드 스냅샷

월드 스냅샷은 단일 프레임에서의 시뮬레이션의 모든 액터의 상태를 포함합니다. 시간 참조가 있는 월드의 일종의 정지 이미지입니다. 비동기 모드에서도 정보는 동일한 시뮬레이션 스텝에서 가져옵니다.

```py
# 현재 프레임의 월드 스냅샷을 가져옵니다.
world_snapshot = world.get_snapshot()
```

[carla.WorldSnapshot](python_api.md#carla.WorldSnapshot)은 [carla.Timestamp](python_api.md#carla.Timestamp)와 [carla.ActorSnapshot](python_api.md#carla.ActorSnapshot) 리스트를 포함합니다. 액터 스냅샷은 액터의 `id`를 사용하여 검색할 수 있습니다. 스냅샷은 그 안에 나타나는 액터들의 `id`를 나열합니다.

```py
timestamp = world_snapshot.timestamp # 시간 참조 가져오기 

for actor_snapshot in world_snapshot: # 액터와 스냅샷 정보 가져오기
    actual_actor = world.get_actor(actor_snapshot.id)
    actor_snapshot.get_transform()
    actor_snapshot.get_velocity()
    actor_snapshot.get_angular_velocity()
    actor_snapshot.get_acceleration()  

actor_snapshot = world_snapshot.find(actual_actor.id) # 특정 액터의 스냅샷 가져오기
```

### 월드 설정

월드는 시뮬레이션을 위한 일부 고급 구성에 접근할 수 있습니다. 이들은 렌더링 조건, 시뮬레이션 타임스텝, 클라이언트와 서버 간의 동기화를 결정합니다. 이들은 도우미 클래스 [carla.WorldSettings](python_api.md#carla.WorldSettings)를 통해 접근할 수 있습니다.

현재로서는 기본 CARLA는 최상의 그래픽 품질, 가변 타임스텝, 비동기 모드로 실행됩니다. 이러한 사항들에 대해 더 자세히 알아보려면 __고급 단계__ 섹션을 참조하세요. [동기화와 타임스텝](adv_synchrony_timestep.md), [렌더링 옵션](adv_rendering_options.md) 페이지가 좋은 시작점이 될 수 있습니다.

---
이것으로 월드와 클라이언트 객체에 대한 설명을 마칩니다. 다음 단계에서는 시뮬레이션에 생명을 불어넣기 위해 액터와 블루프린트에 대해 자세히 살펴보겠습니다.

더 자세히 알아보려면 계속 읽어보세요. 이 글을 읽는 동안 떠오른 의문점이나 제안사항을 포럼에 게시하세요.

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼">
CARLA 포럼</a>
</p>
</div>
<div class="build-buttons">
<p>
<a href="../core_actors" target="_blank" class="btn btn-neutral" title="2장. 액터와 블루프린트">
2장. 액터와 블루프린트</a>
</p>
</div>
</div>