# 개발

CARLA는 오픈 소스이며 높은 확장성을 갖도록 설계되었습니다. 이를 통해 사용자는 전문적인 응용 프로그램이나 특정 요구 사항에 맞는 맞춤형 기능이나 콘텐츠를 만들 수 있습니다. 다음 튜토리얼에서는 CARLA 코드베이스를 사용하여 특정 개발 목표를 달성하는 방법을 자세히 설명합니다:

- [__릴리스 만들기__](tuto_D_make_release.md)
- [__콘텐츠 업그레이드__](tuto_D_contribute_assets.md)
- [__시맨틱 태그 생성__](tuto_D_create_semantic_tags.md)
- [__새로운 센서 만들기__](tuto_D_create_sensor.md)
- [__성능 벤치마킹__](adv_benchmarking.md)
- [__레코더 파일 형식__](ref_recorder_binary_file_format.md)
- [__충돌 경계__](tuto_D_generate_colliders.md)

## 릴리스 만들기

CARLA의 자체 포크를 개발하고 코드의 릴리스를 게시하고 싶다면 [__이 가이드__](tuto_D_make_release.md)를 따르세요.

## 콘텐츠 업그레이드

우리의 콘텐츠는 별도의 Git LFS 저장소에 있습니다. 빌드 시스템의 일부로서, 현재 날짜와 커밋으로 태그된 최신 버전의 콘텐츠가 포함된 패키지를 생성하고 업로드합니다. 정기적으로 CARLA 저장소를 최신 콘텐츠 패키지 링크로 업그레이드합니다. 콘텐츠를 업그레이드하려면 [__이 지침__](tuto_D_contribute_assets.md)을 따르세요.

## 시맨틱 태그 생성

CARLA는 대부분의 사용 사례에 적합한 시맨틱 태그 세트가 이미 정의되어 있습니다. 하지만 추가 클래스가 필요한 경우 [__이 가이드__](tuto_D_create_semantic_tags.md)에 설명된 대로 추가할 수 있습니다.

## 새로운 센서 만들기

맞춤형 사용 사례를 위해 CARLA의 C++ 코드를 수정하여 새로운 센서를 만들 수 있습니다. 자세한 내용은 [__여기__](tuto_D_create_sensor.md)에서 확인하세요.

## 성능 벤치마킹

CARLA에는 시스템의 성능 벤치마킹을 돕는 벤치마킹 스크립트가 있습니다. 자세한 내용은 [__여기__](adv_benchmarking.md)에서 확인하세요.

## 레코더 바이너리 파일 형식

레코더의 바이너리 파일 형식에 대한 자세한 내용은 [__여기__](ref_recorder_binary_file_format.md)에서 확인할 수 있습니다.

## 충돌 경계 생성

차량의 더 정확한 충돌 경계를 생성하는 방법에 대한 자세한 내용은 [__이 가이드__](tuto_D_generate_colliders.md)에서 확인할 수 있습니다.