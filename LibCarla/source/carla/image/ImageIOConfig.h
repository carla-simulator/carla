// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/image/BoostGil.h"

#ifndef LIBCARLA_IMAGE_WITH_PNG_SUPPORT
#  if defined(__has_include) && __has_include("png.h")
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT true
#  else
#    define LIBCARLA_IMAGE_WITH_PNG_SUPPORT false
#  endif
#endif

#ifndef LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
#  if defined(__has_include) && __has_include("jpeglib.h")
#    define LIBCARLA_IMAGE_WITH_JPEG_SUPPORT true
#  else
#    define LIBCARLA_IMAGE_WITH_JPEG_SUPPORT false
#  endif
#endif

#ifndef LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
#  if defined(__has_include) && __has_include("tiffio.h")
#    define LIBCARLA_IMAGE_WITH_TIFF_SUPPORT true
#  else
#    define LIBCARLA_IMAGE_WITH_TIFF_SUPPORT false
#  endif
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT == true
#  ifndef png_infopp_NULL
#    define png_infopp_NULL (png_infopp)NULL
#  endif // png_infopp_NULL
#  ifndef int_p_NULL
#    define int_p_NULL (int*)NULL
#  endif // int_p_NULL
#  include <boost/gil/extension/io/png_io.hpp>
#endif

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT == true
#  include <boost/gil/extension/io/jpeg_io.hpp>
#endif

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT == true
#  include <boost/gil/extension/io/tiff_io.hpp>
#endif

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

namespace carla {
namespace image {
namespace io {

  constexpr bool has_png_support() {
    return LIBCARLA_IMAGE_WITH_PNG_SUPPORT;
  }

  constexpr bool has_jpeg_support() {
    return LIBCARLA_IMAGE_WITH_JPEG_SUPPORT;
  }

  constexpr bool has_tiff_support() {
    return LIBCARLA_IMAGE_WITH_TIFF_SUPPORT;
  }

  static_assert(has_png_support() || has_jpeg_support() || has_tiff_support(),
      "No image format supported, please compile with at least one of "
      "LIBCARLA_IMAGE_WITH_PNG_SUPPORT, LIBCARLA_IMAGE_WITH_JPEG_SUPPORT, "
      "or LIBCARLA_IMAGE_WITH_TIFF_SUPPORT");

namespace detail {

  struct jpeg_reader {
#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
    template <typename Str, typename IMAGE>
    static void read_image(Str &&in_filename, IMAGE &image) {
      static_assert(has_jpeg_support(), "JPEG not supported");
      boost::gil::jpeg_read_image(std::forward<Str>(in_filename), image);
    }
#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
  };

  struct jpeg_writer {
#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
    template <typename Str, typename VIEW>
    static void write_view(Str &&out_filename, const VIEW &view) {
      static_assert(has_jpeg_support(), "JPEG not supported");
      boost::gil::jpeg_write_view(std::forward<Str>(out_filename), view);
    }
#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
  };

  struct png_reader {
#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT
    template <typename Str, typename IMAGE>
    static void read_image(Str &&in_filename, IMAGE &image) {
      static_assert(has_png_support(), "PNG not supported");
      boost::gil::png_read_and_convert_image(std::forward<Str>(in_filename), image);
    }
#endif // LIBCARLA_IMAGE_WITH_PNG_SUPPORT
  };

  struct png_writer {
#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT
    template <typename Str, typename VIEW>
    static void write_view(Str &&out_filename, const VIEW &view) {
      static_assert(has_png_support(), "PNG not supported");
      boost::gil::png_write_view(std::forward<Str>(out_filename), view);
    }
#endif // LIBCARLA_IMAGE_WITH_PNG_SUPPORT
  };

  struct tiff_reader {
#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
    template <typename Str, typename IMAGE>
    static void read_image(Str &&in_filename, IMAGE &image) {
      static_assert(has_tiff_support(), "TIFF not supported");
      boost::gil::tiff_read_and_convert_image(std::forward<Str>(in_filename), image);
    }
#endif // LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
  };

  struct tiff_writer {
#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
    template <typename Str, typename VIEW>
    static void write_view(Str &&out_filename, const VIEW &view) {
      static_assert(has_tiff_support(), "TIFF not supported");
      boost::gil::tiff_write_view(std::forward<Str>(out_filename), view);
    }
#endif // LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
  };

  template <bool IS_SUPPORTED, typename READER, typename WRITER>
  struct image_io;

  template <typename READER, typename WRITER>
  struct image_io<false, READER, WRITER> {
    constexpr static bool is_supported = false;
  };

  template <typename READER, typename WRITER>
  struct image_io<true, READER, WRITER> {
    constexpr static bool is_supported = true;
    using reader_type = READER;
    using writer_type = WRITER;
  };

} // namespace detail

  struct jpeg : public detail::image_io<has_jpeg_support(), detail::jpeg_reader, detail::jpeg_writer> {};

  struct png : public detail::image_io<has_png_support(), detail::png_reader, detail::png_writer> {};

  struct tiff : public detail::image_io<has_tiff_support(), detail::tiff_reader, detail::tiff_writer> {};

} // namespace io
} // namespace image
} // namespace carla
