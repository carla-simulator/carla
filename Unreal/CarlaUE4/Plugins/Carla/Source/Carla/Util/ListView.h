// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

template<typename IT>
class ListView
{
public:

  using iterator = IT;

  explicit ListView(iterator begin, iterator end) : Begin(begin), End(end) {}

  template <typename STL_CONTAINER>
  explicit ListView(STL_CONTAINER &StlContainer) :
    Begin(iterator(StlContainer.begin())),
    End(iterator(StlContainer.end())) {}

  ListView(const ListView &) = default;
  ListView &operator=(const ListView &) = delete;

  iterator begin() const {
    return Begin;
  }

  iterator end() const {
    return End;
  }

  bool empty() const {
    return Begin == End;
  }

private:

  const iterator Begin;

  const iterator End;
};
