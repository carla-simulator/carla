// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/pointcloud/PointCloudIO.h"
#include "carla/sensor/data/LidarData.h"

#if LIBCARLA_IMAGE_SUPPORT_PNG
#  include "carla/image/ImageIO.h"
#endif

#include <filesystem>

namespace {

  namespace fs = std::filesystem;

  std::string Utf8(const fs::path &path) {
    const auto encoded = path.u8string();
    return {encoded.begin(), encoded.end()};
  }

  class SensorOutputUnicode : public testing::Test {
  protected:
    void SetUp() override {
      _temporary_root = fs::temp_directory_path().lexically_normal();
      _folder = _temporary_root / ("carla-sensor-unicode-test-" + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()));
      ASSERT_TRUE(fs::create_directory(_folder));
      _created = true;
    }

    void TearDown() override {
      if (_created && _folder.parent_path() == _temporary_root &&
          _folder.filename().string().find("carla-sensor-unicode-test-") == 0u) {
        std::error_code error;
        fs::remove_all(_folder, error);
      }
    }

    fs::path OutputPath(const fs::path &file) const {
      return _folder / file;
    }

    void CheckPointCloud(const fs::path &path) {
      const std::vector<carla::sensor::data::LidarDetection> points{{1.0f, 2.0f, 3.0f, 0.5f}};
      const auto saved = carla::pointcloud::PointCloudIO::SaveToDisk(
          Utf8(path), points.begin(), points.end());
      EXPECT_EQ(saved, Utf8(path));
      ASSERT_TRUE(fs::is_regular_file(path));
      std::ifstream input(path);
      const std::string contents(std::istreambuf_iterator<char>(input), {});
      EXPECT_NE(contents.find("1.0000 2.0000 3.0000 0.5000"), std::string::npos);
    }

#if LIBCARLA_IMAGE_SUPPORT_PNG
    void CheckImage(const fs::path &path) {
      boost::gil::rgb8_image_t image(2, 1);
      auto pixels = boost::gil::view(image);
      pixels(0, 0) = boost::gil::rgb8_pixel_t(12u, 34u, 56u);
      pixels(1, 0) = boost::gil::rgb8_pixel_t(78u, 90u, 123u);
      EXPECT_EQ(carla::image::ImageIO::WriteView(Utf8(path), boost::gil::const_view(image)), Utf8(path));
      ASSERT_TRUE(fs::is_regular_file(path));
      boost::gil::rgb8_image_t loaded;
      carla::image::ImageIO::ReadImage(Utf8(path), loaded);
      ASSERT_EQ(loaded.width(), 2);
      ASSERT_EQ(loaded.height(), 1);
      EXPECT_EQ(boost::gil::const_view(loaded)(0, 0), pixels(0, 0));
      EXPECT_EQ(boost::gil::const_view(loaded)(1, 0), pixels(1, 0));
    }
#endif

  private:
    fs::path _temporary_root;
    fs::path _folder;
    bool _created = false;
  };

} // namespace

TEST_F(SensorOutputUnicode, point_cloud_unicode_filename) {
  CheckPointCloud(OutputPath(fs::path(u8"測量.ply")));
}

TEST_F(SensorOutputUnicode, point_cloud_existing_unicode_parent) {
  const auto parent = OutputPath(fs::path(u8"道路"));
  ASSERT_TRUE(fs::create_directory(parent));
  CheckPointCloud(parent / "measurement.ply");
}

TEST_F(SensorOutputUnicode, point_cloud_missing_unicode_directories) {
  CheckPointCloud(OutputPath(fs::path(u8"道路/測量/點雲.ply")));
}

TEST_F(SensorOutputUnicode, point_cloud_ascii_control) {
  CheckPointCloud(OutputPath("measurement.ply"));
}

#if LIBCARLA_IMAGE_SUPPORT_PNG
TEST_F(SensorOutputUnicode, image_unicode_filename) {
  CheckImage(OutputPath(fs::path(u8"影像.png")));
}

TEST_F(SensorOutputUnicode, image_existing_unicode_parent) {
  const auto parent = OutputPath(fs::path(u8"道路"));
  ASSERT_TRUE(fs::create_directory(parent));
  CheckImage(parent / "frame.png");
}

TEST_F(SensorOutputUnicode, image_missing_unicode_directories) {
  CheckImage(OutputPath(fs::path(u8"道路/測量/影像.png")));
}

TEST_F(SensorOutputUnicode, read_image_with_unicode_path) {
  const auto ascii = OutputPath("frame.png");
  CheckImage(ascii);
  const auto unicode = OutputPath(fs::path(u8"既存影像.png"));
  fs::rename(ascii, unicode);
  boost::gil::rgb8_image_t loaded;
  carla::image::ImageIO::ReadImage(Utf8(unicode), loaded);
  ASSERT_EQ(loaded.width(), 2);
  ASSERT_EQ(loaded.height(), 1);
  EXPECT_EQ(boost::gil::const_view(loaded)(0, 0), boost::gil::rgb8_pixel_t(12u, 34u, 56u));
}

TEST_F(SensorOutputUnicode, image_ascii_control) {
  CheckImage(OutputPath("frame.png"));
}

TEST_F(SensorOutputUnicode, png_char_pointer_path_control) {
  const auto path = OutputPath("pointer.png");
  CheckImage(path);
  boost::gil::rgb8_image_t loaded;
  const auto native_path = path.string();
  carla::image::io::png::read_image(native_path.c_str(), loaded);
  ASSERT_EQ(loaded.width(), 2);
  ASSERT_EQ(loaded.height(), 1);
  EXPECT_EQ(boost::gil::const_view(loaded)(0, 0), boost::gil::rgb8_pixel_t(12u, 34u, 56u));
}
#endif
