// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#include "StencilType.h"

namespace carla {
namespace road {

  const std::string StencilType::LeftArrowStencil() {
    return "101";
  }
  const std::string StencilType::RightArrowStencil() {
    return "102";
  }
  const std::string StencilType::StopStencil() {
    return "103";
  }
}
}
