// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"

#include <filesystem>
#include <string_view>
#include <string>

namespace carla {
namespace client {

  class FileTransfer {

  public:

    FileTransfer() = delete;

    static bool SetFilesBaseFolder(std::string_view path);

    static std::string GetFilesBaseFolder();

    static bool FileExists(std::string_view file);

    static bool WriteFile(std::string_view path, std::vector<uint8_t> content);

    static std::vector<uint8_t> ReadFile(std::string_view path);

  };

} // namespace client
} // namespace carla
