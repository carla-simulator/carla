# 성능 벤치마킹

우리는 사용자가 자신의 환경에서 CARLA의 성능을 쉽게 분석할 수 있도록 벤치마킹 스크립트를 제공합니다. 스크립트는 다양한 맵, 센서, 날씨 조건을 결합한 여러 시나리오를 실행하도록 구성할 수 있습니다. 요청된 시나리오에서의 평균 FPS와 표준 편차를 보고합니다.

이 섹션에서는 벤치마크를 실행하기 위한 요구사항, 스크립트 위치, 실행되는 시나리오를 커스터마이즈하는 데 사용할 수 있는 플래그, 명령어 실행 예제를 자세히 설명합니다.

또한 차량 수, 물리 활성화 및/또는 Traffic Manager 활성화의 다양한 조합을 사용할 때 특정 환경에서 CARLA의 성능을 측정하는 별도의 벤치마크 결과도 포함했습니다. 결과는 사용된 CARLA 버전과 테스트가 수행된 환경과 함께 제시됩니다.

- [__벤치마크 스크립트__](#벤치마크-스크립트)
    - [__시작하기 전에__](#시작하기-전에)
    - [__개요__](#개요)
        - [__플래그__](#플래그)
- [__CARLA 성능 보고서__](#carla-성능-보고서)

---
## 벤치마크 스크립트

벤치마크 스크립트는 `PythonAPI/util`에서 찾을 수 있습니다. 아래 개요에서 자세히 설명하는 테스트할 시나리오를 커스터마이즈하기 위한 여러 플래그를 사용할 수 있습니다.

### 시작하기 전에

벤치마킹 스크립트를 실행하기 전에 몇 가지 의존성을 설치해야 합니다:

```python
python -m pip install -U py-cpuinfo==5.0.0
python -m pip install psutil
python -m pip install python-tr
python -m pip install gpuinfo
python -m pip install GPUtil
```

### 개요

`python3` [`performance_benchmark.py`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/performance_benchmark.py) [`[--host HOST]`](#-host-ip_address) [`[--port PORT]`](#-port-port) [`[--file FILE]`](#-file-filenamemd) [`[--tm]`](#-tm)
[`[--ticks TICKS]`](#-ticks) [`[--sync]`](#-sync) [`[--async]`](#-async))
[`[--fixed_dt FIXED_DT]`](#-fixed_dt) [`[--render_mode]`](#-render_mode))
[`[--no_render_mode]`](#-no_render_mode) [`[--show_scenarios]`](#-show_scenarios))
[`[--sensors SENSORS [SENSORS ...]]`](#-sensors-integer))
[`[--maps MAPS [MAPS ...]]`](#-maps-townname))
[`[--weather WEATHER [WEATHER ...]]`](#-weather-integer)

#### 플래그

###### `--host`: IP_ADDRESS
>> __기본값__: Localhost

>> 서버의 호스트를 구성합니다.

###### `--port`: PORT
>> __기본값__: 2000

>> 수신할 TCP 포트를 구성합니다.

###### `--file`: filename.md
>> __기본값__: benchmark.md

>> 결과를 마크다운 표 형식으로 파일에 기록합니다.

###### `--tm`
>> Traffic Manager 벤치마크로 전환합니다.

###### `--ticks`
>> __기본값__: 100

>> 각 시나리오에서 사용할 틱 수를 설정합니다.

###### `--sync`
>> __기본 모드__

>> 동기 모드에서 벤치마크를 실행합니다.

###### `--async`
>> 비동기 모드에서 벤치마크를 실행합니다.

###### `--fixed_dt`
>> __기본값__: 0.05

>> 델타 타임스텝을 설정하고 싶을 때 동기 모드와 함께 사용합니다.

###### `--render_mode`
>> 렌더링 모드에서 벤치마크를 실행합니다.

###### `--no_render_mode`
>> __기본 모드__

>> 비렌더링 모드에서 벤치마크를 실행합니다.

###### `--show_scenarios`
>> 이 플래그만으로 스크립트를 실행하면 사용 가능한 모든 시나리오 매개변수의 목록을 볼 수 있습니다.

>> 다른 플래그와 함께 사용하면 실제로 실행하지 않고도 실행될 시나리오의 미리보기를 볼 수 있습니다.

###### `--sensors`: INTEGER
>> __기본값__: 모두

>> 벤치마크에서 사용할 센서. LIDAR와 RGB 카메라 중에서 선택:

>> * __`0`__: cam-300x200
>> * __`1`__: cam-800x600
>> * __`2`__: cam-1900x1080
>> * __`3`__: cam-300x200 cam-300x200 (두 개의 카메라)
>> * __`4`__: LIDAR: 100k
>> * __`5`__: LIDAR: 500k
>> * __`6`__: LIDAR: 1M

###### `--maps`: TownName
>> __기본값__: 모든 맵

>> 계층형과 하위 계층형 모두 모든 [CARLA 맵][carla_maps]을 사용할 수 있습니다.

[carla_maps]: https://carla.readthedocs.io/en/latest/core_map/#carla-maps

###### `--weather`: INTEGER
>> __기본값__: 모든 날씨 조건

>> 날씨 조건 변경:

>> * __`0`__: ClearNoon
>> * __`1`__: CloudyNoon
>> * __`2`__: SoftRainSunset

## 벤치마크 실행 방법

1. CARLA 시작:

        # Linux:
        ./CarlaUE4.sh
        # Windows:
        CarlaUE4.exe
        # Source:
        make launch

2. 별도의 터미널에서 `PythonAPI/util`로 이동하여 `performance_benchmark.py` 스크립트를 찾습니다:

>> * 실행하지 않고 모든 가능한 시나리오 표시:
```shell
python3 performance_benchmark.py --show_scenarios
```

>> * 구성이 적용될 때 실행될 시나리오를 실제로 실행하지 않고 표시:
```shell
python3 performance_benchmark.py --sensors 2 5 --maps Town03 Town05 --weather 0 1 --show_scenarios
```

>> * 해당 시나리오에 대한 성능 벤치마크 실행:
```shell
python3 performance_benchmark.py --sensors 2 5 --maps Town03 Town05 --weather 0 1
```

>> * 비동기 모드와 렌더링 모드에서 벤치마크 수행:
```shell
python3 performance_benchmark.py --async --render_mode
```

---
## CARLA 성능 보고서

다음 표는 증가하는 차량 수와 물리 및 Traffic Manager의 활성화/비활성화의 다양한 조합으로 CARLA를 실행할 때 평균 FPS에 미치는 성능 영향을 자세히 보여줍니다.

* CARLA 버전: Dev 브랜치 2021/01/29 (커밋 198fa38c9b1317c114ac15dff130766253c02832)
* 환경 사양: Intel(R) Xeon(R) CPU E5-1620 v3 @ 3.50GHz / 32 GB / NVIDIA GeForce GTX 1080 Ti

|차량 수|물리: 꺼짐 TM: 꺼짐|물리: 켜짐 TM: 꺼짐|물리: 꺼짐 TM: 켜짐|물리: 켜짐 TM: 켜짐|
|------------|----------------|---------------|---------------|--------------|
|0           |1220            |1102           |702            |729           |
|1           |805             |579            |564            |422           |
|10          |473             |223            |119            |98            |
|50          |179             |64             |37             |26            |
|100         |92              |34             |22             |15            |
|150         |62              |21             |17             |10            |
|200         |47              |15             |14             |7             |
|250         |37              |11             |12             |6             |

---

성능 벤치마크에 대해 궁금한 점이 있다면 포럼에 게시하는 것을 주저하지 마세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="최신 CARLA 릴리스로 이동">
CARLA 포럼</a>
</p>
</div>