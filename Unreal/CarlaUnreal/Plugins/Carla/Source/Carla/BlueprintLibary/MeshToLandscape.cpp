// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "MeshToLandscape.h"

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
		if (SMC == nullptr)
		{
			++i;
			continue;
		}
		UStaticMesh* SM = SMC->GetStaticMesh();
		if (SM == nullptr)
		{
			++i;
			continue;
		}
		if (SM->GetNumLODs() == 0)
		{
			++i;
			continue;
		}
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
}

void UMeshToLandscapeUtil::FilterLandscapeLikeStaticMeshComponentsByPatterns(
	TArray<UStaticMeshComponent*>& StaticMeshComponents,
	const TArray<FString>& ActorNamePatterns)
{
	FString NameTemp;
	if (ActorNamePatterns.IsEmpty())
		return;
	for (int32 i = 0; i != StaticMeshComponents.Num();)
	{
		UStaticMeshComponent* SMC = StaticMeshComponents[i];
		SMC->GetOwner()->GetName(NameTemp);
		bool Match = false;
		for (const FString& Pattern : ActorNamePatterns)
		{
			Match = NameTemp.MatchesWildcard(Pattern);
			if (Match)
				break;
		}
		if (!Match)
			StaticMeshComponents.RemoveAtSwap(i, EAllowShrinking::No);
		else
			++i;
	}
}

ALandscape* UMeshToLandscapeUtil::ConvertMeshesToLandscape(
	const TArray<UStaticMeshComponent*>& StaticMeshComponents,
	int32 HeightmapWidth,
	int32 HeightmapHeight)
{
	if (StaticMeshComponents.Num() == 0)
		return nullptr;
	
	UWorld* World = StaticMeshComponents[0]->GetWorld();

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumZeroed(HeightmapHeight * HeightmapWidth);

	if (HeightmapData.Num() == 0)
	{
		// @TODO: Add Warning.
		return nullptr;
	}

	double Limit = 1e32;
	FVector3d Max = FVector3d(-Limit);
	FVector3d Min = FVector3d(Limit);

	for (UStaticMeshComponent* SMC : StaticMeshComponents)
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
	FVector3d RangeInv = FVector3d::OneVector / Range;
	FVector3d Origin = (Max + Min) * 0.5F;

	auto MapPosition = [=](FVector3d xyz) -> FIntVector3
	{
		xyz -= Min;
		xyz *= RangeInv;
		check(xyz.X + 1e-4 > 0);
		check(xyz.Y + 1e-4 > 0);
		check(xyz.Z + 1e-4 > 0);
		xyz = xyz.GetAbs();
		return FIntVector3(
			std::round(xyz.X * (double)(HeightmapWidth - 1)),
			std::round(xyz.Y * (double)(HeightmapHeight - 1)),
			xyz.Z * std::numeric_limits<uint16_t>::max());
	};

	for (UStaticMeshComponent* SMC : StaticMeshComponents)
	{
		UStaticMesh* SM = SMC->GetStaticMesh();
		const FStaticMeshLODResources& LOD = SM->GetLODForExport(0);
		FStaticMeshLODResourcesMeshAdapter Adapter(&LOD);
		for (int32 i = 0; i != LOD.GetNumVertices(); ++i)
		{
			FVector3d Vertex = Adapter.GetVertex(i);
			FIntVector3 Coord = MapPosition(Vertex);
			volatile auto Offset = Coord.Y * HeightmapWidth + Coord.X;
			HeightmapData[(int32)Offset] = Coord.Z;
		}
	}

	FActorSpawnParameters SpawnParams;
	ALandscape* Landscape = World->SpawnActor<ALandscape>(
		ALandscape::StaticClass(),
		Origin,
		FRotator(),
		SpawnParams);

	TMap<FGuid, TArray<uint16>> LayerHeightMaps;
	LayerHeightMaps.Add(FGuid::NewGuid(), MoveTemp(HeightmapData));

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerImportInfos;

	Landscape->Import(
		FGuid::NewGuid(),
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

void UMeshToLandscapeUtil::EnumerateLandscapeLikeStaticMeshComponents(
	UObject* WorldContextObject,
	const TArray<FString>& ActorNamePatterns,
	double MaxZVariance,
	TArray<UStaticMeshComponent*>& OutStaticMeshComponents)
{
	UWorld* World = WorldContextObject->GetWorld();
	TArray<AActor*> SMAs;
	TArray<UStaticMeshComponent*> SMCs;
	UGameplayStatics::GetAllActorsOfClass(
		WorldContextObject,
		AStaticMeshActor::StaticClass(),
		SMAs);
	for (AActor* SMA : SMAs)
	{
		SMA->GetComponents(SMCs);
		FilterLandscapeLikeStaticMeshComponentsByVariance(SMCs, MaxZVariance);
		FilterLandscapeLikeStaticMeshComponentsByPatterns(SMCs, ActorNamePatterns);
		OutStaticMeshComponents.Append(SMCs);
		SMCs.Reset();
	}
	OutStaticMeshComponents.Shrink();
}
