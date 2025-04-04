// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h"
#include "carla/Version.h"

#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
namespace carla {
namespace client {

  static std::string GetDefaultCacheLocation()
  {
    const char OverridePathEV[] = "CARLA_CACHE_DIR";
    const char HomePathEV[] =
#ifdef _WIN32
      "USERPROFILE";
#else
      "HOME";
#endif
    const char* override = std::getenv(OverridePathEV);
    if (override != NULL)
      return override;
      std::string path;
    path += std::getenv(HomePathEV);
    path += "/carlaCache";
    return path;
  }

  std::string FileTransfer::_filesBaseFolder = GetDefaultCacheLocation();

  bool FileTransfer::SetFilesBaseFolder(const std::string &path) {
    if (path.empty()) return false;

    // Check that the path ends in a slash, add it otherwise
    if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') {
      _filesBaseFolder = path + "/";
  }

    return true;
  }

  const std::string& FileTransfer::GetFilesBaseFolder() {
    return _filesBaseFolder;
  }

  bool FileTransfer::FileExists(std::string file) {
    // Check if the file exists or not
    struct stat buffer;
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version();
    fullpath += "/";
    fullpath += file;

    return (stat(fullpath.c_str(), &buffer) == 0);
  }

  bool FileTransfer::WriteFile(std::string path, std::vector<uint8_t> content) {
    std::string writePath = _filesBaseFolder;
    writePath += "/";
    writePath += ::carla::version();
    writePath += "/";
    writePath += path;

    // Validate and create the file path
    carla::FileSystem::ValidateFilePath(writePath);

    // Open the file to truncate it in binary mode
    std::ofstream out(writePath, std::ios::trunc | std::ios::binary);
    if(!out.good()) return false;

    // Write the content on and close it
    for(auto file : content) {
          out << file;
    }
    out.close();

    return true;
  }

  std::vector<uint8_t> FileTransfer::ReadFile(std::string path) {
    std::string fullpath = _filesBaseFolder;
    fullpath += "/";
    fullpath += ::carla::version();
    fullpath += "/";
    fullpath += path;
    // Read the binary file from the base folder
    std::ifstream file(fullpath, std::ios::binary);
    std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
    return content;
  }

} // namespace client
} // namespace carla
