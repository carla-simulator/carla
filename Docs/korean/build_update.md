# CARLA 업데이트

*   [__업데이트 명령어 요약__](#update-commands-summary)
*   [__최신 바이너리 릴리스 가져오기__](#get-latest-binary-release)
*   [__Linux 및 Windows 빌드 업데이트__](#update-linux-and-windows-build)
	*   [빌드 정리](#clean-the-build)
	*   [원본에서 가져오기](#pull-from-origin)
	*   [에셋 다운로드](#download-the-assets)
	*   [서버 실행](#launch-the-server)
*   [__개발 에셋 가져오기__](#get-development-assets)

예상치 못한 문제, 의문 사항 또는 제안 사항이 있다면 CARLA 포럼에 자유롭게 로그인하여 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="최신 CARLA 릴리스로 이동">
CARLA 포럼</a>
</p>
</div>

---
## 업데이트 명령어 요약

<details>
<summary> CARLA 업데이트 명령어 보기</summary>

```sh
# CARLA 패키지 릴리스 업데이트.
#   1. 현재 버전 삭제.
#   2. 빠른 시작 설치를 따라 원하는 버전을 가져오기.


# Linux 빌드 업데이트.
git checkout master
make clean
git pull origin master
./Update.sh


# Windows 빌드 업데이트.
git checkout master
make clean
git pull origin master
#   `Unreal\CarlaUE4\Content\Carla`의 내용을 삭제.
#   `\Util\ContentVersions.txt`로 이동.
#   최신 콘텐츠 다운로드.
#   새 콘텐츠를 `Unreal\CarlaUE4\Content\Carla`에 압축 해제.


# 개발 에셋 가져오기.
#   이전 에셋이 포함된 `/Carla` 폴더 삭제.
#   메인 carla 폴더로 이동.
git clone https://bitbucket.org/carla-simulator/carla-content Unreal/CarlaUE4/Content/Carla

```
</details>

---
## 최신 바이너리 릴리스 가져오기

바이너리 릴리스는 미리 패키지화되어 있어 특정 CARLA 버전에 연결되어 있습니다. 최신 버전을 가져오려면 이전 버전을 삭제하고 [빠른 시작 설치](start_quickstart.md)를 따라 원하는 버전을 가져오세요.

릴리스는 CARLA 저장소의 __Development__에 나열되어 있습니다. 현재 CARLA의 최신 상태를 포함하는 매우 실험적인 __Nightly build__도 있습니다.

<div class="build-buttons">
[기존 버튼 그대로 유지...]
</div>

---
## Linux 및 Windows 빌드 업데이트

업데이트 전에 로컬 `master` 브랜치에 있는지 확인하세요. 그런 다음, 변경 사항을 다른 브랜치로 병합하거나 리베이스하고 가능한 충돌을 해결하세요.

```sh
git checkout master
```

### 빌드 정리

메인 CARLA 폴더로 이동하여 이전 빌드에서 생성된 바이너리와 임시 파일을 삭제하세요.
```sh
make clean
```

### 원본에서 가져오기

CARLA 저장소의 `master`에서 현재 버전을 가져옵니다.
```sh
git pull origin master
```

### 에셋 다운로드

__Linux.__
```sh
./Update.sh
```

__Windows.__

__1.__ `Unreal\CarlaUE4\Content\Carla`의 이전 콘텐츠를 삭제하세요.
__2.__ `\Util\ContentVersions.txt`로 이동하세요.
__3.__ `latest`용 콘텐츠를 다운로드하세요.
__4.__ 새 콘텐츠를 `Unreal\CarlaUE4\Content\Carla`에 압축 해제하세요.

!!! Note
    CARLA 팀이 개발 중인 내용으로 작업하려면 아래의 __개발 에셋 가져오기__로 이동하세요.

### 서버 실행

모든 것이 제대로 작동하는지 확인하기 위해 관찰자 뷰에서 서버를 실행하세요.

```sh
make launch
```

---
## 개발 에셋 가져오기

CARLA 팀은 아직 개발 중인 에셋으로 작업합니다. 이러한 모델과 맵은 CARLA 팀이 정기적으로 최신 업데이트를 푸시하는 [공개 git 저장소][contentrepolink]가 있습니다. 에셋은 아직 완성되지 않았으며, 개발자에게만 사용이 권장됩니다.

이 저장소를 다루기 위해서는 [git-lfs][gitlfslink] 설치를 권장합니다. 저장소는 정기적으로 수정되며, git-lfs는 대용량 바이너리 파일을 더 빠르게 처리합니다.

저장소를 복제하려면 __메인 CARLA 디렉토리로 이동__하고 다음 명령어를 실행하세요.

```sh
git clone https://bitbucket.org/carla-simulator/carla-content Unreal/CarlaUE4/Content/Carla
```

!!! Warning
    저장소를 복제하기 전에 에셋이 포함된 `/Carla` 폴더를 삭제하세요. 그렇지 않으면 오류가 표시됩니다.

[contentrepolink]: https://bitbucket.org/carla-simulator/carla-content
[gitlfslink]: https://github.com/git-lfs/git-lfs/wiki/Installation