// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h"
#include "carla/Version.h"

#include <fstream>
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
    const char* override_path = std::getenv(OverridePathEV);
    if (override_path != NULL)
      return fs::path(override_path);
    fs::path path = std::getenv(HomePathEV);
    path /= "carlaCache";
    return fs::absolute(path);
  }();

  // The path arriving here comes from the server, so it must never be trusted
  // to stay inside the cache. Normalize it and drop any root, drive or leading
  // ".." components: a hostile or buggy server (e.g. one sending
  // "../../../CarlaUnreal/Content/...") can then neither write nor read
  // outside the cache root, while legitimate nested paths such as
  // "Config/Town10HD_Opt/Vehicles.xml" resolve unchanged. Escaping paths are
  // remapped instead of rejected so that both WriteFile and ReadFile agree on
  // the same in-cache location.
  static fs::path SanitizeRelativePath(std::string_view file) {
    auto normalized = fs::path(file).lexically_normal().relative_path();
    fs::path clean;
    for (const auto &part : normalized)
      if (part != ".." && part != ".")
        clean /= part;
    return clean;
  }

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
    auto fullpath = CachePath;
    fullpath /= carla::version();
    fullpath /= SanitizeRelativePath(file);
    return fs::is_regular_file(fullpath);
  }

  bool FileTransfer::WriteFile(std::string_view path, std::vector<uint8_t> content) {
    auto writePath = CachePath;
    writePath /= carla::version();
    writePath /= SanitizeRelativePath(path);

    // Validate and create the file path
    {
      auto parent = writePath.parent_path();
      if (fs::exists(parent))
      {
        if (!fs::is_directory(parent))
          return false;
      }
      else
      {
        if (!fs::create_directories(parent))
          return false;
      }
    }

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
    fullpath /= SanitizeRelativePath(path);
    std::ifstream file(fullpath, std::ios::binary);
    std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
    return content;
  }
} // namespace carla::client
