# C++ 클라이언트 예제

C++ 클라이언트 예제를 빌드하려면 `make`가 설치되어 있어야 합니다. C++ 클라이언트를 빌드하기 전에 CARLA를 빌드해야 하며, 플랫폼에 맞는 관련 [빌드 지침](build_carla.md)을 따르세요.

CARLA 저장소의 `Examples/CppClient` 폴더로 이동하여 터미널을 엽니다. 이 디렉토리에서 Makefile을 찾을 수 있습니다. Linux에서는 명령 프롬프트에서 `make run`을 실행하여 빌드하고 실행하세요. Windows에서는 같은 디렉토리에 `CMakeLists.txt` 파일을 만들고 [이 파일](cpp_client_cmake_windows.md)의 내용을 추가한 다음 `cmake`를 실행하세요.

이 C++ 예제는 서버에 연결하고, 차량을 스폰하고, 차량에 명령을 적용한 다음 차량을 파괴하고 종료합니다.

### 관련 헤더 파일 포함

이 예제에서는 여러 CARLA 클래스를 사용할 것이므로 CARLA 라이브러리에서 관련 헤더 파일을 포함하고 사용할 표준 라이브러리를 포함해야 합니다:

```cpp
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>

#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/Sensor.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>
```

### C++ 클라이언트를 서버에 연결

`carla/client/Client.h`를 포함하고 클라이언트를 연결하세요:

```cpp
...
#include <carla/client/Client.h>
...
int main(int argc, const char *argv[]) {

    std::string host;
    uint16_t port;
    std::tie(host, port) = ParseArguments(argc, argv);
    ...
    // 클라이언트를 서버에 연결
    auto client = cc::Client(host, port);
    client.SetTimeout(40s);
```

### 맵 로드

이제 무작위로 선택된 맵을 로드해보겠습니다:

```cpp
// 난수 생성기 초기화
std::mt19937_64 rng((std::random_device())());
...
auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
std::cout << "Loading world: " << town_name << std::endl;
auto world = client.LoadWorld(town_name);
```

### 무작위로 선택된 차량 스폰

다음으로 블루프린트 라이브러리를 가져오고, 차량을 필터링하고, 무작위 차량 블루프린트를 선택합니다:

```cpp
auto blueprint_library = world.GetBlueprintLibrary();
auto vehicles = blueprint_library->Filter("vehicle");
auto blueprint = RandomChoice(*vehicles, rng);
```

이제 맵의 스폰 포인트에서 차량을 스폰할 위치를 찾아야 합니다. 맵 객체에 대한 포인터 참조를 가져온 다음 무작위 스폰 포인트를 선택합니다(난수 생성기를 초기화했는지 확인하세요):

```cpp
auto map = world.GetMap();
auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);
```

이제 블루프린트와 스폰 위치가 있으므로 `world.SpawnActor(...)` 메서드를 사용하여 차량을 스폰할 수 있습니다:

```cpp
auto actor = world.SpawnActor(blueprint, transform);
std::cout << "Spawned " << actor->GetDisplayId() << '\n';
// 차량 객체에 대한 포인터 검색
auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
```

### 컨트롤 적용

이제 `ApplyControl(...)` 메서드를 사용하여 차량을 움직이기 위한 컨트롤을 적용해보겠습니다:

```cpp
cc::Vehicle::Control control;
control.throttle = 1.0f;
vehicle->ApplyControl(control);
```

이제 관찰자를 재배치하여 맵에서 새로 스폰된 차량을 볼 수 있게 하겠습니다:

```cpp
auto spectator = world.GetSpectator();
// 보기 위해 transform 조정
transform.location += 32.0f * transform.GetForwardVector();
transform.location.z += 2.0f;
transform.rotation.yaw += 180.0f;
transform.rotation.pitch = -15.0f;
// 이제 관찰자 transform 설정
spectator->SetTransform(transform);
```

클라이언트가 닫히기 전에 시뮬레이션을 잠시 관찰할 수 있도록 프로세스를 10초 동안 대기시킵니다:

```cpp
std::this_thread::sleep_for(10s);
```

다른 명령이 실행되는 동안 클라이언트를 열어두려면 게임 루프를 만드세요. 이제 맵을 로드하고 차량을 스폰했습니다. C++ API를 더 자세히 살펴보려면 [Doxygen 문서를 빌드](ref_cpp.md#c-documentation)하고 브라우저에서 여세요.

CARLA 저장소 외부의 다른 위치에서 C++ 클라이언트를 빌드하려면, `/build` 디렉토리와 CARLA 빌드 위치에 대한 올바른 위치를 참조하도록 Makefile의 처음 5줄을 편집하세요:

```make
CARLADIR=$(CURDIR)/../..
BUILDDIR=$(CURDIR)/build
BINDIR=$(CURDIR)/bin
INSTALLDIR=$(CURDIR)/libcarla-install
TOOLCHAIN=$(CURDIR)/ToolChain.cmake
```