#include "Protocol.h"

#include "CarlaCommunication.h"
#include "carla/CarlaServer.h"

#include <carla/Logging.h>

#include "carla_protocol.pb.h"

#include <ctime>
#include <iostream>
#include <memory>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef CARLA_WITH_PNG_COMPRESSION
#include CARLA_LIBPNG_INCLUDE
static_assert(
        CARLA_LIBPNG_VERSION_MAJOR == PNG_LIBPNG_VER_MAJOR &&
        CARLA_LIBPNG_VERSION_MINOR == PNG_LIBPNG_VER_MINOR &&
        CARLA_LIBPNG_VERSION_RELEASE == PNG_LIBPNG_VER_RELEASE,
    "PNG versions do not match");
#endif // CARLA_WITH_PNG_COMPRESSION

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

  // ===========================================================================
  // -- PNG related functions --------------------------------------------------
  // ===========================================================================

#ifdef CARLA_WITH_PNG_COMPRESSION

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

  static pixel_t * pixel_at(bitmap_t * bitmap, int x, int y) {
    return bitmap->pixels + bitmap->width * y + x;
  }

  void loadPNGData(png_structp png_ptr, png_bytep data, png_size_t length) {
    /* with libpng15 next line causes pointer deference error; use libpng12 */
    img_encode* p = (img_encode*) png_get_io_ptr(png_ptr);

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

  bool LoadBitmap (bitmap_t &bitmap, const Image &image_info) {
    bitmap.width = image_info.width();
    bitmap.height = image_info.height();
    bitmap.pixels = (pixel_t*) calloc (bitmap.width * bitmap.height, sizeof(pixel_t));

    if (!bitmap.pixels) return false;

    size_t x = 0, y = 0;
    for (const Color &color : image_info) {

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
    if (image_info.empty())
      return false;
    if (image_info.size() != image_info.width() * image_info.height()) {
      log_error("Invalid image size:", image_info.size(), "!=", image_info.width(), '*', image_info.height());
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
      log_error("Cannot create png_struct");
      return false;
    }

    TPngDestructor destroyPng(png);

    info = png_create_info_struct(png);
    if (!info){
      log_error("Cannot create info_struct");
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


    // Initialize rows of PNG.
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

  static bool getPNGImages(const Collection<Image> &images, Reward &rwd) {
    std::string image_data;
    std::string sceneFinal_data;
    std::string depth_data;
    std::string semanticSeg_data;
    std::string image_size_data;
    std::string sceneFinal_size_data;
    std::string depth_size_data;
    std::string semanticSeg_size_data;

    for (const Image &img : images){
      img_encode compressedImage;


      if (!GetImage(img, compressedImage)) {
        log_error("Error while encoding image");
        return false;
      }

      switch (img.type()){

        case IMAGE:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) {
            image_data += compressedImage.buffer[i];
          }
          image_size_data += GetBytes(compressedImage.size);
          break;

        case SCENE_FINAL:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) {
            sceneFinal_data += compressedImage.buffer[i];
          }
          sceneFinal_size_data += GetBytes(compressedImage.size);
          break;

        case DEPTH:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) {
            depth_data += compressedImage.buffer[i];
          }
          depth_size_data += GetBytes(compressedImage.size);
          break;

        case SEMANTIC_SEG:
          for (unsigned long int i = 0; i < compressedImage.size; ++i) {
            semanticSeg_data += compressedImage.buffer[i];
          }
          semanticSeg_size_data += GetBytes(compressedImage.size);
          break;
      }

      free(compressedImage.buffer);

    }

    rwd.set_depth_sizes(depth_size_data);
    rwd.set_final_image_sizes(sceneFinal_size_data);
    rwd.set_image_sizes(image_size_data);
    rwd.set_semantic_seg_sizes(semanticSeg_size_data);
    rwd.set_images(image_data);
    rwd.set_final_images(sceneFinal_data);
    rwd.set_depths(depth_data);
    rwd.set_semantic_segs(semanticSeg_data);

    return true;
  }

#else // if not CARLA_WITH_PNG_COMPRESSION, then use bitmap with no compression.

  // ===========================================================================
  // -- Bitmap related functions -----------------------------------------------
  // ===========================================================================

  static bool getBitMapImage(const Image &image_info, char* image){
    if (image_info.empty()){
      log_error("Empty image");
      return false;
    }

    if (image_info.size() != image_info.width() * image_info.height()) {
      log_error("Invalid image size:", image_info.size(), "!=", image_info.width(), '*', image_info.height());
      return false;
    }

    size_t it = 0u;
    for (const Color &color : image_info) {
      image[it++] = color.R;
      image[it++] = color.G;
      image[it++] = color.B;
    }

    return true;
  }

static bool getBitMapImages(const Collection<Image> &images, Reward &rwd) {
    std::string image_data;
    std::string sceneFinal_data;
    std::string depth_data;
    std::string semanticSeg_data;
    std::string image_size_data;
    std::string sceneFinal_size_data;
    std::string depth_size_data;
    std::string semanticSeg_size_data;

    for (const Image &img : images){
      const size_t image_size = img.width() * img.height() * 3u;
      auto image = std::make_unique<char[]>(image_size);

      if (!getBitMapImage(img, image.get())){
        log_error("Error while encoding image");
        return false;
      }

      switch (img.type()){
        case IMAGE:
          for (unsigned long int i = 0u; i < image_size; ++i) {
            image_data += image[i];
          }
          image_size_data += GetBytes(image_size);
          break;

        case SCENE_FINAL:
          for (unsigned long int i = 0u; i < image_size; ++i) {
            sceneFinal_data += image[i];
          }
          sceneFinal_size_data += GetBytes(image_size);
          break;

        case DEPTH:
          for (unsigned long int i = 0u; i < image_size; ++i) {
            depth_data += image[i];
          }
          depth_size_data += GetBytes(image_size);
          break;

        case SEMANTIC_SEG:
          for (unsigned long int i = 0u; i < image_size; ++i) {
            semanticSeg_data += image[i];
          }
          semanticSeg_size_data += GetBytes(image_size);
          break;
      }
    }

    rwd.set_depth_sizes(depth_size_data);
    rwd.set_final_image_sizes(sceneFinal_size_data);
    rwd.set_image_sizes(image_size_data);
    rwd.set_semantic_seg_sizes(semanticSeg_size_data);
    rwd.set_images(image_data);
    rwd.set_final_images(sceneFinal_data);
    rwd.set_depths(depth_data);
    rwd.set_semantic_segs(semanticSeg_data);

    return true;
  }

#endif // CARLA_WITH_PNG_COMPRESSION

  // ===========================================================================
  // -- Protocol ---------------------------------------------------------------
  // ===========================================================================

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
    reward.set_road_intersect(values.intersect_other_lane);
    reward.set_ori_x(values.player_orientation.x);
    reward.set_ori_y(values.player_orientation.y);
    reward.set_ori_z(values.player_orientation.z);
    reward.set_player_y(values.player_location.y);
    reward.set_player_x(values.player_location.x);
    reward.set_speed(values.forward_speed);
    reward.set_platform_timestamp(values.platform_timestamp);
    reward.set_game_timestamp(values.game_timestamp);

#ifdef CARLA_WITH_PNG_COMPRESSION

    if (!getPNGImages(values.images, reward)) {
        log_error("Error compressing image to PNG");
    }

#else

    if (!getBitMapImages(values.images, reward)) {
        log_error("Error encoding bitmap image");
    }

#endif // CARLA_WITH_PNG_COMPRESSION


  }

  void Protocol::LoadScene(Scene &scene, const Scene_Values &values) {

    //scene.set_number_of_cameras(values.projection_matrices.size());

    std::string positions_bytes = "";

    log_debug("------- POSITIONS--------");


    for (auto i = 0u; i < values.possible_positions.size(); ++i) {

      float x = values.possible_positions[i].x;
      float y = values.possible_positions[i].y;

      positions_bytes += GetBytes(x) + GetBytes(y);

      log_debug("x:", x, "byte size:", sizeof(float), "bytes:", GetBytes(x));
      log_debug("y:", y, "byte size:", sizeof(float), "bytes:", GetBytes(y));

    }
    log_debug("---------------");
    log_debug("Final string:", positions_bytes);
    log_debug("---------------");

    scene.set_positions(positions_bytes);

   /* std::string matrices;

    for (auto i = 0u; i < values.projection_matrices.size(); ++i) {
      for (auto e = 0u; e < 16u; ++e){

        float value = values.projection_matrices[i][e];

        char data [sizeof(float)];
        memcpy(data, &value, sizeof value);
        matrices.append(data);

      }
    }

    scene.set_projection_matrices(matrices);*/

  }

/*
  void Protocol::LoadWorld(World &world, const int modes, const int scenes) {
    world.set_modes(modes);
    world.set_scenes(scenes);
  }
*/
}
}
