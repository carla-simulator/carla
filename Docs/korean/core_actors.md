# 액터와 블루프린트

CARLA의 액터(Actors)는 시뮬레이션 내에서 동작을 수행하고 다른 액터들에게 영향을 미칠 수 있는 요소들입니다. CARLA의 액터에는 차량과 보행자뿐만 아니라 센서, 교통 표지판, 신호등, 관찰자(spectator)도 포함됩니다. 이러한 액터들을 어떻게 다루는지 완벽하게 이해하는 것이 매우 중요합니다.

이 섹션에서는 액터의 생성, 제거, 유형 및 관리 방법을 다룹니다. 하지만 가능성은 거의 무한합니다. 실험해보고, 이 문서의 __튜토리얼__ 을 살펴보고, [CARLA 포럼](https://github.com/carla-simulator/carla/discussions/)에서 의문점과 아이디어를 공유해보세요.

- [__블루프린트__](#블루프린트)  
	- [블루프린트 라이브러리 관리](#블루프린트-라이브러리-관리)  
- [__액터 수명 주기__](#액터-수명-주기)  
	- [생성](#생성)  
	- [제어](#제어)  
	- [제거](#제거)  
- [__액터의 종류__](#액터의-종류)  
	- [센서](#센서)  
	- [관찰자](#관찰자)  
	- [교통 표지판과 신호등](#교통-표지판과-신호등)  
	- [차량](#차량)  
	- [보행자](#보행자)  

---
## 블루프린트

블루프린트는 사용자가 새로운 액터를 시뮬레이션에 쉽게 추가할 수 있게 해주는 레이아웃입니다. 이는 애니메이션과 일련의 속성이 포함된 미리 만들어진 모델입니다. 이러한 속성 중 일부는 수정할 수 있고 일부는 수정할 수 없습니다. 이 속성들에는 차량 색상, 라이다 센서의 채널 수, 보행자의 속도 등 다양한 요소들이 포함됩니다.

[블루프린트 라이브러리](bp_library.md)에서 사용 가능한 블루프린트와 그 속성들을 확인할 수 있습니다. 차량과 보행자 블루프린트에는 새로운(gen 2) 또는 이전(gen 1) 에셋인지를 나타내는 세대 속성이 있습니다.
### 블루프린트 라이브러리 관리

[carla.BlueprintLibrary](python_api.md#carla.BlueprintLibrary) 클래스는 [carla.ActorBlueprint](python_api.md#carla.ActorBlueprint) 요소들의 목록을 포함합니다. world 객체를 통해 이 라이브러리에 접근할 수 있습니다.

```py
blueprint_library = world.get_blueprint_library()
```

블루프린트에는 해당 블루프린트와 그것으로 생성된 액터를 식별하는 ID가 있습니다. 라이브러리에서 특정 ID를 찾거나, 무작위로 블루프린트를 선택하거나, [와일드카드 패턴](https://tldp.org/LDP/GNU-Linux-Tools-Summary/html/x11655.htm)을 사용하여 결과를 필터링할 수 있습니다.

```py
# 특정 블루프린트 찾기
collision_sensor_bp = blueprint_library.find('sensor.other.collision')
# 무작위로 차량 블루프린트 선택하기
vehicle_bp = random.choice(blueprint_library.filter('vehicle.*.*'))
```

또한, 각 [carla.ActorBlueprint](python_api.md#carla.ActorBlueprint)는 일련의 [carla.ActorAttribute](python_api.md#carla.ActorAttribute)를 가지고 있어 이를 _가져오거나(get)_ _설정할(set)_ 수 있습니다.

```py
is_bike = [vehicle.get_attribute('number_of_wheels') == 2]
if(is_bike):
    vehicle.set_attribute('color', '255,0,0')
```

!!! 참고
    일부 속성은 수정할 수 없습니다. [블루프린트 라이브러리](bp_library.md)에서 확인하세요.

속성들은 [carla.ActorAttributeType](python_api.md#carla.ActorAttributeType) 변수를 가지고 있습니다. 이는 열거형 목록에서 해당 속성의 유형을 나타냅니다. 또한, 수정 가능한 속성들은 __권장 값 목록__ 을 제공합니다.

```py
for attr in blueprint:
    if attr.is_modifiable:
        blueprint.set_attribute(attr.id, random.choice(attr.recommended_values))
```

!!! 참고
    사용자는 자신만의 차량을 만들 수 있습니다. __튜토리얼(에셋)__ 을 확인하여 방법을 알아보세요. 기여자들은 [새로운 콘텐츠를 CARLA에 추가](tuto_D_contribute_assets.md)할 수 있습니다.
---
## 액터 수명 주기

!!! 중요
    이 섹션에서는 액터와 관련된 여러 메서드를 다룹니다. Python API는 가장 일반적인 메서드들을 하나의 프레임에서 일괄 처리할 수 있는 __[commands](python_api.md#command.SpawnActor)__ 를 제공합니다.

### 생성

__world 객체는 액터를 생성하고 이를 추적하는 책임을 가집니다.__ 생성에는 블루프린트와 액터의 위치 및 회전을 지정하는 [carla.Transform](python_api.md#carla.Transform)만 필요합니다.

world는 액터를 생성하는 두 가지 다른 메서드를 제공합니다.

* [`spawn_actor()`](python_api.md#carla.World.spawn_actor)는 생성이 실패하면 예외를 발생시킵니다.
* [`try_spawn_actor()`](python_api.md#carla.World.try_spawn_actor)는 생성이 실패하면 `None`을 반환합니다.

```py
transform = Transform(Location(x=230, y=195, z=40), Rotation(yaw=180))
actor = world.spawn_actor(blueprint, transform)
```

!!! 중요
    CARLA는 [언리얼 엔진 좌표계](https://carla.readthedocs.io/en/latest/python_api/#carlarotation)를 사용합니다. [`carla.Rotation`](https://carla.readthedocs.io/en/latest/python_api/#carlarotation) 생성자는 `(pitch, yaw, roll)`로 정의되며, 이는 언리얼 엔진 에디터의 `(roll, pitch, yaw)`와 다르다는 점을 기억하세요.

지정된 위치에서 충돌이 발생하면 액터가 생성되지 않습니다. 이는 정적 객체나 다른 액터와의 충돌 모두에 해당합니다. 이러한 원치 않는 생성 충돌을 피하기 위한 방법이 있습니다.

* __차량을 위한__ `map.get_spawn_points()`. 권장되는 생성 지점 목록을 반환합니다.

```py
spawn_points = world.get_map().get_spawn_points()
```

* __보행자를 위한__ `world.get_random_location()`. 보도 위의 무작위 지점을 반환합니다. 이 메서드는 보행자의 목표 위치를 설정할 때도 사용됩니다.

```py
spawn_point = carla.Transform()
spawn_point.location = world.get_random_location_from_navigation()
```

액터는 생성될 때 다른 액터에 부착될 수 있습니다. 액터는 부착된 부모를 따라갑니다. 이는 특히 센서에 유용합니다. 부착은 (정밀한 데이터 수집에 적합한) 강체 방식이거나 부모에 따라 완화된 움직임을 가질 수 있습니다. 이는 헬퍼 클래스 [carla.AttachmentType](python_api.md#carla.AttachmentType)에 의해 정의됩니다.

다음 예제는 카메라를 차량에 강체로 부착하여 상대 위치가 고정되도록 합니다.

```py
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle, carla.AttachmentType.Rigid)
```

!!! 중요
    부착된 액터를 생성할 때, 제공되는 transform은 부모 액터를 기준으로 한 상대적인 것이어야 합니다.
생성되면 world 객체는 액터들을 목록에 추가합니다. 이 목록은 쉽게 검색하거나 반복할 수 있습니다.
```py
actor_list = world.get_actors()
# ID로 액터 찾기
actor = actor_list.find(id)
# 월드의 모든 속도 제한 표지판의 위치 출력하기
for speed_sign in actor_list.filter('traffic.speed_limit.*'):
    print(speed_sign.get_location())
```

### 제어

[carla.Actor](python_api.md#carla.Actor)는 주로 맵 주변의 액터들을 관리하기 위한 _get()_ 과 _set()_ 메서드로 구성되어 있습니다.

```py
print(actor.get_acceleration())
print(actor.get_velocity())

location = actor.get_location()
location.z += 10.0
actor.set_location(location)
```

액터의 물리 효과를 비활성화하여 제자리에 고정할 수 있습니다.

```py
actor.set_simulate_physics(False)
```

또한 액터들은 블루프린트에서 제공하는 태그를 가지고 있습니다. 이는 주로 시맨틱 세그멘테이션 센서에 유용합니다.

!!! 경고
    대부분의 메서드는 시뮬레이터에 비동기적으로 요청을 보냅니다. 시뮬레이터는 각 업데이트마다 이를 처리할 시간이 제한되어 있습니다. _set()_ 메서드를 과도하게 사용하면 상당한 지연이 누적될 수 있습니다.

### 제거

액터는 Python 스크립트가 종료될 때 자동으로 제거되지 않습니다. 명시적으로 자신을 제거해야 합니다.

```py
destroyed_sucessfully = actor.destroy() # 성공하면 True 반환
```

!!! 중요
    액터를 제거하는 것은 프로세스가 완료될 때까지 시뮬레이터를 블록합니다.

---
## 액터의 종류

### 센서

센서는 데이터 스트림을 생성하는 액터입니다. 이들은 별도의 섹션인 [4단계. 센서와 데이터](core_sensors.md)에서 다룹니다. 지금은 일반적인 센서 생성 주기를 살펴보겠습니다.

이 예제는 카메라 센서를 생성하고, 차량에 부착한 다음, 생성된 이미지를 디스크에 저장하도록 카메라에 지시합니다.

```py
camera_bp = blueprint_library.find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, relative_transform, attach_to=my_vehicle)
camera.listen(lambda image: image.save_to_disk('output/%06d.png' % image.frame))
```
* 센서도 블루프린트를 가집니다. 속성 설정이 매우 중요합니다.
* 대부분의 센서는 주변 정보를 수집하기 위해 차량에 부착됩니다.
* 센서는 데이터를 __수신__ 합니다. 데이터가 수신되면 __[Lambda 표현식](https://docs.python.org/3/reference/expressions.html)__ 으로 정의된 함수를 호출합니다 <small>(링크의 6.14 참조)</small>.
### 관찰자

관찰자는 언리얼 엔진에 의해 배치되어 게임 내 시점을 제공합니다. 시뮬레이터 창의 시점을 이동하는 데 사용할 수 있습니다. 다음 예제는 관찰자 액터를 이동시켜 원하는 차량을 바라보도록 시점을 변경합니다.

```py
spectator = world.get_spectator()
transform = vehicle.get_transform()
spectator.set_transform(carla.Transform(transform.location + carla.Location(z=50),
carla.Rotation(pitch=-90)))
```

### 교통 표지판과 신호등

현재 CARLA에서는 정지 표지판, 양보 표지판, 신호등만이 액터로 간주됩니다. 나머지 OpenDRIVE 표지판들은 API에서 [__carla.Landmark__](python_api.md#carla.Landmark)로 접근할 수 있습니다. 이들의 정보는 이러한 인스턴스를 통해 접근할 수 있지만, 시뮬레이션에서 액터로 존재하지는 않습니다. 랜드마크는 다음 단계인 __3단계. 맵과 내비게이션__ 에서 더 자세히 설명됩니다.

시뮬레이션이 시작되면 정지 표지판, 양보 표지판, 신호등은 OpenDRIVE 파일의 정보를 사용하여 자동으로 생성됩니다. __이들은 블루프린트 라이브러리에서 찾을 수 없으며__ 따라서 직접 생성할 수 없습니다.

!!! 참고
    CARLA 맵은 OpenDRIVE 파일에 교통 표지판이나 신호등을 포함하지 않습니다. 이들은 개발자가 수동으로 배치합니다.

[__교통 표지판__](python_api.md#carla.TrafficSign)은 다음 페이지에서 설명하는 것처럼 도로 맵 자체에 정의되어 있지 않습니다. 대신, [carla.BoundingBox](python_api.md#carla.BoundingBox)를 사용하여 그 안에 있는 차량에 영향을 줍니다.

```py
#차량에 영향을 주는 신호등 가져오기
if vehicle_actor.is_at_traffic_light():
    traffic_light = vehicle_actor.get_traffic_light()
```

[__신호등__](python_api.md#carla.TrafficLight)은 교차로에 있습니다. 모든 액터처럼 고유한 ID를 가지고 있지만, 교차로를 위한 `group` ID도 있습니다. 같은 그룹의 신호등을 식별하기 위해 `pole` ID가 사용됩니다.

같은 그룹의 신호등들은 주기를 따릅니다. 첫 번째 신호등이 녹색으로 설정되는 동안 나머지는 빨간색으로 고정됩니다. 활성화된 신호등은 녹색, 노란색, 빨간색에 각각 몇 초를 소비하므로, 모든 신호등이 빨간색인 기간이 있습니다. 그런 다음 다음 신호등이 주기를 시작하고, 이전 신호등은 나머지와 함께 고정됩니다.

API를 사용하여 신호등의 상태를 설정할 수 있습니다. 각 상태에 소비되는 시간도 마찬가지입니다. 가능한 상태는 [carla.TrafficLightState](python_api.md#carla.TrafficLightState)에서 열거형 값으로 설명됩니다.

```py
#빨간색 신호등을 녹색으로 변경
if traffic_light.get_state() == carla.TrafficLightState.Red:
    traffic_light.set_state(carla.TrafficLightState.Green)
    traffic_light.set_set_green_time(4.0)
```

!!! 참고
    차량은 신호등이 빨간색일 때만 신호등을 인식합니다.
### 차량

[__carla.Vehicle__](python_api.md#carla.Vehicle)은 특별한 종류의 액터입니다. 바퀴 달린 차량의 물리학을 시뮬레이션하는 특별한 내부 구성 요소를 포함합니다. 이는 네 가지 다른 종류의 제어를 적용하여 구현됩니다:

* __[carla.VehicleControl](python_api.md#carla.VehicleControl)__ 는 가속, 조향, 제동 등과 같은 주행 명령을 위한 입력을 제공합니다.
```py
    vehicle.apply_control(carla.VehicleControl(throttle=1.0, steer=-1.0))
```
* __[carla.VehiclePhysicsControl](python_api.md#carla.VehiclePhysicsControl)__ 는 차량의 물리적 속성을 정의하고 두 가지 추가 컨트롤러를 포함합니다:

    * [carla.GearPhysicsControl](python_api.md#carla.GearPhysicsControl)로 기어를 제어합니다.
    * [carla.WheelPhysicsControl](python_api.md#carla.WheelPhysicsControl)로 각 바퀴를 개별적으로 제어합니다.

```py
    vehicle.apply_physics_control(carla.VehiclePhysicsControl(max_rpm = 5000.0, center_of_mass = carla.Vector3D(0.0, 0.0, 0.0), torque_curve=[[0,400],[5000,400]]))
```

차량에는 이를 감싸는 [carla.BoundingBox](python_api.md#carla.BoundingBox)가 있습니다. 이 경계 상자는 차량에 물리 법칙이 적용되도록 하고 충돌을 감지할 수 있게 합니다.

```py
    box = vehicle.bounding_box
    print(box.location)         # 차량을 기준으로 한 상대 위치
    print(box.extent)           # XYZ 반경(미터 단위)
```

차량 바퀴의 물리학은 [sweep wheel collision 매개변수][enable_sweep]를 활성화하여 개선할 수 있습니다. 기본 바퀴 물리학은 각 바퀴에 대해 축에서 바닥까지 단일 레이캐스팅을 사용하지만, sweep wheel collision이 활성화되면 바퀴의 전체 부피에 대해 충돌을 확인합니다. 다음과 같이 활성화할 수 있습니다:

```py
    physics_control = vehicle.get_physics_control()
    physics_control.use_sweep_wheel_collision = True
    vehicle.apply_physics_control(physics_control)
```

[enable_sweep]: https://carla.readthedocs.io/en/latest/python_api/#carla.VehiclePhysicsControl.use_sweep_wheel_collision

차량에는 다른 고유한 기능들도 포함되어 있습니다:

* __자율주행 모드__ 는 실제 도시 조건을 시뮬레이션하기 위해 차량을 [Traffic Manager](adv_traffic_manager.md)에 등록합니다. 이 모듈은 하드코딩되어 있으며 머신러닝 기반이 아닙니다.

```py
    vehicle.set_autopilot(True)
```

* __차량 조명__ 은 사용자가 직접 켜고 꺼야 합니다. 각 차량은 [__carla.VehicleLightState__](python_api.md#carla.VehicleLightState)에 나열된 조명 세트를 가지고 있습니다. 모든 차량이 조명을 가지고 있는 것은 아닙니다. 현재 조명이 통합된 차량은 다음과 같습니다:
    * __자전거:__ 모든 자전거는 전방과 후방 위치등을 가지고 있습니다.
    * __오토바이:__ Yamaha와 Harley Davidson 모델.
    * __자동차:__ Audi TT, Chevrolet Impala, 두 종류의 Dodge 경찰차, Dodge Charger, Audi e-tron, Lincoln 2017과 2020, Mustang, Tesla Model 3, Tesla Cybertruck, Volkswagen T2, Mercedes C-Class.
차량의 조명은 [carla.Vehicle.get_light_state](python_api.md#carla.Vehicle.get_light_state)와 [carla.Vehicle.set_light_state](#python_api.md#carla.Vehicle.set_light_state) 메서드를 사용하여 언제든지 확인하고 업데이트할 수 있습니다. 이들은 이진 연산을 사용하여 조명 설정을 커스터마이즈합니다.

```py
# 위치등 켜기
current_lights = carla.VehicleLightState.NONE
current_lights |= carla.VehicleLightState.Position
vehicle.set_light_state(current_lights)
```

### 보행자

[__carla.Walker__](python_api.md#carla.Walker)는 차량과 비슷한 방식으로 작동합니다. 컨트롤러를 통해 제어됩니다.

* [__carla.WalkerControl__](python_api.md#carla.WalkerControl)은 보행자를 특정 방향과 속도로 이동시킵니다. 점프도 가능하게 합니다.
* [__carla.WalkerBoneControl__](python_api.md#carla.WalkerBoneControl)은 3D 스켈레톤을 제어합니다. [이 튜토리얼](tuto_G_control_walker_skeletons.md)에서 제어 방법을 설명합니다.

보행자는 AI로 제어될 수 있습니다. 자율주행 모드는 없습니다. [__carla.WalkerAIController__](python_api.md#carla.WalkerAIController) 액터는 자신이 부착된 액터를 이동시킵니다.

```py
walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
world.SpawnActor(walker_controller_bp, carla.Transform(), parent_walker)
```

!!! 참고
    AI 컨트롤러는 몸체가 없고 물리 효과가 없습니다. 화면에 나타나지 않습니다. 또한, 부모에 대한 상대 위치 `(0,0,0)`는 충돌을 일으키지 않습니다.

__각 AI 컨트롤러는 초기화, 목표, 선택적으로 속도가 필요합니다.__ 컨트롤러 정지도 같은 방식으로 작동합니다.

```py
ai_controller.start()
ai_controller.go_to_location(world.get_random_location_from_navigation())
ai_controller.set_max_speed(1 + random.random())  # 1~2 m/s 사이 (기본값은 1.4 m/s)
...
ai_controller.stop()
```

보행자가 목표 위치에 도달하면 자동으로 다른 무작위 지점으로 걸어갑니다. 목표 지점에 도달할 수 없는 경우, 보행자는 현재 위치에서 가장 가까운 지점으로 이동합니다.

[carla.Client](python_api.md#carla.Client.apply_batch_sync)의 코드 조각은 배치를 사용하여 많은 보행자를 생성하고 돌아다니게 만듭니다.

!!! 중요
    __AI 보행자를 제거하려면__, AI 컨트롤러를 정지시키고 액터와 컨트롤러 모두를 제거해야 합니다.

---
이것으로 CARLA의 액터에 대한 설명을 마무리합니다. 다음 단계에서는 CARLA의 맵, 도로, 교통에 대해 자세히 살펴보겠습니다.

계속 읽어보시거나 포럼을 방문하여 이 글을 읽는 동안 생긴 의문점이나 제안 사항을 게시해주세요.
<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼">
CARLA 포럼</a>
</p>
</div>
<div class="build-buttons">
<p>
<a href="../core_map" target="_blank" class="btn btn-neutral" title="3단계. 맵과 내비게이션">
3단계. 맵과 내비게이션</a>
</p>
</div>
</div>