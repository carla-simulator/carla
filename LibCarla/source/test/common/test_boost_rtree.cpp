// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression guard for boost::geometry::index::rtree after the Boost 1.84 ->
// 1.90 upgrade.  CARLA relies on rtree for road/lane spatial queries in
// carla::road and carla::geom; this suite exercises the public surface
// (insertion, nearest-neighbor, box queries) that LibCarla depends on.

#include "test.h"
#include "carla/geom/Rtree.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using Point2D = bg::model::point<double, 2, bg::cs::cartesian>;
using Point3D = bg::model::point<double, 3, bg::cs::cartesian>;
using Box2D = bg::model::box<Point2D>;
using Value2D = std::pair<Point2D, std::size_t>;
using Value3D = std::pair<Point3D, std::size_t>;

TEST(BoostRtree, InsertAndNearestNeighbour2D) {
  bgi::rtree<Value2D, bgi::quadratic<16>> tree;
  tree.insert({Point2D{0.0, 0.0}, 0u});
  tree.insert({Point2D{1.0, 1.0}, 1u});
  tree.insert({Point2D{10.0, 10.0}, 2u});
  tree.insert({Point2D{-5.0, -5.0}, 3u});

  std::vector<Value2D> result;
  tree.query(bgi::nearest(Point2D{0.9, 1.1}, 1), std::back_inserter(result));
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result.front().second, 1u);
}

TEST(BoostRtree, InsertAndNearestNeighbour3D) {
  bgi::rtree<Value3D, bgi::quadratic<16>> tree;
  tree.insert({Point3D{0.0, 0.0, 0.0}, 0u});
  tree.insert({Point3D{1.0, 1.0, 1.0}, 1u});
  tree.insert({Point3D{2.0, 2.0, 2.0}, 2u});
  tree.insert({Point3D{10.0, 10.0, 10.0}, 3u});
  tree.insert({Point3D{-5.0, -5.0, -5.0}, 4u});

  std::vector<Value3D> result;
  tree.query(bgi::nearest(Point3D{2.1, 2.2, 1.9}, 1), std::back_inserter(result));
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result.front().second, 2u);
}

TEST(BoostRtree, WithinBoxQuery2D) {
  bgi::rtree<Value2D, bgi::quadratic<16>> tree;
  tree.insert({Point2D{0.5, 0.5}, 0u});
  tree.insert({Point2D{2.0, 2.0}, 1u});
  tree.insert({Point2D{5.0, 5.0}, 2u});
  tree.insert({Point2D{-1.0, -1.0}, 3u});

  const Box2D box{Point2D{0.0, 0.0}, Point2D{3.0, 3.0}};
  std::vector<Value2D> result;
  tree.query(bgi::within(box), std::back_inserter(result));
  ASSERT_EQ(result.size(), 2u);

  std::vector<std::size_t> ids;
  for (const auto &entry : result) {
    ids.push_back(entry.second);
  }
  std::sort(ids.begin(), ids.end());
  EXPECT_EQ(ids[0], 0u);
  EXPECT_EQ(ids[1], 1u);
}

TEST(BoostRtree, KNearestReturnsExpectedSet) {
  bgi::rtree<Value2D, bgi::quadratic<16>> tree;
  tree.insert({Point2D{0.0, 0.0}, 0u});
  tree.insert({Point2D{1.0, 0.0}, 1u});
  tree.insert({Point2D{2.0, 0.0}, 2u});
  tree.insert({Point2D{3.0, 0.0}, 3u});

  std::vector<Value2D> result;
  tree.query(bgi::nearest(Point2D{0.0, 0.0}, 3), std::back_inserter(result));
  ASSERT_EQ(result.size(), 3u);

  // The rtree's nearest() query does not guarantee ordering of its k results,
  // only that they are the k nearest. Verify the expected set membership.
  std::vector<std::size_t> ids;
  for (const auto &entry : result) {
    ids.push_back(entry.second);
  }
  std::sort(ids.begin(), ids.end());
  EXPECT_EQ(ids[0], 0u);
  EXPECT_EQ(ids[1], 1u);
  EXPECT_EQ(ids[2], 2u);
}

namespace {

  using PointCloud = carla::geom::PointCloudRtree<int>;
  using SegmentCloud = carla::geom::SegmentCloudRtree<int>;

  std::vector<std::size_t> NeighbourRequests() {
    std::vector<std::size_t> requests{0u, 1u, 2u, 4u, 5u};
    requests.push_back(std::numeric_limits<unsigned int>::max());
    if constexpr (std::numeric_limits<std::size_t>::digits >
                  std::numeric_limits<unsigned int>::digits) {
      // These valid limits must not wrap to zero or one in a small tree.
      const auto narrow_max =
          static_cast<std::size_t>(std::numeric_limits<unsigned int>::max());
      requests.push_back(narrow_max + 1u);
      requests.push_back(narrow_max + 2u);
      requests.push_back(std::numeric_limits<std::size_t>::max());
    }
    return requests;
  }

  template <typename Query>
  void CheckNeighbourLimits(Query query, const std::vector<int> &eligible) {
    for (const auto count : NeighbourRequests()) {
      SCOPED_TRACE(count);
      auto actual = query(count);
      std::sort(actual.begin(), actual.end());
      auto expected = eligible;
      expected.resize(std::min(count, expected.size()));
      EXPECT_EQ(actual, expected);
    }
  }

  PointCloud MakePointCloud() {
    PointCloud tree;
    for (int id = 0; id < 4; ++id) {
      tree.InsertElement(PointCloud::BPoint{3.0f * id, 0.0f, 0.0f}, id);
    }
    return tree;
  }

  SegmentCloud MakeSegmentCloud() {
    SegmentCloud tree;
    for (int id = 0; id < 4; ++id) {
      tree.InsertElement(SegmentCloud::BSegment{
          SegmentCloud::BPoint{3.0f * id, 0.0f, 0.0f},
          SegmentCloud::BPoint{3.0f * id + 1.0f, 0.0f, 0.0f}}, id, id);
    }
    return tree;
  }

} // namespace

TEST(CarlaRtree, PointNeighbourLimitsPreserveSizeT) {
  const auto tree = MakePointCloud();
  CheckNeighbourLimits([&](std::size_t count) {
    std::vector<int> ids;
    for (const auto &entry : tree.GetNearestNeighbours(
             PointCloud::BPoint{0.5f, 0.0f, 0.0f}, count)) {
      ids.push_back(entry.second);
    }
    return ids;
  }, {0, 1, 2, 3});
  EXPECT_TRUE(PointCloud{}.GetNearestNeighbours(
      PointCloud::BPoint{0.0f, 0.0f, 0.0f},
      std::numeric_limits<std::size_t>::max()).empty());
}

TEST(CarlaRtree, FilteredPointNeighbourLimitsPreserveSizeT) {
  const auto tree = MakePointCloud();
  CheckNeighbourLimits([&](std::size_t count) {
    std::vector<int> ids;
    for (const auto &entry : tree.GetNearestNeighboursWithFilter(
             PointCloud::BPoint{0.5f, 0.0f, 0.0f},
             [](const auto &value) { return value.second % 2 == 1; }, count)) {
      ids.push_back(entry.second);
    }
    return ids;
  }, {1, 3});
}

TEST(CarlaRtree, SegmentNeighbourLimitsPreserveSizeT) {
  const auto tree = MakeSegmentCloud();
  CheckNeighbourLimits([&](std::size_t count) {
    std::vector<int> ids;
    for (const auto &entry : tree.GetNearestNeighbours(
             SegmentCloud::BPoint{0.5f, 0.0f, 0.0f}, count)) {
      ids.push_back(entry.second.first);
    }
    return ids;
  }, {0, 1, 2, 3});
  EXPECT_TRUE(SegmentCloud{}.GetNearestNeighbours(
      SegmentCloud::BPoint{0.0f, 0.0f, 0.0f},
      std::numeric_limits<std::size_t>::max()).empty());
}

TEST(CarlaRtree, FilteredSegmentNeighbourLimitsPreserveSizeT) {
  const auto tree = MakeSegmentCloud();
  CheckNeighbourLimits([&](std::size_t count) {
    std::vector<int> ids;
    for (const auto &entry : tree.GetNearestNeighboursWithFilter(
             SegmentCloud::BPoint{0.5f, 0.0f, 0.0f},
             [](const auto &value) { return value.second.first % 2 == 1; },
             count)) {
      ids.push_back(entry.second.first);
    }
    return ids;
  }, {1, 3});
}
