#include "Protocol.h"

#include "CarlaCommunication.h"
#include "carla/CarlaServer.h"

#include "carla_protocol.pb.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#ifdef WITH_TURBOJPEG
#include <turbojpeg.h>
#endif // WITH_TURBOJPEG

namespace carla {
namespace server {

  static bool getJPEGImage(
      const int jpeg_quality,
      const int color_components,
      const int width,
      const int height,
      const std::vector<Color> &image,
      bool depth,
      Reward &rwd){
#ifndef WITH_TURBOJPEG
    return false;
#else
    long unsigned int jpegSize = 0;
    unsigned char *compressedImage;
    if (image.empty())
      return false;
    if (image.size() != width * height) {
      std::cerr << "Invalid image size" << std::endl;
      return false;
    }
    // Convert to char array RGBA.
    std::vector<unsigned char> color_image;
    color_image.reserve(3u * image.size());
    for (const Color &color : image) {
      color_image.push_back(color.R);
      color_image.push_back(color.G);
      color_image.push_back(color.B);
    }
    tjhandle jpegCompressor = tjInitCompress();
    tjCompress2(jpegCompressor, color_image.data(), width, 0, height, TJPF_RGB,
      &compressedImage, &jpegSize, TJSAMP_444, jpeg_quality, TJFLAG_FASTDCT);
    tjDestroy(jpegCompressor);
    if (!depth) {
      rwd.set_image(compressedImage, jpegSize);
    } else {
      rwd.set_depth(compressedImage, jpegSize);
    }
    return true;
#endif // WITH_TURBOJPEG
  }


  Protocol::Protocol(carla::server::CarlaCommunication *communication) {
    _communication = communication;
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

    auto images = {values.image_rgb_0/*, values.image_rgb_1*/};
    for (const std::vector<Color> &image : images) {
      long unsigned int jpegSize = 0;
      if (!getJPEGImage(75, 3, values.image_width, values.image_height, image, false, reward)) {
        std::cout << "ERROR" << std::endl;
      }
    }


    if (_communication -> GetMode() == Mode::STEREO){
      auto depths = {values.image_depth_0, values.image_depth_1};
      for (const std::vector<Color> &image : depths) {
        //if (getPNGImage(image, values.image_width, values.image_height, png_image)) {
        if (getJPEGImage(75, 3, values.image_width, values.image_height, image, true, reward)){
          std::cout << "ERROR" << std::endl;
        }
      }
    }
  }

  void Protocol::LoadScene(Scene &scene, const Scene_Values &values) {
    std::vector<Vector2D> positions = values.possible_positions;
    for (int i = 0; i < positions.size(); ++i) {
      Scene::Position* point = scene.add_position();
      point->set_pos_x(positions[i].x);
      point->set_pos_y(positions[i].y);
    }

    if (_communication->GetMode() == Mode::STEREO) {
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
