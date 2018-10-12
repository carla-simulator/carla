// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/MapBuilder.h>

using namespace carla::road;

TEST(road, add_geometry) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  def.MakeGeometry<GeometryLine>(1.0, 2.0, 3.0, carla::geom::Location());
  def.MakeGeometry<GeometrySpiral>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0, 2.0);
  def.MakeGeometry<GeometryArc>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0);

  builder.AddRoadSegmentDefinition(def);
  builder.Build();
}

TEST(road, add_information) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  class A : public RoadInfo {};
  class B : public RoadInfo {};
  class C : public RoadInfo {};

  def.MakeInfo<A>();
  def.MakeInfo<B>();
  def.MakeInfo<C>();

  builder.AddRoadSegmentDefinition(def);
  builder.Build();
}

TEST(road, add_geom_info) {
  MapBuilder builder;
  RoadSegmentDefinition def(1);

  class A : public RoadInfo {};
  class B : public RoadInfo {};
  class C : public RoadInfo {};

  def.MakeGeometry<GeometryLine>(1.0, 2.0, 3.0, carla::geom::Location());
  def.MakeGeometry<GeometrySpiral>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0, 2.0);
  def.MakeGeometry<GeometryArc>(1.0, 2.0, 3.0, carla::geom::Location(), 1.0);

  def.MakeInfo<A>();
  def.MakeInfo<B>();
  def.MakeInfo<C>();

  builder.AddRoadSegmentDefinition(def);
  builder.Build();
}

TEST(road, connections) {
  MapBuilder builder;
  RoadSegmentDefinition def1(1);
  RoadSegmentDefinition def2(2);
  RoadSegmentDefinition def3(3);
  RoadSegmentDefinition def4(4);

  def1.AddPredecessorID(4);
  def2.AddPredecessorID(1);
  def3.AddPredecessorID(1);
  def3.AddPredecessorID(2);
  def4.AddPredecessorID(3);

  def1.AddSuccessorID(2);
  def1.AddSuccessorID(3);
  def2.AddSuccessorID(3);
  def3.AddSuccessorID(4);

  builder.AddRoadSegmentDefinition(def1);
  builder.AddRoadSegmentDefinition(def2);
  builder.AddRoadSegmentDefinition(def3);
  builder.AddRoadSegmentDefinition(def4);

  Map m = builder.Build();

  ASSERT_TRUE(m.ExistId(1));
  ASSERT_TRUE(m.ExistId(2));
  ASSERT_TRUE(m.ExistId(3));
  ASSERT_TRUE(m.ExistId(4));

  const RoadSegment *s1 = m.GetRoad(1);
  const RoadSegment *s2 = m.GetRoad(2);
  const RoadSegment *s3 = m.GetRoad(3);
  const RoadSegment *s4 = m.GetRoad(4);

  ASSERT_NE(s1, nullptr);
  ASSERT_NE(s2, nullptr);
  ASSERT_NE(s3, nullptr);
  ASSERT_NE(s4, nullptr);

  // Road 1
  ASSERT_TRUE(s1->HavePredecessors());
  ASSERT_TRUE(s1->HaveSuccessors());

  std::vector<id_type> pred = s1->GetPredecessorsIds();
  std::vector<RoadSegment *> pred_ptr = s1->GetPredecessors();

  ASSERT_EQ(pred.size(), 1U);
  ASSERT_EQ(pred[0], 4U); // same id
  ASSERT_EQ(m.GetRoad(pred[0]), s4); // same memory adress
  ASSERT_EQ(s4, pred_ptr[0]); // same memory adress

  std::vector<id_type> succ = s1->GetSuccessorsIds();
  std::vector<RoadSegment *> succ_ptr = s1->GetSuccessors();

  ASSERT_EQ(succ.size(), 2U);
  ASSERT_EQ(succ[0], 2U);
  ASSERT_EQ(succ[1], 3U);
  ASSERT_EQ(m.GetRoad(succ[0]), s2);
  ASSERT_EQ(m.GetRoad(succ[1]), s3);
  ASSERT_EQ(s2, succ_ptr[0]);
  ASSERT_EQ(s3, succ_ptr[1]);

  pred.clear();
  succ.clear();
  pred_ptr.clear();
  succ_ptr.clear();

  // Road 2
  ASSERT_TRUE(s2->HavePredecessors());
  ASSERT_TRUE(s2->HaveSuccessors());

  pred = s2->GetPredecessorsIds();
  pred_ptr = s2->GetPredecessors();

  ASSERT_EQ(pred.size(), 1U);
  ASSERT_EQ(pred[0], 1U);
  ASSERT_EQ(m.GetRoad(pred[0]), s1);
  ASSERT_EQ(s1, pred_ptr[0]);

  succ = s2->GetSuccessorsIds();
  succ_ptr = s2->GetSuccessors();

  ASSERT_EQ(succ.size(), 1U);
  ASSERT_EQ(succ[0], 3U);
  ASSERT_EQ(m.GetRoad(succ[0]), s3);
  ASSERT_EQ(s3, succ_ptr[0]);

  pred.clear();
  succ.clear();
  pred_ptr.clear();
  succ_ptr.clear();

  // Road 3
  ASSERT_TRUE(s3->HavePredecessors());
  ASSERT_TRUE(s3->HaveSuccessors());

  pred = s3->GetPredecessorsIds();
  pred_ptr = s3->GetPredecessors();

  ASSERT_EQ(pred.size(), 2U);
  ASSERT_EQ(pred[0], 1U);
  ASSERT_EQ(pred[1], 2U);
  ASSERT_EQ(m.GetRoad(pred[0]), s1);
  ASSERT_EQ(m.GetRoad(pred[1]), s2);
  ASSERT_EQ(s1, pred_ptr[0]);
  ASSERT_EQ(s2, pred_ptr[1]);

  succ = s3->GetSuccessorsIds();
  succ_ptr = s3->GetSuccessors();

  ASSERT_EQ(succ.size(), 1U);
  ASSERT_EQ(succ[0], 4U);
  ASSERT_EQ(m.GetRoad(succ[0]), s4);
  ASSERT_EQ(s4, succ_ptr[0]);

  pred.clear();
  succ.clear();
  pred_ptr.clear();
  succ_ptr.clear();

  // Road 4
  ASSERT_TRUE(s4->HavePredecessors());
  ASSERT_FALSE(s4->HaveSuccessors());

  pred = s4->GetPredecessorsIds();
  pred_ptr = s4->GetPredecessors();

  ASSERT_EQ(pred.size(), 1U);
  ASSERT_EQ(pred[0], 3U);
  ASSERT_EQ(m.GetRoad(pred[0]), s3);
  ASSERT_EQ(s3, pred_ptr[0]);

  succ = s4->GetSuccessorsIds();
  succ_ptr = s4->GetSuccessors();

  ASSERT_EQ(succ.size(), 0U);

  pred.clear();
  succ.clear();
  pred_ptr.clear();
  succ_ptr.clear();
}