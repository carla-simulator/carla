// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Sensor/UE4_Overridden/LineBatchComponent_CARLA.h"
#include "Sensor/CosmosControlSensor.h"
#include "ConstructorHelpers.h"

FLineBatcherSceneProxy_CARLA::FLineBatcherSceneProxy_CARLA(const ULineBatchComponent_CARLA* InComponent) :
	FLineBatcherSceneProxy(InComponent), Lines(InComponent->BatchedLines),
	Points(InComponent->BatchedPoints), Meshes(InComponent->BatchedMeshes)
{
	bWillEverBeLit = false;
}

FLineBatcherSceneProxy_CARLA::~FLineBatcherSceneProxy_CARLA()
{
	for (auto& Pair : CachedMaterialProxies)
	{
		delete Pair.Value;
	}
	CachedMaterialProxies.Empty();
}

ULineBatchComponent_CARLA::ULineBatchComponent_CARLA(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FObjectFinder<UMaterial> Loader(TEXT("Material'/Carla/PostProcessingMaterials/DebugCosmosMeshMaterial.DebugCosmosMeshMaterial'"));
	CosmosMeshMaterial = Loader.Object;
}

FPrimitiveSceneProxy* ULineBatchComponent_CARLA::CreateSceneProxy()
{
	FLineBatcherSceneProxy_CARLA* proxy = new FLineBatcherSceneProxy_CARLA(this);
	proxy->CosmosMeshMaterial = CosmosMeshMaterial;
	
	return proxy;
}

SIZE_T FLineBatcherSceneProxy_CARLA::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}
/**
*  Returns a struct that describes to the renderer when to draw this proxy.
*	@param		Scene view to use to determine our relevence.
*  @return		View relevance struct
*/
FPrimitiveViewRelevance FLineBatcherSceneProxy_CARLA::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance;

	ViewRelevance.bDrawRelevance = Cast<ACosmosControlSensor>(View->ViewActor) != nullptr;//|| View->ViewActor->IsA(ACosmosControlSensor::StaticClass());
	ViewRelevance.bDynamicRelevance = true;
	// ideally the TranslucencyRelevance should be filled out by the material, here we do it conservative
	ViewRelevance.bSeparateTranslucency = ViewRelevance.bNormalTranslucency = true;
	return ViewRelevance;
}

void FLineBatcherSceneProxy_CARLA::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_LineBatcherSceneProxy_GetDynamicMeshElements);

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			for (int32 i = 0; i < Lines.Num(); i++)
			{
				FLinearColor LinearColor = FLinearColor(Lines[i].Color);
				PDI->DrawLine(Lines[i].Start, Lines[i].End, LinearColor, Lines[i].DepthPriority, Lines[i].Thickness, 0.0f, true);
			}

			for (int32 i = 0; i < Points.Num(); i++)
			{
				FLinearColor LinearColor = FLinearColor(Points[i].Color);
				PDI->DrawPoint(Points[i].Position, LinearColor, Points[i].PointSize, Points[i].DepthPriority);
			}

			for (int32 i = 0; i < Meshes.Num(); i++)
			{
				static FVector const PosX(1.f, 0, 0);
				static FVector const PosY(0, 1.f, 0);
				static FVector const PosZ(0, 0, 1.f);

				FBatchedMesh const& M = Meshes[i];

				// this seems far from optimal in terms of perf, but it's for debugging
				FDynamicMeshBuilder MeshBuilder(View->GetFeatureLevel());

				// set up geometry
				for (int32 VertIdx = 0; VertIdx < M.MeshVerts.Num(); ++VertIdx)
				{
					MeshBuilder.AddVertex(M.MeshVerts[VertIdx], FVector2D::ZeroVector, PosX, PosY, PosZ, FColor::White);
				}
				//MeshBuilder.AddTriangles(M.MeshIndices);
				for (int32 Idx = 0; Idx < M.MeshIndices.Num(); Idx += 3)
				{
					MeshBuilder.AddTriangle(M.MeshIndices[Idx], M.MeshIndices[Idx + 1], M.MeshIndices[Idx + 2]);
				}

				// Create a unique key from the color
				uint32 ColorKey = M.Color.DWColor();
				FColoredMaterialRenderProxy** CachedProxy = CachedMaterialProxies.Find(ColorKey);
				FMaterialRenderProxy* MaterialRenderProxy = nullptr;
				
				if (CachedProxy)
				{
					MaterialRenderProxy = *CachedProxy;
				}
				else
				{
					// Create new proxy and cache it
					FMaterialRenderProxy* BaseMaterialProxy = CosmosMeshMaterial == nullptr ? GEngine->DebugMeshMaterial->GetRenderProxy() : CosmosMeshMaterial->GetRenderProxy();
					
					FLinearColor LinearColor = M.Color.ReinterpretAsLinear();
					FColoredMaterialRenderProxy* NewProxy = new FColoredMaterialRenderProxy(
						BaseMaterialProxy,
						LinearColor);
					
					CachedMaterialProxies.Add(ColorKey, NewProxy);
					MaterialRenderProxy = NewProxy;
				}
				
				MeshBuilder.GetMesh(FMatrix::Identity, MaterialRenderProxy, M.DepthPriority, false, false, ViewIndex, Collector);
			}
		}
	}
}
