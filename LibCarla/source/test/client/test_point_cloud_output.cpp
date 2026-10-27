// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/pointcloud/PointCloudIO.h"
#include "carla/sensor/data/LidarData.h"

#include <filesystem>
#include <sstream>

#ifndef _WIN32
#  include <csignal>
#  include <sys/resource.h>
#endif

namespace {

  namespace fs = std::filesystem;
  using carla::pointcloud::PointCloudIO;
  using carla::sensor::data::LidarDetection;
  using carla::sensor::data::SemanticLidarDetection;

  struct CustomDetection {
    explicit CustomDetection(int value) : value(value) {}

    void WritePlyHeaderInfo(std::ostream &output) const {
      output << "property int custom";
    }

    void WriteDetection(std::ostream &output) const {
      output << value;
    }

    int value;
  };

  struct DefaultCustomDetection : CustomDetection {
    DefaultCustomDetection() : CustomDetection(0) {}
  };

  class PointCloudOutput : public testing::Test {
  protected:
    void SetUp() override {
      _temporary_root = fs::temp_directory_path().lexically_normal();
      _folder = _temporary_root / ("carla-point-cloud-output-test-" + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()));
      ASSERT_TRUE(fs::create_directory(_folder));
      _created = true;
    }

    void TearDown() override {
      if (_created && _folder.parent_path() == _temporary_root &&
          _folder.filename().string().find("carla-point-cloud-output-test-") == 0u) {
        std::error_code error;
        fs::remove_all(_folder, error);
      }
    }

    fs::path OutputPath(const std::string &file) const {
      return _folder / file;
    }

    template <typename Detection>
    std::string Save(const std::string &name, const std::vector<Detection> &points) {
      const auto path = PointCloudIO::SaveToDisk(
          OutputPath(name).string(), points.begin(), points.end());
      EXPECT_EQ(fs::path(path).extension(), ".ply");
      std::ifstream input(path);
      EXPECT_TRUE(input.good());
      return {std::istreambuf_iterator<char>(input), {}};
    }

  private:
    fs::path _temporary_root;
    fs::path _folder;
    bool _created = false;
  };

#if !defined(_WIN32) && !defined(LIBCARLA_NO_EXCEPTIONS)
  class ZeroFileSizeLimit {
  public:
    bool Enable() {
      if (getrlimit(RLIMIT_FSIZE, &_previous) != 0) {
        return false;
      }
      _handler = std::signal(SIGXFSZ, SIG_IGN);
      if (_handler == SIG_ERR) {
        return false;
      }
      _has_handler = true;
      const rlimit limit{0u, _previous.rlim_max};
      _active = setrlimit(RLIMIT_FSIZE, &limit) == 0;
      return _active;
    }

    ~ZeroFileSizeLimit() {
      if (_active) {
        setrlimit(RLIMIT_FSIZE, &_previous);
      }
      if (_has_handler) {
        std::signal(SIGXFSZ, _handler);
      }
    }

  private:
    rlimit _previous{};
    void (*_handler)(int) = SIG_DFL;
    bool _has_handler = false;
    bool _active = false;
  };
#endif

} // namespace

TEST_F(PointCloudOutput, empty_lidar_measurement) {
  const auto contents = Save("empty", std::vector<LidarDetection>{});
  EXPECT_NE(contents.find("element vertex 0\n"), std::string::npos);
  EXPECT_NE(contents.find("property float32 I\nend_header\n"), std::string::npos);
}

TEST_F(PointCloudOutput, empty_semantic_lidar_measurement) {
  const auto contents = Save("empty-semantic", std::vector<SemanticLidarDetection>{});
  EXPECT_NE(contents.find("element vertex 0\n"), std::string::npos);
  EXPECT_NE(contents.find("property uint32 ObjTag\nend_header\n"), std::string::npos);
}

TEST_F(PointCloudOutput, empty_raw_pointer_range) {
  const LidarDetection *points = nullptr;
  std::ostringstream output;
  PointCloudIO::Dump(output, points, points);
  EXPECT_NE(output.str().find("element vertex 0\n"), std::string::npos);
  EXPECT_NE(output.str().find("property float32 I\nend_header\n"), std::string::npos);
}

TEST_F(PointCloudOutput, populated_lidar_control) {
  const auto contents = Save("lidar", std::vector<LidarDetection>{{1.0f, 2.0f, 3.0f, 0.5f}});
  EXPECT_NE(contents.find("element vertex 1\n"), std::string::npos);
  EXPECT_NE(contents.find("1.0000 2.0000 3.0000 0.5000\n"), std::string::npos);
}

TEST_F(PointCloudOutput, populated_semantic_lidar_control) {
  const auto contents = Save("semantic",
      std::vector<SemanticLidarDetection>{{1.0f, 2.0f, 3.0f, 0.5f, 7u, 10u}});
  EXPECT_NE(contents.find("element vertex 1\n"), std::string::npos);
  EXPECT_NE(contents.find("1.0000 2.0000 3.0000 0.5000 7 10\n"), std::string::npos);
}

TEST_F(PointCloudOutput, populated_pointer_range_control) {
  const LidarDetection points[]{{1.0f, 2.0f, 3.0f, 0.5f}};
  std::ostringstream output;
  PointCloudIO::Dump(output, std::begin(points), std::end(points));
  EXPECT_NE(output.str().find("element vertex 1\n"), std::string::npos);
  EXPECT_NE(output.str().find("1.0000 2.0000 3.0000 0.5000\n"), std::string::npos);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST_F(PointCloudOutput, output_directory_is_reported_as_failure) {
  const auto path = OutputPath("directory.ply");
  ASSERT_TRUE(fs::create_directory(path));
  const std::vector<LidarDetection> points{{1.0f, 2.0f, 3.0f, 0.5f}};
  EXPECT_THROW(PointCloudIO::SaveToDisk(path.string(), points.begin(), points.end()),
               std::runtime_error);
  EXPECT_TRUE(fs::is_directory(path));
}

#ifndef _WIN32
TEST_F(PointCloudOutput, failed_flush_is_reported_as_failure) {
  const std::vector<LidarDetection> points{{1.0f, 2.0f, 3.0f, 0.5f}};
  const auto path = OutputPath("limited.ply").string();
  ZeroFileSizeLimit limit;
  ASSERT_TRUE(limit.Enable());
  EXPECT_THROW(PointCloudIO::SaveToDisk(path, points.begin(), points.end()), std::runtime_error);
}
#endif
#endif

TEST_F(PointCloudOutput, custom_instance_schema_control) {
  const auto contents = Save("custom", std::vector<CustomDetection>{CustomDetection(7)});
  EXPECT_NE(contents.find("property int custom\nend_header\n7\n"), std::string::npos);
}

TEST_F(PointCloudOutput, empty_default_constructible_custom_schema) {
  const auto contents = Save("custom-empty", std::vector<DefaultCustomDetection>{});
  EXPECT_NE(contents.find("element vertex 0\n"), std::string::npos);
  EXPECT_NE(contents.find("property int custom\nend_header\n"), std::string::npos);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST_F(PointCloudOutput, empty_custom_range_without_schema_reports_error) {
  const std::vector<CustomDetection> points;
  std::ostringstream output;
  EXPECT_THROW(PointCloudIO::Dump(output, points.begin(), points.end()), std::invalid_argument);
}
#endif
