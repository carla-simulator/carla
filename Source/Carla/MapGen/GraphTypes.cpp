// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "GraphTypes.h"

namespace MapGen {

#ifdef CARLA_ROAD_GENERATOR_PRINT_OUT

  template <> uint32 DataIndex<'n'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'e'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'f'>::NEXT_INDEX = 0u;

#endif // CARLA_ROAD_GENERATOR_PRINT_OUT

} // namespace MapGen
