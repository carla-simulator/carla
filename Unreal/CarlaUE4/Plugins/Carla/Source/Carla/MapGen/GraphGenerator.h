// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
