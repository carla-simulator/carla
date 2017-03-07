// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "GraphTypes.h"

namespace MapGen {

#ifdef WITH_EDITOR

  template <> uint32 DataIndex<'n'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'e'>::NEXT_INDEX = 0u;
  template <> uint32 DataIndex<'f'>::NEXT_INDEX = 0u;

#endif // WITH_EDITOR

} // namespace MapGen
