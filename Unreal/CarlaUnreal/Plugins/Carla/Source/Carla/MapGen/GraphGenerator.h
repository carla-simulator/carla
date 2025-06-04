// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "DoublyConnectedEdgeList.h"

namespace MapGen {

  /// Random DoublyConnectedEdgeList generator.
  class GraphGenerator : private NonCopyable
  {
  public:

    /// Create a squared DoublyConnectedEdgeList of size @a SizeX times @a SizeY
    /// and generate random connections inside using fixed @a Seed.
    static TUniquePtr<DoublyConnectedEdgeList> Generate(uint32 SizeX, uint32 SizeY, int32 Seed);
  };

} // namespace MapGen
