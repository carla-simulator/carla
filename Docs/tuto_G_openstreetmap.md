# OpenStreetMap으로 맵 생성하기

이 가이드에서 다음 내용을 배우게 됩니다:

- OpenStreetMaps에서 맵을 내보내는 방법
- CARLA에서 사용할 수 있는 다양한 맵 형식과 각 형식의 제한사항
- 기본 `.osm` 형식을 `.xodr` 형식으로 변환하는 방법
- `.xodr` 파일에 신호등 정보를 포함하는 방법
- 최종 맵을 CARLA 시뮬레이션에서 실행하는 방법

[OpenStreetMap](https://www.openstreetmap.org)은 수천 명의 기여자들이 개발하고 [Open Data Commons Open Database License](https://opendatacommons.org/licenses/odbl/) 하에 라이선스가 부여된 세계의 오픈 데이터 맵입니다. 맵의 일부를 XML 형식의 `.osm` 파일로 내보낼 수 있습니다. CARLA는 이 파일을 OpenDRIVE 형식으로 변환하고 [OpenDRIVE 독립 실행 모드](#adv_opendrive.md)를 사용하여 가져올 수 있습니다.

- [__OpenStreetMap으로 맵 내보내기__](#openstreetmap으로-맵-내보내기)
- [__CARLA에서 OpenStreetMaps 사용하기__](#carla에서-openstreetmaps-사용하기)
- [__OpenStreetMap 형식을 OpenDRIVE 형식으로 변환__](#openstreetmap-형식을-opendrive-형식으로-변환)
    - [Linux](#linux)
    - [Windows](#windows)
    - [신호등 생성](#신호등-생성)
- [__CARLA로 가져오기__](#carla로-가져오기)

---
## OpenStreetMap으로 맵 내보내기

이 섹션에서는 Open Street Map에서 원하는 맵 정보를 내보내는 방법을 설명합니다:

__1.__ [Open Street Map 웹사이트](https://www.openstreetmap.org)로 이동합니다. 맵 뷰와 창 오른쪽에 다양한 맵 레이어를 구성하고, 다양한 기능을 쿼리하고, 범례를 토글하는 등의 작업을 할 수 있는 패널이 보일 것입니다.

__2.__ 원하는 위치를 검색하고 특정 영역으로 확대합니다.

![openstreetmap_view](img/tuto_g_osm_web.jpg)

!!! 참고
    파리와 같은 넓은 지역의 맵을 사용하고 싶다면, CARLA의 [__대형 맵__ 기능](large_map_overview.md)을 고려해볼 수 있습니다.

__3.__ 창 왼쪽 상단의 _내보내기_를 클릭하여 _내보내기_ 패널을 엽니다.

__4.__ _내보내기_ 패널에서 _다른 영역 수동 선택_을 클릭합니다.

__5.__ 뷰포트에서 사각형 영역의 모서리를 드래그하여 사용자 지정 영역을 선택합니다.

__6.__ _내보내기_ 패널에서 _내보내기_ 버튼을 클릭하고 선택한 영역의 맵 정보를 `.osm` 파일로 저장합니다.

![openstreetmap_area](img/tuto_g_osm_area.jpg)

---
## CARLA에서 OpenStreetMaps 사용하기

Open Street Map 데이터는 CARLA에서 세 가지 다른 방법으로 사용할 수 있습니다. 사용하는 방법은 데이터가 원본 `.osm` 형식인지 또는 다음 섹션에서 설명하는 변환 방법을 사용하여 파일을 `.xodr`로 변환했는지에 따라 달라집니다. 파일을 `.osm` 형식으로 유지하는 것이 가장 제한적인 방법이며 설정 사용자 지정을 허용하지 않습니다.

__`.xodr` 형식에 사용 가능한 옵션:__

- 자체 스크립트에서 맵 생성. __이 방법은 매개변수화를 허용합니다.__
- CARLA의 `config.py`에 파일을 매개변수로 전달. __이 방법은 매개변수화를 허용하지 않습니다.__

__`.osm` 형식에 사용 가능한 옵션:__

- CARLA의 `config.py`에 파일을 매개변수로 전달. __이 방법은 매개변수화를 허용하지 않습니다.__

다음 섹션에서는 위에 나열된 옵션에 대해 더 자세히 설명합니다.

---

## OpenStreetMap 형식을 OpenDRIVE 형식으로 변환

이 섹션에서는 Python API를 사용하여 이전 섹션에서 내보낸 `.osm` 파일을 CARLA에서 사용할 수 있도록 `.xodr` 형식으로 변환하는 방법을 보여줍니다.

[carla.Osm2OdrSettings](python_api.md#carla.Osm2OdrSettings) 클래스는 오프셋 값, 신호등 생성, 원점 좌표 등과 같은 변환 설정을 구성하는 데 사용됩니다. 구성 가능한 매개변수의 전체 목록은 Python API [문서](python_api.md#carla.Osm2OdrSettings)에서 찾을 수 있습니다. [carla.Osm2Odr](python_api.md#carla.Osm2Odr) 클래스는 이러한 설정을 사용하여 `.osm` 데이터를 구문 분석하고 `.xodr` 형식으로 출력합니다.

Windows에서는 `.osm` 파일을 `UTF-8`로 인코딩해야 합니다. Linux에서는 이 작업이 필요하지 않습니다. 다음은 운영 체제에 따라 파일 변환을 수행하는 방법을 보여주는 예제 코드 스니펫입니다:

##### Linux

```py
# .osm 데이터 읽기
f = open("path/to/osm/file", 'r')
osm_data = f.read()
f.close()

# 원하는 설정을 정의. 이 경우에는 기본값 사용.
settings = carla.Osm2OdrSettings()
# OpenDRIVE로 내보낼 OSM 도로 유형 설정
settings.set_osm_way_types(["motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified", "residential"])
# .xodr로 변환
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# opendrive 파일 저장
f = open("path/to/output/file", 'w')
f.write(xodr_data)
f.close()
```

##### Windows

```py
import io

# .osm 데이터 읽기
f = io.open("test", mode="r", encoding="utf-8")
osm_data = f.read()
f.close()

# 원하는 설정을 정의. 이 경우에는 기본값 사용.
settings = carla.Osm2OdrSettings()
# OpenDRIVE로 내보낼 OSM 도로 유형 설정
settings.set_osm_way_types(["motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified", "residential"])
# .xodr로 변환
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# opendrive 파일 저장
f = open("path/to/output/file", 'w')
f.write(xodr_data)
f.close()
```
<br>

---
### 신호등 생성

Open Street Map 데이터는 어떤 교차로가 신호등으로 제어되는지 정의할 수 있습니다. CARLA에서 이 신호등 데이터를 사용하려면 `.osm` 파일을 `.xodr` 형식으로 변환하기 전에 Python API를 통해 OSM 맵 설정에서 이를 활성화해야 합니다:

```py
# 원하는 설정을 정의. 이 경우에는 기본값 사용.
settings = carla.Osm2OdrSettings()
# OSM 데이터에서 신호등 생성 활성화
settings.generate_traffic_lights = True
# .xodr로 변환
xodr_data = carla.Osm2Odr.convert(osm_data, settings)
```

신호등 데이터 품질은 데이터를 추출하는 지역에 따라 다를 수 있습니다. 일부 신호등 정보가 완전히 누락될 수 있습니다. 이러한 제한 사항 내에서 작업하기 위해 Python API를 사용하여 모든 교차로를 신호등으로 제어하도록 구성할 수 있습니다:

```py
settings.all_junctions_with_traffic_lights = True
```

또한 고속도로 연결로와 같은 특정 도로를 신호등 생성에서 제외할 수 있습니다:

```
settings.set_traffic_light_excluded_way_types(["motorway_link"])
```

---
## CARLA로 가져오기

이 섹션에서는 [OpenDRIVE 독립 실행 모드](adv_opendrive.md)를 사용하여 OpenStreetMap 정보를 CARLA로 가져오는 데 사용할 수 있는 여러 옵션들을 설명합니다.

다음 세 가지 옵션을 사용할 수 있습니다:

[__A)__](#a-자체-스크립트-사용하기) 변환된 `.xodr` 파일을 사용하여 자체 Python 스크립트에서 맵을 생성. __이 방법은 매개변수 설정이 가능합니다.__  
[__B)__](#b-xodr-파일을-configpy에-전달하기) 변환된 `.xodr` 파일을 CARLA의 `config.py` 스크립트에 매개변수로 전달. __이 방법은 매개변수 설정이 불가능합니다.__  
[__C)__](#c-osm-파일을-configpy에-전달하기) 원본 `.osm` 파일을 CARLA의 `config.py` 스크립트에 매개변수로 전달. __이 방법은 매개변수 설정이 불가능합니다.__  

###### A) 자체 스크립트 사용하기

[`client.generate_opendrive_world()`](python_api.md#carla.Client.generate_opendrive_world)를 호출하여 새 맵을 생성하고 맵이 준비될 때까지 시뮬레이션을 대기시킵니다. [carla.OpendriveGenerationParameters](python_api.md#carla.OpendriveGenerationParameters) 클래스를 사용하여 메시 생성을 구성할 수 있습니다. 예시는 다음과 같습니다:

```py
vertex_distance = 2.0  # 미터 단위
max_road_length = 500.0 # 미터 단위
wall_height = 0.0      # 미터 단위
extra_width = 0.6      # 미터 단위
world = client.generate_opendrive_world(
    xodr_xml, carla.OpendriveGenerationParameters(
        vertex_distance=vertex_distance,
        max_road_length=max_road_length,
        wall_height=wall_height,
        additional_width=extra_width,
        smooth_junctions=True,
        enable_mesh_visibility=True))
```

!!! 참고
    `wall_height = 0.0` 설정을 강력히 권장합니다. OpenStreetMap은 반대 방향의 차선들을 서로 다른 도로로 정의합니다. 벽이 생성되면 벽이 겹치고 원치 않는 충돌이 발생할 수 있습니다.

###### B) `.xodr` 파일을 `config.py`에 전달하기

CARLA 서버를 시작한 후, 별도의 터미널에서 다음 명령을 실행하여 OpenStreetMap을 로드합니다:

```sh
cd PythonAPI/util

python3 config.py -x=/path/to/xodr/file
```

[기본 매개변수](python_api.md#carla.OpendriveGenerationParameters)가 사용됩니다.

###### C) `.osm` 파일을 `config.py`에 전달하기

CARLA 서버를 시작한 후, 별도의 터미널에서 다음 명령을 실행하여 OpenStreetMap을 로드합니다:

```sh
cd PythonAPI/util

python3 config.py --osm-path=/path/to/OSM/file
```

[기본 매개변수](python_api.md#carla.OpendriveGenerationParameters)가 사용됩니다.

사용한 방법에 관계없이 맵이 CARLA로 가져와지며 결과는 아래 이미지와 유사할 것입니다:

![opendrive_meshissue](img/tuto_g_osm_carla.jpg)
<div style="text-align: right"><i>OpenStreetMap을 사용한 CARLA 맵 생성 결과.</i></div>

<br>
!!! 경고
    생성된 도로들은 맵의 경계에서 갑자기 끝납니다. 이로 인해 차량이 다음 웨이포인트를 찾을 수 없을 때 Traffic Manager가 충돌할 수 있습니다. 이를 방지하기 위해 Traffic Manager의 OSM 모드는 기본적으로 __True__로 설정되어 있습니다([`set_osm_mode()`](python_api.md#carlatrafficmanager)). 이 경우 필요할 때 경고를 표시하고 차량을 제거합니다.

---

이 주제와 관련된 모든 문제와 의문사항은 CARLA 포럼에 게시할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>