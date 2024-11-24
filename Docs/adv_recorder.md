# Recorder

이 기능은 이전 시뮬레이션을 기록하고 재연할 수 있게 해줍니다. 발생한 모든 이벤트는 [recorder 파일](ref_recorder_binary_file_format.md)에 기록됩니다. 이러한 이벤트들을 추적하고 연구하기 위한 상위 수준의 쿼리들이 있습니다.

* [__녹화하기__](#recording)
* [__시뮬레이션 재생__](#simulation-playback)
   * [시간 배율 설정](#setting-a-time-factor)
* [__녹화된 파일__](#recorded-file)
* [__쿼리__](#queries)
   * [충돌](#collisions)
   * [막힌 액터들](#blocked-actors)
* [__예제 Python 스크립트__](#sample-python-scripts)

---
## 녹화하기

모든 데이터는 서버 측에서만 바이너리 파일로 작성됩니다. 하지만 recorder는 [carla.Client](python_api.md#carla.Client)를 사용하여 관리됩니다.

액터들은 기록된 파일에 포함된 데이터에 따라 매 프레임마다 업데이트됩니다. 현재 시뮬레이션에서 기록에 나타나는 액터들은 이를 모방하기 위해 이동되거나 다시 스폰됩니다. 기록에 나타나지 않는 액터들은 아무 일도 없었던 것처럼 계속 진행됩니다.

!!! Important
    재생이 끝나면 차량들은 자율주행 모드로 설정되지만, __보행자들은 멈춥니다__.

recorder 파일은 다양한 요소들에 대한 정보를 포함합니다.

* __Actors__ — 생성과 소멸, 경계 상자와 트리거 박스
* __Traffic lights__ — 상태 변경과 시간 설정
* __Vehicles__ — 위치와 방향, 선형 및 각속도, 라이트 상태, 물리 제어
* __Pedestrians__ — 위치와 방향, 선형 및 각속도
* __Lights__ — 건물, 거리, 차량의 조명 상태
녹화를 시작하려면 파일 이름만 있으면 됩니다. 파일 이름에 `\`, `/` 또는 `:`를 사용하면 절대 경로로 정의됩니다. 경로를 지정하지 않으면 파일은 `CarlaUE4/Saved`에 저장됩니다.

```py
client.start_recorder("/home/carla/recording01.log")
```

기본적으로 recorder는 시뮬레이션을 재생하는 데 필요한 정보만 저장하도록 설정되어 있습니다. 앞서 언급한 모든 정보를 저장하려면 녹화를 시작할 때 `additional_data` 인자를 설정해야 합니다.

```py
client.start_recorder("/home/carla/recording01.log", True)
```

!!! Note
    추가 데이터에는 다음이 포함됩니다: 차량과 보행자의 선형 및 각속도, 신호등 시간 설정, 실행 시간, 액터의 트리거 및 경계 상자, 차량의 물리 제어.

녹화를 중지하는 것도 간단합니다.

```py
client.stop_recorder()
```

!!! Note
    추정치로, 50개의 신호등과 100대의 차량이 있는 1시간 녹화는 약 200MB의 크기를 차지합니다.

---
## 시뮬레이션 재생

재생은 시뮬레이션 중 언제든지 시작할 수 있습니다. 로그 파일 경로 외에도 이 메서드에는 몇 가지 매개변수가 필요합니다.

```py
client.replay_file("recording01.log", start, duration, camera)
```

| Parameter | 설명 | 참고 |
| --- | --- | --- |
| `start` | 시뮬레이션을 시작할 녹화 시간(초) | 양수면 녹화 시작부터의 시간 <br> 음수면 끝에서부터의 시간 |
| `duration` | 재생할 시간(초). 0은 전체 녹화 | 재생이 끝나면 차량은 자율주행 모드로 설정되고 보행자는 멈춤 |
| `camera` | 카메라가 초점을 맞출 액터의 ID | `0`으로 설정하면 관전자가 자유롭게 이동 가능 |

<br>

### 시간 배율 설정

time factor는 재생 속도를 결정합니다. 재생을 중지하지 않고도 언제든지 변경할 수 있습니다.

```py
client.set_replayer_time_factor(2.0)
```

| Parameter | Default | Fast motion | Slow motion |
| --- | --- | --- | --- |
| `time_factor` | **1\.0** | **\>1.0** | **<1.0** |

<br>

!!! Important
    `time_factor>2.0`인 경우, 액터의 위치 보간이 비활성화되고 단순히 업데이트만 됩니다. 보행자의 애니메이션은 time factor의 영향을 받지 않습니다.

time factor가 약 __20x__일 때 교통 흐름을 쉽게 파악할 수 있습니다.

![flow](img/RecorderFlow2.gif)
---
## 녹화된 파일

녹화의 세부 사항은 간단한 API 호출을 통해 검색할 수 있습니다. 기본적으로는 이벤트가 기록된 프레임만 검색합니다. `show_all` 매개변수를 설정하면 모든 프레임의 정보를 반환합니다. 데이터가 저장되는 방식에 대한 자세한 내용은 [recorder의 참조 문서](ref_recorder_binary_file_format.md)에 설명되어 있습니다.

```py
# Show info for relevant frames
print(client.show_recorder_file_info("recording01.log"))
```

* __Opening information.__ 시뮬레이션이 녹화된 맵, 날짜 및 시간.

* __Frame information.__ 액터 스폰이나 충돌과 같이 발생할 수 있는 모든 이벤트. 액터의 ID와 추가 정보를 포함합니다.

* __Closing information.__ 기록된 프레임 수와 총 시간.

```
Version: 1
Map: Town05
Date: 02/21/19 10:46:20

Frame 1 at 0 seconds
 Create 2190: spectator (0) at (-260, -200, 382.001)
 Create 2191: traffic.traffic_light (3) at (4255, 10020, 0)
 Create 2192: traffic.traffic_light (3) at (4025, 7860, 0)
 ...
 Create 2258: traffic.speed_limit.90 (0) at (21651.7, -1347.59, 15)
 Create 2259: traffic.speed_limit.90 (0) at (5357, 21457.1, 15)

Frame 2 at 0.0254253 seconds
 Create 2276: vehicle.mini.cooperst (1) at (4347.63, -8409.51, 120)
  number_of_wheels = 4
  object_type =
  color = 255,241,0
  role_name = autopilot
... 
Frame 2350 at 60.2805 seconds
 Destroy 2276

Frame 2351 at 60.3057 seconds
 Destroy 2277
...

Frames: 2354
Duration: 60.3753 seconds
```

---
## 쿼리

### 충돌

충돌을 기록하려면 차량에 [collision detector](ref_sensors.md#collision-detector)가 부착되어 있어야 합니다. 충돌에 관련된 액터의 유형을 필터링하는 인자를 사용하여 이를 쿼리할 수 있습니다. 예를 들어, `h`는 일반적으로 사용자가 관리하는 차량에 할당되는 `role_name = hero`를 가진 액터를 식별합니다. 쿼리에 사용할 수 있는 특정 액터 유형 집합이 있습니다.

* __h__ = Hero
* __v__ = Vehicle
* __w__ = Walker
* __t__ = Traffic light
* __o__ = Other
* __a__ = Any

!!! Note
    `manual_control.py` 스크립트는 ego 차량에 `role_name = hero`를 할당합니다.

충돌 쿼리는 충돌을 필터링하기 위한 두 개의 플래그가 필요합니다. 다음 예제는 차량과 다른 모든 객체 사이의 충돌을 보여줍니다.
```py
print(client.show_recorder_collisions("recording01.log", "v", "a"))
```

출력은 충돌 시간과 관련된 액터들의 유형, ID 및 설명을 요약합니다.

```
Version: 1
Map: Town05
Date: 02/19/19 15:36:08

    Time  Types     Id Actor 1                                 Id Actor 2
      16   v v     122 vehicle.yamaha.yzf                     118 vehicle.dodge_charger.police
      27   v o     122 vehicle.yamaha.yzf                       0

Frames: 790
Duration: 46 seconds
```

!!! Important
    `hero` 또는 `ego` 차량이 충돌을 기록하기 때문에, 이는 항상 `Actor 1`이 됩니다.

recorder를 사용하고 이벤트 몇 초 전으로 설정하여 충돌을 재연할 수 있습니다.

```py
client.replay_file("col2.log", 13, 0, 122)
```
이 경우, 재생은 다음과 같이 보여줍니다.

![collision](img/collision1.gif)

### 막힌 액터들

녹화 중에 멈춰있던 차량을 감지합니다. 액터가 특정 시간 동안 최소 거리를 이동하지 않으면 막힌 것으로 간주됩니다. 이 정의는 쿼리 중에 사용자가 지정합니다.

```py
print(client.show_recorder_actors_blocked("recording01.log", min_time, min_distance))
```

| Parameter | 설명 | Default |
| --- | --- | --- |
| `min_time` | `min_distance`를 이동하는 데 필요한 최소 시간(초) | 30초 |
| `min_distance` | 막힌 것으로 간주되지 않기 위해 이동해야 하는 최소 거리(cm) | 10cm |

---

!!! Note
    때때로 차량들이 예상보다 오래 신호등에 멈춰있을 수 있습니다.

다음 예제는 차량이 60초 동안 1미터 미만으로 이동할 때 막힌 것으로 간주합니다.

```py
client.show_recorder_actors_blocked("col3.log", 60, 100)
```

출력은 "막힌" 상태를 벗어나 `min_distance`를 이동하는 데 걸린 시간을 나타내는 __duration__으로 정렬됩니다.
```
Version: 1
Map: Town05
Date: 02/19/19 15:45:01

    Time     Id Actor                                 Duration
      36    173 vehicle.nissan.patrol                      336
      75    214 vehicle.chevrolet.impala                   295
     302    143 vehicle.bmw.grandtourer                     67

Frames: 6985
Duration: 374 seconds
```

차량 `173`은 시간 `36`초에서 `336`초 동안 멈춰있었습니다. 36초 몇 초 전으로 시뮬레이션을 되감아 확인해볼 수 있습니다.

```py
client.replay_file("col3.log", 34, 0, 173)
```

![accident](img/accident.gif)

---
## 예제 Python 스크립트

`PythonAPI/examples`에 제공된 스크립트 중 일부는 recorder의 사용을 용이하게 합니다.

* __start_recording.py__ 녹화를 시작합니다. 녹화 시간을 설정할 수 있으며, 녹화 시작 시 액터를 스폰할 수 있습니다.

| Parameter | 설명 |
| --- | --- |
| `-f` | 파일 이름 |
| `-n`<small> (optional)</small> | 스폰할 차량 수. 기본값은 10 |
| `-t`<small> (optional)</small> | 녹화 시간 |

* __start_replaying.py__ 녹화 재생을 시작합니다. 시작 시간, 지속 시간, 팔로우할 액터를 설정할 수 있습니다.

| Parameter | 설명 |
| --- | --- |
| `-f` | 파일 이름 |
| `-s`<small> (optional)</small> | 시작 시간. 기본값은 10 |
| `-d`<small> (optional)</small> | 지속 시간. 기본값은 전체 |
| `-c`<small> (optional)</small> | 팔로우할 액터의 ID |

* __show_recorder_file_info.py__ 녹화 파일의 모든 정보를 보여줍니다. 기본적으로는 이벤트가 기록된 프레임만 표시하지만, 모든 프레임을 표시할 수 있습니다.

| Parameter | 설명 |
| --- | --- |
| `-f` | 파일 이름 |
| `-s`<small> (optional)</small> | 모든 세부 정보를 표시하는 플래그 |
* __show_recorder_collisions.py__ 유형 __A__와 __B__의 액터들 사이의 기록된 충돌을 보여줍니다. `-t = vv`는 차량들 간의 모든 충돌을 보여줍니다.

| Parameter | 설명 |
| --- | --- |
| `-f` | 파일 이름 |
| `-t` | 관련된 액터들의 플래그 <br>`h` = hero <br> `v` = vehicle <br> `w` = walker <br> `t` = traffic light <br>`o` = other <br>`a` = any |

* __show_recorder_actors_blocked.py__ 막힌 것으로 간주되는 차량들을 나열합니다. 특정 시간 동안 최소 거리를 이동하지 않으면 액터가 막힌 것으로 간주됩니다.

| Parameter | 설명 |
| --- | --- |
| `-f` | 파일 이름 |
| `-t`<small> (optional)</small> | 막힌 것으로 간주되기 전에 `-d`를 이동하는 시간 |
| `-d`<small> (optional)</small> | 막힌 것으로 간주되지 않기 위해 이동해야 하는 거리 |

---
이제 잠시 실험해볼 시간입니다. recorder를 사용하여 시뮬레이션을 재생하고, 이벤트를 추적하고, 새로운 결과를 보기 위해 변경해보세요. CARLA 포럼에서 이 주제에 대해 자유롭게 의견을 나누어 주세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>