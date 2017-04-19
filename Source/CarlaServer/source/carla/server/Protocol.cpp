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

  template<typename Type>
  static std::string GetBytes(Type n) {
    union{
      Type num;
      unsigned char bytes[4];
    } value;

    value.num = n;

    std::string out_bytes;
    for (int i = 0 ; i < 4 ; ++i) out_bytes += value.bytes[i];

    return out_bytes;
  }

#ifdef WITH_TURBOJPEG

  static bool GetImage(const int jpeg_quality, const Image &image_info, unsigned char **compressedImage, long unsigned int &jpegSize){
    if (image_info.image.empty())
      return false;
    if (image_info.image.size() != image_info.width * image_info.height) {
      std::cerr << "Invalid image size" << std::endl;
      return false;
    }

    std::vector<unsigned char> color_image;
    color_image.reserve(3u * image_info.image.size());
    for (const Color &color : image_info.image) {
      color_image.push_back(color.R);
      color_image.push_back(color.G);
      color_image.push_back(color.B);
    }

    tjhandle jpegCompressor = tjInitCompress();
    tjCompress2(jpegCompressor, color_image.data(), image_info.width, 0, image_info.height, TJPF_RGB,
      compressedImage, &jpegSize, TJSAMP_444, jpeg_quality, TJFLAG_FASTDCT);
    tjDestroy(jpegCompressor);

    return true;
  }


static bool getJPEGImages(const int jpeg_quality, const std::vector<Image> &images, Reward &rwd){
    std::string image_data;
    std::string depth_data;
    std::string image_size_data;
    std::string depth_size_data;
    //nt images_count = 0, depth_count = 0;

    for (const Image &img : images){
      unsigned char *compressedImage;
      long unsigned int jpegSize = 0;
      if (!GetImage(jpeg_quality, img, &compressedImage, jpegSize)) {
        std::cerr << "Error while encoding image" << std::endl;
        return false;
      }

      switch (img.type){
        case IMAGE:
          for (unsigned long int i = 0; i < jpegSize; ++i) image_data += compressedImage[i];
          image_size_data += GetBytes(jpegSize);
          break;
        case DEPTH:
          for (unsigned long int i = 0; i < jpegSize; ++i) depth_data += compressedImage[i];
          depth_size_data += GetBytes(jpegSize);
          break;
      }
    }
    std::cout << "send depth size: " << depth_size_data.size() <<
    " send image size: " << image_size_data.size()<<
    " send image: " << image_data.size()<<
    " send depth: " << depth_data.size() << std::endl;
    rwd.set_depth_sizes(depth_size_data);
    rwd.set_image_sizes(image_size_data);
    rwd.set_images(image_data);
    rwd.set_depths(depth_data);
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

    constexpr int JPEG_QUALITY = 75;

    if (!getJPEGImages(JPEG_QUALITY, values.images, reward)) {
        std::cerr << "Error compressing image to JPEG" << std::endl;
    }

#endif // WITH_TURBOJPEG

  }

  void Protocol::LoadScene(Scene &scene, const Scene_Values &values) {

    scene.set_number_of_cameras(values.projection_matrices.size());

    std::string positions_bytes = "";

    std::cout << "------- POSITIONS--------" << std::endl;


    for (auto i = 0u; i < values.possible_positions.size(); ++i) {

      float x = values.possible_positions[i].x;
      float y = values.possible_positions[i].y;

      positions_bytes += GetBytes(x) + GetBytes(y);

      std::cout << "x: " << x << " byte size: " << sizeof(float) << " bytes: " << GetBytes(x) << std::endl;
      std::cout << "y: " << y << " byte size: " << sizeof(float) << " bytes: " << GetBytes(y) << std::endl;

    }
    std::cout << "---------------" << std::endl;
    std::cout << "Final string: "<< positions_bytes << std::endl;
    std::cout << "---------------" << std::endl;

    scene.set_positions(positions_bytes);

    std::string matrices;

    for (auto i = 0u; i < values.projection_matrices.size(); ++i) {
      for (auto e = 0u; e < 16u; ++e){

        float value = values.projection_matrices[i][e];

        char data [sizeof(float)];
        memcpy(data, &value, sizeof value);
        matrices.append(data);

      }
    }

    scene.set_projection_matrices(matrices);

  }

  void Protocol::LoadWorld(World &world, const int modes, const int scenes) {
    world.set_modes(modes);
    world.set_scenes(scenes);
  }

}
}
