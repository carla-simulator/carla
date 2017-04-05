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

#ifdef WITH_TURBOJPEG

  enum ImageType{
    IMAGE_1,
    IMAGE_2,
    DEPTH_1,
    DEPTH_2,
  };

  static bool getJPEGImage(
      const int jpeg_quality,
      const size_t width,
      const size_t height,
      const std::vector<Color> &image,
      ImageType image_type,
      Reward &rwd){
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


    switch (image_type){
      case IMAGE_1:
        rwd.set_image1(compressedImage, jpegSize);
      break;
      case IMAGE_2:
        rwd.set_image2(compressedImage, jpegSize);
      break;
      case DEPTH_1:
        rwd.set_depth1(compressedImage, jpegSize);
      break;
      case DEPTH_2:
        rwd.set_depth2(compressedImage, jpegSize);
      break;
    }

    return true;
  }

#endif // WITH_TURBOJPEG


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

#ifdef WITH_TURBOJPEG
    // Compress images to JPEG

    struct ImageHolder {
      ImageType type;
      const decltype(values.image_rgb_0) &image;
    };

    std::vector<ImageHolder> images;
    if (_communication->GetMode() == Mode::MONO) {
      images.push_back({IMAGE_1, values.image_rgb_0});

    } else if (_communication->GetMode() == Mode::STEREO) {
      images.reserve(4u);
      images.push_back({IMAGE_1, values.image_rgb_0});
      images.push_back({IMAGE_2, values.image_rgb_1});
      images.push_back({DEPTH_1, values.image_depth_0});
      images.push_back({DEPTH_2, values.image_depth_1});
    } else {
      std::cerr << "Error, invalid mode" << std::endl;
      return;
    }

    constexpr int JPEG_QUALITY = 75;
    for (const auto &holder : images) {
      if (!getJPEGImage(
            JPEG_QUALITY,
            values.image_width,
            values.image_height,
            holder.image,
            holder.type,
            reward)) {
        std::cerr << "Error compressing image to JPEG" << std::endl;
      }
    }
#endif // WITH_TURBOJPEG
  }

  void Protocol::LoadScene(Scene &scene, const Scene_Values &values) {
    for (auto i = 0u; i < values.possible_positions.size(); ++i) {
      Scene::Position *point = scene.add_position();
      point->set_pos_x(values.possible_positions[i].x);
      point->set_pos_y(values.possible_positions[i].y);
    }

    if (_communication->GetMode() == Mode::STEREO) {
      for (auto i = 0u; i < values.projection_matrices.size(); ++i) {
        Scene::Projection_Matrix *matrix = scene.add_camera_matrix();
        for (auto e = 0u; e < 16u; ++e) {
          matrix->add_cam_param(values.projection_matrices[i][e]);
        }
      }
    }
  }

  void Protocol::LoadWorld(World &world, const int modes, const int scenes) {
    world.set_modes(modes);
    world.set_scenes(scenes);
  }

}
}
