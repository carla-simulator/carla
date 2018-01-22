// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/gil/image.hpp>

#if __has_include("jpeglib.h")
#  define IMAGE_CONVERTER_WITH_JPEG_SUPPORT true
#  include <boost/gil/extension/io/jpeg_io.hpp>
#else
#  define IMAGE_CONVERTER_WITH_JPEG_SUPPORT false
#endif

#if __has_include("png.h")
#  define IMAGE_CONVERTER_WITH_PNG_SUPPORT true
#  ifndef png_infopp_NULL
#    define png_infopp_NULL (png_infopp)NULL
#  endif // png_infopp_NULL
#  ifndef int_p_NULL
#    define int_p_NULL (int*)NULL
#  endif // int_p_NULL
#  include <boost/gil/extension/io/png_io.hpp>
#else
#  define IMAGE_CONVERTER_WITH_PNG_SUPPORT false
#endif

#if __has_include("tiffio.h")
#  define IMAGE_CONVERTER_WITH_TIFF_SUPPORT true
#  include <boost/gil/extension/io/tiff_io.hpp>
#else
#  define IMAGE_CONVERTER_WITH_TIFF_SUPPORT false
#endif

static_assert(
        IMAGE_CONVERTER_WITH_JPEG_SUPPORT ||
        IMAGE_CONVERTER_WITH_PNG_SUPPORT ||
        IMAGE_CONVERTER_WITH_TIFF_SUPPORT,
    "No image format supported, please install at least one of libjpeg, libpng, libtiff");

namespace image_converter {

  // ===========================================================================
  // -- Format support checks --------------------------------------------------
  // ===========================================================================

  constexpr bool has_jpeg_support() {
    return IMAGE_CONVERTER_WITH_JPEG_SUPPORT;
  }

  constexpr bool has_png_support() {
    return IMAGE_CONVERTER_WITH_PNG_SUPPORT;
  }

  constexpr bool has_tiff_support() {
    return IMAGE_CONVERTER_WITH_TIFF_SUPPORT;
  }

  // ===========================================================================
  // -- readers and writers ----------------------------------------------------
  // ===========================================================================

namespace detail {

  struct jpeg_reader {
#if IMAGE_CONVERTER_WITH_JPEG_SUPPORT
    template <typename IMAGE>
    static void read_image(const char *in_filename, IMAGE &image) {
      static_assert(has_jpeg_support(), "JPEG not supported");
      boost::gil::jpeg_read_image(in_filename, image);
    }
#endif // IMAGE_CONVERTER_WITH_JPEG_SUPPORT
  };

  struct jpeg_writer {
#if IMAGE_CONVERTER_WITH_JPEG_SUPPORT
    template <typename VIEW>
    static void write_view(const char *out_filename, const VIEW &view) {
      static_assert(has_jpeg_support(), "JPEG not supported");
      boost::gil::jpeg_write_view(out_filename, view);
    }
#endif // IMAGE_CONVERTER_WITH_JPEG_SUPPORT
  };

  struct png_reader {
#if IMAGE_CONVERTER_WITH_PNG_SUPPORT
    template <typename IMAGE>
    static void read_image(const char *in_filename, IMAGE &image) {
      static_assert(has_png_support(), "PNG not supported");
      boost::gil::png_read_and_convert_image(in_filename, image);
    }
#endif // IMAGE_CONVERTER_WITH_PNG_SUPPORT
  };

  struct png_writer {
#if IMAGE_CONVERTER_WITH_PNG_SUPPORT
    template <typename VIEW>
    static void write_view(const char *out_filename, const VIEW &view) {
      static_assert(has_png_support(), "PNG not supported");
      boost::gil::png_write_view(out_filename, view);
    }
#endif // IMAGE_CONVERTER_WITH_PNG_SUPPORT
  };

  struct tiff_reader {
#if IMAGE_CONVERTER_WITH_TIFF_SUPPORT
    template <typename IMAGE>
    static void read_image(const char *in_filename, IMAGE &image) {
      static_assert(has_tiff_support(), "TIFF not supported");
      boost::gil::tiff_read_and_convert_image(in_filename, image);
    }
#endif // IMAGE_CONVERTER_WITH_TIFF_SUPPORT
  };

  struct tiff_writer {
#if IMAGE_CONVERTER_WITH_TIFF_SUPPORT
    template <typename VIEW>
    static void write_view(const char *out_filename, const VIEW &view) {
      static_assert(has_tiff_support(), "TIFF not supported");
      boost::gil::tiff_write_view(out_filename, view);
    }
#endif // IMAGE_CONVERTER_WITH_TIFF_SUPPORT
  };

} // namespace detail

  // ===========================================================================
  // -- image_io definitions ---------------------------------------------------
  // ===========================================================================

namespace detail {

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

  struct jpeg_io : public detail::image_io<has_jpeg_support(), detail::jpeg_reader, detail::jpeg_writer> {};

  struct png_io : public detail::image_io<has_png_support(), detail::png_reader, detail::png_writer> {};

  struct tiff_io : public detail::image_io<has_tiff_support(), detail::tiff_reader, detail::tiff_writer> {};

  // ===========================================================================
  // -- any_image_io -----------------------------------------------------------
  // ===========================================================================

  // ===========================================================================
  // -- image_file -------------------------------------------------------------
  // ===========================================================================

  template <
      typename IO_READER,
      typename IO_WRITER=IO_READER,
      typename IMAGE=boost::gil::rgb8_image_t>
  class image_file {
  public:

    static_assert(IO_READER::is_supported, "I/O format not supported!");

    using image_type = IMAGE;
    using reader_type = typename IO_READER::reader_type;
    using writer_type = typename IO_WRITER::writer_type;

    explicit image_file(const char *in_filename) {
      reader_type::read_image(in_filename, _image);
    }

    explicit image_file(const std::string &in_filename) :
      image_file(in_filename.c_str()) {}

    auto view() {
      return boost::gil::view(_image);
    }

    auto view() const {
      return boost::gil::const_view(_image);
    }

    template <typename PIXEL_CONVERTER>
    void apply(PIXEL_CONVERTER pixel_converter) {
      for (auto &pixel : view()) {
        pixel_converter(pixel);
      }
    }

    template <typename OTHER_FORMAT=IO_WRITER>
    void write(const char *out_filename) const {
      static_assert(OTHER_FORMAT::is_supported, "I/O format not supported!");
      OTHER_FORMAT::writer_type::write_view(out_filename, view());
    }

    template <typename OTHER_FORMAT=IO_WRITER>
    void write(const std::string &out_filename) const {
      write<OTHER_FORMAT>(out_filename.c_str());
    }

  private:

    image_type _image;
  };

} // namespace image_converter
