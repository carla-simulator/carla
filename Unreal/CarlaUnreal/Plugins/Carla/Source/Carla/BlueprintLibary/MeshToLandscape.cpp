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

static TAutoConsoleVariable<int32> CVMeshToLandscapeMaxTraceRetries(
	TEXT("CARLA.MeshToLandscape.MaxTraceRetries"),
	256,
	TEXT("Max parallel line trace retries."));

static TAutoConsoleVariable<int32> CVDrawDebugBoxes(
	TEXT("CARLA.MeshToLandscape.DrawDebugBoxes"),
	0,
	TEXT("Whether to debug-draw the bounding box used for tracing."));

constexpr int32 KernelSide = 3;

static void ComputeBinomialKernel(
	uint16 Out[KernelSide][KernelSide])
{
	int32 N = KernelSide;

	auto BinCoef = [](int32 n, int32 x)
	{
		int32 r = 1;
		for (int32 i = 1; i != n; ++i)
			r *= (n + 1 - i);
		for (int32 i = 1; i != n; ++i)
			r /= i;
		return r;
	};

	for (int32 i = 0; i != KernelSide; ++i)
	{
		auto a = BinCoef(N - 1, i);
		for (int32 j = 0; j != KernelSide; ++j)
		{
			auto b = BinCoef(N - 1, j);
			Out[i][j] = a * b >> (2 * (N - 1));
		}
	}
}

static void ApplyKernel(
	TArrayView<uint16> Image,
	FIntPoint Extent,
	uint16 Kernel[KernelSide][KernelSide])
{
	TArray<uint16> Temp;
	Temp.SetNumUninitialized(Image.Num());
	ParallelFor(Image.Num(), [&](int32 Index)
	{
		int32 Y = Index / Extent.X;
		int32 X = Index % Extent.X;
	});
}

void UMeshToLandscapeUtil::FilterByClassList(
	TArray<UActorComponent*>& Components,
	const TArray<UClass*>& Blacklist,
	const TArray<UClass*>& Whitelist)
{
	for (int32 i = 0; i != Components.Num();)
	{
		UActorComponent* Component = Components[i];

		bool IsWhitelisted = false;
		bool IsBlacklisted = false;

		for (UClass* Class : Whitelist)
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

		for (UClass* Class : Blacklist)
		{
			IsBlacklisted = Component->IsA(Class);
			if (IsBlacklisted)
				break;
		}

		if (!IsBlacklisted)
		{
			++i;
			continue;
		}

		Components.RemoveAtSwap(i, EAllowShrinking::No);
	}
}

void UMeshToLandscapeUtil::FilterInvalidStaticMeshComponents(
	TArray<UActorComponent*>& Components)
{
	for (int32 i = 0; i != Components.Num();)
	{
		UActorComponent* Component = Components[i];
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Component);
		if (SMC == nullptr)
		{
			++i;
			continue;
		}
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
}

void UMeshToLandscapeUtil::FilterStaticMeshComponentsByVariance(
	TArray<UActorComponent*>& Components,
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
	TArray<UActorComponent*>& Components,
	const TArray<FString>& PatternWhitelist,
	const TArray<FString>& PatternBlacklist)
{
	if (PatternWhitelist.IsEmpty() ||
		PatternBlacklist.IsEmpty())
		return;
	for (int32 i = 0; i != Components.Num();)
	{
		UActorComponent* Component = Components[i];

		bool Match = false;

		FString ComponentName =
			UKismetSystemLibrary::GetDisplayName(Component);

		FString ActorName =
			UKismetSystemLibrary::GetDisplayName(Component->GetOwner());

		for (const FString& Pattern : PatternBlacklist)
		{
			Match = ComponentName.MatchesWildcard(Pattern);
			if (Match)
				break;
		}

		if (Match)
		{
			Components.RemoveAtSwap(i, EAllowShrinking::No);
			continue;
		}

		for (const FString& Pattern : PatternBlacklist)
		{
			Match = ActorName.MatchesWildcard(Pattern);
			if (Match)
				break;
		}

		if (Match)
		{
			Components.RemoveAtSwap(i, EAllowShrinking::No);
			continue;
		}

		for (const FString& Pattern : PatternWhitelist)
		{
			Match = ComponentName.MatchesWildcard(Pattern);
			if (Match)
				break;
		}

		if (Match)
		{
			++i;
			continue;
		}

		for (const FString& Pattern : PatternWhitelist)
		{
			Match = ActorName.MatchesWildcard(Pattern);
			if (Match)
				break;
		}

		if (Match)
		{
			++i;
			continue;
		}
		Components.RemoveAtSwap(i, EAllowShrinking::No);
	}
}

ALandscape* UMeshToLandscapeUtil::ConvertMeshesToLandscape(
	const TArray<UActorComponent*>& Components,
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

	FBox3d Bounds = FBox3d(ForceInit);

	for (UActorComponent* Component : Components)
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
		if (PrimitiveComponent != nullptr)
		{
			FBox BoxBounds = PrimitiveComponent->Bounds.GetBox();
			if (CVDrawDebugBoxes.GetValueOnAnyThread())
			{
				DrawDebugBox(
					World,
					BoxBounds.GetCenter(),
					BoxBounds.GetExtent(),
					FColor::Cyan,
					false,
					10.0F);
			}
			Bounds += BoxBounds;
		}
	}

	int32 ComponentSizeQuads = SubsectionSizeQuads * NumSubsections;
	
	FVector3d Max = Bounds.Max;
	FVector3d Min = Bounds.Min;
	FVector3d Range = Max - Min;
	FVector3d Center = (Max + Min) / 2;
	FIntPoint RangeMeters = FIntPoint( // QuadCount
		(int32)std::lround(Range.X * UE_CM_TO_M),
		(int32)std::lround(Range.Y * UE_CM_TO_M));
	FIntPoint HeightmapExtent = RangeMeters;

	int32 HeightmapNum = HeightmapExtent.X * HeightmapExtent.Y;

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumZeroed(HeightmapExtent.X * HeightmapExtent.Y);

	if (HeightmapData.Num() == 0)
	{
		// @TODO: Add Warning.
		return nullptr;
	}

	{
		FVector2d CellSize = FVector2d(Range) / (FVector2d(HeightmapExtent) - FVector2d(1.0));

		const TSet<UActorComponent*> ComponentMap(Components);
		TArray<TPair<FVector3d, FVector3d>> Failures;
		FCriticalSection FailuresCS;

		int32 MaxRetries = CVMeshToLandscapeMaxTraceRetries.GetValueOnAnyThread();

		auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(
			World->GetPhysicsScene());

		ParallelFor(HeightmapData.Num(), [&](int32 Index)
		{
			int32 Y = Index / HeightmapExtent.X;
			int32 X = Index % HeightmapExtent.X;

			FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
			FVector3d Begin = FVector3d(XY.X, XY.Y, Max.Z);
			FVector3d End = FVector3d(XY.X, XY.Y, Min.Z);
			FHitResult Hit;
			double HitZ = 0.0F;

			FCollisionQueryParams CQParams = FCollisionQueryParams::DefaultQueryParam;
			CQParams.bTraceComplex = true;
			CQParams.bFindInitialOverlaps = true;
			CQParams.bReturnPhysicalMaterial = false;
			CQParams.MobilityType = EQueryMobilityType::Any;
			CQParams.bIgnoreTouches = true;

			FRandomStream PRNG(Index);

			bool Failed = false;
			int32 Retry = 0;
			for (; Retry != MaxRetries; ++Retry)
			{
				if (!World->ParallelLineTraceSingleByChannel(
					Hit,
					Begin, End,
					ECollisionChannel::ECC_GameTraceChannel2,
					CQParams))
				{
					// double dx = PRNG.FRandRange(0.0, UE_CM_TO_M * 5);
					// double dy = PRNG.FRandRange(0.0, UE_CM_TO_M * 5);
					// Begin.X += dx;
					// Begin.Y += dy;
					// End.X += dx;
					// End.Y += dy;
					// continue;
					break;
				}
				if (ComponentMap.Contains(Hit.GetComponent()))
				{
					HitZ = Hit.Location.Z;
					break;
				}
				CQParams.AddIgnoredComponent(Hit.GetComponent());
			}

			HeightmapData[Index] = EncodeZ(HitZ);

			if (Retry == MaxRetries)
			{
				FailuresCS.Lock();
				Failures.Add({ Begin, End });
				FailuresCS.Unlock();
			}
		}, EParallelForFlags::Unbalanced);
		LockedPhysObject.Release();

		if (CVDrawDebugBoxes.GetValueOnAnyThread())
		{
			DrawDebugBox(
				World,
				Bounds.GetCenter(),
				Bounds.GetExtent(),
				FColor::Yellow,
				false,
				15.0F);
		}

		if (CVDrawDebugLandscapeTraces.GetValueOnAnyThread() != 0)
		{
			for (int32 Index = 0; Index != HeightmapData.Num(); ++Index)
			{
				int32 Y = Index / HeightmapExtent.X;
				int32 X = Index % HeightmapExtent.X;
				FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
				FVector3d Begin = FVector3d(XY.X, XY.Y, Max.Z);
				FVector3d End = FVector3d(XY.X, XY.Y, Min.Z);
				DrawDebugLine(World, Begin, End, FColor::Green, false, 10.0F);
			}

			for (auto [Begin, End] : Failures)
				DrawDebugLine(World, Begin, End, FColor::Red, false, 20.0F);
		}

		for (auto [Begin, End] : Failures)
		{
			UE_LOG(
				LogCarla,
				Warning,
				TEXT("Failed to trace against world from (%f, %f, %f) to (%f, %f, %f), too many objects."),
				Begin.X, Begin.Y, Begin.Z,
				End.X, End.Y, End.Z);
		}
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

	// Landscape->EnableNaniteSkirts(true, 0.1F, false);

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
	const TArray<FString>& PatternWhitelist,
	const TArray<FString>& PatternBlacklist,
	const TArray<UClass*>& ClassWhitelist,
	const TArray<UClass*>& ClassBlacklist,
	double MaxZVariance,
	TArray<UActorComponent*>& OutComponents)
{
	UWorld* World = WorldContextObject->GetWorld();
	TArray<AActor*> Actors;
	TArray<UActorComponent*> Components;
	UGameplayStatics::GetAllActorsOfClass(
		WorldContextObject,
		AActor::StaticClass(),
		Actors);
	for (AActor* Actor : Actors)
	{
		bool AnySMC = false;
		
		Actor->GetComponents(Components);
		for (UActorComponent* PrimitiveComponent : Components)
		{
			AnySMC = PrimitiveComponent->IsA<UStaticMeshComponent>();
			if (AnySMC)
				break;
		}

		FilterByClassList(Components, ClassBlacklist, ClassWhitelist);
		if (AnySMC)
		{
			FilterInvalidStaticMeshComponents(Components);
			FilterStaticMeshComponentsByVariance(Components, MaxZVariance);
		}
		FilterComponentsByPatterns(
			Components,
			PatternWhitelist,
			PatternBlacklist);
		OutComponents.Append(Components);
		Components.Reset();
	}
	OutComponents.Shrink();
}
