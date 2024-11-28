# 보행자 내비게이션 생성

보행자가 맵을 탐색할 수 있게 하려면 보행자 내비게이션 파일을 생성해야 합니다. 이 가이드는 사용할 메시와 파일을 생성하는 방법을 자세히 설명합니다.

- [__시작하기 전에__](#before-you-begin)
- [__보행자 이동 가능 메시__](#pedestrian-navigable-meshes)
- [__선택적 보행자 내비게이션 옵션__](#optional-pedestrian-navigation-options)
- [__보행자 내비게이션 생성__](#generate-the-pedestrian-navigation)

---

## 시작하기 전에

맵 커스터마이즈(건물 추가, 도로 페인팅, 지형 요소 추가 등)는 두 요소 간의 간섭이나 충돌을 피하고 보행자 내비게이션을 두 번째로 생성해야 하는 상황을 방지하기 위해 보행자 내비게이션을 생성하기 전에 완료해야 합니다.

---

## 보행자 이동 가능 메시

보행자는 특정 메시만 탐색할 수 있습니다. 보행자 내비게이션에 포함하고 싶은 메시의 이름을 아래 표의 명명 규칙에 따라 지정해야 합니다:

| 유형 | 이름 포함 | 설명 |
|------|------------|-------------|
| 지면 | `Road_Sidewalk` 또는 `Roads_Sidewalk` | 보행자가 이 메시를 자유롭게 걸을 수 있습니다. |
| 횡단보도 | `Road_Crosswalk` 또는 `Roads_Crosswalk` | 지면이 없을 경우 보행자가 이 메시를 두 번째 선택지로 걸을 수 있습니다. |
| 잔디 | `Road_Grass` 또는 `Roads_Grass` | 지정된 비율의 보행자만 이 메시를 걸을 수 있습니다. |
| 도로 | `Road_Road` 또는 `Roads_Road` <br> `Road_Curb` 또는 `Roads_Curb` <br> `Road_Gutter` 또는 `Roads_Gutter` <br> `Road_Marking` 또는 `Roads_Marking` | 보행자는 이 메시를 통해서만 도로를 건널 수 있습니다. |

<br>

---

## 선택적 보행자 내비게이션 옵션

다음 단계는 보행자 내비게이션을 생성하는 데 필수적이지는 않지만, 어느 정도 보행자 활동을 커스터마이즈할 수 있게 해줍니다.

- __새로운 횡단보도 생성하기__

횡단보도가 이미 `.xodr` 파일에 정의되어 있다면 중복을 피하기 위해 이 작업을 하지 마세요:

1. 연결하고 싶은 두 보도 위로 약간 확장되는 평면 메시를 만듭니다.
2. 메시를 지면과 겹치게 배치하고 물리와 렌더링을 비활성화합니다.
3. 메시의 이름을 `Road_Crosswalk` 또는 `Roads_Crosswalk`로 변경합니다.

![ue_crosswalks](../img/ue_crosswalks.jpg)

---
## 보행자 내비게이션 생성하기

__1.__ 맵이 너무 커서 내보내기가 불가능한 것을 방지하기 위해 __BP_Sky 객체__를 선택하고 `NoExport` 태그를 추가합니다. 보행자 내비게이션과 관련이 없는 다른 특별히 큰 메시가 있다면, 그것들에도 `NoExport` 태그를 추가하세요.

![ue_skybox_no_export](../img/ue_noexport.png)

__2.__ 메시 이름을 다시 한 번 확인하세요. 보행자가 걸을 수 있는 영역으로 인식되려면 메시 이름이 아래 나열된 적절한 형식 중 하나로 시작해야 합니다. 기본적으로 보행자는 보도, 횡단보도, 잔디(나머지에 대해서는 적은 영향)를 걸을 수 있습니다:

*   보도 = `Road_Sidewalk` 또는 `Roads_Sidewalk`
*   횡단보도 = `Road_Crosswalk` 또는 `Roads_Crosswalk`
*   잔디 = `Road_Grass` 또는 `Roads_Grass`

![ue_meshes](../img/ue_meshes.jpg)

__3.__ `ctrl + A`를 눌러 모든 것을 선택하고 `File` -> `Carla Exporter`를 선택하여 맵을 내보냅니다. `Unreal/CarlaUE4/Saved`에 `<맵이름>.obj` 파일이 생성됩니다.

__4.__ `<맵이름>.obj`와 `<맵이름>.xodr`을 `Util/DockerUtils/dist`로 이동합니다.

__5.__ 다음 명령어를 실행하여 내비게이션 파일을 생성합니다:

*   __Windows__
```sh
build.bat <맵이름> # <맵이름>에는 확장자를 포함하지 않습니다
```
*   __Linux__
```sh
./build.sh <맵이름> # <맵이름>에는 확장자를 포함하지 않습니다
```

__6.__ `<맵이름>.bin` 파일이 생성됩니다. 이 파일은 맵의 보행자 내비게이션 정보를 포함합니다. 이 파일을 맵이 포함된 패키지의 `Nav` 폴더로 이동하세요.

__7.__ 시뮬레이션을 시작하고 `PythonAPI/examples`의 예제 스크립트 `generate_traffic.py`를 실행하여 보행자 내비게이션을 테스트하세요.

!!! 참고
    **맵을 업데이트한 후 보행자 내비게이션을 다시 빌드해야 하는 경우**, CARLA 캐시를 삭제해야 합니다. 이는 보통 Ubuntu의 홈 디렉토리(`cd ~`)나 Windows의 사용자 디렉토리(환경 변수 `USERPROFILE`에 할당된 디렉토리)에 있습니다. `carlaCache`라는 이름의 폴더와 그 내용을 모두 제거하세요. 크기가 클 수 있습니다.

---

과정에 대해 궁금한 점이 있다면 [포럼](https://github.com/carla-simulator/carla/discussions)에서 질문할 수 있습니다.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>