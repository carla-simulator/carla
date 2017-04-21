#include "Protocol.h"

#include "CarlaCommunication.h"
#include "carla/CarlaServer.h"

#include "carla_protocol.pb.h"

#include <iostream>

#include <stdio.h>

#include <png.h>
#include <stdlib.h>
#include <stdint.h>

//#include <ctime>

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

  typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  } pixel_t;

  typedef struct {
    pixel_t* pixels;
    size_t width;
    size_t height;

  } bitmap_t;


  struct img_encode{
    char* buffer;
    size_t size;
  };

  struct TPngDestructor {
    png_struct *p;
    TPngDestructor(png_struct *p) : p(p)  {}
    ~TPngDestructor() { if (p) {  png_destroy_write_struct(&p, NULL); } }
  };

  static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
  {
    return bitmap->pixels + bitmap->width * y + x;
  }

  void loadPNGData(png_structp png_ptr, png_bytep data, png_size_t length)
  {
    /* with libpng15 next line causes pointer deference error; use libpng12 */
    img_encode* p=(img_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
    size_t nsize = p->size + length;

    /* allocate or grow buffer */
    if(p->buffer)
      p->buffer = (char*) realloc(p->buffer, nsize);
    else
      p->buffer = (char*) malloc(nsize);

    if(!p->buffer)
      png_error(png_ptr, "Write Error");

    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->size, data, length);
    p->size += length;


  }

  bool LoadBitmap (bitmap_t &bitmap, const Image &image_info){

    bitmap.width = image_info.width;
    bitmap.height = image_info.height;
    bitmap.pixels = (pixel_t*) calloc (bitmap.width * bitmap.height, sizeof(pixel_t));

    if (!bitmap.pixels) return false;

    size_t x = 0, y = 0;
    for (const Color &color : image_info.image) {

      if (x >= bitmap.width) {
        x = 0;
        ++y;
      }

      pixel_t* pixel = pixel_at(&bitmap, x, y);
      pixel->red = color.R;
      pixel->green = color.G;
      pixel->blue = color.B;
      ++x;
      //pixel->alpha = color.A;
    }

    return true;
  }


  static bool GetImage(const Image &image_info, img_encode &compressedImage){
    if (image_info.image.empty())
      return false;
    if (image_info.image.size() != image_info.width * image_info.height) {
      std::cerr << "Invalid image size" << std::endl;
      return false;
    }

    bitmap_t bitmap;
    if (!LoadBitmap(bitmap, image_info)) return false;

    int depth = 8;
    int pixel_size = 3;
    png_structp png = NULL;
    png_infop info = NULL;
    size_t x,y;
    png_byte ** row_pointers = NULL;

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
      std::cerr << "Cannot create png_struct" << std::endl;
      return false;
    }

    TPngDestructor destroyPng(png);

    info = png_create_info_struct(png);
    if (!info){
      std::cerr << "Cannot create info_struct" << std::endl;
      return false;
    }

    if (setjmp(png_jmpbuf(png))) return false;

    png_set_IHDR (png,
                  info,
                  bitmap.width,
                  bitmap.height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);


    /* Initialize rows of PNG. */
    row_pointers = (png_byte **) png_malloc (png, bitmap.height * sizeof (png_byte *));
    for (y = 0; y < bitmap.height; ++y) {
        png_byte *row = (png_byte *) png_malloc (png, sizeof (uint8_t) * bitmap.width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap.width; ++x) {
            pixel_t * pixel = pixel_at (&bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }

    compressedImage.buffer = NULL;
    compressedImage.size = 0;


    png_set_write_fn(png, &compressedImage, loadPNGData, NULL);

    png_set_rows (png, info, row_pointers);
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);


    for (size_t y = 0; y < bitmap.height; y++) {
        png_free (png, row_pointers[y]);
    }

    png_free (png, row_pointers);
    free(bitmap.pixels);
    free(info);

    return true;
  }


static bool getJPEGImages(const std::vector<Image> &images, Reward &rwd){
    std::string image_data;
    std::string depth_data;
    std::string image_size_data;
    std::string depth_size_data;

    for (const Image &img : images){
      img_encode compressedImage;

      /*
      {
      using namespace std;
      clock_t begin = clock();
      if (!GetImage(img, compressedImage)) {
        std::cerr << "Error while encoding image" << std::endl;
        return false;
      }
      clock_t end = clock();
      double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
      cout << "Time to encode the image: " << elapsed_secs << " sec" << endl;
      }*/
      

      if (!GetImage(img, compressedImage)) {
        std::cerr << "Error while encoding image" << std::endl;
        return false;
      }

      switch (img.type){
        case IMAGE:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) image_data += compressedImage.buffer[i];
          image_size_data += GetBytes(compressedImage.size);
          break;
        case DEPTH:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) depth_data += compressedImage.buffer[i];
          depth_size_data += GetBytes(compressedImage.size);
          break;
      }

      free (compressedImage.buffer);

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

    //constexpr int JPEG_QUALITY = 75;

    if (!getJPEGImages(values.images, reward)) {
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
