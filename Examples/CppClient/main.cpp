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

// 使用 Carla 模块的命名空间简化代码。
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

using namespace std::chrono_literals;
using namespace std::string_literals;

// 简化断言宏，用于验证条件是否满足。
#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }

/// 从给定范围中随机选择一个元素。
/// @param range 包含元素的范围。
/// @param generator 随机数生成器。
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator) {
  EXPECT_TRUE(range.size() > 0u); // 验证范围非空。
  std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
  return range[dist(std::forward<RNG>(generator))];
}

/// 解析命令行参数，返回主机名和端口号。
/// @param argc 参数个数。
/// @param argv 参数值。
/// @return 包含主机名和端口号的元组。
static auto ParseArguments(int argc, const char *argv[]) {
  EXPECT_TRUE((argc == 1u) || (argc == 3u)); // 参数必须为 1 或 3。
  using ResultType = std::tuple<std::string, uint16_t>;
  return argc == 3u ?
      ResultType{argv[1u], std::stoi(argv[2u])} :
      ResultType{"localhost", 2000u}; // 默认连接到 localhost:2000。
}

int main(int argc, const char *argv[]) {
  try {
    // 解析命令行参数，获取主机名和端口号。
    std::string host;
    uint16_t port;
    std::tie(host, port) = ParseArguments(argc, argv);

    // 初始化随机数生成器。
    std::mt19937_64 rng((std::random_device())());

    // 创建客户端并设置超时时间。
    auto client = cc::Client(host, port);
    client.SetTimeout(40s);

    // 输出客户端和服务端的 API 版本。
    std::cout << "Client API version : " << client.GetClientVersion() << '\n';
    std::cout << "Server API version : " << client.GetServerVersion() << '\n';

    // 随机选择一个可用的地图并加载。
    auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
    std::cout << "Loading world: " << town_name << std::endl;
    auto world = client.LoadWorld(town_name);

    // 从蓝图库中随机选择一个车辆蓝图。
    auto blueprint_library = world.GetBlueprintLibrary();
    auto vehicles = blueprint_library->Filter("vehicle");
    auto blueprint = RandomChoice(*vehicles, rng);

    // 随机化车辆蓝图的属性（如颜色）。
    if (blueprint.ContainsAttribute("color")) {
      auto &attribute = blueprint.GetAttribute("color");
      blueprint.SetAttribute(
          "color",
          RandomChoice(attribute.GetRecommendedValues(), rng));
    }

    // 从推荐的生成点中随机选择一个位置。
    auto map = world.GetMap();
    auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);

    // 在选定位置生成车辆。
    auto actor = world.SpawnActor(blueprint, transform);
    std::cout << "Spawned " << actor->GetDisplayId() << '\n';
    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

    // 应用控制命令以使车辆前进。
    cc::Vehicle::Control control;
    control.throttle = 1.0f; // 油门设为最大值。
    vehicle->ApplyControl(control);

    // 调整观察者的位置以查看车辆。
    auto spectator = world.GetSpectator();
    transform.location += 32.0f * transform.GetForwardVector();
    transform.location.z += 2.0f; // 提升视角高度。
    transform.rotation.yaw += 180.0f; // 调整观察方向。
    transform.rotation.pitch = -15.0f; // 向下倾斜视角。
    spectator->SetTransform(transform);

/*
    // 获取语义分割相机的蓝图。
    auto camera_bp = blueprint_library->Find("sensor.camera.semantic_segmentation");
    EXPECT_TRUE(camera_bp != nullptr);

    // 在车辆上安装相机。
    auto camera_transform = cg::Transform{
        cg::Location{-5.5f, 0.0f, 2.8f},   // x, y, z.
        cg::Rotation{-15.0f, 0.0f, 0.0f}}; // pitch, yaw, roll.
    auto cam_actor = world.SpawnActor(*camera_bp, camera_transform, actor.get());
    auto camera = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    // 注册回调函数，将语义分割图像保存到磁盘。
    camera->Listen([](auto data) {
        auto image = boost::static_pointer_cast<csd::Image>(data);
        EXPECT_TRUE(image != nullptr);
        SaveSemSegImageToDisk(*image);
    });

    // 模拟运行一段时间以捕获图像。
    std::this_thread::sleep_for(10s);

    // 销毁相机。
    camera->Destroy();
*/

    // 销毁车辆。
    vehicle->Destroy();
    std::cout << "Actors destroyed." << std::endl;

  } catch (const cc::TimeoutException &e) {
    // 处理客户端超时异常。
    std::cout << '\n' << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    // 处理其他异常。
    std::cout << "\nException: " << e.what() << std::endl;
    return 2;
  }
}
