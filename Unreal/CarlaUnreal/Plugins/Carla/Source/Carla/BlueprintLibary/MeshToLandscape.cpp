// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "MeshToLandscape.h"

#include <util/ue-header-guard-begin.h>
#include "Landscape.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "StaticMeshLODResourcesAdapter.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Async/ParallelFor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/CollisionProfile.h"
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
	FIntPoint HeightmapExtent,
	int32 SubsectionCount,
	int32 SubsectionSizeQuads)
{
	if (StaticMeshComponents.Num() == 0)
		return nullptr;
	
	UWorld* World = StaticMeshComponents[0]->GetWorld();

	constexpr uint16 HeightmapZero = 32768;

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumUninitialized(HeightmapExtent.X * HeightmapExtent.Y);
	for (uint16& Value : HeightmapData)
		Value = HeightmapZero;

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
		AActor* SMCOwner = SMC->GetOwner();
		check(SMCOwner != nullptr);
		UStaticMesh* SM = SMC->GetStaticMesh();
		check(SM->HasValidRenderData());
		check(!SM->IsNaniteEnabled() || SM->HasValidNaniteData());
		const FStaticMeshLODResources& LOD = SM->GetLODForExport(0);
		FStaticMeshLODResourcesMeshAdapter Adapter(&LOD);
		int32 VertexCount = LOD.GetNumVertices();
		for (int32 i = 0; i != LOD.GetNumVertices(); ++i)
		{
			FVector3d Vertex = Adapter.GetVertex(i);
			Vertex = SMC->GetComponentTransform().TransformPosition(Vertex);
			Max = FVector3d::Max(Max, Vertex);
			Min = FVector3d::Min(Min, Vertex);
		}
	}

	{
		FVector3d Range = Max - Min;
		FVector3d RangeInv = FVector3d::OneVector / Range;
		FVector2d CellSize = FVector2d(Min) / FVector2d(HeightmapExtent);

		TArray<std::atomic<uint16_t>> SharedHeightmapData;
		SharedHeightmapData.SetNumUninitialized(HeightmapData.Num());
		for (std::atomic<uint16_t>& Value : SharedHeightmapData)
			std::construct_at(&Value, 0);

		auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(World->GetPhysicsScene());
		ParallelFor(HeightmapData.Num(), [&](int32 Index)
		{
			int32 Y = Index / HeightmapExtent.X;
			int32 X = Index % HeightmapExtent.X;
			std::atomic<uint16_t>& TargetCell = SharedHeightmapData[Index];
			FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
			FVector3d Begin = FVector3d(XY.X, XY.Y, Max.Z);
			FVector3d End = FVector3d(XY.X, XY.Y, Max.Z);
			FHitResult Hit;
			FCollisionQueryParams CQParams =
				FCollisionQueryParams(FName(TEXT("Heightmap query trace")));
			CQParams.bTraceComplex = true;
			CQParams.bFindInitialOverlaps = true;
			if (World->ParallelLineTraceSingleByChannel(
				Hit,
				Begin, End,
				ECollisionChannel::ECC_EngineTraceChannel2,
				CQParams,
				FCollisionResponseParams::DefaultResponseParam))
			{
				FVector3d HitLocation = Hit.Location;
				double HitZ = HitLocation.Z;
				// ???:
				HitZ -= Min.Z;
				HitZ *= RangeInv.Z;
				check(HitZ + 1e-4 > 0);
				HitZ -= 0.5;
				HitZ *= Range.Z;
				uint16 Desired = (int32)std::clamp<int32>(
					HitZ, 0, std::numeric_limits<uint16>::max()) +
					HeightmapZero;
				while (true)
				{
					uint16 Current = TargetCell.load(std::memory_order_acquire);
					if (Current > Desired)
						break;
					if (TargetCell.compare_exchange_weak(
						Current, Desired,
						std::memory_order_release, std::memory_order_relaxed))
					{
						UE_LOG(LogCarla, Warning, TEXT("Set (%i, %i) to %u"), X, Y, Desired);
						return;
					}
				}
			}
		}, EParallelForFlags::BackgroundPriority);

		for (int32 i = 0; i != SharedHeightmapData.Num(); ++i)
			HeightmapData[i] = SharedHeightmapData[i].load(std::memory_order_relaxed);

		LockedPhysObject.Release();
	}

	FActorSpawnParameters SpawnParams;
	ALandscape* Landscape = World->SpawnActor<ALandscape>(
		ALandscape::StaticClass(),
		Min,
		FRotator::ZeroRotator,
		SpawnParams);
	FGuid LandscapeGUID = FGuid::NewGuid();

	TMap<FGuid, TArray<uint16>> LayerHeightMaps;
	LayerHeightMaps.Add(FGuid(), MoveTemp(HeightmapData));

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerImportInfos;
	LayerImportInfos.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

	Landscape->Import(
		LandscapeGUID,
		0, 0,
		HeightmapExtent.X - 1, HeightmapExtent.Y - 1,
		SubsectionCount, SubsectionSizeQuads,
		LayerHeightMaps,
		nullptr,
		LayerImportInfos,
		ELandscapeImportAlphamapType::Layered);

#if WITH_EDITOR
	Landscape->PostEditChange();
	Landscape->RegisterAllComponents();
	Landscape->MarkPackageDirty();
#endif

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
		for (int32 i = 0; i != SMCs.Num();)
		{
			UStaticMesh* SM = SMCs[i]->GetStaticMesh();
			if (SM != nullptr) // Some SMCs have NULL SM.
			{
				if (SM->HasValidRenderData() && (!SM->IsNaniteEnabled() || SM->HasValidNaniteData()))
				{
					++i;
					continue;
				}
				else
				{
					// @TODO: ADD WARNING
				}
			}
			else
			{
				// @TODO: ADD WARNING
			}
			SMCs.RemoveAtSwap(i, EAllowShrinking::No);
		}
		FilterLandscapeLikeStaticMeshComponentsByVariance(SMCs, MaxZVariance);
		FilterLandscapeLikeStaticMeshComponentsByPatterns(SMCs, ActorNamePatterns);
		OutStaticMeshComponents.Append(SMCs);
		SMCs.Reset();
	}
	OutStaticMeshComponents.Shrink();
}
