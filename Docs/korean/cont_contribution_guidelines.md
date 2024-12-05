# CARLA에 기여하기

CARLA 팀은 협력하고자 하는 모든 분들의 기여를 기쁘게 받아들입니다. 기여자의 능력에 따라 프로젝트에 기여할 수 있는 다양한 방법이 있습니다. 팀은 기여가 CARLA에 성공적으로 통합될 수 있도록 최선을 다할 것입니다.

살펴보시고 주저하지 마세요!

*   [__버그 보고__](#버그-보고)  
*   [__기능 요청__](#기능-요청)  
*   [__코드 기여__](#코드-기여)  
    *   [언리얼 엔진 학습](#언리얼-엔진-학습)  
    *   [시작하기 전에](#시작하기-전에)  
    *   [코딩 표준](#코딩-표준)  
    *   [제출](#제출)  
    *   [체크리스트](#체크리스트)  
*   [__아트 기여__](#아트-기여)  
*   [__문서 기여__](#문서-기여)  

---
## 버그 보고

GitHub의 [이슈 섹션][issueslink]에서 문제를 보고할 수 있습니다. 새로운 버그를 보고하기 전에 몇 가지 확인 사항을 점검하세요.

__1. 버그가 이미 보고되었는지 확인하세요.__ GitHub의 동일한 이슈 섹션에서 검색해보세요.

__2. 문서를 읽으세요.__ 해당 이슈가 CARLA가 작동하는 방식에 대한 오해가 아닌 버그인지 확인하세요. [문서][docslink]에서 이슈와 관련된 페이지를 읽고 [FAQ][faqlink] 페이지를 살펴보세요.

[issueslink]: https://github.com/carla-simulator/carla/issues
[docslink]: http://carla.readthedocs.io
[faqlink]: build_faq.md

---
## 기능 요청

새로운 기능에 대한 아이디어도 기여하는 좋은 방법입니다. 사용자 경험을 개선할 수 있는 모든 제안은 [여기][frlink]에 있는 GitHub 섹션에서 제출할 수 있습니다.

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22+sort%3Acomments-desc

---
## 코드 기여

코딩을 시작하기 전에 중복을 피하기 위해 [이슈 보드][issueboard]를 확인하여 팀이 이미 작업 중인 내용을 확인하세요. 의문이 있거나 진행 방법에 대해 논의하고 싶다면 팀원에게 연락하거나 이메일(<carla.simulator@gmail.com>)을 보내주세요.

작업을 시작하려면 [CARLA 저장소를 포크](https://docs.github.com/en/enterprise/2.13/user/articles/fork-a-repo)하고 해당 포크를 컴퓨터에 복제하세요. [포크를 원본 저장소와 동기화](https://docs.github.com/en/enterprise/2.13/user/articles/syncing-a-fork)하는 것을 잊지 마세요.

[issueboard]: https://github.com/carla-simulator/carla/issues

### 언리얼 엔진 학습

UE4를 사용한 C++ 프로그래밍에 대한 기본적인 소개는 언리얼의 [C++ 프로그래밍 튜토리얼][ue4tutorials]에서 찾을 수 있습니다. 온라인에는 다른 옵션들도 있으며, 일부는 유료입니다. [Udemy의 언리얼 C++ 과정][ue4course]은 매우 완성도가 높으며 일반적으로 할인이 있어 매우 저렴하게 이용할 수 있습니다.

[ue4tutorials]: https://docs.unrealengine.com/latest/INT/Programming/Tutorials/
[ue4course]: https://www.udemy.com/unrealcourse/

### 시작하기 전에

CARLA를 구성하는 다양한 모듈에 대한 아이디어를 얻으려면 [CARLA 설계](index.md)<!-- @todo --> 문서를 확인하세요. 새로운 기능을 담을 가장 적절한 모듈을 선택하세요. 프로세스 중에 의문이 생기면 [Discord 서버](https://discord.com/invite/8kqACuC)에서 팀에게 자유롭게 연락하세요.

### 코딩 표준

새로운 코드를 제출할 때는 현재의 [코딩 표준](cont_coding_standard.md)을 따르세요.

### 제출

기여와 새로운 기능은 `master` 브랜치가 아닌 `dev`라는 중간 브랜치에 병합됩니다. 이 [Gitflow](https://nvie.com/posts/a-successful-git-branching-model/) 브랜칭 모델을 통해 안정적인 master 브랜치를 유지하기가 더 쉬워집니다. 이 모델은 기여를 위한 특정 워크플로우가 필요합니다.

*   `dev` 브랜치를 항상 최신 변경사항으로 업데이트하세요.
*   `dev`에서 `username/name_of_the_contribution`이라는 이름의 자식 브랜치에서 기여를 개발하세요.
*   기여가 준비되면 브랜치에서 `dev`로 풀 리퀘스트를 제출하세요. 설명을 작성할 때 최대한 자세히 작성하세요. 새로운 코드가 병합 전에 통과해야 하는 몇 가지 검사가 있습니다. 검사는 지속적 통합 시스템에 의해 자동으로 실행됩니다. 검사가 성공하면 녹색 체크 표시가 나타납니다. 빨간 표시가 나타나면 그에 맞게 코드를 수정하세요.

기여가 `dev`에 병합되면 다른 새로운 기능들과 함께 테스트될 수 있습니다. 다음 릴리스 시점에 `dev` 브랜치가 `master`로 병합되어 기여가 사용 가능하게 되고 공지됩니다.

### 체크리스트

*   [ ] 브랜치가 `dev` 브랜치와 동기화되어 있고 최신 변경사항으로 테스트되었습니다.
*   [ ] 필요한 경우 README/문서를 확장했습니다.
*   [ ] 코드가 올바르게 컴파일됩니다.
*   [ ] `make check`로 모든 테스트가 통과됩니다.

---
## 아트 기여

아트 기여에는 CARLA에서 사용할 차량, 보행자, 맵 또는 기타 유형의 에셋이 포함됩니다. 이들은 계정 공간 제한이 있는 BitBucket 저장소에 저장됩니다. 이러한 이유로, 기여자는 CARLA 팀과 연락하여 기여를 위한 콘텐츠 저장소에 브랜치를 만들어달라고 요청해야 합니다.

__1. BitBucket 계정 생성__ [BitBucket 페이지](https://bitbucket.org)를 방문하세요.

__2. 콘텐츠 저장소 접근을 위해 아트 팀에 연락__ [Discord 서버](https://discord.com/invite/8kqACuC)에 참여하세요. __Contributors__ 채널로 가서 콘텐츠 저장소 접근을 요청하세요.

__3. 각 기여자를 위한 브랜치가 생성됩니다.__ 브랜치는 `contributors/contributor_name`으로 명명됩니다. 해당 사용자의 모든 기여는 해당 브랜치에서 이루어져야 합니다.

__4. CARLA 빌드__ 기여하려면 CARLA 빌드가 필요합니다. [Linux](https://carla.readthedocs.io/en/latest/build_linux/) 또는 [Windows](https://carla.readthedocs.io/en/latest/build_windows/)에서 빌드하는 지침을 따르세요.

__5. 콘텐츠 저장소 다운로드__ [여기](https://carla.readthedocs.io/en/latest/build_update/#get-development-assets)의 지침에 따라 콘텐츠를 업데이트하세요.

__6. 브랜치를 master와 동기화하여 업데이트__ 브랜치는 항상 master의 최신 변경사항으로 업데이트되어 있어야 합니다.

__7. 기여 업로드__ 해당하는 변경을 하고 브랜치를 origin에 푸시하세요.

__8. 아트 팀의 검토를 기다리세요.__ 기여가 업로드되면 팀이 master와 병합될 준비가 되었는지 확인할 것입니다.

---
## 문서 기여

문서가 누락되었거나, 모호하거나, 부정확한 경우 다른 버그와 마찬가지로 보고할 수 있습니다([버그 보고 방법](#버그-보고) 참조). 하지만 사용자가 직접 문서를 작성하여 기여할 수도 있습니다.

문서는 [Markdown](https://www.markdownguide.org/)과 HTML 태그의 조합으로 작성되며, 표나 [도시 슬라이더](https://carla.readthedocs.io/en/latest/core_map/#carla-maps)와 같은 기능을 위한 추가 CSS 코드가 있습니다. 아래 단계를 따라 문서 작성을 시작하세요.

!!! 중요
    문서 기여를 제출하려면 [코드 기여](#제출)에서 설명한 것과 동일한 워크플로우를 따르세요. 요약하면, 기여는 `dev`의 자식 브랜치에서 이루어지고 해당 브랜치에 병합됩니다.

__1. CARLA를 소스에서 빌드하세요.__ [Linux](build_linux.md) 또는 [Windows](build_windows.md)에서 빌드하는 단계를 따르세요.

__2. [MkDocs](http://www.mkdocs.org/) 설치__ MkDocs는 문서를 빌드하는 데 사용되는 정적 사이트 생성기입니다.

```sh
sudo pip install mkdocs
```

__3. 문서 시각화__ 메인 CARLA 폴더에서 다음 명령을 실행하고 터미널에 나타나는 링크(http://127.0.0.1:8000)를 클릭하여 문서의 로컬 시각화를 엽니다.

```sh
mkdocs serve
```

__4. git 브랜치 생성__ 새 브랜치를 만들 때 `dev` 브랜치에 있고(최신 변경사항으로 업데이트됨) 확인하세요.

```sh
git checkout -b <contributor_name>/<branch_name>
```

__5. 문서 작성__ [문서 표준](cont_doc_standard.md) 페이지의 지침에 따라 파일을 편집하세요.

__6. 변경사항 제출__ GitHub 저장소에서 풀 리퀘스트를 생성하고 제안된 검토자 중 한 명을 추가하세요. 풀 리퀘스트 설명을 작성할 때 최대한 자세히 작성하세요.

__7. 검토 대기__ 팀이 병합 준비가 되었는지 또는 필요한 변경사항이 있는지 확인할 것입니다.

!!! 경고
    로컬 저장소는 `dev` 브랜치의 최신 업데이트로 업데이트되어 있어야 합니다.