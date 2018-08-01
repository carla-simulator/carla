// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaUE4.h"
#include "CarlaMapGenerator.h"

ACarlaMapGenerator::ACarlaMapGenerator(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
#define SET_STATIC_MESH(Tag, Folder, FileName) \
    { \
      static const ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj( \
          TEXT("StaticMesh'" Folder "/" FileName "." FileName "'")); \
      SetStaticMesh(ECityMapMeshTag:: Tag, MeshObj.Object); \
    }

#define PREFIX_FOLDER "/Game/Carla/Static/"

  SET_STATIC_MESH(RoadTwoLanes_LaneLeft,           PREFIX_FOLDER "Road",        "St_Road_TileRoad_RoadL");
  SET_STATIC_MESH(RoadTwoLanes_LaneRight,          PREFIX_FOLDER "Road",        "St_Road_TileRoad_RoadR");
  SET_STATIC_MESH(RoadTwoLanes_SidewalkLeft,       PREFIX_FOLDER "SideWalk",    "St_Road_TileRoad_SidewalkL");
  SET_STATIC_MESH(RoadTwoLanes_SidewalkRight,      PREFIX_FOLDER "SideWalk",    "St_Road_TileRoad_SidewalkR");
  SET_STATIC_MESH(RoadTwoLanes_LaneMarkingSolid,   PREFIX_FOLDER "RoadLines",   "St_Road_TileRoad_LaneMarkingSolid");
  SET_STATIC_MESH(RoadTwoLanes_LaneMarkingBroken,  PREFIX_FOLDER "RoadLines",   "St_Road_TileRoad_LaneMarkingBroken");

  SET_STATIC_MESH(Road90DegTurn_Lane0,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road0");
  SET_STATIC_MESH(Road90DegTurn_Lane1,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road1");
  SET_STATIC_MESH(Road90DegTurn_Lane2,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road2");
  SET_STATIC_MESH(Road90DegTurn_Lane3,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road3");
  SET_STATIC_MESH(Road90DegTurn_Lane4,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road4");
  SET_STATIC_MESH(Road90DegTurn_Lane5,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road5");
  SET_STATIC_MESH(Road90DegTurn_Lane6,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road6");
  SET_STATIC_MESH(Road90DegTurn_Lane7,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road7");
  SET_STATIC_MESH(Road90DegTurn_Lane8,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road8");
  SET_STATIC_MESH(Road90DegTurn_Lane9,             PREFIX_FOLDER "Road",        "St_Road_Curve_Road9");
  SET_STATIC_MESH(Road90DegTurn_Sidewalk0,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk1");
  SET_STATIC_MESH(Road90DegTurn_Sidewalk1,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk2");
  SET_STATIC_MESH(Road90DegTurn_Sidewalk2,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk3");
  SET_STATIC_MESH(Road90DegTurn_Sidewalk3,         PREFIX_FOLDER "SideWalk",    "St_Road_Curve_Sidewalk4");
  SET_STATIC_MESH(Road90DegTurn_LaneMarking,       PREFIX_FOLDER "RoadLines",   "St_Road_Curve_LaneMarking");

  SET_STATIC_MESH(RoadTIntersection_Lane0,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road0");
  SET_STATIC_MESH(RoadTIntersection_Lane1,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road1");
  SET_STATIC_MESH(RoadTIntersection_Lane2,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road2");
  SET_STATIC_MESH(RoadTIntersection_Lane3,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road3");
  SET_STATIC_MESH(RoadTIntersection_Lane4,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road4");
  SET_STATIC_MESH(RoadTIntersection_Lane5,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road5");
  SET_STATIC_MESH(RoadTIntersection_Lane6,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road6");
  SET_STATIC_MESH(RoadTIntersection_Lane7,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road7");
  SET_STATIC_MESH(RoadTIntersection_Lane8,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road8");
  SET_STATIC_MESH(RoadTIntersection_Lane9,         PREFIX_FOLDER "Road",        "St_Road_TCross_Road9");
  SET_STATIC_MESH(RoadTIntersection_Sidewalk0,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk1");
  SET_STATIC_MESH(RoadTIntersection_Sidewalk1,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk2");
  SET_STATIC_MESH(RoadTIntersection_Sidewalk2,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk3");
  SET_STATIC_MESH(RoadTIntersection_Sidewalk3,     PREFIX_FOLDER "SideWalk",    "St_Road_TCross_Sidewalk4");
  SET_STATIC_MESH(RoadTIntersection_LaneMarking,   PREFIX_FOLDER "RoadLines",   "St_Road_TCross_LaneMarking");

  SET_STATIC_MESH(RoadXIntersection_Lane0,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road0");
  SET_STATIC_MESH(RoadXIntersection_Lane1,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road1");
  SET_STATIC_MESH(RoadXIntersection_Lane2,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road2");
  SET_STATIC_MESH(RoadXIntersection_Lane3,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road3");
  SET_STATIC_MESH(RoadXIntersection_Lane4,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road4");
  SET_STATIC_MESH(RoadXIntersection_Lane5,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road5");
  SET_STATIC_MESH(RoadXIntersection_Lane6,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road6");
  SET_STATIC_MESH(RoadXIntersection_Lane7,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road7");
  SET_STATIC_MESH(RoadXIntersection_Lane8,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road8");
  SET_STATIC_MESH(RoadXIntersection_Lane9,         PREFIX_FOLDER "Road",        "St_Road_XCross_Road9");
  SET_STATIC_MESH(RoadXIntersection_Sidewalk0,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk1");
  SET_STATIC_MESH(RoadXIntersection_Sidewalk1,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk2");
  SET_STATIC_MESH(RoadXIntersection_Sidewalk2,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk3");
  SET_STATIC_MESH(RoadXIntersection_Sidewalk3,     PREFIX_FOLDER "SideWalk",    "St_Road_XCross_Sidewalk4");
  SET_STATIC_MESH(RoadXIntersection_LaneMarking,   PREFIX_FOLDER "RoadLines",   "St_Road_XCross_LaneMarking");

#undef PREFIX_FOLDER
#undef SET_STATIC_MESH
}
