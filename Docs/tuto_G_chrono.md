# Chrono 통합

이 가이드는 Chrono가 무엇인지, CARLA에서 어떻게 사용하는지, 그리고 통합에 있어서의 제한 사항을 설명합니다.

- [__Project Chrono__](#project-chrono)
- [__CARLA에서 Chrono 사용하기__](#carla에서-chrono-사용하기)
    - [서버 구성](#서버-구성)
    - [Chrono 물리 활성화](#chrono-물리-활성화)
- [__제한 사항__](#제한-사항)

---

## Project Chrono

[Project Chrono](https://projectchrono.org/)는 템플릿 기반 접근 방식을 사용하여 고도로 현실적인 차량 동역학을 제공하는 오픈 소스 멀티 물리 시뮬레이션 엔진입니다. CARLA와의 통합을 통해 사용자는 맵을 탐색하면서 차량 동역학을 시뮬레이션하기 위해 Chrono 템플릿을 활용할 수 있습니다.

---

## CARLA에서 Chrono 사용하기

Chrono 통합을 사용하려면 먼저 시작 시 서버를 태그로 구성한 다음 PythonAPI를 사용하여 스폰된 차량에서 이를 활성화해야 합니다. 자세한 내용은 계속 읽어보세요.

### 서버 구성

Chrono는 CARLA 서버가 Chrono 태그로 컴파일된 경우에만 작동합니다.

__CARLA의 소스에서 빌드한 버전에서__, 서버를 시작하려면 다음 명령을 실행하세요:

```sh
make launch ARGS="--chrono"
```

---

### Chrono 물리 활성화

Chrono 물리는 [Actor](python_api.md#carlaactor) 클래스를 통해 사용할 수 있는 `enable_chrono_physics` 메서드를 사용하여 활성화됩니다. 서브스텝과 서브스텝 델타 시간 값 외에도 세 가지 템플릿 파일과 이러한 파일을 찾기 위한 기본 경로가 필요합니다:

- __`base_path`:__ 템플릿 파일이 포함된 디렉토리의 경로. 템플릿 파일에서 참조하는 보조 파일이 검색할 공통 기본 경로를 가질 수 있도록 하는 데 필요합니다.
- __`vehicle_json`:__ `base_path`를 기준으로 한 차량 템플릿 파일의 상대 경로.
- __`tire_json`:__ `base_path`를 기준으로 한 타이어 템플릿 파일의 상대 경로.
- __`powertrain_json`:__ `base_path`를 기준으로 한 파워트레인 템플릿 파일의 상대 경로.

!!! 중요
    경로를 다시 한 번 확인하세요. 잘못된 경로나 누락된 경로로 인해 언리얼 엔진이 충돌할 수 있습니다.

`Build/chrono-install/share/chrono/data/vehicle`에서 다양한 차량에 대한 예제 템플릿 파일을 사용할 수 있습니다. 차량 예제와 템플릿 만드는 방법에 대해 자세히 알아보려면 Project Chrono [문서](https://api.projectchrono.org/manual_vehicle.html)를 참조하세요.

다음은 Chrono 물리를 활성화하는 예시입니다:

```python
    # 차량 스폰
    vehicle = world.spawn_actor(bp, spawn_point)

    # 기본 경로 설정
    base_path = "/path/to/carla/Build/chrono-install/share/chrono/data/vehicle/"

    # 템플릿 파일 설정
    vehicle_json = "sedan/vehicle/Sedan_Vehicle.json"
    powertrain_json = "sedan/powertrain/Sedan_SimpleMapPowertrain.json"
    tire_json = "sedan/tire/Sedan_TMeasyTire.json"

    # Chrono 물리 활성화
    vehicle.enable_chrono_physics(5000, 0.002, vehicle_json, powertrain_json, tire_json, base_path)
```

`PythonAPI/examples`에 있는 예제 스크립트 `manual_control_chrono.py`를 사용하여 Chrono 물리 통합을 시도해볼 수 있습니다. 스크립트를 실행한 후 `Ctrl + o`를 눌러 Chrono를 활성화하세요.

---

### 제한 사항

이 통합은 충돌을 지원하지 않습니다. __충돌이 발생하면 차량은 CARLA 기본 물리로 되돌아갑니다.__