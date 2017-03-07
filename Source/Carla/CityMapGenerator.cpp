// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CityMapGenerator.h"

#include "MapGen/GraphGenerator.h"

#include <algorithm>

#ifdef WITH_EDITOR
#include <sstream>
#endif

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACityMapGenerator::ACityMapGenerator(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  UpdateMap();
}

ACityMapGenerator::~ACityMapGenerator() {}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

#if WITH_EDITOR
void ACityMapGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    UpdateMap();
  }
}
#endif

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
    BuildingGenerationSeed = Seed;
  }
}

void ACityMapGenerator::GenerateGraph() {
  if ((MapSizeX < 5u) || (MapSizeY < 5u)) {
    MapSizeX = 5u;
    MapSizeY = 5u;
    UE_LOG(LogCarla, Warning, TEXT("Map size changed, was too small"));
  }
#ifdef WITH_EDITOR
  // Delete the dcel before the new one is created so indices are restored.
  Dcel.Reset(nullptr);
#endif // WITH_EDITOR
  Dcel = MapGen::GraphGenerator::Generate(MapSizeX, MapSizeY, RoadPlanningSeed);
  UE_LOG(LogCarla, Log,
      TEXT("Generated DCEL with: { %d vertices, %d half-edges, %d faces }"),
      Dcel->CountNodes(),
      Dcel->CountHalfEdges(),
      Dcel->CountFaces());
  DcelParser = MakeUnique<MapGen::GraphParser>(*Dcel);
#ifdef WITH_EDITOR
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
#endif // WITH_EDITOR
}

void ACityMapGenerator::GenerateRoads() {
  constexpr auto basicRoadTag = ECityMapMeshTag::RoadTwoLanes;
  constexpr auto basicIntersectionTag = ECityMapMeshTag::RoadXIntersection;

  // Rotation for vertical roads.
  const FQuat rotation(FVector(0.0f, 0.0f, 1.0f), HALF_PI);

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
        AddInstance(basicRoadTag, source.x, y, HALF_PI);
      }
    } else if (source.y == target.y) {
      // horizontal
      auto x = 1u + margin + std::min(source.x, target.x);
      auto end = std::max(source.x, target.x) - margin;
      for (; x < end; ++x) {
        AddInstance(basicRoadTag, x, source.y);
      }
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Diagonal edge ignored"));
    }
  }

  // For each node add the intersection.
  for (auto &node : graph.GetNodes()) {
    const auto coords = node.GetPosition();
    ECityMapMeshTag tag = basicIntersectionTag;
    switch (node.IntersectionType) {
      case MapGen::EIntersectionType::Turn90Deg:
        tag = ECityMapMeshTag::Road90DegTurn;
        break;
      case MapGen::EIntersectionType::TIntersection:
        tag = ECityMapMeshTag::RoadTIntersection;
        break;
      case MapGen::EIntersectionType::XIntersection:
        tag = ECityMapMeshTag::RoadXIntersection;
        break;
      default:
        UE_LOG(LogCarla, Warning, TEXT("Intersection type not implemented"));
    }
    FString tagStr = CityMapMeshTag::ToString(tag);
    std::wstringstream sout;
    for (float a : node.Rots)
      sout << a << " ";
    UE_LOG(
        LogCarla,
        Log,
        TEXT("Add instance \"%s\" at {%d, %d} with rotation %f, { %s }"),
        *tagStr, coords.x, coords.y, node.Rotation, sout.str().c_str());
    AddInstance(tag, coords.x, coords.y, node.Rotation);
  }
}
