// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/image/ImageIOConfig.h"

namespace carla {
namespace image {

  class ImageIO {
  public:

    template <typename ImageT, typename IO = io::any>
    static void ReadImage(const std::string &in_filename, ImageT &image, IO = IO()) {
      IO::read_image(in_filename, image);
    }

    template <typename ViewT, typename IO = io::any>
    static std::string WriteView(std::string out_filename, const ViewT &image_view, IO = IO()) {
      IO::write_view(out_filename, image_view);
      return out_filename;
    }
  };

} // namespace image
} // namespace carla
