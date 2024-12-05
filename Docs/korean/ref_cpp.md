# C++ 레퍼런스

## C++ 클라이언트

C++ 클라이언트는 Linux에서는 `make`로, Windows에서는 `cmake`로 빌드할 수 있습니다. C++ 클라이언트 예제는 저장소의 `CARLA_ROOT/Examples/CppClient/main.cpp`에서 제공됩니다. 이 예제는 C++ 클라이언트를 CARLA 서버에 연결하고 몇 가지 간단한 작업을 위해 API를 사용하는 방법을 보여줍니다.

예제 C++ 클라이언트를 빌드하려면, 저장소의 `CARLA_ROOT/Examples/CppClient` 디렉토리에서 터미널을 엽니다. 이 폴더에서 `make`를 실행한 다음 `./bin/cpp_client`를 실행하여 예제를 실행하세요. 이 예제는 사용 가능한 맵 중에서 무작위로 맵을 선택한 다음 로드합니다. 그런 다음 차량을 스폰하고 차량에 컨트롤을 적용합니다.

이 예제 스크립트에 대한 자세한 내용은 [C++ 클라이언트 예제](adv_cpp_client.md)를 참조하세요.

## C++ 문서

우리는 C++ 코드의 문서를 생성하기 위해 Doxygen을 사용합니다:

[Libcarla/Source](http://carla.org/Doxygen/html/dir_b9166249188ce33115fd7d5eed1849f2.html)<br>
[Unreal/CarlaUE4/Source](http://carla.org/Doxygen/html/dir_733e9da672a36443d0957f83d26e7dbf.html)<br>
[Unreal/CarlaUE4/Carla/Plugins](http://carla.org/Doxygen/html/dir_8fc34afb5f07a67966c78bf5319f94ae.html)

생성된 문서는 다음 링크에서 확인할 수 있습니다: **<http://carla.org/Doxygen/html/index.html>**

!!! 참고
    문서 업데이트는 GitHub에 의해 자동으로 수행됩니다.

### Doxygen 문서 생성하기

!!! 중요
    문서를 생성하려면 [Doxygen](http://www.doxygen.nl/index.html)이 필요하며,
    그래프 그리기 도구를 위해 [Graphviz](https://www.graphviz.org/)가 필요합니다.

1- 다음 명령으로 doxygen과 graphviz를 설치합니다:

```sh
# linux
> sudo apt-get install doxygen graphviz
```

2- 설치가 완료되면 _Doxyfile_ 파일이 있는 프로젝트 루트 폴더로 이동하여
다음 명령을 실행합니다:

```sh
> doxygen
```

이것으로 문서 웹페이지 빌드가 시작됩니다.  
생성된 웹페이지는 Doxygen/html/ 에서 찾을 수 있습니다.

3- 브라우저에서 _index.html_을 엽니다. 이제 로컬 C++ 문서가 준비되었습니다!