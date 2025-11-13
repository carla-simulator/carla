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

static TAutoConsoleVariable<int32> CVDrawDebugLandscapeTraces(
	TEXT("CARLA.DrawDebugLandscapeTraces"),
	0,
	TEXT("Whether to debug-draw the traces during landscape construction from static mesh components."));

void UMeshToLandscapeUtil::FilterStaticMeshComponentsByVariance(
	TArray<UPrimitiveComponent*>& Components,
	double MaxZVariance)
{
	if (MaxZVariance < 0.0)
		return;
	for (int32 i = 0; i != Components.Num();)
	{
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Components[i]);
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
			Components.RemoveAtSwap(i, EAllowShrinking::No);
		else
			++i;
	}
}

void UMeshToLandscapeUtil::FilterComponentsByPatterns(
	TArray<UPrimitiveComponent*>& Components,
	const TArray<FString>& Patterns)
{
	FString NameTemp;
	if (Patterns.IsEmpty())
		return;
	for (int32 i = 0; i != Components.Num();)
	{
		UPrimitiveComponent* SMC = Components[i];
		bool Match = false;
		SMC->GetName(NameTemp);
		for (const FString& Pattern : Patterns)
		{
			Match = NameTemp.MatchesWildcard(Pattern);
			if (Match)
				break;
		}
		if (!Match)
		{
			SMC->GetOwner()->GetName(NameTemp);
			for (const FString& Pattern : Patterns)
			{
				Match = NameTemp.MatchesWildcard(Pattern);
				if (Match)
					break;
			}
		}
		if (!Match)
			Components.RemoveAtSwap(i, EAllowShrinking::No);
		else
			++i;
	}
}

ALandscape* UMeshToLandscapeUtil::ConvertMeshesToLandscape(
	const TArray<UPrimitiveComponent*>& Components,
	int32 SubsectionSizeQuads,
	int32 NumSubsections)
{
	if (Components.Num() == 0)
		return nullptr;
	
	UWorld* World = Components[0]->GetWorld();

	double Scale = 100.0;

	auto EncodeZ = [&](double Z)
	{
		const uint16 U16Max = TNumericLimits<uint16>::Max();
		Z /= Scale;
		Z += 256.0;
		Z /= 512.0;
		Z *= (double)U16Max;
		return std::min<uint16>((uint16)std::lround(Z), U16Max);
	};

	double Limit = 1e32;
	FVector3d Max = FVector3d(-Limit);
	FVector3d Min = FVector3d(Limit);

	for (UPrimitiveComponent* Component : Components)
	{
		check(Component->GetWorld() == World);
		AActor* Owner = Component->GetOwner();
		check(Owner != nullptr);
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Component);
		if (SMC == nullptr)
		{
			FBox Box = Component->Bounds.GetBox();
			Min = FVector3d::Min(Box.Min, Min);
			Max = FVector3d::Max(Box.Max, Max);
			continue;
		}
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

	int32 ComponentSizeQuads = SubsectionSizeQuads * NumSubsections;
	double MetersPerQuad = 5.0;

	FVector3d Range = Max - Min;
	FVector3d Center = (Max + Min) / 2;
	FIntPoint RangeMeters = FIntPoint( // QuadCount
		(int32)std::lround(Range.X * UE_CM_TO_M),
		(int32)std::lround(Range.Y * UE_CM_TO_M));
	int32 NumComponents = RangeMeters.GetMax() / (ComponentSizeQuads * MetersPerQuad);
	FIntPoint HeightmapExtent = RangeMeters;

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumZeroed(HeightmapExtent.X * HeightmapExtent.Y);

	if (HeightmapData.Num() == 0)
	{
		// @TODO: Add Warning.
		return nullptr;
	}

	{
		FVector3d RangeInv = FVector3d::OneVector / Range;
		FVector2d CellSize = FVector2d(Range) / (FVector2d(HeightmapExtent) - FVector2d(1.0));

		if (CVDrawDebugLandscapeTraces.GetValueOnAnyThread())
		{
			for (int32 Index = 0; Index != HeightmapData.Num(); ++Index)
			{
				int32 Y = Index / HeightmapExtent.X;
				int32 X = Index % HeightmapExtent.X;
				FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
				FVector3d Begin = FVector3d(XY.X, XY.Y, Max.Z);
				FVector3d End = FVector3d(XY.X, XY.Y, Min.Z);
				DrawDebugLine(World, Begin, End, FColor::Red, false, 10.0F);
			}
		}

		TSet<UPrimitiveComponent*> ComponentMap(Components);

		auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(World->GetPhysicsScene());
		{
			TArray<std::atomic<uint16_t>> SharedHeightmapData;
			SharedHeightmapData.SetNumUninitialized(HeightmapData.Num());
			for (std::atomic<uint16_t>& Value : SharedHeightmapData)
				std::construct_at(&Value, 0);

			ParallelFor(HeightmapData.Num(), [&](int32 Index)
			{
				int32 Y = Index / HeightmapExtent.X;
				int32 X = Index % HeightmapExtent.X;
				std::atomic<uint16_t>& TargetCell = SharedHeightmapData[Index];
				FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
				FVector3d Begin = FVector3d(XY.X, XY.Y, Max.Z);
				FVector3d End = FVector3d(XY.X, XY.Y, Min.Z);
				FHitResult Hit;
				double HitZ = 0.0F;

				FCollisionQueryParams CQParams =
					FCollisionQueryParams(FName(TEXT("Heightmap query trace")));
				CQParams.bTraceComplex = true;
				CQParams.bFindInitialOverlaps = true;
				for (int32 Retry = 0; Retry != 32; ++Retry)
				{
					if (!World->ParallelLineTraceSingleByChannel(
						Hit,
						Begin, End,
						ECollisionChannel::ECC_GameTraceChannel2,
						CQParams,
						FCollisionResponseParams::DefaultResponseParam))
					{
						break;
					}
					if (ComponentMap.Contains(Hit.GetComponent()))
					{
						HitZ = Hit.Location.Z;
						break;
					}
					CQParams.AddIgnoredComponent(Hit.GetComponent());
				}

				uint16 Desired = EncodeZ(HitZ);
				while (true)
				{
					uint16 Current = TargetCell.load(std::memory_order_acquire);
					if (Current > Desired)
						break;
					if (TargetCell.compare_exchange_weak(
						Current, Desired,
						std::memory_order_release, std::memory_order_relaxed))
					{
						// UE_LOG(LogCarla, Warning, TEXT("Set (%i, %i) to %u"), X, Y, Desired);
						return;
					}
				}
			}, EParallelForFlags::Unbalanced);

			for (int32 i = 0; i != SharedHeightmapData.Num(); ++i)
				HeightmapData[i] = SharedHeightmapData[i].load(std::memory_order_relaxed);
		}
		LockedPhysObject.Release();
	}

	FActorSpawnParameters SpawnParams;
	ALandscape* Landscape = World->SpawnActor<ALandscape>(
		ALandscape::StaticClass(),
		FVector3d::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
	FGuid LandscapeGUID = FGuid::NewGuid();

	Landscape->ComponentSizeQuads = ComponentSizeQuads;
	Landscape->SubsectionSizeQuads = SubsectionSizeQuads;
	Landscape->NumSubsections = NumSubsections;

	Landscape->SetLandscapeGuid(LandscapeGUID);
	Landscape->CreateLandscapeInfo();
	Landscape->SetActorTransform(FTransform(
		FQuat::Identity,
		FVector3d(Min.X, Min.Y, 0.0),
		FVector3d(100.0)));
	
	TMap<FGuid, TArray<uint16>> LayerHeightMaps;
	LayerHeightMaps.Add(FGuid(), MoveTemp(HeightmapData));

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerImportInfos;
	LayerImportInfos.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
	
	Landscape->Import(
		LandscapeGUID,
		0, 0,
		HeightmapExtent.X - 1, HeightmapExtent.Y - 1,
		NumSubsections, SubsectionSizeQuads,
		LayerHeightMaps,
		nullptr,
		LayerImportInfos,
		ELandscapeImportAlphamapType::Layered);

	Landscape->EnableNaniteSkirts(true, 0.1F, false);

#if WITH_EDITOR
	Landscape->RegisterAllComponents();
	Landscape->RecreateCollisionComponents();
	Landscape->PostEditChange();
	Landscape->CreateLandscapeInfo();
	Landscape->MarkPackageDirty();
#endif

	return Landscape;
}

void UMeshToLandscapeUtil::EnumerateLandscapeLikeStaticMeshComponents(
	UObject* WorldContextObject,
	const TArray<FString>& ActorNamePatterns,
	const TArray<UClass*>& ClassWhitelist,
	const TArray<UClass*>& ClassBlacklist,
	double MaxZVariance,
	TArray<UPrimitiveComponent*>& OutComponents)
{
	UWorld* World = WorldContextObject->GetWorld();
	TArray<AActor*> Actors;
	TArray<UPrimitiveComponent*> Components;
	UGameplayStatics::GetAllActorsOfClass(
		WorldContextObject,
		AActor::StaticClass(),
		Actors);
	for (AActor* Actor : Actors)
	{
		bool AnySMC = false;
		Actor->GetComponents(Components);
		for (int32 i = 0; i != Components.Num();)
		{
			UPrimitiveComponent* Component = Components[i];

			bool IsWhitelisted = false;
			for (UClass* Class : ClassWhitelist)
			{
				IsWhitelisted = Component->IsA(Class);
				if (IsWhitelisted)
					break;
			}

			if (IsWhitelisted)
			{
				++i;
				continue;
			}

			bool IsBlacklisted = false;
			for (UClass* Class : ClassBlacklist)
			{
				IsBlacklisted = Component->IsA(Class);
				if (IsBlacklisted)
					break;
			}

			if (IsBlacklisted)
			{
				Components.RemoveAtSwap(i, EAllowShrinking::No);
				continue;
			}

			bool IsSMC = Component->IsA<UStaticMeshComponent>();
			AnySMC = AnySMC || IsSMC;
			if (!IsSMC)
			{
				++i;
				continue;
			}
			UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Component);
			UStaticMesh* SM = SMC->GetStaticMesh();
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
			Components.RemoveAtSwap(i, EAllowShrinking::No);
		}
		if (AnySMC)
			FilterStaticMeshComponentsByVariance(Components, MaxZVariance);
		FilterComponentsByPatterns(Components, ActorNamePatterns);
		OutComponents.Append(Components);
		Components.Reset();
	}
	OutComponents.Shrink();
}
