# 튜토리얼

여기서는 CARLA의 다양한 기능을 사용하는 방법을 이해하는 데 도움이 되는 다양한 튜토리얼을 찾을 수 있습니다.

## 일반

### CARLA 기능

* [__시뮬레이션 데이터 검색__](tuto_G_retrieve_data.md) — 레코더를 사용하여 데이터를 제대로 수집하는 단계별 가이드
* [__교통 관리자__](tuto_G_traffic_manager.md) — 교통 관리자를 사용하여 도시 주변의 교통을 안내하는 방법
* [__텍스처 스트리밍__](tuto_G_texture_streaming.md) — 다양성을 추가하기 위해 실시간으로 맵 객체의 텍스처 수정
* [__인스턴스 분할 카메라__](tuto_G_instance_segmentation_sensor.md) — 인스턴스 분할 카메라를 사용하여 동일한 클래스의 객체 구분
* [__경계 상자__](tuto_G_bounding_boxes.md) — CARLA 객체의 경계 상자를 카메라에 투영
* [__보행자 뼈대__](tuto_G_pedestrian_bones.md) — 보행자 골격을 카메라 평면에 투영
* [__보행자 골격 제어__](tuto_G_control_walker_skeletons.md) — 골격을 사용하여 보행자 애니메이션

### 빌드 및 통합

* [__Docker에서 언리얼 엔진과 CARLA 빌드__](build_docker_unreal.md) — Docker에서 언리얼 엔진과 CARLA 빌드
* [__CarSim 통합__](tuto_G_carsim_integration.md) — CarSim 차량 동역학 엔진을 사용하여 시뮬레이션을 실행하는 방법 튜토리얼
* [__RLlib 통합__](tuto_G_rllib_integration.md) — RLlib 라이브러리를 사용하여 자체 실험을 실행하는 방법 알아보기
* [__Chrono 통합__](tuto_G_chrono.md) — 물리 시뮬레이션을 위한 Chrono 통합 사용
* [__PyGame 제어__](tuto_G_pygame.md) — PyGame을 사용하여 카메라 센서의 출력 표시

## 에셋과 맵

* [__OpenStreetMap으로 맵 생성__](tuto_G_openstreetmap.md) — OpenStreetMap을 사용하여 시뮬레이션에서 사용할 맵 생성
* [__새로운 차량 추가__](tuto_A_add_vehicle.md) — CARLA에서 사용할 차량 준비
* [__새로운 소품 추가__](tuto_A_add_props.md) — CARLA에 추가 소품 가져오기
* [__독립 실행형 패키지 만들기__](tuto_A_create_standalone.md) — 에셋을 위한 독립 실행형 패키지 생성 및 처리
* [__재질 커스터마이제이션__](tuto_A_material_customization.md) — 차량 및 건물 재질 편집

## 개발자

* [__콘텐츠 업그레이드 방법__](tuto_D_contribute_assets.md) — CARLA에 새로운 콘텐츠 추가
* [__센서 만들기__](tuto_D_create_sensor.md) — CARLA에서 사용할 새로운 센서 개발
* [__시맨틱 태그 만들기__](tuto_D_create_semantic_tags.md) — 시맨틱 분할을 위한 커스텀 태그 정의
* [__차량 서스펜션 커스터마이즈__](tuto_D_customize_vehicle_suspension.md) — 차량의 서스펜션 시스템 수정
* [__상세 충돌체 생성__](tuto_D_generate_colliders.md) — 차량을 위한 상세 충돌체 생성
* [__릴리스 만들기__](tuto_D_make_release.md) — CARLA 릴리스를 만드는 방법

## 비디오 튜토리얼

* [__기초__](https://www.youtube.com/watch?v=pONr1R1dy88) — CARLA의 기본 개념을 배우고 첫 번째 스크립트 시작하기 [__코드__](https://carla-releases.s3.us-east-005.backblazeb2.com/Docs/Fundamentals.ipynb)
* [__CARLA의 센서에 대한 심층 분석__](https://www.youtube.com/watch?v=om8klsBj4rc) — CARLA의 센서와 이를 사용하는 방법에 대한 심층 분석 [__코드__](https://carla-releases.s3.us-east-005.backblazeb2.com/Docs/Sensors_code.zip)