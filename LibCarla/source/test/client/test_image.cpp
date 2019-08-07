// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>

#include <memory>

template <typename ViewT, typename PixelT>
struct TestImage {
  TestImage(TestImage &&) = default;
  using pixel_type = PixelT;
  std::unique_ptr<PixelT[]> data;
  ViewT view;
};

template <typename PixelT>
static auto MakeTestImage(size_t width, size_t height) {
  auto data = std::make_unique<PixelT[]>(sizeof(PixelT) * width * height);
  auto view = boost::gil::interleaved_view(
      width,
      height,
      reinterpret_cast<PixelT*>(data.get()),
      static_cast<long>(sizeof(PixelT) * width));
  return TestImage<decltype(view), PixelT>{std::move(data), view};
}

#ifndef PLATFORM_WINDOWS
TEST(image, support) {
  using namespace carla::image::io;
  carla::logging::log("PNG  support =", has_png_support());
  carla::logging::log("JPEG support =", has_jpeg_support());
  carla::logging::log("TIFF support =", has_tiff_support());
}
#endif // PLATFORM_WINDOWS

TEST(image, depth) {
#ifndef NDEBUG
  carla::log_info("This test only happens in release (too slow).");
#else
  using namespace boost::gil;
  using namespace carla::image;

  constexpr auto width = 256 * 256 * 256;
  constexpr auto height = 1u;

  auto img_bgra8 = MakeTestImage<bgra8_pixel_t>(width, height);
  auto img_gray8 = MakeTestImage<gray8_pixel_t>(width, height);

  auto depth_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::Depth());
  auto ldepth_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::LogarithmicDepth());

  auto p = *depth_view.begin();
  static_assert(std::is_same<decltype(p), gray8_pixel_t>::value, "Not the pixel I was looking for!");
  auto lp = *ldepth_view.begin();
  static_assert(std::is_same<decltype(lp), gray8_pixel_t>::value, "Not the pixel I was looking for!");

  ASSERT_EQ(depth_view.size(), img_bgra8.view.size());
  ASSERT_EQ(ldepth_view.size(), img_bgra8.view.size());

  {
    auto it_bgra8 = img_bgra8.view.begin();
    auto it_gray8 = img_gray8.view.begin();

    for (auto r = 0u; r < 256u; ++r) {
      for (auto g = 0u; g < 256u; ++g) {
        for (auto b = 0u; b < 256u; ++b) {
          decltype(img_bgra8)::pixel_type &p_bgra8 = *it_bgra8;
          decltype(img_gray8)::pixel_type &p_gray8 = *it_gray8;
          get_color(p_bgra8, red_t()) = static_cast<uint8_t>(r);
          get_color(p_bgra8, green_t()) = static_cast<uint8_t>(g);
          get_color(p_bgra8, blue_t()) = static_cast<uint8_t>(b);
          const float depth = r + (g * 256) + (b  * 256 * 256);
          const float normalized = depth / static_cast<float>(256 * 256 * 256 - 1);
          p_gray8[0] = static_cast<uint8_t>(255.0 * normalized);

          ++it_bgra8;
          ++it_gray8;
        }
      }
    }
  }

  auto img_copy = MakeTestImage<bgra8_pixel_t>(width, height);
  ImageConverter::CopyPixels(img_bgra8.view, img_copy.view);
  ImageConverter::ConvertInPlace(img_copy.view, ColorConverter::LogarithmicDepth());

  {
    auto it_gray8 = img_gray8.view.begin();
    auto it_depth = depth_view.begin();
    auto it_ldepth = ldepth_view.begin();
    auto it_copy = img_copy.view.begin();

    for (auto i = 0u; i < width; ++i) {
      auto p_gray8 = *it_gray8;
      auto p_depth = *it_depth;
      auto p_ldepth = *it_ldepth;
      auto p_copy = *it_copy;
      ASSERT_NEAR(int(p_depth[0]), int(p_gray8[0]), 1)
          << "at XY(" << i << ",0)";
      decltype(p_copy) ld;
      color_convert(p_ldepth, ld);
      ASSERT_EQ(ld, p_copy)
          << "at XY(" << i << ",0)";
      ++it_gray8;
      ++it_depth;
      ++it_ldepth;
      ++it_copy;
    }
  }
#endif // NDEBUG
}

TEST(image, semantic_segmentation) {
  using namespace boost::gil;
  using namespace carla::image;

  constexpr auto width = CityScapesPalette::GetNumberOfTags();
  constexpr auto height = 1u;

  auto img_bgra8 = MakeTestImage<bgra8_pixel_t>(width, height);
  auto img_ss = MakeTestImage<rgb8_pixel_t>(width, height);

  auto semseg_view = ImageView::MakeColorConvertedView(
      img_bgra8.view,
      ColorConverter::CityScapesPalette());

  auto p = *semseg_view.begin();
  static_assert(std::is_same<decltype(p), bgra8_pixel_t>::value, "Not the pixel I was looking for!");

  ASSERT_EQ(semseg_view.size(), img_bgra8.view.size());

  {
    auto it_bgra8 = img_bgra8.view.begin();
    auto it_ss = img_ss.view.begin();

    for (auto tag = 0u; tag < width; ++tag) {
      decltype(img_bgra8)::pixel_type &p_bgra8 = *it_bgra8;
      get_color(p_bgra8, red_t()) = static_cast<uint8_t>(tag);
      get_color(p_bgra8, green_t()) = 0u;
      get_color(p_bgra8, blue_t()) = 0u;
      decltype(img_ss)::pixel_type &p_ss = *it_ss;
      auto color = CityScapesPalette::GetColor(static_cast<uint8_t>(tag));
      get_color(p_ss, red_t()) =  color[0u];
      get_color(p_ss, green_t()) = color[1u];
      get_color(p_ss, blue_t()) = color[2u];
      ++it_bgra8;
      ++it_ss;
    }
  }

  auto img_copy = MakeTestImage<rgba8_pixel_t>(width, height);
  ImageConverter::CopyPixels(img_bgra8.view, img_copy.view);
  ImageConverter::ConvertInPlace(img_copy.view, ColorConverter::CityScapesPalette());

  {
    auto it_ss = img_ss.view.begin();
    auto it_ssv = semseg_view.begin();
    auto it_copy = img_copy.view.begin();

    for (auto i = 0u; i < width; ++i) {
      auto p_ssv = *it_ssv;
      auto p_copy = *it_copy;
      auto _p_ss = *it_ss;
      decltype(p_ssv) p_ss;
      color_convert(_p_ss, p_ss);
      ASSERT_EQ(p_ssv, p_ss)
          << "at XY(" << i << ",0)";
      decltype(p_copy) css;
      color_convert(p_ssv, css);
      ASSERT_EQ(p_ssv, p_copy)
          << "at XY(" << i << ",0)";
      ++it_ss;
      ++it_ssv;
      ++it_copy;
    }
  }
}
