// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/Exception.h>
#include <carla/client/Client.h>
#include <carla/client/FileTransfer.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>

#include <rpc/server.h>

#include <chrono>
#include <filesystem>

#ifndef _WIN32
#include <csignal>
#include <sys/resource.h>
#endif

namespace {

namespace fs = std::filesystem;
using carla::client::FileTransfer;

#ifndef _WIN32
class ScopedFileSizeLimit {
public:
  ScopedFileSizeLimit() {
    if (getrlimit(RLIMIT_FSIZE, &original) != 0) {
      carla::throw_exception(std::runtime_error("Could not read the file size limit"));
    }
    previous_signal = std::signal(SIGXFSZ, SIG_IGN);
    if (previous_signal == SIG_ERR) {
      carla::throw_exception(std::runtime_error("Could not temporarily ignore SIGXFSZ"));
    }
    auto limited = original;
    limited.rlim_cur = 1u;
    if (setrlimit(RLIMIT_FSIZE, &limited) != 0) {
      std::signal(SIGXFSZ, previous_signal);
      carla::throw_exception(std::runtime_error("Could not set the temporary file size limit"));
    }
  }

  ~ScopedFileSizeLimit() {
    setrlimit(RLIMIT_FSIZE, &original);
    std::signal(SIGXFSZ, previous_signal);
  }

private:
  struct rlimit original;
  using SignalHandler = void (*)(int);
  SignalHandler previous_signal;
};
#endif

class ClientFileCache : public testing::Test {
protected:
  void SetUp() override {
    original_cache = FileTransfer::GetFilesBaseFolder();
    directory = fs::temp_directory_path() /
        ("carla-file-cache-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    created_directory = fs::create_directory(directory);
    ASSERT_TRUE(created_directory);
    ASSERT_TRUE(FileTransfer::SetFilesBaseFolder(directory.string()));
  }

  void TearDown() override {
    FileTransfer::SetFilesBaseFolder(original_cache);
    if (created_directory) {
      std::error_code error;
      fs::remove_all(directory, error);
    }
  }

  fs::path directory;
  std::string original_cache;
  bool created_directory = false;
};

TEST_F(ClientFileCache, failed_write_stops_the_requested_file_operation) {
  ::rpc::server server("127.0.0.1", 0u);
  server.bind("request_file", [](carla::rpc::Metadata, std::string) {
    return carla::rpc::Response<std::vector<uint8_t>>{
        std::vector<uint8_t>{0u, 255u, 13u, 10u}};
  });
  server.async_run(1u);
  carla::client::Client client("127.0.0.1", server.port(), 1u);
  // A directory where the downloaded file should be makes the actual output
  // stream fail to open on both platforms; no real cache or disk is modified.
  fs::create_directories(directory / client.GetClientVersion() / "blocked.bin");
  EXPECT_THROW(client.RequestFile("blocked.bin"), std::runtime_error);
  EXPECT_TRUE(fs::is_directory(directory / client.GetClientVersion() / "blocked.bin"));
  server.stop();
}

TEST_F(ClientFileCache, successful_download_retains_every_binary_byte_control) {
  const std::vector<uint8_t> bytes{0u, 255u, 13u, 10u, 127u, 128u};
  ::rpc::server server("127.0.0.1", 0u);
  server.bind("request_file", [bytes](carla::rpc::Metadata, std::string) {
    return carla::rpc::Response<std::vector<uint8_t>>{bytes};
  });
  server.async_run(1u);
  carla::client::Client client("127.0.0.1", server.port(), 1u);
  client.RequestFile("nested/binary.bin");
  EXPECT_TRUE(FileTransfer::FileExists("nested/binary.bin"));
  EXPECT_EQ(FileTransfer::ReadFile("nested/binary.bin"), bytes);
  server.stop();
}

#ifndef _WIN32
TEST_F(ClientFileCache, failed_flush_is_reported_and_partial_file_is_not_cached) {
  bool wrote;
  {
    ScopedFileSizeLimit limit;
    wrote = FileTransfer::WriteFile("partial.bin", {1u, 2u, 3u, 4u});
  }
  // Restore the process limit and handler before any GTest assertions/output.
  EXPECT_FALSE(wrote);
  EXPECT_FALSE(FileTransfer::FileExists("partial.bin"));
}
#endif

} // namespace
