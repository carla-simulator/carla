# SUMO 코시뮬레이션

CARLA는 SUMO와의 코시뮬레이션 기능을 개발했습니다. 이를 통해 작업을 원하는 대로 분산하고 각 시뮬레이션의 기능을 사용자에게 유리하게 활용할 수 있습니다.

*   [__필수 요구사항__](#필수-요구사항)  
*   [__커스텀 코시뮬레이션 실행__](#커스텀-코시뮬레이션-실행)  
    *   [CARLA vtypes 생성](#carla-vtypes-생성)  
    *   [SUMO 네트워크 생성](#sumo-네트워크-생성)  
    *   [동기화 실행](#동기화-실행)  
*   [__SUMO가 제어하는 NPC 스폰__](#sumo가-제어하는-npc-스폰)  

---
## 필수 요구사항

우선, 코시뮬레이션을 실행하려면 [__SUMO를 설치__](https://sumo.dlr.de/docs/Installing.html)해야 합니다. 코시뮬레이션을 개선하는 새로운 기능과 수정 사항이 있으므로 단순 설치보다는 소스에서 빌드하는 것을 권장합니다.

설치가 완료되면 SUMO 환경 변수를 설정합니다.  
```sh
echo "export SUMO_HOME=/usr/share/sumo" >> ~/.bashrc && source ~/.bashrc
```

이제 SUMO로 코시뮬레이션을 실행할 준비가 되었습니다. `Co-Simulation/Sumo/examples`에 __Town01__, __Town04__, __Town05__에 대한 예제가 있습니다. 이 `.sumocfg` 파일들은 시뮬레이션의 설정(예: 네트워크, 경로, 차량 유형 등)을 설명합니다. 이 중 하나를 사용하여 코시뮬레이션을 테스트하세요. 스크립트에는 [아래](#동기화-실행)에서 자세히 설명하는 다양한 옵션이 있습니다. 일단은 __Town04__에 대한 간단한 예제를 실행해 보겠습니다.

__Town04__로 CARLA 시뮬레이션을 실행합니다.  
```sh
cd ~/carla
./CarlaUE4.sh
cd PythonAPI/util
python3 config.py --map Town04
```

그런 다음, SUMO 코시뮬레이션 예제를 실행합니다.  
```sh
cd ~/carla/Co-Simulation/Sumo
python3 run_synchronization.py examples/Town04.sumocfg  --sumo-gui
```
!!! 중요
    CARLA 시뮬레이션이 실행 중이어야 합니다.

---
## 커스텀 코시뮬레이션 실행

### CARLA vtypes 생성

`Co-Simulation/Sumo/util/create_sumo_vtypes.py` 스크립트를 사용하면 CARLA 블루프린트 라이브러리를 기반으로 SUMO *vtypes*(CARLA 블루프린트와 동등)를 생성할 수 있습니다.

*   __`--carla-host`__ *(기본값: 127.0.0.1)* — CARLA 호스트 서버의 IP
*   __`--carla-port`__ *(기본값: 2000)* — 수신할 TCP 포트
*   __`--output-file`__ *(기본값: carlavtypes.rou.xml)* — *vtypes*가 포함된 생성된 파일

이 스크립트는 `data/vtypes.json`에 저장된 정보를 사용하여 SUMO *vtypes*를 생성합니다. 해당 파일을 편집하여 이를 수정할 수 있습니다.

!!! 경고
    스크립트를 실행하려면 CARLA 시뮬레이션이 실행 중이어야 합니다.

### SUMO 네트워크 생성

CARLA와 동기화되는 SUMO 네트워크를 생성하는 권장 방법은 `Co-Simulation/Sumo/util/netconvert_carla.py` 스크립트를 사용하는 것입니다. 이는 SUMO에서 제공하는 [netconvert](https://sumo.dlr.de/docs/NETCONVERT.html) 도구를 활용합니다. 스크립트를 실행하려면 몇 가지 인수가 필요합니다.

*   __`xodr_file`__ — OpenDRIVE 파일 `.xodr`
*   __`--output'`__ *(기본값:`net.net.xml`)* — 출력 파일 `.net.xml`
*   __`--guess-tls`__ *(기본값:false)* — SUMO는 특정 차선에만 신호등을 설정할 수 있지만 CARLA는 그렇지 않습니다. __True__로 설정하면 SUMO는 특정 차선에 대한 신호등을 구분하지 않으며 CARLA와 동기화됩니다.

스크립트의 출력은 __[NETEDIT](https://sumo.dlr.de/docs/NETEDIT.html)__를 사용하여 편집할 수 있는 `.net.xml`입니다. 이를 사용하여 경로를 편집하고, 수요를 추가하고, 결국 `.sumocfg`로 저장할 수 있는 시뮬레이션을 준비하세요.

이 과정에서 제공된 예제가 도움이 될 수 있습니다. `Co-Simulation/Sumo/examples`를 살펴보세요. 각 `example.sumocfg`에는 동일한 이름으로 여러 관련 파일이 있습니다. 이들은 모두 코시뮬레이션 예제를 구성합니다.

### 동기화 실행

시뮬레이션이 준비되어 `.sumocfg`로 저장되면 실행할 준비가 된 것입니다. 코시뮬레이션의 설정을 변경하기 위한 몇 가지 선택적 매개변수가 있습니다.

*   __`sumo_cfg_file`__ — SUMO 설정 파일
*   __`--carla-host`__ *(기본값: 127.0.0.1)* — CARLA 호스트 서버의 IP
*   __`--carla-port`__ *(기본값: 2000)* — 수신할 TCP 포트
*   __`--sumo-host`__ *(기본값: 127.0.0.1)* — SUMO 호스트 서버의 IP
*   __`--sumo-port`__ *(기본값: 8813)* — 수신할 TCP 포트
*   __`--sumo-gui`__ — SUMO의 GUI 버전을 시각화하는 창을 엽니다
*   __`--step-length`__ *(기본값: 0.05s)* — 시뮬레이션 타임스텝의 고정 델타 초를 설정합니다
*   __`--sync-vehicle-lights`__ *(기본값: False)* — 차량 조명을 동기화합니다
*   __`--sync-vehicle-color`__ *(기본값: False)* — 차량 색상을 동기화합니다
*   __`--sync-vehicle-all`__ *(기본값: False)* — 모든 차량 속성을 동기화합니다
*   __`--tls-manager`__ *(기본값: none)* — 어느 시뮬레이터가 신호등을 관리할지 선택합니다. 다른 시뮬레이터는 그에 따라 업데이트합니다. 옵션은 `carla`, `sumo`, `none`입니다. `none`을 선택하면 신호등이 동기화되지 않습니다. 각 차량은 자신을 스폰한 시뮬레이터의 신호등만 준수합니다.

```sh
python3 run_synchronization.py <SUMOCFG FILE> --tls-manager carla --sumo-gui
```

!!! 경고
    코시뮬레이션을 중지하려면 스크립트를 실행한 터미널에서 `Ctrl+C`를 누르세요.

---
## SUMO가 제어하는 NPC 스폰

SUMO와의 코시뮬레이션은 추가 기능을 제공합니다. 차량은 SUMO를 통해 CARLA에서 스폰될 수 있으며, Traffic Manager가 하는 것처럼 SUMO에 의해 관리됩니다.

`spawn_npc_sumo.py` 스크립트는 이미 알려진 `generate_traffic.py`와 거의 동일합니다. 이 스크립트는 CARLA의 활성 도시를 기반으로 임시 폴더에 SUMO 네트워크를 자동으로 생성합니다. 스크립트는 임의의 경로를 생성하고 차량이 주변을 돌아다니도록 합니다.

스크립트는 동기 시뮬레이션을 실행하고 그 안에 차량을 스폰하므로, 인수는 `run_synchronization.py`와 `generate_traffic.py`에 나타나는 것과 동일합니다.

*   __`--host`__ *(기본값: 127.0.0.1)* — 호스트 서버의 IP
*   __`--port`__ *(기본값: 2000)* — 수신할 TCP 포트
*   __`-n,--number-of-vehicles`__ *(기본값: 10)* — 스폰된 차량의 수
*   __`--safe`__ — 사고가 발생하기 쉬운 차량의 스폰을 피합니다
*   __`--filterv`__ *(기본값: "vehicle.*")* — 스폰된 차량의 블루프린트를 필터링합니다
*   __`--sumo-gui`__ — SUMO를 시각화하는 창을 엽니다
*   __`--step-length`__ *(기본값: 0.05s)* — 시뮬레이션 타임스텝의 고정 델타 초를 설정합니다
*   __`--sync-vehicle-lights`__ *(기본값: False)* — 차량 조명 상태를 동기화합니다
*   __`--sync-vehicle-color`__ *(기본값: False)* — 차량 색상을 동기화합니다
*   __`--sync-vehicle-all`__ *(기본값: False)* — 모든 차량 속성을 동기화합니다
*   __`--tls-manager`__ *(기본값: none)* — 어느 시뮬레이터가 신호등의 상태를 변경할지 선택합니다. 다른 시뮬레이터는 그에 따라 업데이트합니다. `none`이면 신호등이 동기화되지 않습니다

```sh
# Traffic Manager 대신 SUMO가 관리할 10대의 차량을 스폰합니다.
# CARLA가 신호등을 담당합니다.
# SUMO 시각화를 위한 창을 엽니다.
python3 spawn_sumo_npc.py -n 10 --tls-manager carla --sumo-gui
```

---

지금까지 CARLA와의 SUMO 코시뮬레이션에 대해 알아보았습니다.

CARLA를 열고 잠시 실험해보세요. 궁금한 점이 있다면 포럼에 자유롭게 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>