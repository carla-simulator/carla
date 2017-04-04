// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CityMapGenerator.h"

#include "MapGen/GraphGenerator.h"

#include <algorithm>

#ifdef CARLA_ROAD_GENERATOR_PRINT_OUT
#include <sstream>
#endif // CARLA_ROAD_GENERATOR_PRINT_OUT

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACityMapGenerator::ACityMapGenerator(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer) {}

ACityMapGenerator::~ACityMapGenerator() {}

// =============================================================================
// -- Map construction and update related methods ------------------------------
// =============================================================================

void ACityMapGenerator::UpdateMap() {
  UpdateSeeds();
  GenerateGraph();
  if (bGenerateRoads) {
    GenerateRoads();
  }
}

void ACityMapGenerator::UpdateSeeds() {
  if (!bUseFixedSeed) {
    bUseMultipleFixedSeeds = false;
    FRandomStream randomStream;
    randomStream.GenerateNewSeed();
    Seed = randomStream.GetCurrentSeed();
  }
  if (!bUseMultipleFixedSeeds) {
    RoadPlanningSeed = Seed;
  }
}

void ACityMapGenerator::GenerateGraph() {
  if ((MapSizeX < 5u) || (MapSizeY < 5u)) {
    MapSizeX = 5u;
    MapSizeY = 5u;
    UE_LOG(LogCarla, Warning, TEXT("Map size changed, was too small"));
  }
#ifdef CARLA_ROAD_GENERATOR_PRINT_OUT
  // Delete the dcel before the new one is created so indices are restored.
  Dcel.Reset(nullptr);
#endif // CARLA_ROAD_GENERATOR_PRINT_OUT
  Dcel = MapGen::GraphGenerator::Generate(MapSizeX, MapSizeY, RoadPlanningSeed);
  UE_LOG(LogCarla, Log,
      TEXT("Generated DCEL with: { %d vertices, %d half-edges, %d faces }"),
      Dcel->CountNodes(),
      Dcel->CountHalfEdges(),
      Dcel->CountFaces());
  DcelParser = MakeUnique<MapGen::GraphParser>(*Dcel);
#ifdef CARLA_ROAD_GENERATOR_PRINT_OUT
  { // print the results of the parser.
    std::wstringstream sout;
    sout << "\nGenerated " << DcelParser->CityAreaCount() << " city areas: ";
    for (auto i = 0u; i < DcelParser->CityAreaCount(); ++i) {
      sout << "{ ";
      auto &cityArea = DcelParser->GetCityAreaAt(i);
      for (size_t j = 0u; j < cityArea.NodeCount(); ++j) {
        sout << cityArea.GetNodeAt(j) << " ";
      }
      sout << "} ";
    }
    sout << "\nGenerated " << DcelParser->RoadSegmentCount() << " road segments: ";
    for (auto i = 0u; i < DcelParser->RoadSegmentCount(); ++i) {
      sout << "{ ";
      auto &roadSegment = DcelParser->GetRoadSegmentAt(i);
      for (size_t j = 0u; j < roadSegment.Size(); ++j) {
        sout << roadSegment[j] << " ";
      }
      sout << "} ";
    }
    UE_LOG(LogCarla, Log, TEXT("\n%s"), sout.str().c_str());
  }
#endif // CARLA_ROAD_GENERATOR_PRINT_OUT
}

void ACityMapGenerator::GenerateRoads() {
  check(Dcel != nullptr);
  using Graph = MapGen::DoublyConnectedEdgeList;
  const Graph &graph = *Dcel;

  const uint32 margin = CityMapMeshTag::GetRoadIntersectionSize() / 2u;

  // For each edge add road segment.
  for (auto &edge : graph.GetHalfEdges()) {
    auto source = Graph::GetSource(edge).GetPosition();
    auto target = Graph::GetTarget(edge).GetPosition();

    if (source.x == target.x) {
      // vertical
      auto y = 1u + margin + std::min(source.y, target.y);
      auto end = std::max(source.y, target.y) - margin;
      for (; y < end; ++y) {
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          source.x, y, HALF_PI);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneRight,         source.x, y, HALF_PI);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      source.x, y, HALF_PI);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     source.x, y, HALF_PI);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneMarkingBroken, source.x, y, HALF_PI);
      }
    } else if (source.y == target.y) {
      // horizontal
      auto x = 1u + margin + std::min(source.x, target.x);
      auto end = std::max(source.x, target.x) - margin;
      for (; x < end; ++x) {
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneLeft,          x, source.y);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneRight,         x, source.y);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkLeft,      x, source.y);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_SidewalkRight,     x, source.y);
        AddInstance(ECityMapMeshTag::RoadTwoLanes_LaneMarkingBroken, x, source.y);
      }
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Diagonal edge ignored"));
    }
  }

#define ADD_INTERSECTION(tag, x, y, angle) \
    AddInstance(tag ##_Lane0, x, y, angle); \
    AddInstance(tag ##_Lane1, x, y, angle); \
    AddInstance(tag ##_Lane2, x, y, angle); \
    AddInstance(tag ##_Lane3, x, y, angle); \
    AddInstance(tag ##_Sidewalk0, x, y, angle); \
    AddInstance(tag ##_Sidewalk1, x, y, angle); \
    AddInstance(tag ##_Sidewalk2, x, y, angle); \
    AddInstance(tag ##_Sidewalk3, x, y, angle); \
    AddInstance(tag ##_LaneMarking, x, y, angle);

  // For each node add the intersection.
  for (auto &node : graph.GetNodes()) {
    const auto coords = node.GetPosition();
    switch (node.IntersectionType) {
      case MapGen::EIntersectionType::Turn90Deg:
        ADD_INTERSECTION(ECityMapMeshTag::Road90DegTurn, coords.x, coords.y, node.Rotation);
        break;
      case MapGen::EIntersectionType::TIntersection:
        ADD_INTERSECTION(ECityMapMeshTag::RoadTIntersection, coords.x, coords.y, node.Rotation);
        break;
      case MapGen::EIntersectionType::XIntersection:
        ADD_INTERSECTION(ECityMapMeshTag::RoadXIntersection, coords.x, coords.y, node.Rotation);
        break;
      default:
        UE_LOG(LogCarla, Warning, TEXT("Intersection type not implemented"));
    }
  }

#undef ADD_INTERSECTION
}
