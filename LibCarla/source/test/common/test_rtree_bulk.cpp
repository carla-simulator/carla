// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/geom/Rtree.h"

#include <algorithm>
#include <random>
#include <set>

namespace {

  template <typename Tree>
  std::vector<typename Tree::TreeElement> MakePoints() {
    std::mt19937 random(913);
    std::uniform_real_distribution<float> coordinate(-100.0f, 100.0f);
    std::vector<typename Tree::TreeElement> entries;
    for (unsigned int i = 0; i < 513; ++i) {
      typename Tree::BPoint point;
      boost::geometry::set<0>(point, coordinate(random));
      if constexpr (boost::geometry::dimension<typename Tree::BPoint>::value > 1) {
        boost::geometry::set<1>(point, coordinate(random));
      }
      if constexpr (boost::geometry::dimension<typename Tree::BPoint>::value > 2) {
        boost::geometry::set<2>(point, coordinate(random));
      }
      entries.emplace_back(point, i);
    }
    return entries;
  }

  template <typename Tree>
  void CheckPointBatches() {
    const auto entries = MakePoints<Tree>();
    Tree tree;
    tree.InsertElements({});
    EXPECT_EQ(tree.GetTreeSize(), 0u);
    tree.InsertElements(entries);
    ASSERT_EQ(tree.GetTreeSize(), entries.size());
    for (size_t i = 0; i < entries.size(); i += 13) {
      const auto &query = entries[i].first;
      for (const size_t count : {1u, 7u, 31u}) {
        auto result = tree.GetNearestNeighbours(query, count);
        ASSERT_EQ(result.size(), count);
        std::vector<double> actual, expected;
        for (const auto &entry : result) {
          actual.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
        for (const auto &entry : entries) {
          expected.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
        std::sort(actual.begin(), actual.end());
        std::sort(expected.begin(), expected.end());
        expected.resize(count);
        EXPECT_EQ(actual, expected);
      }
      const auto filtered = tree.GetNearestNeighboursWithFilter(query,
          [](const auto &entry) { return entry.second % 7 == 0; }, 5);
      ASSERT_EQ(filtered.size(), 5u);
      std::vector<double> actual, expected;
      for (const auto &entry : filtered) {
        EXPECT_EQ(entry.second % 7, 0u);
        actual.push_back(boost::geometry::comparable_distance(query, entry.first));
      }
      for (const auto &entry : entries) {
        if (entry.second % 7 == 0) {
          expected.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
      }
      std::sort(actual.begin(), actual.end());
      std::sort(expected.begin(), expected.end());
      expected.resize(5);
      EXPECT_EQ(actual, expected);
    }

    // Later range insertion must append, retaining the initial packed tree.
    tree.InsertElements(entries);
    tree.InsertElement(entries.front());
    tree.InsertElements({});
    EXPECT_EQ(tree.GetTreeSize(), entries.size() * 2 + 1);
    const auto all = tree.GetNearestNeighbours(entries.front().first, tree.GetTreeSize() + 5);
    ASSERT_EQ(all.size(), tree.GetTreeSize());
    EXPECT_EQ(std::count_if(all.begin(), all.end(), [](const auto &entry) {
      return entry.second == 0;
    }), 3);
  }

  template <size_t Dimension>
  void CheckSegmentBatches() {
    using Tree = carla::geom::SegmentCloudRtree<unsigned int, Dimension>;
    using PointTree = carla::geom::PointCloudRtree<unsigned int, Dimension>;
    const auto points = MakePoints<PointTree>();
    std::vector<typename Tree::TreeElement> entries;
    for (size_t i = 1; i < points.size(); ++i) {
      auto end = points[i].first;
      boost::geometry::set<0>(end, boost::geometry::get<0>(end) + 0.5f);
      entries.emplace_back(typename Tree::BSegment(points[i].first, end),
          std::make_pair(static_cast<unsigned int>(i), static_cast<unsigned int>(i + 1)));
    }
    Tree tree;
    tree.InsertElements({});
    tree.InsertElements(entries);
    ASSERT_EQ(tree.GetTreeSize(), entries.size());
    for (size_t i = 0; i < points.size(); i += 13) {
      const auto &query = points[i].first;
      for (const size_t count : {1u, 7u, 31u}) {
        const auto result = tree.GetNearestNeighbours(query, count);
        ASSERT_EQ(result.size(), count);
        std::vector<double> actual, expected;
        for (const auto &entry : result) {
          actual.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
        for (const auto &entry : entries) {
          expected.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
        std::sort(actual.begin(), actual.end());
        std::sort(expected.begin(), expected.end());
        expected.resize(count);
        EXPECT_EQ(actual, expected);
      }
      const auto result = tree.GetNearestNeighboursWithFilter(query,
          [](const auto &entry) { return entry.second.first % 7 == 0; }, 5);
      ASSERT_EQ(result.size(), 5u);
      std::vector<double> actual, expected;
      for (const auto &entry : result) {
        EXPECT_EQ(entry.second.first % 7, 0u);
        actual.push_back(boost::geometry::comparable_distance(query, entry.first));
      }
      for (const auto &entry : entries) {
        if (entry.second.first % 7 == 0) {
          expected.push_back(boost::geometry::comparable_distance(query, entry.first));
        }
      }
      std::sort(actual.begin(), actual.end());
      std::sort(expected.begin(), expected.end());
      expected.resize(5);
      EXPECT_EQ(actual, expected);
    }
    tree.InsertElements(entries);
    tree.InsertElement(entries.front());
    tree.InsertElements({});
    EXPECT_EQ(tree.GetTreeSize(), entries.size() * 2 + 1);
    const auto all = tree.GetNearestNeighbours(points.front().first, tree.GetTreeSize() + 5);
    EXPECT_EQ(all.size(), tree.GetTreeSize());
    EXPECT_EQ(std::count_if(all.begin(), all.end(), [](const auto &entry) {
      return entry.second.first == 1;
    }), 3);
  }

} // namespace

TEST(RtreeBulk, Points1D) {
  CheckPointBatches<carla::geom::PointCloudRtree<unsigned int, 1>>();
}

TEST(RtreeBulk, Points2D) {
  CheckPointBatches<carla::geom::PointCloudRtree<unsigned int, 2>>();
}

TEST(RtreeBulk, Points3D) {
  CheckPointBatches<carla::geom::PointCloudRtree<unsigned int, 3>>();
}

TEST(RtreeBulk, Segments1D) {
  CheckSegmentBatches<1>();
}

TEST(RtreeBulk, Segments2D) {
  CheckSegmentBatches<2>();
}

TEST(RtreeBulk, Segments3D) {
  CheckSegmentBatches<3>();
}

TEST(RtreeBulk, IntersectionAndCoincidentSegmentsPreservePayloads) {
  using Tree = carla::geom::SegmentCloudRtree<unsigned int, 2>;
  const Tree::BSegment crossing(Tree::BPoint(-2.0f, 0.0f), Tree::BPoint(2.0f, 0.0f));
  const Tree::BSegment outside(Tree::BPoint(2.0f, 2.0f), Tree::BPoint(3.0f, 3.0f));
  Tree tree;
  tree.InsertElements({{crossing, {1, 2}}, {outside, {3, 4}}, {crossing, {5, 6}}});
  const auto result = tree.GetIntersections(
      boost::geometry::model::box<Tree::BPoint>(Tree::BPoint(-1.0f, -1.0f), Tree::BPoint(1.0f, 1.0f)));
  ASSERT_EQ(result.size(), 2u);
  std::set<unsigned int> ids;
  for (const auto &entry : result) ids.insert(entry.second.first);
  EXPECT_EQ(ids, (std::set<unsigned int>{1, 5}));
}

TEST(RtreeBulk, SingleInsertThenBatchDoesNotReplaceExistingEntries) {
  using Tree = carla::geom::PointCloudRtree<unsigned int>;
  Tree tree;
  tree.InsertElement(Tree::BPoint(0.0f, 0.0f, 0.0f), 5);
  tree.InsertElements({{Tree::BPoint(1.0f, 0.0f, 0.0f), 6}});
  EXPECT_EQ(tree.GetTreeSize(), 2u);
  const auto result = tree.GetNearestNeighbours(Tree::BPoint(0.0f, 0.0f, 0.0f));
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result.front().second, 5u);
}
