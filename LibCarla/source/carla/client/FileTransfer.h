
// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>

namespace carla {
namespace client {

  class FileTransfer {

  public:

    FileTransfer() = delete;

    static bool SetFilesBaseFolder(const std::string &path) {
      if (path.empty()) return false;

      // Check that the path ends in a slash, add it otherwise
      if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') {
        _filesBaseFolder = path + "/";
      }

      return true;
    }

    static const std::string& GetFilesBaseFolder() {
      return _filesBaseFolder;
    }

    static bool FileExists(std::string file) {
      // Check if the file exists or not
      struct stat buffer;
      return (stat((_filesBaseFolder + file).c_str(), &buffer) == 0);
    }

    static bool WriteFile(std::string path, std::vector<uint8_t> content) {
      std::string writePath = _filesBaseFolder + path;

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

    static std::vector<uint8_t> ReadFile(std::string path) {
      // Read the binary file from the base folder
      std::ifstream file(_filesBaseFolder + path, std::ios::binary);
      std::vector<uint8_t> content(std::istreambuf_iterator<char>(file), {});
      return content;
    }

  private:

    static std::string _filesBaseFolder;

  };

    #ifdef _WIN32
      std::string FileTransfer::_filesBaseFolder = std::string(getenv("USER")) + "/carlaCache/";
    #else
      std::string FileTransfer::_filesBaseFolder = std::string(getenv("HOME")) + "/carlaCache/";
    #endif

} // namespace client
} // namespace carla