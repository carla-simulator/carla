// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace road {

  class StencilType {
  public:
    static const std::string StopStencil();
    static const std::string RightArrowStencil();
    static const std::string LeftArrowStencil();
  };

} // road
} // carla
