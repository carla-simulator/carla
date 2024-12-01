# 에셋을 위한 배포 패키지 생성하기

CARLA에서는 에셋을 독립 실행형 패키지로 관리하는 것이 일반적인 관행입니다. 에셋을 별도로 관리하면 빌드 크기를 줄일 수 있습니다. 이러한 에셋 패키지는 언제든지 CARLA 패키지로 쉽게 가져올 수 있습니다. 또한 에셋을 체계적으로 배포하는 데 매우 유용합니다.

- [__소스에서 빌드한 CARLA에서 패키지 내보내기__](#소스에서-빌드한-carla에서-패키지-내보내기)
- [__도커를 사용하여 패키지 내보내기__](#도커를-사용하여-패키지-내보내기)
- [__CARLA 패키지로 에셋 가져오기__](#carla-패키지로-에셋-가져오기)

---
## 소스에서 빌드한 CARLA에서 패키지 내보내기

에셋을 언리얼로 가져온 후, 사용자는 이를 위한 __독립 실행형 패키지__를 생성할 수 있습니다. 이는 CARLA 0.9.8과 같은 CARLA 패키지에 콘텐츠를 배포하는 데 사용됩니다.

패키지를 내보내려면 아래 명령을 실행하면 됩니다.

```sh
make package ARGS="--packages=Package1,Package2"
```

이 명령은 나열된 각 패키지에 대해 `.tar.gz` 파일로 압축된 독립 실행형 패키지를 생성합니다. 파일은 Linux에서는 `Dist` 폴더에, Windows에서는 `/Build/UE4Carla/`에 저장됩니다.

---

## 도커를 사용하여 패키지 내보내기

언리얼 엔진과 CARLA는 도커 이미지로 빌드할 수 있으며, 이후 이 이미지를 사용하여 패키지를 생성하거나 패키지에서 사용할 에셋을 내보낼 수 있습니다.

도커 이미지를 생성하려면 [여기](build_docker_unreal.md)의 튜토리얼을 따르세요.

이미지가 준비되면:

1. `Util/Docker`로 이동합니다.
2. 다음 명령 중 하나를 실행하여 CARLA 패키지를 생성하거나 패키지에서 사용할 에셋을 준비합니다:

```sh
# 독립 실행형 패키지를 생성하려면
./docker_tools.py --output /output/path

# CARLA 패키지에서 사용할 에셋을 요리하려면
./docker_tools.py --input /assets/to/import/path --output /output/path --packages PkgeName1,PkgeName2
```

---
## CARLA 패키지로 에셋 가져오기

독립 실행형 패키지는 `.tar.gz` 파일에 포함되어 있습니다. 이 파일을 추출하는 방법은 플랫폼에 따라 다릅니다.

* __Windows에서는__ 압축 파일을 CARLA 메인 루트 폴더에 추출합니다.
* __Linux에서는__ 압축 파일을 `Import` 폴더로 이동하고 다음 스크립트를 실행합니다.

```sh
cd Import
./ImportAssets.sh
```

!!! 참고
    독립 실행형 패키지는 CARLA 빌드로 직접 가져올 수 없습니다. [프롭](tuto_A_add_props.md), [맵](tuto_M_custom_map_overview.md) 또는 [차량](tuto_A_add_vehicle.md)을 가져오는 튜토리얼을 따르세요.

---

이것으로 CARLA에서 독립 실행형 패키지를 생성하고 사용하는 방법에 대한 설명을 마칩니다. 예상치 못한 문제가 발생하면 포럼에 자유롭게 게시하세요.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="CARLA 포럼으로 이동">
CARLA 포럼</a>
</p>
</div>