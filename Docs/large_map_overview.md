# 대규모 맵 개요

- [__대규모 맵 개요__](#대규모-맵-개요)
- [__타일 스트리밍__](#타일-스트리밍)
- [__휴면 액터__](#휴면-액터)

---

## 대규모 맵 개요

CARLA의 대규모 맵 기능을 사용하면 광범위한 규모의 시뮬레이션을 수행할 수 있습니다. CARLA에서 대규모 맵은 2km x 2km를 넘지 않는 정사각형 타일로 분할됩니다. 이러한 타일들은 자아 차량(ego vehicle)과의 거리(스트리밍 거리)에 따라 서버에서 로드되거나 언로드됩니다. 맵 상의 다른 액터들도 자아 차량과의 스트리밍 거리에 따라 관리됩니다.

---

## 타일 스트리밍

자아 차량은 맵 타일의 로딩과 언로딩에 핵심적인 역할을 합니다. 타일들은 자아 차량으로부터의 스트리밍 거리 값을 기준으로 서버에서 로드되거나 언로드됩니다. 예를 들어, 스트리밍 거리 밖에 위치한 타일은 시뮬레이션에서 렌더링되지 않으며, 스트리밍 거리 내에 있는 타일은 렌더링됩니다. 렌더링되는 타일은 자아 차량이 이동함에 따라 변경됩니다.

차량을 자아로 설정하려면 아래와 같이 [`set_attribute`](python_api.md#carla.ActorBlueprint.set_attribute) 메서드를 사용하세요:

```py
blueprint.set_attribute('role_name', 'hero' )
world.spawn_actor(blueprint, spawn_point)
```

다음 코드를 사용하면 자아 차량으로부터 2km 반경 내에서 타일이 로드되도록 스트리밍 거리를 설정할 수 있습니다:

```py
settings = world.get_settings()
settings.tile_stream_distance = 2000
world.apply_settings(settings)
```

`config.py`를 사용하여 스트리밍 거리를 설정할 수도 있습니다:

```sh
cd PythonAPI/util
python3 config.py --tile-stream-distance 2000
```

!!! 참고
    대규모 맵은 현재 한 번에 하나의 자아 차량만 지원합니다.

---

## 휴면 액터

대규모 맵 기능은 CARLA에 휴면 액터라는 개념을 도입합니다. 휴면 액터는 대규모 맵에서만 존재하는 개념입니다. 휴면 액터는 자아 차량의 __액터 활성화 거리__ 밖에 위치한 비 자아 차량 액터를 말합니다(예: 자아 차량으로부터 멀리 떨어진 차량들). 액터 활성화 거리는 스트리밍 거리보다 작거나 같을 수 있습니다.

액터가 자아 차량의 액터 활성화 거리 밖에 있는 경우, 해당 액터는 휴면 상태가 됩니다. 액터는 여전히 존재하지만 렌더링되지 않습니다. 물리 연산은 계산되지 않지만(교통 관리자를 통한 하이브리드 모드에서 실행하는 경우 제외), [위치](python_api.md#carla.Actor.set_location)와 [변환](python_api.md#carla.Actor.set_transform)은 여전히 설정할 수 있습니다. 휴면 액터가 다시 자아 차량의 액터 활성화 거리 내로 들어오면 깨어나서 정상적으로 렌더링과 물리 연산이 재개됩니다.

교통 관리자가 제어하는 액터들은 대규모 맵에서 작동할 때 구성할 수 있는 고유한 동작을 가집니다. 이것이 어떻게 작동하는지 자세히 알아보려면 [교통 관리자 문서](adv_traffic_manager.md#traffic-manager-in-large-maps)를 참조하세요.

액터는 [`world.tick()`](python_api.md#carla.World.tick) 호출 시 휴면 상태가 되거나 깨어납니다.

자아 차량 주변 2km 반경으로 액터 활성화 거리를 설정하려면:

```py
settings = world.get_settings()
settings.actor_active_distance = 2000
world.apply_settings(settings)
```

`config.py`를 사용하여 액터 활성화 거리를 설정할 수도 있습니다:

```sh
cd PythonAPI/util
python3 config.py --actor-active-distance 2000
```

액터가 휴면 상태인지 확인하려면 Python API를 사용할 수 있습니다:

```py
actor.is_dormant
```

---

대규모 맵에 대해 질문이 있으시다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 문의하실 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>