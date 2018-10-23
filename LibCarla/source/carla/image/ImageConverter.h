// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/image/ImageView.h"

namespace carla {
namespace image {

  class ImageConverter {
  public:

    template <typename SrcViewT, typename DstViewT>
    static void CopyPixels(const SrcViewT &src, DstViewT &dst) {
      boost::gil::copy_pixels(src, dst);
    }

    template <typename ColorConverter, typename MutableImageView>
    static void ConvertInPlace(
        MutableImageView &image_view,
        ColorConverter converter = ColorConverter()) {
      using DstPixelT = typename MutableImageView::value_type;
      CopyPixels(
          ImageView::MakeColorConvertedView<MutableImageView, DstPixelT>(image_view, converter),
          image_view);
    }
  };

} // namespace image
} // namespace carla
