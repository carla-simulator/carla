// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDrive.h"

#ifndef LIBCARLA_TEST_CONTENT_FOLDER
#  error Please define LIBCARLA_TEST_CONTENT_FOLDER.
#endif

#include <carla/FileSystem.h>

#include <fstream>
#include <streambuf>

namespace util {

  std::vector<std::string> OpenDrive::GetAvailableFiles() {
    return carla::FileSystem::ListFolder(
        LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/",
        "*.xodr");
  }

  std::string OpenDrive::Load(const std::string &filename) {
    const std::string opendrive_folder = LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/";
    std::ifstream file(opendrive_folder + filename);
    return std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  }

} // namespace util
