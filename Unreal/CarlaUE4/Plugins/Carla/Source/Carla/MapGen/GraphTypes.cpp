// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "GraphTypes.h"

namespace MapGen {

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG

  template <> uint32 DataIndex<'n'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'e'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'f'>::NEXT_INDEX = 0u;

#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

} // namespace MapGen
