// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h"
#include "carla/Version.h"

#include <fstream>
#include <sys/stat.h>
#include <cstdlib>

namespace fs = std::filesystem;

namespace carla::client {

  static fs::path CachePath = [] {
    constexpr char OverridePathEV[] = "CARLA_CACHE_DIR";
    constexpr char HomePathEV[] =
#ifdef _WIN32
      "USERPROFILE";
#else
      "HOME";
#endif
    auto override = std::getenv(OverridePathEV);
    if (override != NULL)
      return fs::path(override);
    auto path = fs::path(std::getenv(HomePathEV));
    path /= "carlaCache";
    return path;
  }();

  bool FileTransfer::SetFilesBaseFolder(std::string_view path) {
    if (path.empty())
      return false;
    CachePath = path;
    return true;
  }

  std::string FileTransfer::GetFilesBaseFolder() {
    return CachePath.string();
  }

  bool FileTransfer::FileExists(std::string_view file) {
    // Check if the file exists or not
    struct stat buffer;
    auto fullpath = CachePath;
    fullpath /= carla::version();
    fullpath /= file;
    return (stat(fullpath.string().c_str(), &buffer) == 0);
  }

  bool FileTransfer::WriteFile(std::string_view path, std::vector<uint8_t> content) {
    auto writePath = CachePath;
    writePath /= carla::version();
    writePath /= path;

    // Validate and create the file path
    if (!fs::exists(writePath))
      fs::create_directories(writePath);

    // Open the file to truncate it in binary mode
    std::ofstream out(
      writePath,
      std::ios::trunc | std::ios::binary);

    if (!out.good())
      return false;

    // Write the content on and close it
    for (auto file : content) {
      out << file;
    }
    out.close();

    return true;
  }

  std::vector<uint8_t> FileTransfer::ReadFile(std::string_view path) {
    auto fullpath = CachePath;
    fullpath /= carla::version();
    fullpath /= path;
    std::ifstream file(fullpath, std::ios::binary);
    std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
    return content;
  }
} // namespace carla::client
