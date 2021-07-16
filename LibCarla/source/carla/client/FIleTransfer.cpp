// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "FileTransfer.h"
namespace carla {
namespace client {

#ifdef _WIN32
      std::string FileTransfer::_filesBaseFolder = std::string(getenv("USERPROFILE")) + "/carlaCache/";
#else
      std::string FileTransfer::_filesBaseFolder = std::string(getenv("HOME")) + "/carlaCache/";
#endif

} // namespace client
} // namespace carla