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

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <algorithm>
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
