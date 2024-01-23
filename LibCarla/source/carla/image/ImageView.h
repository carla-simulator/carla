// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/image/BoostGil.h"
#include "carla/image/ColorConverter.h"
#include "carla/sensor/data/Color.h"
#include "carla/sensor/data/ImageTmpl.h"

#include <type_traits>

namespace carla {
namespace image {

  class ImageView {
  private:

    template <typename SrcViewT>
    using GrayPixelLayout = boost::gil::pixel<typename boost::gil::channel_type<SrcViewT>::type, boost::gil::gray_layout_t>;

    template <typename DstPixelT, typename ImageT>
    static auto MakeViewFromSensorImage(ImageT &image) {
      using namespace boost::gil;
      namespace sd = carla::sensor::data;
      static_assert(
          std::is_same<typename ImageT::pixel_type, sd::Color>::value,
          "Invalid pixel type");
      static_assert(
          sizeof(sd::Color) == sizeof(DstPixelT),
          "Invalid pixel size");
      return interleaved_view(
          image.GetWidth(),
          image.GetHeight(),
          reinterpret_cast<DstPixelT*>(image.data()),
          sizeof(sd::Color) * image.GetWidth()); // row length in bytes.
    }

  public:

    template <typename ImageT>
    static auto MakeView(ImageT &image) {
      return boost::gil::view(image);
    }

    static auto MakeView(sensor::data::ImageTmpl<sensor::data::Color> &image) {
      return MakeViewFromSensorImage<boost::gil::bgra8_pixel_t>(image);
    }

    static auto MakeView(const sensor::data::ImageTmpl<sensor::data::Color> &image) {
      return MakeViewFromSensorImage<boost::gil::bgra8c_pixel_t>(image);
    }

    template <typename SrcViewT, typename DstPixelT, typename CC>
    static auto MakeColorConvertedView(const SrcViewT &src, CC cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc);
    }

    template <typename SrcViewT, typename DstPixelT = GrayPixelLayout<SrcViewT>>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::Depth cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc);
    }

    template <typename SrcViewT, typename DstPixelT = GrayPixelLayout<SrcViewT>>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::LogarithmicDepth) {
      auto intermediate_view = _MakeColorConvertedView<boost::gil::gray32f_pixel_t>(src, ColorConverter::Depth());
      return _MakeColorConvertedView<DstPixelT>(intermediate_view, ColorConverter::LogarithmicLinear());
    }

    template <typename SrcViewT, typename DstPixelT = typename SrcViewT::value_type>
    static auto MakeColorConvertedView(const SrcViewT &src, ColorConverter::CityScapesPalette cc) {
      return _MakeColorConvertedView<DstPixelT>(src, cc);
    }

  private:

    template <typename SrcView, typename DstP, typename CC>
    struct color_converted_type {
    private:
      typedef boost::gil::color_convert_deref_fn<typename SrcView::const_t::reference, DstP, CC> deref_t;
      typedef typename SrcView::template add_deref<deref_t> add_ref_t;
    public:
      typedef typename add_ref_t::type type;
      static type make(const SrcView &sv, CC cc) { return add_ref_t::make(sv, deref_t(cc)); }
    };

    template <typename DstPixelT, typename SrcViewT, typename CC>
    static auto _MakeColorConvertedView(const SrcViewT &src, CC cc) {
      return color_converted_type<SrcViewT, DstPixelT, CC>::make(src, cc);
    }
  };

} // namespace image
} // namespace carla
