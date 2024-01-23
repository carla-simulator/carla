// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ListView.h"
#include "carla/NonCopyable.h"

#include <iterator>
#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>

namespace carla {
namespace road {

  /// A set of elements ordered by its position on the road.
  template <typename T>
  class RoadElementSet : private MovableNonCopyable {
  public:

    using mapped_type = T;

    using key_type = double;

    RoadElementSet() = default;

    /// Explicit move constructor.
    template <typename InputTypeT>
    RoadElementSet(std::vector<InputTypeT> &&range)
      : _vec([](auto &&input) {
          static_assert(!std::is_const<InputTypeT>::value, "Input type cannot be const");
          std::sort(std::begin(input), std::end(input), LessComp());
          return decltype(_vec){
              std::make_move_iterator(std::begin(input)),
              std::make_move_iterator(std::end(input))};
        }(std::move(range))) {}

    /// Return all values from the set.
    const std::vector<mapped_type> &GetAll() const {
      return _vec;
    }

    /// Return a reversed list of elements that have key
    /// value GetDistance() <= s.
    auto GetReverseSubset(const key_type k) const {
      return MakeListView(
          std::make_reverse_iterator(std::upper_bound(_vec.begin(), _vec.end(), k, LessComp())),
          _vec.rend());
    }

    /// Return a list of elements that have a key value in the range [min_k, max_k]
    auto GetSubsetInRange(const key_type min_k, const key_type max_k) const {
      auto low_bound = (std::lower_bound(_vec.begin(), _vec.end(), min_k, LessComp()));
      auto up_bound = (std::upper_bound(_vec.begin(), _vec.end(), max_k, LessComp()));
      return MakeListView((low_bound), (up_bound));
    }

    auto GetReverseSubsetInRange(const key_type min_k, const key_type max_k) const {
      auto low_bound = (std::lower_bound(_vec.begin(), _vec.end(), min_k, LessComp()));
      auto up_bound = (std::upper_bound(low_bound, _vec.end(), max_k, LessComp()));
      return MakeListView(std::make_reverse_iterator(up_bound), std::make_reverse_iterator(low_bound));
    }

    bool empty() const {
      return _vec.empty();
    }

    size_t size() const {
      return _vec.size();
    }

    auto begin() const {
      return _vec.begin();
    }

    auto end() const {
      return _vec.end();
    }

  private:

    static key_type GetDistance(const key_type key) {
      return key;
    }

    template <typename ValueT>
    static key_type GetDistance(const ValueT &value) {
      return value.GetDistance();
    }

    template <typename ValueT>
    static key_type GetDistance(const ValueT *value) {
      return value->GetDistance();
    }

    template <typename ValueT>
    static key_type GetDistance(const std::unique_ptr<ValueT> &value) {
      return value->GetDistance();
    }

    struct LessComp {
      using is_transparent = void;
      template <typename LhsT, typename RhsT>
      bool operator()(
          const LhsT &a,
          const RhsT &b) const {
        return GetDistance(a) < GetDistance(b);
      }
    };

    std::vector<mapped_type> _vec;
  };

} // road
} // carla
