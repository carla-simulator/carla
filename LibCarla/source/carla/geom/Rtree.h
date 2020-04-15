// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace carla {
namespace geom {

  /// Rtree class working with 3D point clouds.
  /// Asociates a T element with a 3D point
  /// Useful to perform fast k-NN searches
  template <typename T, size_t Dimension = 3>
  class PointCloudRtree {
  public:

    typedef boost::geometry::model::point<float, Dimension, boost::geometry::cs::cartesian> BPoint;
    typedef std::pair<BPoint, T> TreeElement;

    void InsertElement(const BPoint &point, const T &element) {
      _rtree.insert(std::make_pair(point, element));
    }

    void InsertElement(const TreeElement &element) {
      _rtree.insert(element);
    }

    void InsertElements(const std::vector<TreeElement> &elements) {
      _rtree.insert(elements.begin(), elements.end());
    }

    /// Return nearest neighbors with a user defined filter.
    /// The filter reveices as an argument a TreeElement value and needs to
    /// return a bool to accept or reject the value
    /// [&](Rtree::TreeElement const &element){if (IsOk(element)) return true;
    /// else return false;}
    template <typename Filter>
    std::vector<TreeElement> GetNearestNeighboursWithFilter(const BPoint &point, Filter filter,
        size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      auto nearest = boost::geometry::index::nearest(point, static_cast<unsigned int>(number_neighbours));
      auto satisfies = boost::geometry::index::satisfies(filter);
      // Using explicit operator&& to workaround Bullseye coverage issue
      // https://www.bullseye.com/help/trouble-logicalOverload.html.
      _rtree.query(operator&&(nearest, satisfies), std::back_inserter(query_result));
      return query_result;
    }

    std::vector<TreeElement> GetNearestNeighbours(const BPoint &point, size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(boost::geometry::index::nearest(point, static_cast<unsigned int>(number_neighbours)),
      std::back_inserter(query_result));
      return query_result;
    }

    size_t GetTreeSize() const {
      return _rtree.size();
    }

  private:

    boost::geometry::index::rtree<TreeElement, boost::geometry::index::linear<16>> _rtree;

  };

  /// Rtree class working with 3D segment clouds.
  /// Stores a pair of T elements (one for each end of the segment)
  /// Useful to perform fast k-NN searches.
  template <typename T, size_t Dimension = 3>
  class SegmentCloudRtree {
  public:

    typedef boost::geometry::model::point<float, Dimension, boost::geometry::cs::cartesian> BPoint;
    typedef boost::geometry::model::segment<BPoint> BSegment;
    typedef std::pair<BSegment, std::pair<T, T>> TreeElement;

    void InsertElement(const BSegment &segment, const T &element_start, const T &element_end) {
      _rtree.insert(std::make_pair(segment, std::make_pair(element_start, element_end)));
    }

    void InsertElement(const TreeElement &element) {
      _rtree.insert(element);
    }

    void InsertElements(const std::vector<TreeElement> &elements) {
      _rtree.insert(elements.begin(), elements.end());
    }

    /// Return nearest neighbors with a user defined filter.
    /// The filter reveices as an argument a TreeElement value and needs to
    /// return a bool to accept or reject the value
    /// [&](Rtree::TreeElement const &element){if (IsOk(element)) return true;
    /// else return false;}
    template <typename Geometry, typename Filter>
    std::vector<TreeElement> GetNearestNeighboursWithFilter(
        const Geometry &geometry,
        Filter filter,
        size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::nearest(geometry, static_cast<unsigned int>(number_neighbours)) &&
              boost::geometry::index::satisfies(filter),
          std::back_inserter(query_result));
      return query_result;
    }

    template<typename Geometry>
    std::vector<TreeElement> GetNearestNeighbours(const Geometry &geometry, size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::nearest(geometry, static_cast<unsigned int>(number_neighbours)),
          std::back_inserter(query_result));
      return query_result;
    }

    /// Returns segments that intersec the specified geometry
    /// Warning: intersection between 3D segments is not implemented by boost
    template<typename Geometry>
    std::vector<TreeElement> GetIntersections(const Geometry &geometry) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::intersects(geometry),
          std::back_inserter(query_result));
      return query_result;
    }

    size_t GetTreeSize() const {
      return _rtree.size();
    }

  private:

    boost::geometry::index::rtree<TreeElement, boost::geometry::index::linear<16>> _rtree;

  };

} // namespace geom
} // namespace carla
