#include "Protocol.h"

#include "CarlaCommunication.h"
#include "carla/CarlaServer.h"
//#include "lodepng.h"

#include "carla_protocol.pb.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>


#include <turbojpeg.h> 
//#include "jpeglib.h"


namespace carla {
namespace server {



  static void WriteImage(const unsigned char *image, long unsigned int size){
    FILE *out;
    out = fopen("test.jpeg","w");
    fwrite(image,1,size,out);
    fclose(out);
  }


  //static bool getPNGImage(
  //    const std::vector<Color> &in,
  //    uint32_t width,
  //    uint32_t height,
  //    std::vector<unsigned char> &outPNG) {
  static bool getJPEGImage(
    const int jpeg_quality,
    const int color_components, 
    const int width,
    const int height,
    const std::vector<Color> &image,
    bool depth,
    Reward &rwd
    ){

    long unsigned int jpegSize = 0;

    unsigned char *compressedImage;

    if (image.empty())
      return false;


    if (image.size() != width * height) {
      std::cerr << "Invalid image size" << std::endl;
      return false;
    }
    // Convert to char array RGBA.
    int size = 3u * image.size();
    unsigned char color_img[size];

    for (int i = 0, color_it = 0; i < size; ++color_it){
      color_img[i++] = image[color_it].R;
      color_img[i++] = image[color_it].G;
      color_img[i++] = image[color_it].B;
    }

    tjhandle jpegCompressor = tjInitCompress();

    tjCompress2(jpegCompressor, color_img, width, 0, height, TJPF_RGB,
      &compressedImage, &jpegSize, TJSAMP_444, jpeg_quality, TJFLAG_FASTDCT);


    tjDestroy(jpegCompressor);

    WriteImage(compressedImage, jpegSize);

    if (!depth) rwd.set_image(compressedImage, jpegSize);

    else rwd.set_depth(compressedImage, jpegSize);

    return true;
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
      unsigned char *png_image;
      long unsigned int jpegSize = 0;
      if (!getJPEGImage(75, 3, values.image_width, values.image_height, image, false, reward)){

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


/*    std::cout << "POSSIBLE POSITIONS 3"<< std::endl;

    for (int i=0; i<values.possible_positions.size(); ++i){
      std::cout << "   x: " << values.possible_positions[i].x << " y: " << values.possible_positions[i].y << std::endl;
    }

    std::cout << "POSSIBLE POSITIONS 4"<< std::endl;
*/
    for (int i = 0; i < positions.size(); ++i) {
      Scene::Position* point = scene.add_position();
      point->set_pos_x(positions[i].x);
      point->set_pos_y(positions[i].y);
      
      //std::cout << "   x: " << point->pos_x() << " y: " << point->pos_y() << std::endl;
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
