// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/FileTransfer.h"
#include "carla/Version.h"

#include <barrier>
#include <chrono>
#include <fstream>
#include <thread>

namespace {

  namespace fs = std::filesystem;

  class FileCacheDirectories : public testing::Test {
  protected:
    void SetUp() override {
      _previous = carla::client::FileTransfer::GetFilesBaseFolder();
      _temporary_root = fs::temp_directory_path().lexically_normal();
      _folder = _temporary_root / ("carla-cache-directory-test-" + std::to_string(
          std::chrono::steady_clock::now().time_since_epoch().count()));
      ASSERT_TRUE(fs::create_directory(_folder));
      _created = true;
      ASSERT_TRUE(carla::client::FileTransfer::SetFilesBaseFolder(_folder.string()));
    }

    void TearDown() override {
      carla::client::FileTransfer::SetFilesBaseFolder(_previous);
      if (_created && _folder.parent_path() == _temporary_root &&
          _folder.filename().string().find("carla-cache-directory-test-") == 0u) {
        std::error_code error;
        fs::remove_all(_folder, error);
      }
    }

    fs::path CachePath(const std::string &file) const {
      return _folder / carla::version() / file;
    }

  private:
    std::string _previous;
    fs::path _temporary_root;
    fs::path _folder;
    bool _created = false;
  };

} // namespace

TEST_F(FileCacheDirectories, concurrent_writers_create_shared_parent) {
  constexpr size_t workers_count = 32u;
  constexpr size_t rounds = 8u;
  const std::vector<uint8_t> content{0u, 13u, 255u, 10u};
  size_t failures = 0u;
  size_t successful_retries = 0u;
  for (size_t round = 0u; round < rounds; ++round) {
    const auto directory = "group-" + std::to_string(round) + "/nested/";
    std::barrier start(static_cast<std::ptrdiff_t>(workers_count));
    std::vector<std::thread> workers;
    std::vector<uint8_t> outcomes(workers_count, 0u);
    for (size_t index = 0u; index < workers_count; ++index) {
      workers.emplace_back([&, index] {
        const auto file = directory + std::to_string(index) + ".bin";
        start.arrive_and_wait();
        outcomes[index] = carla::client::FileTransfer::WriteFile(file, content);
      });
    }
    for (auto &worker : workers) {
      worker.join();
    }
    for (size_t index = 0u; index < workers_count; ++index) {
      const auto file = directory + std::to_string(index) + ".bin";
      if (!outcomes[index]) {
        ++failures;
        if (fs::is_directory(CachePath(file).parent_path()) &&
            carla::client::FileTransfer::WriteFile(file, content)) {
          ++successful_retries;
        }
      }
      EXPECT_EQ(carla::client::FileTransfer::ReadFile(file), content);
    }
  }
  EXPECT_EQ(failures, successful_retries);
  EXPECT_EQ(failures, 0u) << successful_retries
                        << " writes succeeded when retried after directory creation";
}

TEST_F(FileCacheDirectories, existing_directory_control) {
  const std::vector<uint8_t> content{0u, 13u, 255u, 10u};
  ASSERT_TRUE(fs::create_directories(CachePath("existing/file.bin").parent_path()));
  EXPECT_TRUE(carla::client::FileTransfer::WriteFile("existing/file.bin", content));
  EXPECT_EQ(carla::client::FileTransfer::ReadFile("existing/file.bin"), content);
}

TEST_F(FileCacheDirectories, regular_file_parent_control) {
  const auto parent = CachePath("parent-file");
  ASSERT_TRUE(fs::create_directories(parent.parent_path()));
  {
    std::ofstream stream(parent, std::ios::binary);
    stream << "keep";
    ASSERT_TRUE(stream.good());
  }
  EXPECT_FALSE(carla::client::FileTransfer::WriteFile("parent-file/child.bin", {1u}));
  EXPECT_TRUE(fs::is_regular_file(parent));
  EXPECT_EQ(fs::file_size(parent), 4u);
}
