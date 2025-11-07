// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibrary/MeshToLandscape.h"

#include <util/ue-header-guard-begin.h>
#include "Landscape.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "StaticMeshLODResourcesAdapter.h"
#include <util/ue-header-guard-end.h>

void UMeshToLandscapeUtil::FilterLandscapeLikeStaticMeshComponentsByVariance(
	TArray<UStaticMeshComponent*>& StaticMeshComponents,
	double MaxZVariance)
{
	if (MaxZVariance < 0.0)
		return;
	for (int32 i = 0; i != StaticMeshComponents.Num();)
	{
		UStaticMeshComponent* SMC = StaticMeshComponents[i];
		UStaticMesh* SM = SMC->GetStaticMesh();
		const FStaticMeshLODResources& LOD = SM->GetLODForExport(0);
		FStaticMeshLODResourcesMeshAdapter Adapter(&LOD);
		int32 VertexCount = LOD.GetNumVertices();
		double M, M2;
		M = M2 = 0.0;
		for (int32 j = 0; j != VertexCount; ++j)
		{
			double Z = Adapter.GetVertex(j).Z;
			double D = Z - M;
			M = std::fma(D, 1.0 / (double)j, M);
			double D2 = Z - M;
			M2 = std::fma(D, D2, M2);
		}
		double V = M2 / (double)(VertexCount - 1);
		if (V >= MaxZVariance)
			StaticMeshComponents.RemoveAtSwap(i, EAllowShrinking::No);
		else
			++i;
	}
	StaticMeshComponents.Shrink();
}

ALandscape* UMeshToLandscapeUtil::ConvertMeshesToLandscape(
	const TArray<UStaticMeshComponent*>& InStaticMeshComponents,
	int32 HeightmapWidth,
	int32 HeightmapHeight)
{
	if (InStaticMeshComponents.Num() == 0)
		return nullptr;
	
	UWorld* World = InStaticMeshComponents[0]->GetWorld();

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumZeroed(HeightmapHeight * HeightmapWidth);

	double Limit = 1e32;
	FVector3d Max = FVector3d(-Limit);
	FVector3d Min = FVector3d(Limit);

	for (UStaticMeshComponent* SMC : InStaticMeshComponents)
	{
		check(SMC->GetWorld() == World);
		UStaticMesh* SM = SMC->GetStaticMesh();
		const FStaticMeshLODResources& LOD = SM->GetLODForExport(0);
		FStaticMeshLODResourcesMeshAdapter Adapter(&LOD);
		int32 VertexCount = LOD.GetNumVertices();

		for (int32 i = 0; i != LOD.GetNumVertices(); ++i)
		{
			FVector3d Vertex = Adapter.GetVertex(i);
			Max = FVector3d::Max(Max, Vertex);
			Min = FVector3d::Min(Min, Vertex);
		}
	}

	FVector3d Range = Max - Min;

	auto MapPosition = [=](FVector3d xyz) -> FIntVector3
	{
		xyz /= Range;
		check(xyz.X >= 0);
		check(xyz.Y >= 0);
		check(xyz.Z >= 0);
		xyz = xyz.GetAbs();
		return FIntVector3(
			std::round(xyz.X * (double)HeightmapWidth),
			std::round(xyz.Y * (double)HeightmapHeight),
			xyz.Z * std::numeric_limits<uint16_t>::max());
	};

	for (UStaticMeshComponent* SMC : InStaticMeshComponents)
	{
		UStaticMesh* SM = SMC->GetStaticMesh();
		const FStaticMeshLODResources& LOD = SM->GetLODForExport(0);
		FStaticMeshLODResourcesMeshAdapter Adapter(&LOD);
		for (int32 i = 0; i != LOD.GetNumVertices(); ++i)
		{
			FVector3d Vertex = Adapter.GetVertex(i);
			FIntVector3 Coord = MapPosition(Vertex);
			HeightmapData[Coord.Y * HeightmapWidth + Coord.X] = Coord.Z;
		}
	}

	ALandscape* Landscape = World->SpawnActor<ALandscape>(
		ALandscape::StaticClass());

	TMap<FGuid, TArray<uint16>> LayerHeightMaps;
	LayerHeightMaps.Add(FGuid::NewGuid(), MoveTemp(HeightmapData));

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerImportInfos;

	Landscape->Import(
		Landscape->GetLandscapeGuid(),
		0, 0,
		HeightmapWidth - 1, HeightmapHeight - 1,
		1, 63,
		LayerHeightMaps,
		nullptr,
		LayerImportInfos,
		ELandscapeImportAlphamapType::Layered,
		nullptr);

	Landscape->PostEditChange();
	Landscape->RegisterAllComponents();
	return Landscape;
}

void UMeshToLandscapeUtil::EnumerateLandscapeLikeStaticMeshComponentsByVariance(
	AActor* WorldContextObject,
	TArray<UStaticMeshComponent*>& OutStaticMeshComponents,
	double MaxZVariance)
{
	UWorld* World = WorldContextObject->GetWorld();
	TArray<AActor*> SMAs;
	UGameplayStatics::GetAllActorsOfClass(
		WorldContextObject,
		AStaticMeshActor::StaticClass(),
		SMAs);
	for (AActor* SMA : SMAs)
	{
		TArray<UStaticMeshComponent*> SMCs;
		SMA->GetComponents(SMCs);
		FilterLandscapeLikeStaticMeshComponentsByVariance(SMCs, MaxZVariance);
		OutStaticMeshComponents.Append(SMCs);
	}
}
