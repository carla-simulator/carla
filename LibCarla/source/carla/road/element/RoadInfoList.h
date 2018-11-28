// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ListView.h"
#include "carla/road/element/RoadInfoVisitor.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoList {
    using SharedPtrList = std::vector<std::shared_ptr<const RoadInfo>>;

  public:

    RoadInfoList(const SharedPtrList &l) : _list(l) {}

    template <typename T>
    auto Get() const {
      auto begin = MakeRoadInfoIterator<T>(_list.begin(), _list.end());
      auto end = MakeRoadInfoIterator<T>(_list.end(), _list.end());
      return MakeListView(begin, end);
    }

  private:

    SharedPtrList _list;
  };

} // namespace element
} // namespace road
} // namespace carla
