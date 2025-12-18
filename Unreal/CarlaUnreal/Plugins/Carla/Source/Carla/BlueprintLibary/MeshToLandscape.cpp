// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "MeshToLandscape.h"

#include <util/ue-header-guard-begin.h>
#include "Landscape.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshLODResourcesAdapter.h"
#include "PhysicsEngine/PhysicsObjectExternalInterface.h"
#include "Async/ParallelFor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/CollisionProfile.h"
#include <util/ue-header-guard-end.h>

static TAutoConsoleVariable<int32> CVDrawDebugLandscapeTraces(
	TEXT("CARLA.DrawDebugLandscapeTraces"),
	0,
	TEXT("Whether to debug-draw the traces during landscape construction."));

static TAutoConsoleVariable<int32> CVDrawDebugLandscapeTraceFailures(
	TEXT("CARLA.DrawDebugLandscapeTraceFailures"),
	0,
	TEXT("Whether to debug-draw trace failures during landscape construction."));

static TAutoConsoleVariable<int32> CVMeshToLandscapeMaxTraceRetries(
	TEXT("CARLA.MeshToLandscape.MaxTraceRetries"),
	8,
	TEXT("Max parallel line trace retries."));

static TAutoConsoleVariable<int32> CVDrawDebugBoxes(
	TEXT("CARLA.MeshToLandscape.DrawDebugBoxes"),
	0,
	TEXT("Whether to debug-draw the bounding box used for tracing."));

constexpr int32 KernelSide = 3;

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
			bool OK = SM->HasValidRenderData();
			OK = OK && (
#if WITH_EDITOR
				!SM->IsNaniteEnabled() ||
#endif
				SM->HasValidNaniteData());
			if (OK)
			{
				++i;
				continue;
			}
			else
			{
				UE_LOG(
					LogCarla, Warning,
					TEXT("Skipping static mesh asset %s due to invalid RenderData or invalid Nanite data."),
					*UKismetSystemLibrary::GetDisplayName(Component));
			}
		}
		else
		{
			UE_LOG(
				LogCarla, Warning,
				TEXT("Skipping static mesh asset %s due to missing static mesh data (GetStaticMesh returned nullptr)."),
				*UKismetSystemLibrary::GetDisplayName(Component));
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
	if (PatternWhitelist.IsEmpty() &&
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
	int32 NumSubsections,
	int32 SampleFrequency,
	const TArray<FHitDisplacementEntry>& HitDisplacementMap)
{
#if WITH_EDITOR
	if (Components.Num() == 0)
		return nullptr;

	int32 ComponentSizeQuads = SubsectionSizeQuads * NumSubsections;

	if (ComponentSizeQuads == 0)
	{
		UE_LOG(
			LogCarla, Warning,
			TEXT("Skipping landscape generation, SubsectionSizeQuads=%i NumSubsections=%i"),
			SubsectionSizeQuads,
			NumSubsections);
		return nullptr;
	}

	UWorld* World = Components[0]->GetWorld();

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

	FVector3d Max = Bounds.Max;
	FVector3d Min = Bounds.Min;
	FVector3d Range = Max - Min;
	FVector3d Center = Bounds.GetCenter();

	FVector3d RangeMeters = Range * UE_CM_TO_M;
	FIntPoint DesiredVertexCount = FIntPoint(
		FMath::Max(1, (int32)FMath::CeilToInt(RangeMeters.X * SampleFrequency) + 1),
		FMath::Max(1, (int32)FMath::CeilToInt(RangeMeters.Y * SampleFrequency) + 1));

	FIntPoint DesiredQuadCount = (DesiredVertexCount - FIntPoint(1)).ComponentMax(FIntPoint(1));
	FIntPoint ComponentSizeQuads2 = FIntPoint(ComponentSizeQuads);
	FIntPoint RequiredQuadCount =
		((DesiredQuadCount + ComponentSizeQuads2 - 1) / ComponentSizeQuads2) *
		ComponentSizeQuads2;
	FIntPoint HeightmapExtent =
		RequiredQuadCount +
		FIntPoint(1);

	TArray<uint16_t> HeightmapData;
	HeightmapData.SetNumZeroed(HeightmapExtent.X * HeightmapExtent.Y);

	FVector2d CellSize = FVector2d(Range) / (FVector2d(HeightmapExtent) - FVector2d(1.0));

	{
		const TSet<UActorComponent*> ComponentMap(Components);
		TArray<TPair<FVector3d, FVector3d>> Failures;
		FCriticalSection FailuresCS;

		int32 MaxRetries = CVMeshToLandscapeMaxTraceRetries.GetValueOnAnyThread();

		constexpr auto RingSampleCount = 8U;
		const double I2R = 2.0 * PI / (double)RingSampleCount;

		double MaxOffsetRadius = CellSize.GetMin() * 0.5;

		FVector2d OffsetRing[RingSampleCount];
		for (size_t i = 0; i != RingSampleCount; ++i)
		{
			FVector2d D;
			FMath::SinCos(&D.Y, &D.X, (double)i * I2R);
			OffsetRing[i] = D;
		}

		FCollisionShape TestGeometry;
		TestGeometry.SetBox(
			FVector3f(
				CellSize.X,
				CellSize.Y,
				UE_M_TO_CM) * 2.5F);

		auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(
			World->GetPhysicsScene());

		ParallelFor(HeightmapData.Num(), [&](int32 Index)
		{
			int32 Y = Index / HeightmapExtent.X;
			int32 X = Index % HeightmapExtent.X;

			FVector2d XY = FVector2d(Min) + CellSize * FVector2d(X, Y);
			FVector3d Begin0 = FVector3d(XY.X, XY.Y, Max.Z);
			FVector3d End0 = FVector3d(XY.X, XY.Y, Min.Z);
			FVector3d Begin = Begin0;
			FVector3d End = End0;
			FHitResult Hit;
			double HitZ = Min.Z;

			FCollisionQueryParams CQParams = FCollisionQueryParams::DefaultQueryParam;
			CQParams.bTraceComplex = true;
			CQParams.bFindInitialOverlaps = true;
			CQParams.bReturnPhysicalMaterial = false;
			CQParams.MobilityType = EQueryMobilityType::Any;
			CQParams.bIgnoreTouches = true;

			FRandomStream PRNG(Index);

			if (!World->ParallelSweepSingleByChannel(
				Hit,
				Begin,
				End,
				FQuat::Identity,
				ECollisionChannel::ECC_WorldStatic,
				TestGeometry,
				CQParams))
			{
				HeightmapData[Index] = 0;
				return;
			}

			int32 Retry = 0;
			for (; Retry < MaxRetries; ++Retry) // Sometimes, LTSBC fails on geometry seams
			{
				if (World->ParallelLineTraceSingleByChannel(
					Hit,
					Begin, End,
					ECollisionChannel::ECC_GameTraceChannel2,
					CQParams))
				{
					if (ComponentMap.Contains(Hit.GetComponent())) // Done
					{
						HitZ = Hit.Location.Z;
						break;
					}
					CQParams.AddIgnoredComponent(Hit.GetComponent());
				}
				else
				{
					int32 Index = PRNG.RandRange(0, (int32)RingSampleCount - 1);
					FVector3d Offset = FVector3d(OffsetRing[Index], 0.0);
					Offset *= PRNG.FRandRange(0.0, MaxOffsetRadius);
					Begin = Begin0 + Offset;
					End = End0 + Offset;
				}
			}

			if (Retry != MaxRetries)
			{
				UPrimitiveComponent* HitComponent = Hit.GetComponent();
				FString TestName;
				double ZDisplacement = 0.0;
				for (auto& [Pattern, Displacement] : HitDisplacementMap)
				{
					if (std::abs(Displacement) <= std::abs(ZDisplacement))
						continue;
					TestName = UKismetSystemLibrary::GetDisplayName(HitComponent);
					if (TestName.MatchesWildcard(Pattern))
					{
						ZDisplacement = Displacement;
						continue;
					}
					TestName = UKismetSystemLibrary::GetDisplayName(HitComponent->GetOwner());
					if (TestName.MatchesWildcard(Pattern))
					{
						ZDisplacement = Displacement;
						continue;
					}
				}
				HitZ += ZDisplacement;
				HitZ -= Min.Z;
				HitZ *= UE_CM_TO_M;
				HitZ += 256.0;
				HitZ /= 512.0;
				HitZ = std::clamp(HitZ, 0.0, 1.0);
				HeightmapData[Index] = (uint16)std::lround(
					HitZ * TNumericLimits<uint16>::Max());
			}
			else
			{
				FailuresCS.Lock();
				Failures.Add({ Begin, End });
				FailuresCS.Unlock();
				HeightmapData[Index] = 0;
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
		}

		if (CVDrawDebugLandscapeTraceFailures.GetValueOnAnyThread())
		{
			for (auto [Begin, End] : Failures)
				DrawDebugLine(World, Begin, End, FColor::Red, false, 20.0F);
		}

		if (Failures.Num() != 0 && false)
		{
			FString FailureString;
			for (auto [Begin, End] : Failures)
			{
				FailureString += FString::Printf(
					TEXT("Failed to trace against world from (%f, %f, %f) to (%f, %f, %f), too many objects.\n"),
					Begin.X, Begin.Y, Begin.Z,
					End.X, End.Y, End.Z);
			}
			FFileHelper::SaveStringToFile(
				FailureString,
				*FString::Printf(TEXT("TraceFailures.txt")));
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
	Landscape->bUseCompressedHeightmapStorage = true;

	TMap<FGuid, TArray<uint16>> LayerHeightMaps;
	LayerHeightMaps.Add(FGuid(), MoveTemp(HeightmapData));

	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerImportInfos;
	LayerImportInfos.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

	FVector3d LandscapeScale = FVector3d(CellSize, 100.0);
	Landscape->SetActorTransform(FTransform(
		FQuat::Identity,
		Min,
		LandscapeScale));

	Landscape->Import(
		LandscapeGUID,
		0, 0,
		HeightmapExtent.X - 1, HeightmapExtent.Y - 1,
		NumSubsections, SubsectionSizeQuads,
		LayerHeightMaps,
		nullptr,
		LayerImportInfos,
		ELandscapeImportAlphamapType::Additive,
		TArrayView<const FLandscapeLayer>());

	Landscape->SetLandscapeGuid(LandscapeGUID);

	Landscape->CreateLandscapeInfo();

	Landscape->ReregisterAllComponents();
	Landscape->RecreateCollisionComponents();
	Landscape->PostEditChange();
	Landscape->MarkPackageDirty();

	return Landscape;
#else
	return nullptr;
#endif
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
