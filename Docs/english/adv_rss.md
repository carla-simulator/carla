# RSS

CARLA는 클라이언트 라이브러리에 [책임 민감형 안전을 위한 C++ 라이브러리](https://github.com/intel/ad-rss-lib)를 통합합니다. 이 기능을 통해 사용자는 아무것도 구현하지 않고도 RSS의 동작을 조사할 수 있습니다. CARLA는 입력을 제공하고 자율주행 시스템에 실시간으로 출력을 적용하는 작업을 처리합니다.

*   [__개요__](#개요)
*   [__컴파일__](#컴파일)
    *   [의존성](#의존성)
    *   [빌드](#빌드)
*   [__현재 상태__](#현재-상태)
    *   [RssSensor](#rsssensor)
    *   [RssRestrictor](#rssrestrictor)

!!! 중요
    이 기능은 현재 개발 중입니다. 현재는 Linux 빌드에서만 사용할 수 있습니다.

---
## 개요

RSS 라이브러리는 안전 보장을 위한 수학적 모델을 구현합니다. 센서 정보를 받아 차량 컨트롤러에 제한을 제공합니다. 요약하면, RSS 모듈은 센서 데이터를 사용하여 __상황__을 정의합니다. 상황은 자아 차량과 환경 요소의 상태를 설명합니다. 각 상황에 대해 안전 검사가 이루어지고 적절한 응답이 계산됩니다. 전체 응답은 모든 응답의 조합 결과입니다. 라이브러리에 대한 자세한 정보는 [문서](https://intel.github.io/ad-rss-lib/)를 읽어보세요. 특히 [배경 섹션](https://intel.github.io/ad-rss-lib/ad_rss/Overview/)을 참조하세요.

이는 CARLA에서 두 가지 요소를 사용하여 구현됩니다.

*   __RssSensor__는 *ad-rss-lib*를 사용하여 상황 분석과 응답 생성을 담당합니다.
*   __RssRestrictor__는 차량의 명령을 제한하여 응답을 적용합니다.

다음 이미지는 CARLA 아키텍처에 __RSS__를 통합하는 과정을 보여줍니다.

![RSS를 CARLA에 통합](img/rss_carla_integration_architecture.png)

__1. 서버__

- 클라이언트에 카메라 이미지를 보냅니다. <small>(클라이언트가 시각화를 필요로 하는 경우에만)</small>
- RssSensor에 월드 데이터를 제공합니다.
- RssRestrictor에 차량의 물리 모델을 보냅니다. <small>(기본값이 재정의된 경우에만)</small>

__2. 클라이언트__

- *RssSensor*에 고려해야 할 [매개변수](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/)를 제공합니다.
- *RssResrictor*에 초기 [carla.VehicleControl](python_api.md#carla.VehicleControl)을 보냅니다.

__3. RssSensor__

- *ad-rss-lib*를 사용하여 상황을 추출하고, 안전 검사를 수행하며, 응답을 생성합니다.
- *RssRestrictor*에 적절한 응답과 적용할 가속도 제한이 포함된 응답을 보냅니다.

__4. RssRestrictor__

- 클라이언트가 요청하면 [carla.VehicleControl](python_api.md#carla.VehicleControl)에 응답을 적용하고 결과를 반환합니다.

[![CARLA의 RSS 센서](img/rss_carla_integration.png)](https://www.youtube.com/watch?v=UxKPXPT2T8Q)
<div style="text-align: right"><i>RssSensor 결과의 시각화.</i></div>

---
## 컴파일

RSS 통합은 CARLA의 나머지 부분과 별도로 빌드해야 합니다. __ad-rss-lib__는 충돌을 유발하는 LGPL-2.1 오픈소스 라이센스와 함께 제공됩니다. *libCarla*에 정적으로 연결되어야 합니다.

현재까지 이 기능은 Linux 빌드에서만 사용할 수 있다는 점을 다시 한 번 알려드립니다.

### 의존성

RSS와 그 의존성을 빌드하기 위해 추가적인 전제 조건이 필요합니다. 이에 대해 자세히 알아보려면 [공식 문서](https://intel.github.io/ad-rss-lib/BUILDING)를 참조하세요.

Ubuntu(>= 16.04)에서 제공하는 의존성.
```sh
sudo apt-get install libgtest-dev libpython-dev libpugixml-dev libtbb-dev
```

의존성은 [colcon](https://colcon.readthedocs.io/en/released/user/installation.html)을 사용하여 빌드되므로 설치해야 합니다.
```sh
pip3 install --user -U colcon-common-extensions
```

Python 바인딩을 위한 추가 의존성이 있습니다.
```sh
sudo apt-get install castxml
pip3 install --user pygccxml pyplusplus
```

### 빌드

이 작업이 완료되면 전체 의존성 세트와 RSS 구성 요소를 빌드할 수 있습니다.

*   RSS와 함께 작동하도록 LibCarla를 컴파일합니다.

```sh
make LibCarla.client.rss
```

*   RSS 기능을 포함하도록 PythonAPI를 컴파일합니다.

```sh
make PythonAPI.rss
```

*   대안으로, 패키지를 직접 빌드할 수 있습니다.
```sh
make package.rss
```

---
## 현재 상태

### RssSensor

[__carla.RssSensor__](python_api.md#carla.RssSensor)는 교차로, [stay on road](https://intel.github.io/ad-rss-lib/ad_rss_map_integration/HandleRoadBoundaries/) 지원 및 [비구조화된 배치(예: 보행자)](https://intel.github.io/ad-rss-lib/ad_rss/UnstructuredConstellations/)를 포함하여 [ad-rss-lib v4.2.0 기능 세트](https://intel.github.io/ad-rss-lib/RELEASE_NOTES_AND_DISCLAIMERS)를 완전히 지원합니다.

지금까지 서버는 다른 교통 참여자와 신호등의 상태를 포함하는 주변 환경의 실측 데이터를 센서에 제공합니다.

### RssRestrictor

클라이언트가 요청하면 [__carla.RssRestrictor__](python_api.md#carla.RssRestrictor)는 주어진 응답에 의해 원하는 가속도나 감속도에 가장 잘 도달하도록 차량 컨트롤러를 수정합니다.

[carla.VehicleControl](python_api.md#carla.VehicleControl) 객체의 구조로 인해 적용되는 제한에는 특정 한계가 있습니다. 이러한 컨트롤러는 `throttle`, `brake`, `steering` 값을 포함합니다. 하지만 차량 물리와 단순한 제어 옵션으로 인해 이러한 값이 충족되지 않을 수 있습니다. 제한은 평행한 차선 방향으로 반대 조향을 하여 측면 방향으로만 개입합니다. RSS가 요청한 감속이 필요한 경우 브레이크가 작동됩니다. 이는 [carla.Vehicle](python_api.md#carla.Vehicle)이 제공하는 차량 질량과 제동 토크에 따라 달라집니다.

!!! 참고
    자동화된 차량 컨트롤러에서는 제한에 맞춰 계획된 궤적을 조정할 수 있을 것입니다. 빠른 제어 루프(>1KHz)를 사용하여 이러한 제한이 준수되도록 할 수 있습니다.

---

지금까지 CARLA의 RSS 센서에 대한 기본 사항을 다루었습니다. 구체적인 속성과 매개변수에 대해서는 [센서 레퍼런스](ref_sensors.md#rss-sensor)에서 자세히 알아보세요.

CARLA를 열고 잠시 실험해보세요. 궁금한 점이 있다면 포럼에 자유롭게 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>