// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/FileSystem.h"
#include "carla/pointcloud/PointCloudIO.h"
#include "carla/sensor/data/LidarData.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

  namespace fs = std::filesystem;

  class FileOutputPaths : public testing::Test {
  protected:
    void SetUp() override {
      _previous = fs::current_path();
      _temporary_root = fs::temp_directory_path().lexically_normal();
      _folder = _temporary_root / ("carla-file-output-test-" + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()));
      ASSERT_TRUE(fs::create_directory(_folder));
      _created = true;
      fs::current_path(_folder);
    }

    void TearDown() override {
      fs::current_path(_previous);
      if (_created && _folder.parent_path() == _temporary_root &&
          _folder.filename().string().find("carla-file-output-test-") == 0u) {
        std::error_code error;
        fs::remove_all(_folder, error);
      }
    }

    void CheckWritable(std::string path, const std::string &extension,
                       const fs::path &expected) {
      carla::FileSystem::ValidateFilePath(path, extension);
      EXPECT_EQ(fs::path(path), expected);
      {
        std::ofstream output(path, std::ios::binary);
        output << "sensor-output";
        ASSERT_TRUE(output.good());
      }
      EXPECT_TRUE(fs::is_regular_file(expected));
      EXPECT_EQ(fs::file_size(expected), 13u);
    }

  private:
    fs::path _previous;
    fs::path _temporary_root;
    fs::path _folder;
    bool _created = false;
  };

} // namespace

TEST_F(FileOutputPaths, basename_with_extension) {
  CheckWritable("frame.png", ".png", "frame.png");
}

TEST_F(FileOutputPaths, basename_gets_default_extension) {
  CheckWritable("cloud", ".ply", "cloud.ply");
}

TEST_F(FileOutputPaths, basename_keeps_extension_without_default) {
  CheckWritable("frame.png", "", "frame.png");
}

TEST_F(FileOutputPaths, explicit_current_directory_control) {
  CheckWritable("./frame.png", ".png", fs::path(".") / "frame.png");
}

TEST_F(FileOutputPaths, missing_nested_directory_control) {
  CheckWritable("nested/output/cloud", ".ply", fs::path("nested/output/cloud.ply"));
}

TEST_F(FileOutputPaths, absolute_directory_control) {
  const auto path = fs::current_path() / "absolute" / "frame.png";
  CheckWritable(path.string(), ".png", path);
}

TEST_F(FileOutputPaths, point_cloud_writer_accepts_basename) {
  const std::vector<carla::sensor::data::LidarDetection> points{{1.0f, 2.0f, 3.0f, 0.5f}};
  const auto path = carla::pointcloud::PointCloudIO::SaveToDisk(
      "measurement", points.begin(), points.end());
  EXPECT_EQ(path, "measurement.ply");
  std::ifstream input(path);
  const std::string contents(std::istreambuf_iterator<char>(input), {});
  EXPECT_NE(contents.find("element vertex 1\n"), std::string::npos);
  EXPECT_NE(contents.find("1.0000 2.0000 3.0000 0.5000"), std::string::npos);
}
