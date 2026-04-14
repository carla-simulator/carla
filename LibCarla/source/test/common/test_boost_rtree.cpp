// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Regression guard for boost::geometry::index::rtree after the Boost 1.84->1.89
// upgrade.  Mirrors how InMemoryMap.h and geom/Rtree.h consume the rtree:
// 2D and 3D point insertions, nearest-neighbour queries, and within-box queries.

#include "test.h"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <vector>

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;

using Point2D = bg::model::point<float, 2, bg::cs::cartesian>;
using Point3D = bg::model::point<float, 3, bg::cs::cartesian>;
using Value2D = std::pair<Point2D, int>;
using Value3D = std::pair<Point3D, int>;
using Rtree2D = bgi::rtree<Value2D, bgi::quadratic<16>>;
using Rtree3D = bgi::rtree<Value3D, bgi::quadratic<16>>;

TEST(BoostRtree, InsertAndNearestNeighbour2D) {
  Rtree2D tree;
  tree.insert({Point2D(0.f, 0.f), 0});
  tree.insert({Point2D(1.f, 0.f), 1});
  tree.insert({Point2D(0.f, 1.f), 2});
  tree.insert({Point2D(5.f, 5.f), 3});

  ASSERT_EQ(static_cast<int>(tree.size()), 4);

  std::vector<Value2D> result;
  tree.query(bgi::nearest(Point2D(0.1f, 0.1f), 1), std::back_inserter(result));
  ASSERT_EQ(static_cast<int>(result.size()), 1);
  EXPECT_EQ(result[0].second, 0) << "Nearest to (0.1, 0.1) should be point 0";
}

TEST(BoostRtree, InsertAndNearestNeighbour3D) {
  Rtree3D tree;
  tree.insert({Point3D(0.f, 0.f, 0.f), 0});
  tree.insert({Point3D(1.f, 0.f, 0.f), 1});
  tree.insert({Point3D(0.f, 1.f, 0.f), 2});
  tree.insert({Point3D(0.f, 0.f, 1.f), 3});
  tree.insert({Point3D(10.f, 10.f, 10.f), 4});

  ASSERT_EQ(static_cast<int>(tree.size()), 5);

  std::vector<Value3D> result;
  tree.query(bgi::nearest(Point3D(0.05f, 0.05f, 0.05f), 1),
             std::back_inserter(result));
  ASSERT_EQ(static_cast<int>(result.size()), 1);
  EXPECT_EQ(result[0].second, 0);
}

TEST(BoostRtree, WithinBoxQuery2D) {
  Rtree2D tree;
  tree.insert({Point2D(0.5f, 0.5f), 0});
  tree.insert({Point2D(1.5f, 1.5f), 1});
  tree.insert({Point2D(3.f,  3.f),  2});

  bg::model::box<Point2D> box(Point2D(0.f, 0.f), Point2D(2.f, 2.f));

  std::vector<Value2D> result;
  tree.query(bgi::within(box), std::back_inserter(result));
  EXPECT_EQ(static_cast<int>(result.size()), 2)
      << "Box [0,2]x[0,2] should contain points 0 and 1 only";
}

TEST(BoostRtree, KNearestReturnOrdering) {
  Rtree2D tree;
  tree.insert({Point2D(1.f, 0.f), 1});
  tree.insert({Point2D(2.f, 0.f), 2});
  tree.insert({Point2D(3.f, 0.f), 3});
  tree.insert({Point2D(0.f, 0.f), 0});

  std::vector<Value2D> result;
  tree.query(bgi::nearest(Point2D(0.f, 0.f), 2), std::back_inserter(result));
  ASSERT_EQ(static_cast<int>(result.size()), 2);

  // Both results must be among the two closest: indices 0 (dist 0) and 1 (dist 1).
  for (const auto &v : result) {
    EXPECT_TRUE(v.second == 0 || v.second == 1)
        << "Unexpected point index " << v.second << " in k=2 nearest results";
  }
}
