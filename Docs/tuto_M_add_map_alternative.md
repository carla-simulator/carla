# 맵 가져오기 대체 방법

이 가이드에서는 CARLA에 맵을 가져오는 대체 방법들을 설명합니다. 이 방법들은 [패키지](tuto_M_add_map_package.md)와 [소스](tuto_M_add_map_source.md) 가져오기 가이드에서 설명된 과정보다 수동 단계가 더 많이 필요합니다. 먼저 RoadRunner 플러그인 방식을 설명한 후 수동 가져오기 방식을 설명하겠습니다.

- [__RoadRunner 플러그인 가져오기__](#roadrunner-plugin-import)
- [__수동 가져오기__](#manual-import)

---

## RoadRunner 플러그인 가져오기

MathWorks의 RoadRunner 소프트웨어는 CARLA에 맵을 쉽게 가져올 수 있도록 언리얼 엔진용 플러그인을 제공합니다.

#### 플러그인 설치

__1.__ 플러그인은 [MathWorks 웹사이트](https://www.mathworks.com/help/roadrunner/ug/Downloading-Plugins.html)에서 다운로드할 수 있습니다. MathWorks는 이 튜토리얼과 유사하게 플러그인을 사용해 CARLA에 맵을 가져오는 방법에 대한 [전체 튜토리얼](https://www.mathworks.com/help/roadrunner/ug/Exporting-to-CARLA.html)도 제공합니다.

__2.__ 다운로드한 폴더의 내용을 추출하고 `RoadRunnerImporter`, `RoadRunnerCarlaIntegration`, `RoadRunnerMaterials` 폴더를 `<carla>/Unreal/CarlaUE4/Plugins/` 경로로 이동시킵니다.

__3.__ 아래 지침에 따라 플러그인을 다시 빌드합니다:

*   __Windows의 경우__
	* `<carla>/Unreal/CarlaUE4`에 있는 `.uproject` 파일을 우클릭하고 `Visual Studio 프로젝트 파일 생성`을 선택합니다.
	* CARLA 루트 폴더에서 다음 명령어를 실행합니다:

```sh
make launch
```

*   __Linux의 경우__
	* 다음 명령어를 실행합니다:
```sh
UE4_ROOT/GenerateProjectFiles.sh -project="carla/Unreal/CarlaUE4/CarlaUE4.uproject" -game -engine
```

__4.__ 언리얼 엔진 창에서 `편집 > 플러그인`에서 두 플러그인 모두 체크박스가 선택되어 있는지 확인합니다.

![rr_ue_plugins](../img/rr-ue4_plugins.jpg)

### 맵 가져오기

__1.__ `/Content/Carla/Maps` 아래의 새 폴더에 `<맵이름>.fbx` 파일을 `가져오기` 버튼을 사용하여 가져옵니다.

![ue_import](../img/ue_import_mapname.jpg)

__2.__ `Scene > Hierarchy Type`을 _Create One Blueprint Asset_(기본 선택)으로 설정합니다.
__3.__ `Static Meshes > Normal Import Method`를 _Import Normals_로 설정합니다.

![ue_import_options](../img/ue_import_options.jpg)

__4.__ `가져오기`를 클릭합니다.
__5.__ 현재 레벨을 `파일` -> `다른 이름으로 저장...` -> `<맵이름>`으로 저장합니다.

이제 새 맵이 언리얼 엔진 _콘텐츠 브라우저_의 다른 맵들과 함께 표시되어야 합니다.

![ue_level_content](../img/ue_level_content.jpg)
</details>

!!! 참고
    시맨틱 세그멘테이션을 위한 태그는 에셋 이름에 따라 할당됩니다. 에셋은 `Content/Carla/PackageName/Static`의 해당 폴더로 이동됩니다. 이를 변경하려면 가져오기 후 수동으로 이동하세요.

---

## 수동 가져오기

이 맵 가져오기 방법은 일반적인 `.fbx`와 `.xodr` 파일에 사용할 수 있습니다. RoadRunner를 사용하는 경우, `Firebox (.fbx)`, `OpenDRIVE (.xodr)` 또는 `Unreal (.fbx + .xml)` 내보내기 방식을 사용해야 합니다. `.fbx` 파일과의 호환성 문제가 발생할 수 있으므로 `Carla Exporter` 옵션은 사용하지 마세요.

언리얼 엔진에 맵을 수동으로 가져오려면:

__1.__ 시스템의 파일 탐색기에서 `.xodr` 파일을 `<carla-root>/Unreal/CarlaUE4/Content/Carla/Maps/OpenDrive`로 복사합니다.

__2.__ carla 루트 디렉토리에서 `make launch`를 실행하여 언리얼 엔진 에디터를 엽니다. 에디터의 _콘텐츠 브라우저_에서 `Content/Carla/Maps/BaseMap`으로 이동하여 `BaseMap`을 복제합니다. 이렇게 하면 기본 하늘과 조명 객체가 포함된 빈 맵이 제공됩니다.

>>![ue_duplicate_basemap](../img/ue_duplicate_basemap.png)

__3.__ `Content/Carla/Maps` 디렉토리에 맵 패키지의 이름으로 새 폴더를 만들고 복제된 맵을 `.fbx`와 `.xodr` 파일과 동일한 이름으로 저장합니다.

__4.__ 언리얼 엔진 에디터의 _콘텐츠 브라우저_에서 `Content/Carla/Maps`로 돌아갑니다. 회색 영역을 우클릭하고 _Import Asset_ 항목 아래의 `Import to /Game/Carla/Maps...`를 선택합니다.

>>![ue_import_asset](../img/ue_import_asset.png)

__5.__ 표시되는 구성 창에서 다음을 확인하세요:

>- 다음 옵션들은 체크 해제:
    *   Auto Generate Collision
    *   Combine Meshes
    *   Force Front xAxis
- 다음 드롭다운에서 해당 옵션 선택:
    *   Normal Import Method - _Import Normals_
    *   Material Import Method - _Create New Materials_
- 다음 옵션들은 체크:
    *   Convert Scene Unit
    *   Import Textures

>>![ue_import_file](../img/ue_import_file.jpg)

__6.__ `가져오기`를 클릭합니다.

__7.__ 메시들이 _콘텐츠 브라우저_에 나타납니다. 메시들을 선택하여 장면으로 드래그합니다.

>>![ue_meshes](../img/ue_drag_meshes.jpg)

__8.__ 메시들을 0,0,0 위치로 중앙에 배치합니다.

>>![Transform_Map](../img/transform.jpg)

__9.__ _콘텐츠 브라우저_에서 콜라이더가 필요한 모든 메시를 선택합니다. 이는 보행자나 차량과 상호작용할 모든 메시를 의미합니다. 콜라이더는 이들이 맵 밖으로 떨어지는 것을 방지합니다. 선택한 메시들을 우클릭하고 `Asset Actions > Bulk Edit via Property Matrix...`를 선택합니다.

>>![ue_selectmesh_collision](../img/ue_selectmesh_collision.jpg)

__10.__ 검색창에서 _collision_을 검색합니다.

__11.__ `Collision Complexity`를 `Project Default`에서 `Use Complex Collision As Simple`로 변경하고 창을 닫습니다.

>>![ue_collision_complexity](../img/ue_collision_complexity.jpg)

__12.__ `Alt + c`를 눌러 충돌 설정이 올바르게 적용되었는지 확인합니다. 메시 위에 검은색 망이 보일 것입니다.

__13.__ 시맨틱 세그멘테이션 센서의 기준 진실을 생성하기 위해 정적 메시를 아래 구조에 따라 해당하는 `Carla/Static/<segment>` 폴더로 이동합니다:

        Content
        └── Carla
            ├── Blueprints
            ├── Config
            ├── Exported Maps
            ├── HDMaps
            ├── Maps
            └── Static
                ├── Terrain
                │   └── mapname
                │       └── Static Meshes
                │
                ├── Road
                │   └── mapname
                │       └── Static Meshes
                │
                ├── RoadLines  
                |   └── mapname
                |       └── Static Meshes
                └── Sidewalks  
                    └── mapname
                        └── Static Meshes

__14.__ _Modes_ 패널에서 __Open Drive Actor__를 검색하여 장면으로 드래그합니다.

>>![ue_opendrive_actor](../img/ue_opendrive_actor.jpg)

__15.__ _Details_ 패널에서 `Add Spawners`를 체크하고 `Generate Routes` 옆의 상자를 클릭합니다. 이렇게 하면 `<carla-root>/Unreal/CarlaUE4/Content/Carla/Maps/OpenDrive` 디렉토리에서 동일한 맵 이름을 가진 `.xodr` 파일을 찾아 일련의 _RoutePlanner_와 _VehicleSpawnPoint_ 액터들을 생성합니다.

>>![ue_generate_routes](../img/ue_generate_routes.png)

---

## 다음 단계

이제 언리얼 에디터에서 맵을 열고 시뮬레이션을 실행할 수 있습니다. 여기서 맵을 커스터마이즈하고 보행자 내비게이션 데이터를 생성할 수 있습니다. 보행자 경로가 장애물에 의해 차단될 가능성을 없애기 위해 모든 커스터마이즈가 완료된 후에 보행자 내비게이션을 생성하는 것을 권장합니다.

CARLA는 맵 커스터마이즈를 위한 여러 도구와 가이드를 제공합니다:

- [맵에 서브레벨 구현하기](tuto_M_custom_layers.md)
- [신호등과 표지판 추가 및 설정하기](tuto_M_custom_add_tl.md)
- [절차적 건물 도구로 건물 추가하기](tuto_M_custom_buildings.md)
- [도로 페인터 도구로 도로 커스터마이즈하기](tuto_M_custom_road_painter.md)
- [날씨 커스터마이즈하기](tuto_M_custom_weather_landscape.md#weather-customization)
- [연속 메시로 지형 커스터마이즈하기](tuto_M_custom_weather_landscape.md#add-serial-meshes)

커스터마이즈가 완료되면 [보행자 내비게이션 정보를 생성](tuto_M_generate_pedestrian_navigation.md)할 수 있습니다.

---

[CARLA 패키지](tuto_M_add_map_package.md)와 [CARLA 소스 빌드](tuto_M_add_map_source.md) 가이드에 설명된 자동화된 맵 가져오기 프로세스를 사용하는 것이 권장되지만, 필요한 경우 이 섹션에 나열된 방법들을 사용할 수 있습니다. 대체 방법들을 사용하면서 문제가 발생하면 [포럼](https://github.com/carla-simulator/carla/discussions)에 자유롭게 게시하세요.