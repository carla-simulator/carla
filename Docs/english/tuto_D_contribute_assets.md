# 컨텐츠 업그레이드 방법

우리의 컨텐츠는 별도의 [Git LFS 저장소][contentrepolink]에 있습니다. 빌드 시스템의 일부로, 현재 날짜와 커밋으로 태그된 최신 버전의 컨텐츠가 포함된 패키지를 생성하고 업로드합니다. 정기적으로 [CARLA 저장소][carlarepolink]를 최신 버전의 컨텐츠 패키지 링크로 업그레이드합니다. 이 문서에는 이 링크를 최신 버전으로 업데이트하는 데 필요한 수동 단계가 포함되어 있습니다.

1. **연결하고자 하는 컨텐츠 패키지의 태그를 복사합니다.**<br>
   이 태그는 최신 [Jenkins 빌드][jenkinslink]의 아티팩트 섹션에서 생성된 패키지 이름에서 찾을 수 있습니다(예: `20190617_086f97f.tar.gz`).

2. **ContentVersions.txt에 태그를 붙여넣습니다.**<br>
   [ContentVersions.txt 편집][cvlink]에서 파일 끝에 태그를 붙여넣습니다(예: `Latest: 20190617_086f97f`, `.tar.gz` 부분은 제외).

3. **Pull Request를 엽니다.**<br>
   변경 사항을 커밋하고 새 Pull Request를 엽니다.

[contentrepolink]: https://bitbucket.org/carla-simulator/carla-content
[carlarepolink]: https://github.com/carla-simulator/carla
[jenkinslink]: http://35.181.165.160:8080/blue/organizations/jenkins/carla-content/activity
[cvlink]: https://github.com/carla-simulator/carla/edit/master/Util/ContentVersions.txt