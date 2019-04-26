// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"
#include "carla/Logging.h"
#include "carla/StringUtil.h"
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
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wignored-qualifiers"
#    pragma clang diagnostic ignored "-Wparentheses"
#  endif
#    include <boost/gil/extension/io/png.hpp>
#  if defined(__clang__)
#    pragma clang diagnostic pop
#  endif
#endif

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT == true
#  include <boost/gil/extension/io/jpeg.hpp>
#endif

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT == true
#  include <boost/gil/extension/io/tiff.hpp>
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

  template <typename ViewT, typename IOTag>
  struct is_write_supported {
    static constexpr bool value = boost::gil::is_write_supported<typename boost::gil::get_pixel_type<ViewT>::type, IOTag>::value;
  };

  struct io_png {

    static constexpr bool is_supported = has_png_support();

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT

    static constexpr const char *get_default_extension() {
      return "png";
    }

    template <typename Str>
    static bool match_extension(const Str &str) {
      return StringUtil::EndsWith(str, get_default_extension());
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::png_tag());
    }

    template <typename Str, typename ViewT>
    static void write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::png_tag());
    }

#endif // LIBCARLA_IMAGE_WITH_PNG_SUPPORT
  };

  struct io_jpeg {

    static constexpr bool is_supported = has_jpeg_support();

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT

    static constexpr const char *get_default_extension() {
      return "jpeg";
    }

    template <typename Str>
    static bool match_extension(const Str &str) {
      return StringUtil::EndsWith(str, get_default_extension()) ||
             StringUtil::EndsWith(str, "jpg");
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_image(std::forward<Str>(in_filename), image, boost::gil::jpeg_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::jpeg_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::jpeg_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view),
          boost::gil::jpeg_tag());
    }

#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
  };

  struct io_tiff {

    static constexpr bool is_supported = has_tiff_support();

#if LIBCARLA_IMAGE_WITH_TIFF_SUPPORT

    static constexpr const char *get_default_extension() {
      return "tiff";
    }

    template <typename Str>
    static bool match_extension(const Str &str) {
      return StringUtil::EndsWith(str, get_default_extension());
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    static typename std::enable_if<!is_write_supported<ViewT, boost::gil::tiff_tag>::value>::type
    write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view),
          boost::gil::tiff_tag());
    }

#endif // LIBCARLA_IMAGE_WITH_TIFF_SUPPORT
  };

  struct io_resolver {

    template <typename IO, typename Str>
    static typename std::enable_if<IO::is_supported, bool>::type match_extension(const Str &str) {
      return IO::match_extension(str);
    }

    template <typename IO, typename Str>
    static typename std::enable_if<!IO::is_supported, bool>::type match_extension(const Str &) {
      return false;
    }

    template <typename IO, typename Str, typename... Args>
    static typename std::enable_if<IO::is_supported>::type read_image(const Str &path, Args &&... args) {
      log_debug("reading", path, "as", IO::get_default_extension());
      IO::read_image(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<!IO::is_supported>::type read_image(Args &&...) {
      DEBUG_ASSERT(false);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<IO::is_supported>::type write_view(std::string &path, Args &&... args) {
      FileSystem::ValidateFilePath(path, IO::get_default_extension());
      log_debug("writing", path, "as", IO::get_default_extension());
      IO::write_view(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    static typename std::enable_if<!IO::is_supported>::type write_view(Args &&...) {
      DEBUG_ASSERT(false);
    }
  };

  template <typename... IOs>
  struct io_impl;

  template <typename IO>
  struct io_impl<IO> {
    constexpr static bool is_supported = IO::is_supported;

    template <typename... Args>
    static void read_image(Args &&... args) {
      io_resolver::read_image<IO>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void write_view(Args &&... args) {
      io_resolver::write_view<IO>(std::forward<Args>(args)...);
    }

    template <typename Str, typename... Args>
    static bool try_read_image(const Str &filename, Args &&... args) {
      if (io_resolver::match_extension<IO>(filename)) {
        io_resolver::read_image<IO>(filename, std::forward<Args>(args)...);
        return true;
      }
      return false;
    }

    template <typename Str, typename... Args>
    static bool try_write_view(Str &filename, Args &&... args) {
      if (io_resolver::match_extension<IO>(filename)) {
        io_resolver::write_view<IO>(filename, std::forward<Args>(args)...);
        return true;
      }
      return false;
    }
  };

  template <typename IO, typename... IOs>
  struct io_impl<IO, IOs...> {
  private:
    using self = io_impl<IO>;
    using recursive = io_impl<IOs...>;
  public:

    constexpr static bool is_supported = self::is_supported || recursive::is_supported;

    template <typename... Args>
    static void read_image(Args &... args) {
      if (!recursive::try_read_image(args...)) {
        self::read_image(args...);
      }
    }

    template <typename... Args>
    static bool try_read_image(Args &... args) {
      return recursive::try_read_image(args...) || self::try_read_image(args...);
    }

    template <typename... Args>
    static void write_view(Args &... args) {
      if (!recursive::try_write_view(args...)) {
        self::write_view(args...);
      }
    }

    template <typename... Args>
    static bool try_write_view(Args &... args) {
      return recursive::try_write_view(args...) || self::try_write_view(args...);
    }
  };

  template <typename DefaultIO, typename... IOs>
  struct io_any : detail::io_impl<DefaultIO, IOs...> {
    static_assert(DefaultIO::is_supported, "Default IO needs to be supported.");
  };

} // namespace detail

  struct png : detail::io_impl<detail::io_png> {};

  struct jpeg : detail::io_impl<detail::io_jpeg> {};

  struct tiff : detail::io_impl<detail::io_tiff> {};

#if LIBCARLA_IMAGE_WITH_PNG_SUPPORT

  struct any : detail::io_any<detail::io_png, detail::io_tiff, detail::io_jpeg> {};

#elif LIBCARLA_IMAGE_WITH_TIFF_SUPPORT

  struct any : detail::io_any<detail::io_tiff, detail::io_jpeg> {};

#else // Then for sure this one is available.

  struct any : detail::io_any<detail::io_jpeg> {};

#endif

} // namespace io
} // namespace image
} // namespace carla
