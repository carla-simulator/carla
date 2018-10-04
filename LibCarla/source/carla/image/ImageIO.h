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

    template <typename Str, typename ImageT, typename IO = io::any>
    static void ReadImage(Str &&in_filename, ImageT &image, IO = IO()) {
      IO::read_image(std::forward<Str>(in_filename), image);
    }

    template <typename Str, typename ViewT, typename IO = io::any>
    static void WriteView(Str &&out_filename, const ViewT &image_view, IO = IO()) {
      IO::write_view(std::forward<Str>(out_filename), image_view);
    }
  };

} // namespace image
} // namespace carla
