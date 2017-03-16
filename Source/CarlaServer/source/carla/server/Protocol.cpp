#include "Protocol.h"

#include "Server.h"
#include "lodepng.h"

#include "carla_protocol.pb.h"

#include <iostream>

namespace carla {
namespace server {

  static bool getPNGImage(
      const std::vector<Color> &in,
      uint32_t width,
      uint32_t height,
      std::vector<unsigned char> &outPNG) {
    if (in.empty())
      return false;
    if (in.size() != width * height) {
      std::cerr << "Invalid image size" << std::endl;
      return false;
    }
    // Convert to char array RGBA.
    std::vector<unsigned char> color_img;
    color_img.reserve(4u * in.size());
    for (const Color &color : in) {
      color_img.emplace_back(color.R);
      color_img.emplace_back(color.G);
      color_img.emplace_back(color.B);
      color_img.emplace_back(color.A);
    }
    // Compress to png.
    lodepng::State state;
    outPNG.reserve(color_img.size());
    const int error = lodepng::encode(outPNG, color_img, width, height, state);
    if (error) {
      std::cerr << "Error compressing image to PNG: error " << error << std::endl;
      return false;
    }
    return true;
  }

  Protocol::Protocol(carla::server::Server *server) {
    _server = server;
  }

  Protocol::~Protocol() {}

  void Protocol::LoadReward(Reward &reward, const Reward_Values &values) {
    reward.set_collision_car(values.collision_car);
    reward.set_collision_gen(values.collision_general);
    reward.set_collision_ped(values.collision_pedestrian);
    reward.set_acceleration_x(values.player_acceleration.x);
    reward.set_acceleration_y(values.player_acceleration.y);
    reward.set_acceleration_z(values.player_acceleration.z);
    reward.set_sidewalk_intersect(values.intersect_offroad);
    reward.set_ori_x(values.player_orientation.x);
    reward.set_ori_y(values.player_orientation.y);
    reward.set_ori_z(values.player_orientation.z);
    reward.set_player_y(values.player_location.y);
    reward.set_player_x(values.player_location.x);
    reward.set_speed(values.forward_speed);
    reward.set_timestamp(values.timestamp);

    auto images = {values.image_rgb_0, values.image_rgb_1};
    for (const std::vector<Color> &image : images) {
      std::vector<unsigned char> png_image;
      if (getPNGImage(image, values.image_width, values.image_height, png_image)) {
        reward.add_image(std::string(png_image.begin(), png_image.end()));
      }
    }

    auto depths = {values.image_depth_0, values.image_depth_1};
    for (const std::vector<Color> &image : depths) {
      std::vector<unsigned char> png_image;
      if (getPNGImage(image, values.image_width, values.image_height, png_image)) {
        reward.add_depth(std::string(png_image.begin(), png_image.end()));
      }
    }
  }

  void Protocol::LoadScene(Scene &scene, const Scene_Values &values) {
    std::vector<Vector2D> positions = values.possible_positions;
    for (int i = 0; i < positions.size(); ++i) {
      Scene::Position* point;
      point = scene.add_position();
      point->set_pos_x(positions[i].x);
      point->set_pos_y(positions[i].y);
    }

    if (_server->GetMode() == Mode::STEREO) {
      Scene::Projection_Matrix* matrix;
      std::vector<std::array<float,16>> projection_matrix = values.projection_matrices;
      for (int i = 0; i < projection_matrix.size(); ++i) {
        matrix = scene.add_camera_matrix();
        for (int e = 0; e < 16; ++e) matrix->add_cam_param(projection_matrix[i][e]);
      }
    }
  }

  void Protocol::LoadWorld(World &world, const int modes, const int scenes) {
    world.set_modes(modes);
    world.set_scenes(scenes);
  }

}
}
