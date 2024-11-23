# 센서와 데이터

센서는 주변 환경에서 데이터를 수집하는 액터입니다. 이들은 주행 에이전트의 학습 환경을 만드는 데 매우 중요합니다.

이 페이지는 센서 사용을 시작하는 데 필요한 모든 내용을 요약합니다. 사용 가능한 센서 유형과 센서 수명 주기에 대한 단계별 가이드를 소개합니다. 각 센서에 대한 자세한 내용은 [센서 레퍼런스](ref_sensors.md)에서 확인할 수 있습니다.

* [__센서 단계별 가이드__](#센서-단계별-가이드)
  * [설정](#설정)
  * [스폰](#스폰)
  * [리스닝](#리스닝)
  * [데이터](#데이터)
* [__센서 유형__](#센서-유형)
  * [카메라](#카메라)
  * [감지기](#감지기)
  * [기타](#기타)
* [__센서 레퍼런스__](ref_sensors.md)

---
## 센서 단계별 가이드

[carla.Sensor](python_api.md#carla.Sensor) 클래스는 데이터를 측정하고 스트리밍할 수 있는 특별한 유형의 액터를 정의합니다.

* __이 데이터는 무엇인가요?__ 센서 유형에 따라 크게 다릅니다. 모든 유형의 데이터는 일반적인 [carla.SensorData](python_api.md#carla.SensorData)를 상속받습니다.
* __데이터는 언제 수집되나요?__ 시뮬레이션의 매 단계마다 또는 특정 이벤트가 등록될 때 수집됩니다. 센서 유형에 따라 다릅니다.
* __데이터는 어떻게 수집되나요?__ 모든 센서에는 데이터를 수신하고 관리하는 `listen()` 메서드가 있습니다.

차이점에도 불구하고 모든 센서는 비슷한 방식으로 사용됩니다.

### 설정

다른 액터와 마찬가지로, 블루프린트를 찾고 특정 속성을 설정합니다. 이는 센서를 다룰 때 필수적입니다. 이러한 속성들이 얻게 될 결과를 결정합니다. 자세한 내용은 [센서 레퍼런스](ref_sensors.md)에서 확인할 수 있습니다.

다음 예제는 HD 대시보드 카메라를 설정합니다.

```py
# 센서의 블루프린트를 찾습니다.
blueprint = world.get_blueprint_library().find('sensor.camera.rgb')
# 이미지 해상도와 시야각을 설정하기 위해 블루프린트의 속성을 수정합니다.
blueprint.set_attribute('image_size_x', '1920')
blueprint.set_attribute('image_size_y', '1080')
blueprint.set_attribute('fov', '110')
# 센서 캡처 사이의 시간(초)을 설정합니다
blueprint.set_attribute('sensor_tick', '1.0')
```

### 스폰

`attachment_to`와 `attachment_type`은 매우 중요합니다. 센서는 일반적으로 차량에 부착되어 차량을 따라다니며 정보를 수집해야 합니다. 부착 유형은 차량에 대한 위치 업데이트 방식을 결정합니다.

* __강체 부착.__ 부모 위치에 대해 엄격한 움직임을 갖습니다. 시뮬레이션에서 데이터를 수집하는 데 적합한 부착 방식입니다.
* __스프링암 부착.__ 작은 가속과 감속으로 움직임이 완화됩니다. 이 부착은 시뮬레이션에서 영상을 녹화할 때만 권장됩니다. 카메라 위치 업데이트 시 움직임이 부드럽고 "튀는 현상"이 방지됩니다.
* __스프링암고스트 부착.__ 이전 방식과 비슷하지만 충돌 테스트를 하지 않아 카메라나 센서가 벽이나 다른 기하학적 구조물을 통과할 수 있습니다.

```py
transform = carla.Transform(carla.Location(x=0.8, z=1.7))
sensor = world.spawn_actor(blueprint, transform, attach_to=my_vehicle)
```

!!! 중요
    부착하여 스폰할 때는 위치가 부모 액터를 기준으로 상대적이어야 합니다.

### 리스닝

모든 센서에는 [`listen()`](python_api.md#carla.Sensor.listen) 메서드가 있습니다. 이는 센서가 데이터를 수집할 때마다 호출됩니다.

`callback` 인자는 [람다 함수](https://www.w3schools.com/python/python_lambda.asp)입니다. 이는 데이터가 수집될 때 센서가 수행해야 할 작업을 설명합니다. 수집된 데이터를 인자로 가져야 합니다.

```py
# 카메라가 새 이미지를 생성할 때마다 do_something()이 호출됩니다.
sensor.listen(lambda data: do_something(data))

...

# 이 충돌 센서는 충돌이 감지될 때마다 출력합니다.
def callback(event):
    for actor_id in event:
        vehicle = world_ref().get_actor(actor_id)
        print('차량이 너무 가깝습니다: %s' % vehicle.type_id)

sensor02.listen(callback)
```

### 데이터

대부분의 센서 데이터 객체에는 정보를 디스크에 저장하는 함수가 있습니다. 이를 통해 다른 환경에서도 데이터를 사용할 수 있습니다.

센서 데이터는 센서 유형에 따라 크게 다릅니다. 자세한 설명은 [센서 레퍼런스](ref_sensors.md)를 참조하세요. 하지만 모든 센서 데이터에는 항상 다음과 같은 기본 정보가 태그되어 있습니다.

| 센서 데이터 속성 | 유형 | 설명 |
| -------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `frame` | int | 측정이 이루어진 프레임 번호. |
| `timestamp` | double | 에피소드 시작 이후의 시뮬레이션 시간(초) 기준 측정 타임스탬프. |
| `transform` | [carla.Transform](<../python_api#carlatransform>) | 측정 시점의 센서의 월드 기준 위치. |

<br>

!!! 중요
    `is_listening()`은 센서가 `listen`으로 등록된 콜백을 가지고 있는지 확인하는 __센서 메서드__입니다.
    `stop()`은 센서가 리스닝을 중지하도록 하는 __센서 메서드__입니다.
    `sensor_tick`은 데이터 수신 간 시뮬레이션 시간을 설정하는 __블루프린트 속성__입니다.

---
## 센서 유형

### 카메라

자신의 시점에서 월드의 사진을 찍습니다. [carla.Image](<../python_api#carlaimage>)를 반환하는 카메라의 경우 [carla.ColorConverter](python_api.md#carla.ColorConverter) 헬퍼 클래스를 사용하여 이미지를 수정하여 다른 정보를 표현할 수 있습니다.

* __데이터 수집__ 시뮬레이션 매 단계마다 수행.

| 센서 | 출력 | 개요 |
| ----------------- | ---------- | ------------------ |
| [Depth](ref_sensors.md#depth-camera) | [carla.Image](<../python_api#carlaimage>) | 시야 내 요소들의 깊이를 그레이스케일 맵으로 렌더링합니다. |
| [RGB](ref_sensors.md#rgb-camera) | [carla.Image](<../python_api#carlaimage>) | 주변 환경을 선명하게 보여줍니다. 장면의 일반적인 사진처럼 보입니다. |
| [Optical Flow](ref_sensors.md#optical-flow-camera) | [carla.Image](<../python_api#carlaimage>) | 카메라의 모든 픽셀의 움직임을 렌더링합니다. |
| [Semantic segmentation](ref_sensors.md#semantic-segmentation-camera) | [carla.Image](<../python_api#carlaimage>) | 시야 내 요소들을 태그에 따라 특정 색상으로 렌더링합니다. |
| [Instance segmentation](ref_sensors.md#instance-segmentation-camera) | [carla.Image](<../python_api#carlaimage>) | 시야 내 요소들을 태그와 고유 객체 ID에 따라 특정 색상으로 렌더링합니다. |
| [DVS](ref_sensors.md#dvs-camera) | [carla.DVSEventArray](<../python_api#carladvseventarray>) | 밝기 강도의 변화를 비동기적으로 이벤트 스트림으로 측정합니다. |

<br>

### 감지기

부착된 객체가 특정 이벤트를 등록할 때 데이터를 수집합니다.

* __데이터 수집__ 트리거될 때 수행.

| 센서 | 출력 | 개요 |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Collision](ref_sensors.md#collision-detector) | [carla.CollisionEvent](<../python_api#carlacollisionevent>) | 부모와 다른 액터 간의 충돌을 감지합니다. |
| [Lane invasion](ref_sensors.md#lane-invasion-detector) | [carla.LaneInvasionEvent](<../python_api#carlalaneinvasionevent>) | 부모가 차선 표시를 침범할 때 등록합니다. |
| [Obstacle](ref_sensors.md#obstacle-detector) | [carla.ObstacleDetectionEvent](<../python_api#carlaobstacledetectionevent>) | 부모 앞의 가능한 장애물을 감지합니다. |

<br>

### 기타

내비게이션, 물리적 속성 측정, 장면의 2D/3D 포인트 맵과 같은 다양한 기능을 제공합니다.

* __데이터 수집__ 시뮬레이션 매 단계마다 수행.

| 센서 | 출력 | 개요 |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [GNSS](ref_sensors.md#gnss-sensor) | [carla.GNSSMeasurement](<../python_api#carlagnssmeasurement>) | 센서의 지리적 위치를 수집합니다. |
| [IMU](ref_sensors.md#imu-sensor) | [carla.IMUMeasurement](<../python_api#carlaimumeasurement>) | 가속도계, 자이로스코프, 나침반으로 구성됩니다. |
| [LIDAR](ref_sensors.md#lidar-sensor) | [carla.LidarMeasurement](<../python_api#carlalidarmeasurement>) | 회전하는 LIDAR입니다. 주변 환경을 모델링하기 위해 좌표와 강도 정보가 포함된 4D 포인트 클라우드를 생성합니다. |
| [Radar](ref_sensors.md#radar-sensor) | [carla.RadarMeasurement](<../python_api#carlaradarmeasurement>) | 시야 내 요소와 이들의 센서에 대한 움직임을 모델링하는 2D 포인트 맵입니다. |
| [RSS](ref_sensors.md#rss-sensor) | [carla.RssResponse](<../python_api#carlarssresponse>) | 안전성 검사에 따라 차량에 적용되는 컨트롤러를 수정합니다. 이 센서는 다른 센서와 다른 방식으로 작동하며, 별도의 [RSS 문서](<../adv_rss>)가 있습니다. |
| [Semantic LIDAR](ref_sensors.md#semantic-lidar-sensor) | [carla.SemanticLidarMeasurement](<../python_api#carlasemanticlidarmeasurement>) | 회전하는 LIDAR입니다. 인스턴스와 시맨틱 세그멘테이션에 대한 추가 정보가 포함된 3D 포인트 클라우드를 생성합니다. |

<br>

---
이것으로 센서와 이들이 시뮬레이션 데이터를 수집하는 방법에 대한 설명을 마무리합니다.

이로써 CARLA 소개를 마칩니다. 하지만 아직 배울 것이 많이 남아있습니다.

* __학습을 계속하세요.__ CARLA에는 렌더링 옵션, 교통 관리자, 레코더 등 몇 가지 고급 기능이 있습니다. 지금이 이러한 기능들에 대해 더 자세히 알아볼 좋은 기회입니다.

<div class="build-buttons">
<p>
<a href="../adv_synchrony_timestep" target="_blank" class="btn btn-neutral" title="동기화와 타임스텝">
동기화와 타임스텝</a>
</p>
</div>

* __자유롭게 실험하세요.__ 이 문서의 __참조__ 섹션을 살펴보세요. Python API의 클래스, 센서, 코드 스니펫 등에 대한 자세한 정보가 포함되어 있습니다.

<div class="build-buttons">
<p>
<a href="../python_api" target="_blank" class="btn btn-neutral" title="Python API 레퍼런스">
Python API 레퍼런스</a>
</p>
</div>

* __의견을 공유하세요.__ 모든 의문점, 제안 및 아이디어는 포럼에서 환영합니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>