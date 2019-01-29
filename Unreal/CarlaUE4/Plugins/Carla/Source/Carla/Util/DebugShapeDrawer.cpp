// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/DebugShapeDrawer.h"

#include "DrawDebugHelpers.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/DebugShape.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

struct FShapeVisitor
{
  using Shape = carla::rpc::DebugShape;

  FShapeVisitor(UWorld &InWorld, FColor InColor, float InLifeTime, bool bInPersistentLines)
    : World(&InWorld),
      Color(InColor),
      LifeTime(InLifeTime),
      bPersistentLines(bInPersistentLines) {}

  void operator()(const Shape::Point &Point) const
  {
    DrawDebugPoint(
        World,
        Point.location,
        1e2f * Point.size,
        Color,
        bPersistentLines,
        LifeTime,
        DepthPriority);
  }

  void operator()(const Shape::Line &Line) const
  {
    DrawDebugLine(
        World,
        Line.begin,
        Line.end,
        Color,
        bPersistentLines,
        LifeTime,
        DepthPriority,
        1e2f * Line.thickness);
  }

  void operator()(const Shape::Arrow &Arrow) const
  {
    DrawDebugDirectionalArrow(
        World,
        Arrow.line.begin,
        Arrow.line.end,
        1e2f * Arrow.arrow_size,
        Color,
        bPersistentLines,
        LifeTime,
        DepthPriority,
        1e2f * Arrow.line.thickness);
  }

  void operator()(const Shape::Box &Box) const
  {
    FVector Extent = {Box.box.extent.x, Box.box.extent.y, Box.box.extent.z};
    DrawDebugBox(
        World,
        Box.box.location,
        1e2f * Extent,
        FQuat(FRotator(Box.rotation)),
        Color,
        bPersistentLines,
        LifeTime,
        DepthPriority,
        1e2f * Box.thickness);
  }

  void operator()(const Shape::String &Str) const
  {
    DrawDebugString(
        World,
        Str.location,
        carla::rpc::ToFString(Str.text),
        nullptr,
        Color,
        LifeTime,
        Str.draw_shadow);
  }

private:

  UWorld *World;

  FColor Color;

  float LifeTime;

  bool bPersistentLines;

  uint8 DepthPriority = 0;
};

void FDebugShapeDrawer::Draw(const carla::rpc::DebugShape &Shape)
{
  auto Visitor = FShapeVisitor(World, Shape.color, Shape.life_time, Shape.persistent_lines);
  boost::apply_visitor(Visitor, Shape.primitive);
}
