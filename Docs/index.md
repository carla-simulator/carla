# CARLA 사용 설명서

안녕하세요! CARLA 사용 설명서에 오신 것을 환영합니다.

이 메인 페이지에서는 문서의 각 섹션별 간단한 설명을 찾아보실 수 있습니다. 관심 있는 부분부터 자유롭게 살펴보시면 됩니다. 처음 CARLA를 접하시는 분들을 위해 몇 가지 추천 순서를 소개해드립니다.

* __CARLA 설치하기__ 
  - CARLA 최신 버전을 바로 사용하고 싶다면 [빠른 설치 가이드](start_quickstart.md)를 참고하세요.
  - 직접 빌드하고 싶다면 [빌드 가이드](build_carla.md)를 확인하세요.
* __CARLA 시작하기__ 
  - [기본 개념](foundations.md)에서 핵심 내용을 먼저 살펴보시고
  - [처음 시작하기](tuto_first_steps.md) 가이드를 따라 실습해보세요.
* __API 살펴보기__ 
  - 사용 가능한 모든 클래스와 메서드를 정리해둔 [Python API 문서](python_api.md)를 참고하세요.

궁금한 점이나 제안사항이 있으시다면 언제든 CARLA 포럼을 방문해주세요.
<div class="build-buttons">
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼 방문하기">
CARLA 포럼 바로가기</a>
</div>

<br>

!!! 주의사항
    __현재 사용 중인 CARLA 버전에 맞는 설명서를 확인하세요__. 
    - 화면 우측 하단의 버전 선택 패널을 통해 원하는 버전의 설명서로 이동할 수 있습니다. 
    - __'Latest' 버전은 현재 개발 중인 `dev` 브랜치의 설명서__ 입니다. 아직 정식 배포되지 않은 기능이나 업데이트된 설명이 포함되어 있을 수 있으니 참고해주세요. ![docs_version_panel](img/docs_version_panel.jpg)

---

## 시작하기

[__CARLA 소개__](start_introduction.md) — CARLA가 제공하는 기능과 특징을 소개합니다.  
[__빠른 설치 가이드__](start_quickstart.md) — 최신 CARLA를 바로 설치하고 실행해보세요.    
[__처음 시작하기__](tuto_G_getting_started.md) — CARLA를 처음 사용하는 방법을 알아봅니다.  
[__직접 빌드하기__](build_carla.md) — 소스 코드부터 CARLA를 직접 빌드하는 방법을 설명합니다.

## CARLA 구성 요소
[__핵심 개념__](core_concepts.md) — CARLA의 기본 구성 요소들을 이해합니다.    
[__액터(Actors)__](core_actors.md) — 시뮬레이션 상의 모든 객체, 액터에 대해 알아봅니다.  
[__지도__](core_map.md) — 다양한 지도와 차량 주행 방식을 살펴봅니다.  
[__센서와 데이터__](core_sensors.md) — 센서를 활용해 시뮬레이션 데이터를 수집하는 방법을 배웁니다.  
[__교통 시스템__](ts_traffic_simulation_overview.md) — 실제와 같은 교통 상황을 구현하는 다양한 방법을 소개합니다.  
[__외부 도구 연동__](3rd_party_integrations.md) — 다른 프로그램이나 라이브러리와 연동하는 방법을 설명합니다.   
[__기능 개발__](development_tutorials.md) — CARLA의 새로운 기능을 개발하는 방법을 안내합니다.  
[__커스텀 에셋 제작__](custom_assets_tutorials.md) — 직접 자산을 제작하고 추가하는 방법을 설명합니다.

## 참고 자료
[__블루프린트 라이브러리__](bp_library.md) — 액터 생성에 사용되는 모든 블루프린트를 소개합니다.   
[__Python API 문서__](python_api.md) — Python API의 모든 클래스와 메서드를 설명합니다.   
[__C++ 레퍼런스__](ref_cpp.md) — CARLA C++ 코드의 클래스와 메서드를 설명합니다.    

## CARLA 생태계

[__ROS 브릿지__](ros_documentation.md) — ROS와 연동하는 방법을 안내합니다.  
[__매스웍스 연동__](large_map_roadrunner.md) — RoadRunner로 지도를 제작하는 방법을 설명합니다.    
[__SUMO 연동__](adv_sumo.md) — SUMO와 연동하여 교통 시뮬레이션을 구현하는 방법을 안내합니다.   
[__Scenic 활용__](tuto_G_scenic.md) — Scenic을 이용해 다양한 시나리오를 생성하는 방법을 설명합니다.    
[__Chrono 물리엔진__](tuto_G_chrono.md) — Chrono 물리엔진과 연동하는 방법을 안내합니다.   
[__OpenDRIVE 지원__](adv_opendrive.md) — OpenDRIVE 포맷 지원에 대해 설명합니다.  
[__PTV-Vissim 연동__](adv_ptv.md) — PTV-Vissim과 연동하는 방법을 안내합니다.    
[__RSS 기능__](adv_rss.md) — 안전 주행을 위한 RSS 라이브러리 활용법을 설명합니다.  
[__AWS 활용__](tuto_G_rllib_integration) — AWS에서 RLlib으로 CARLA를 분산 실행하는 방법을 안내합니다.  
[__ANSYS 레이더__](ecosys_ansys.md) — ANSYS 실시간 레이더 모델 연동에 대해 설명합니다.  
[__웹 시각화 도구(carlaviz)__](plugins_carlaviz.md) — 웹 브라우저에서 시뮬레이션을 모니터링하고 데이터를 확인하는 방법을 안내합니다.

## 개발 참여하기
[__기여 가이드__](cont_contribution_guidelines.md) — CARLA 개발에 참여하는 방법을 안내합니다.   
[__코딩 규칙__](cont_coding_standard.md) — 개발 시 지켜야 할 코딩 규칙을 설명합니다.  
[__문서 작성 규칙__](cont_doc_standard.md) — 문서 작성 시 따라야 할 규칙을 안내합니다.     

## 실습 가이드

CARLA의 다양한 기능을 실제로 사용해볼 수 있는 예제 코드와 상세한 설명을 [실습 가이드 페이지](tutorials.md)에서 확인하실 수 있습니다.

## 심화 설명서

위 내용은 CARLA의 주요 개념과 기능을 다룬 것입니다. 더 자세한 고급 기능이나 심화 내용은 [심화 설명서](ext_docs.md)를 참고해주세요.