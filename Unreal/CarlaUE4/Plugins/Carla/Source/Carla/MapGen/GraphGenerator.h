// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "DoublyConnectedEdgeList.h"

namespace MapGen {

  /// Random DoublyConnectedEdgeList generator.
  class CARLA_API GraphGenerator : private NonCopyable
  {
  public:

    /// Create a squared DoublyConnectedEdgeList of size @a SizeX times @a SizeY
    /// and generate random connections inside using fixed @a Seed.
    static TUniquePtr<DoublyConnectedEdgeList> Generate(uint32 SizeX, uint32 SizeY, int32 Seed);
  };

} // namespace MapGen
