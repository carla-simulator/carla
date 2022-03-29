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

    static bool SetFilesBaseFolder(const std::string &path);

    static const std::string& GetFilesBaseFolder();

    static bool FileExists(std::string file);

    static bool WriteFile(std::string path, std::vector<uint8_t> content);

    static std::vector<uint8_t> ReadFile(std::string path);

  private:

    static std::string _filesBaseFolder;

  };

} // namespace client
} // namespace carla
