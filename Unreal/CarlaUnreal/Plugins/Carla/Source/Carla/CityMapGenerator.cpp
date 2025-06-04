// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CityMapGenerator.h"
#include "Carla.h"
#include "MapGen/GraphGenerator.h"
#include "MapGen/RoadMap.h"
#include "Game/Tagger.h"

#include <util/ue-header-guard-begin.h>
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

#include <algorithm>
#include <unordered_set>

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
#include <sstream>
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

namespace crp = carla::rpc;

// =============================================================================
// -- Private types ------------------------------------------------------------
// =============================================================================

class FHalfEdgeCounter {
public:

  using HalfEdge = MapGen::DoublyConnectedEdgeList::HalfEdge;

  bool Insert(const HalfEdge &InHalfEdge)
  {
    return Set.insert(&InHalfEdge).second &&
           Set.insert(&MapGen::DoublyConnectedEdgeList::GetPair(InHalfEdge)).second;
  }

private:

  std::unordered_set<const HalfEdge *> Set;
};

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACityMapGenerator::ACityMapGenerator(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  RoadMap = ObjectInitializer.CreateDefaultSubobject<URoadMap>(this, TEXT("RoadMap"));
}

ACityMapGenerator::~ACityMapGenerator() {}

// =============================================================================
// -- Overriden from UObject ---------------------------------------------------
// =============================================================================

void ACityMapGenerator::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
#if WITH_EDITOR
  if (bGenerateRoadMapOnSave) {
    // Generate road map only if we are not cooking.
    FCoreUObjectDelegates::OnObjectSaved.Broadcast(this);
    if (!GIsCookerLoadingPackage) {
      check(RoadMap != nullptr);
      GenerateRoadMap();
    }
  }
#endif // WITH_EDITOR

  Super::PreSave(ObjectSaveContext);
}

// =============================================================================
// -- Overriden from ACityMapMeshHolder ----------------------------------------
// =============================================================================

void ACityMapGenerator::UpdateMap()
{
  UpdateSeeds();
  GenerateGraph();
  if (bGenerateRoads) {
    GenerateRoads();
  }
  if (bTriggerRoadMapGeneration) {
    bTriggerRoadMapGeneration = false;
    GenerateRoadMap();
  }
}

// =============================================================================
// -- Map construction and update related methods ------------------------------
// =============================================================================

void ACityMapGenerator::UpdateSeeds()
{
  if (!bUseFixedSeed) {
    FRandomStream randomStream;
    randomStream.GenerateNewSeed();
    Seed = randomStream.GetCurrentSeed();
  }
}

void ACityMapGenerator::GenerateGraph()
{
  if ((MapSizeX < 5u) || (MapSizeY < 5u)) {
    MapSizeX = 5u;
    MapSizeY = 5u;
    UE_LOG(LogCarla, Warning, TEXT("Map size changed, was too small"));
  }
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
  // Delete the dcel before the new one is created so indices are restored.
  Dcel.Reset(nullptr);
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
  Dcel = MapGen::GraphGenerator::Generate(MapSizeX, MapSizeY, Seed);
  UE_LOG(LogCarla, Log,
      TEXT("Generated DCEL with: { %d vertices, %d half-edges, %d faces }"),
      Dcel->CountNodes(),
      Dcel->CountHalfEdges(),
      Dcel->CountFaces());
  DcelParser = MakeUnique<MapGen::GraphParser>(*Dcel);
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
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
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
}

void ACityMapGenerator::GenerateRoads()
{
  check(Dcel != nullptr);
  using Graph = MapGen::DoublyConnectedEdgeList;
  const Graph &graph = *Dcel;

  const uint32 margin = CityMapMeshTag::GetRoadIntersectionSize() / 2u;

  FHalfEdgeCounter HalfEdgeCounter;

  // For each edge add road segment.
  for (auto &edge : graph.GetHalfEdges()) {
    if (HalfEdgeCounter.Insert(edge)) {
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
  }

#define ADD_INTERSECTION(tag, x, y, angle) \
    AddInstance(tag ##_Lane0, x, y, angle); \
    AddInstance(tag ##_Lane1, x, y, angle); \
    AddInstance(tag ##_Lane2, x, y, angle); \
    AddInstance(tag ##_Lane3, x, y, angle); \
    AddInstance(tag ##_Lane4, x, y, angle); \
    AddInstance(tag ##_Lane5, x, y, angle); \
    AddInstance(tag ##_Lane6, x, y, angle); \
    AddInstance(tag ##_Lane7, x, y, angle); \
    AddInstance(tag ##_Lane8, x, y, angle); \
    AddInstance(tag ##_Lane9, x, y, angle); \
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

// Find first component of type road.
static bool LineTrace(
    UWorld *World,
    const FVector &Start,
    const FVector &End,
    FHitResult &HitResult)
{
  TArray <FHitResult> OutHits;
  static FName TraceTag = FName(TEXT("RoadTrace"));
  const bool Success = World->LineTraceMultiByObjectType(
        OutHits,
        Start,
        End,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
        FCollisionQueryParams(TraceTag, true));

  if (Success) {
    for (FHitResult &Item : OutHits) {
      if (ATagger::MatchComponent(*Item.Component, crp::CityObjectLabel::Roads)) {
        HitResult = Item;
        return true;
      }
    }
  }
  return false;
}

void ACityMapGenerator::GenerateRoadMap()
{
  UE_LOG(LogCarla, Log, TEXT("Generating road map..."));

  auto World = GetWorld();
  check(GetWorld() != nullptr);
  check(RoadMap != nullptr);

  ATagger::TagActorsInLevel(*GetWorld(), bTagForSemanticSegmentation); // We need the tags.

  const float IntersectionSize = CityMapMeshTag::GetRoadIntersectionSize();
  const uint32 Margin = IntersectionSize / 2u;
  const float Offset = GetMapScale() * Margin;

  const float CmPerPixel = GetMapScale() / static_cast<float>(PixelsPerMapUnit);

  const uint32 SizeX = PixelsPerMapUnit * (MapSizeX + 2u * Margin);
  const uint32 SizeY = PixelsPerMapUnit * (MapSizeY + 2u * Margin);

  const FTransform &ActorTransform = GetActorTransform();

  const FVector MapOffset(-Offset, -Offset, 0.0f);
  RoadMap->Reset(SizeX, SizeY, 1.0f / CmPerPixel, ActorTransform.Inverse(), MapOffset);

  for (uint32 PixelY = 0u; PixelY < SizeY; ++PixelY) {
    for (uint32 PixelX = 0u; PixelX < SizeX; ++PixelX) {
      const float X = static_cast<float>(PixelX) * CmPerPixel - Offset;
      const float Y = static_cast<float>(PixelY) * CmPerPixel - Offset;
      const FVector Start = ActorTransform.TransformPosition(FVector(X, Y, 50.0f));
      const FVector End = ActorTransform.TransformPosition(FVector(X, Y, -50.0f));

      // Do the ray tracing.
      FHitResult Hit;
      if (LineTrace(World, Start, End, Hit)) {
        auto StaticMeshComponent = Cast<UStaticMeshComponent>(Hit.Component.Get());
        if (StaticMeshComponent == nullptr) {
          UE_LOG(LogCarla, Error, TEXT("Road component is not UInstancedStaticMeshComponent"));
        } else {
          RoadMap->SetPixelAt(
            PixelX,
            PixelY,
            GetTag(*StaticMeshComponent->GetStaticMesh()),
            StaticMeshComponent->GetOwner()->GetTransform(),
            bLeftHandTraffic);
        }
      }
    }
  }

#if WITH_EDITOR
  RoadMap->Log();
#endif // WITH_EDITOR

  if (bSaveRoadMapToDisk) {
    RoadMap->SaveAsPNG(FPaths::ProjectSavedDir(), World->GetMapName());
  }

#if WITH_EDITOR
  RoadMap->DrawDebugPixelsToLevel(GetWorld(), !bDrawDebugPixelsToLevel);
#endif // WITH_EDITOR
}
