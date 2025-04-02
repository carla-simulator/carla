// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"
#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/image/BoostGil.h"

#include <string_view>

#ifndef LIBCARLA_IMAGE_SUPPORT_PNG
  #define LIBCARLA_IMAGE_SUPPORT_PNG false
#endif

#ifndef LIBCARLA_IMAGE_SUPPORT_JPEG
  #define LIBCARLA_IMAGE_SUPPORT_JPEG false
#endif

#ifndef LIBCARLA_IMAGE_SUPPORT_TIFF
  #define LIBCARLA_IMAGE_SUPPORT_TIFF false
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#if LIBCARLA_IMAGE_SUPPORT_PNG
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

#if LIBCARLA_IMAGE_SUPPORT_JPEG
#  include <boost/gil/extension/io/jpeg.hpp>
#endif

#if LIBCARLA_IMAGE_SUPPORT_TIFF
#  include <boost/gil/extension/io/tiff.hpp>
#endif

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

namespace carla::image::io {

  constexpr bool has_png_support() {
    return LIBCARLA_IMAGE_SUPPORT_PNG;
  }

  constexpr bool has_jpeg_support() {
    return LIBCARLA_IMAGE_SUPPORT_JPEG;
  }

  constexpr bool has_tiff_support() {
    return LIBCARLA_IMAGE_SUPPORT_TIFF;
  }

  static_assert(
    has_png_support() || has_jpeg_support() || has_tiff_support(),
    "No image format supported, please compile with at least one of "
    "LIBCARLA_IMAGE_SUPPORT_PNG, "
    "LIBCARLA_IMAGE_SUPPORT_JPEG "
    "or LIBCARLA_IMAGE_SUPPORT_TIFF");

namespace detail {

  template <typename ViewType, typename IOTag>
  constexpr bool is_write_supported = boost::gil::is_write_supported<
    typename boost::gil::get_pixel_type<ViewType>::type,
    IOTag>::value;

  template <typename ViewType, typename IOTag>
  constexpr bool is_read_supported = boost::gil::is_read_supported<
    typename boost::gil::get_pixel_type<ViewType>::type,
    IOTag>::value;
  
  struct io_png {

    static constexpr bool is_supported = has_png_support();

#if LIBCARLA_IMAGE_SUPPORT_PNG

    using io_tag = boost::gil::png_tag;

    static constexpr std::string_view get_default_extension() {
      return ".png";
    }

    template <typename Str>
    static bool match_extension(Str&& str) {
      auto sv = std::string_view(str);
      return sv.ends_with(get_default_extension());
    }

    template <typename Str, typename ImageT>
    static void read_image(Str&& in_filename, ImageT &image) {
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::png_tag());
    }

    template <typename Str, typename ViewT>
    requires (is_write_supported<ViewT, boost::gil::png_tag>)
    static void write_view(Str&& out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::png_tag());
    }

#endif // LIBCARLA_IMAGE_SUPPORT_PNG
  };

  struct io_jpeg {

    static constexpr bool is_supported = has_jpeg_support();

#if LIBCARLA_IMAGE_WITH_JPEG_SUPPORT

    using io_tag = boost::gil::jpeg_tag;

    static constexpr std::string_view get_default_extension() {
      return ".jpeg";
    }

    template <typename Str>
    static bool match_extension(Str&& str) {
      auto sv = std::string_view(str);
      return sv.ends_with(get_default_extension()) ||
             sv.ends_with("jpg");
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_image(std::forward<Str>(in_filename), image, boost::gil::jpeg_tag());
    }

    template <typename Str, typename ViewT>
    requires (is_write_supported<ViewT, boost::gil::jpeg_tag>)
    static void write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::jpeg_tag());
    }

    template <typename Str, typename ViewT>
    requires (!is_write_supported<ViewT, boost::gil::jpeg_tag>)
    static void write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view),
          boost::gil::jpeg_tag());
    }

#endif // LIBCARLA_IMAGE_WITH_JPEG_SUPPORT
  };

  struct io_tiff {

    static constexpr bool is_supported = has_tiff_support();

#if LIBCARLA_IMAGE_SUPPORT_TIFF

    using io_tag = boost::gil::tiff_tag;

    static constexpr std::string_view get_default_extension() {
      return ".tiff";
    }

    template <typename Str>
    static bool match_extension(const Str &str) {
      auto sv = std::string_view(str);
      return sv.ends_with(get_default_extension());
    }

    template <typename Str, typename ImageT>
    static void read_image(Str &&in_filename, ImageT &image) {
      boost::gil::read_and_convert_image(std::forward<Str>(in_filename), image, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    requires (is_write_supported<ViewT, boost::gil::tiff_tag>)
    static void write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(std::forward<Str>(out_filename), view, boost::gil::tiff_tag());
    }

    template <typename Str, typename ViewT>
    requires (!is_write_supported<ViewT, boost::gil::tiff_tag>)
    static void write_view(Str &&out_filename, const ViewT &view) {
      boost::gil::write_view(
          std::forward<Str>(out_filename),
          boost::gil::color_converted_view<boost::gil::rgb8_pixel_t>(view),
          boost::gil::tiff_tag());
    }

#endif // LIBCARLA_IMAGE_SUPPORT_TIFF
  };

  struct io_resolver {

    template <typename IO, typename Str>
    static bool match_extension(const Str &str) {
      if constexpr (IO::is_supported)
        return IO::match_extension(str);
      else
        return false;
    }

    template <typename IO, typename Str, typename... Args>
    requires (IO::is_supported)
    static void read_image(const Str &path, Args &&... args) {
      log_debug("reading", path, "as", IO::get_default_extension());
      IO::read_image(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    requires (!IO::is_supported)
    static void read_image(Args &&...) {
      DEBUG_ASSERT(false);
    }

    template <typename IO, typename... Args>
    requires (IO::is_supported)
    static void write_view(std::string &path, Args &&... args) {
      FileSystem::ValidateFilePath(path, IO::get_default_extension());
      log_debug("writing", path, "as", IO::get_default_extension());
      IO::write_view(path, std::forward<Args>(args)...);
    }

    template <typename IO, typename... Args>
    requires (!IO::is_supported)
    static void write_view(Args &&...) {
      DEBUG_ASSERT(false);
    }
  };

  template <typename... IOs>
  class io_implementation
  {
    template <typename F>
    static constexpr void io_foreach_with_break(F&& callback)
    {
      bool done = false;
      ([&]()
      {
        if (!done)
          done = callback(IOs());
      }(), ...);
    }

  public:

    static constexpr bool is_supported = (IOs::is_supported || ...);
    
    template <typename S, typename V, typename... T>
    static void try_read_image(S&& filename, V&& view, T&&... args)
    {
      using ViewType = std::remove_cvref_t<V>;
      io_foreach_with_break([&](auto io)
      {
        using IO = decltype(io);
        if constexpr (IO::is_supported)
        {
          if constexpr (is_read_supported<ViewType, typename IO::io_tag>)
          {
            if (io_resolver::match_extension<IO>(filename))
            {
              io_resolver::read_image<IO>(std::forward<S>(filename), std::forward<V>(view), std::forward<T>(args)...);
              return true;
            }
          }
        }
        return false;
      });
    }

    template <typename V, typename... T>
    static void read_image(V&& view, T&&... args)
    {
      using ViewType = std::remove_cvref_t<V>;
      io_foreach_with_break([&](auto io)
      {
        using IO = decltype(io);
        if constexpr (IO::is_supported)
        {
          if constexpr (is_read_supported<ViewType, typename IO::io_tag>)
          {
            io_resolver::read_image<IO>(std::forward<V>(view), std::forward<T>(args)...);
            return true;
          }
        }
        return false;
      });
    }
    
    template <typename S, typename V, typename... T>
    static void try_write_view(S&& filename, V&& view, T&&... args)
    {
      using ViewType = std::remove_cvref_t<V>;
      io_foreach_with_break([&](auto io)
      {
        using IO = decltype(io);
        if constexpr (IO::is_supported)
        {
          if constexpr (is_write_supported<ViewType, typename IO::io_tag>)
          {
            if (io_resolver::match_extension<IO>(filename))
            {
              io_resolver::write_view<IO>(std::forward<S>(filename), std::forward<V>(view), std::forward<T>(args)...);
              return true;
            }
          }
        }
        return false;
      });
    }
    
    template <typename V, typename... T>
    static void write_view(V&& view, T&&... args)
    {
      using ViewType = std::remove_cvref_t<V>;
      io_foreach_with_break([&](auto io)
      {
        using IO = decltype(io);
        if constexpr (IO::is_supported)
        {
          if constexpr (is_write_supported<ViewType, typename IO::io_tag>)
          {
            io_resolver::write_view<IO>(std::forward<V>(view), std::forward<T>(args)...);
            return true;
          }
        }
        return false;
      });
    }
  };

  template <typename DefaultIO, typename... IOs>
  struct io_any : detail::io_implementation<DefaultIO, IOs...> {
    static_assert(DefaultIO::is_supported, "Default IO needs to be supported.");
  };

} // namespace detail

  struct png : detail::io_implementation<detail::io_png> {};

  struct jpeg : detail::io_implementation<detail::io_jpeg> {};

  struct tiff : detail::io_implementation<detail::io_tiff> {};

#if LIBCARLA_IMAGE_SUPPORT_PNG

  struct any : detail::io_any<detail::io_png, detail::io_tiff, detail::io_jpeg> {};

#elif LIBCARLA_IMAGE_SUPPORT_TIFF

  struct any : detail::io_any<detail::io_tiff, detail::io_jpeg> {};

#else // Then for sure this one is available.

  struct any : detail::io_any<detail::io_jpeg> {};

#endif

} // namespace carla::image::io
