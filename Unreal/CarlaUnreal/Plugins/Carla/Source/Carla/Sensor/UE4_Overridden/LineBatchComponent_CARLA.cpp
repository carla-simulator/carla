// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/UE4_Overridden/LineBatchComponent_CARLA.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "PrimitiveViewRelevance.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "UObject/Class.h"
#include "UObject/ConstructorHelpers.h"
#include <util/ue-header-guard-end.h>

namespace {

  // The view-actor class this proxy draws for.  ACosmosControlSensor is
  // ported in a later task on this branch, so it is resolved by name to keep
  // this component self-contained.  The lookup is cached on first use.
  static const UClass* GetCosmosControlSensorClass()
  {
    static const UClass* CosmosClass =
        UClass::TryFindTypeSlowSafe<UClass>(TEXT("CosmosControlSensor"));
    return CosmosClass;
  }

} // namespace

FLineBatcherSceneProxy_CARLA::FLineBatcherSceneProxy_CARLA(
    const ULineBatchComponent_CARLA* InComponent)
  : FPrimitiveSceneProxy(InComponent),
    Lines(InComponent->BatchedLines),
    Points(InComponent->BatchedPoints),
    Meshes(InComponent->BatchedMeshes)
{
  bWillEverBeLit = false;
}

SIZE_T FLineBatcherSceneProxy_CARLA::GetTypeHash() const
{
  static size_t UniquePointer;
  return reinterpret_cast<size_t>(&UniquePointer);
}

FPrimitiveViewRelevance FLineBatcherSceneProxy_CARLA::GetViewRelevance(
    const FSceneView* View) const
{
  FPrimitiveViewRelevance ViewRelevance;

  const UClass* CosmosClass = GetCosmosControlSensorClass();
  ViewRelevance.bDrawRelevance =
      (CosmosClass != nullptr) &&
      (View->ViewActor != nullptr) &&
      View->ViewActor->IsA(CosmosClass);
  ViewRelevance.bDynamicRelevance = true;
  // Ideally the TranslucencyRelevance should be filled out by the material;
  // here we do it conservatively.
  ViewRelevance.bSeparateTranslucency =
      ViewRelevance.bNormalTranslucency = true;
  return ViewRelevance;
}

uint32 FLineBatcherSceneProxy_CARLA::GetMemoryFootprint(void) const
{
  return sizeof(*this) + GetAllocatedSize();
}

uint32 FLineBatcherSceneProxy_CARLA::GetAllocatedSize(void) const
{
  return FPrimitiveSceneProxy::GetAllocatedSize() +
         Lines.GetAllocatedSize() +
         Points.GetAllocatedSize() +
         Meshes.GetAllocatedSize();
}

void FLineBatcherSceneProxy_CARLA::GetDynamicMeshElements(
    const TArray<const FSceneView*>& Views,
    const FSceneViewFamily& ViewFamily,
    uint32 VisibilityMap,
    FMeshElementCollector& Collector) const
{
  QUICK_SCOPE_CYCLE_COUNTER(STAT_LineBatcherSceneProxy_CARLA_GetDynamicMeshElements);

  for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
  {
    if (VisibilityMap & (1 << ViewIndex))
    {
      const FSceneView* View = Views[ViewIndex];
      FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

      for (const FBatchedLine& Line : Lines)
      {
        PDI->DrawLine(
            Line.Start, Line.End, Line.Color, Line.DepthPriority,
            Line.Thickness, 0.0f, true);
      }

      for (const FBatchedPoint& Point : Points)
      {
        PDI->DrawPoint(
            Point.Position, Point.Color, Point.PointSize,
            Point.DepthPriority);
      }

      for (const FBatchedMesh& M : Meshes)
      {
        static const FVector3f PosX(1.f, 0, 0);
        static const FVector3f PosY(0, 1.f, 0);
        static const FVector3f PosZ(0, 0, 1.f);

        // This seems far from optimal in terms of perf, but it is for
        // debugging.
        FDynamicMeshBuilder MeshBuilder(View->GetFeatureLevel());

        // Set up geometry.
        for (const FVector& Vert : M.MeshVerts)
        {
          MeshBuilder.AddVertex(
              (FVector3f)Vert, FVector2f::ZeroVector,
              PosX, PosY, PosZ, FColor::White);
        }
        for (int32 Idx = 0; Idx < M.MeshIndices.Num(); Idx += 3)
        {
          MeshBuilder.AddTriangle(
              M.MeshIndices[Idx],
              M.MeshIndices[Idx + 1],
              M.MeshIndices[Idx + 2]);
        }

        FMaterialRenderProxy* const BaseMaterialProxy =
            CosmosMeshMaterial == nullptr ?
                GEngine->DebugMeshMaterial->GetRenderProxy() :
                CosmosMeshMaterial->GetRenderProxy();

        FMaterialRenderProxy* const MaterialRenderProxy =
            new FColoredMaterialRenderProxy(
                BaseMaterialProxy, M.Color.ReinterpretAsLinear());
        Collector.RegisterOneFrameMaterialProxy(MaterialRenderProxy);

        MeshBuilder.GetMesh(
            FMatrix::Identity, MaterialRenderProxy, M.DepthPriority,
            false, false, ViewIndex, Collector);
      }
    }
  }
}

ULineBatchComponent_CARLA::ULineBatchComponent_CARLA(
    const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  ConstructorHelpers::FObjectFinder<UMaterial> Loader(TEXT(
      "Material'/Carla/PostProcessingMaterials/"
      "DebugCosmosMeshMaterial.DebugCosmosMeshMaterial'"));
  CosmosMeshMaterial = Loader.Object;
}

FPrimitiveSceneProxy* ULineBatchComponent_CARLA::CreateSceneProxy()
{
  FLineBatcherSceneProxy_CARLA* Proxy = new FLineBatcherSceneProxy_CARLA(this);
  Proxy->CosmosMeshMaterial = CosmosMeshMaterial;

  return Proxy;
}
