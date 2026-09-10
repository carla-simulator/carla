// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMap.h"
#include "StreetMapRuntime.h"
#include "EditorFramework/AssetImportData.h"
#include "Components/SplineComponent.h"

DEFINE_LOG_CATEGORY(LogStreetMapObject);


UStreetMap::UStreetMap()
{
#if WITH_EDITORONLY_DATA
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		AssetImportData = NewObject<UAssetImportData>( this, TEXT( "AssetImportData" ) );
	}
#endif
}


void UStreetMap::GetAssetRegistryTags( TArray<FAssetRegistryTag>& OutTags ) const
{
#if WITH_EDITORONLY_DATA
	if( AssetImportData )
	{
		OutTags.Add( FAssetRegistryTag( SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden ) );
	}
#endif

	Super::GetAssetRegistryTags( OutTags );
}


TArray<USplineComponent*> UStreetMap::SpawnTaggedTerrainSplines(UWorld* World)
{
	TArray<USplineComponent*> Result;
	if( World == nullptr )
	{
		UE_LOG(LogStreetMapObject, Warning, TEXT("World is null or no terrains available to spawn splines."));
		return TArray<USplineComponent*>();
	}

	if( Terrains.Num() == 0 )
	{
		UE_LOG(LogStreetMapObject, Warning, TEXT("No terrains available to spawn splines."));
		return TArray<USplineComponent*>();
	}

	int index = 0;
	for ( const FStreetMapTerrain& Terrain : Terrains )
	{
		if( Terrain.RoadPoints.Num() < 2 )
		{
			continue; // Skip terrains with less than 2 points
		}

		FVector2D StartLocation = Terrain.RoadPoints[0];
		AActor* TerrainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(StartLocation,0), FRotator::ZeroRotator);
		USplineComponent* SplineComponent = NewObject<USplineComponent>(TerrainActor);

		FString LabelName = FString::Printf(TEXT("%s_%d"), *Terrain.TerrainType, index);

		if( GetTerrainSupportedTypes().Contains(Terrain.TerrainType)){
			LabelName = FString::Printf(TEXT("VegetationContainer_%d"), index);
		}

#if WITH_EDITOR
		TerrainActor->SetActorLabel(LabelName);
#endif
		TerrainActor->SetRootComponent(SplineComponent);
		SplineComponent->RegisterComponent();
		TerrainActor->SetActorLocation(FVector(StartLocation, 0.0f)); // Assuming Z=0 for flat terrain
		if( GetTerrainSupportedTypes().Contains(Terrain.TerrainType) )
		{
			TerrainActor->Tags.Add(FName("VegetationContainer"));
			SplineComponent->SetClosedLoop(true);
		}
		else
		{
			TerrainActor->Tags.Add(FName(*Terrain.TerrainType));
			SplineComponent->SetClosedLoop(false);
		}

		SplineComponent->ClearSplinePoints();
		SplineComponent->UpdateSpline();

		for( const FVector2D & Point : Terrain.RoadPoints )
		{
			FVector WorldLocation = FVector(Point, 0.0f); // Assuming Z=0 for flat terrain
			SplineComponent->AddSplinePoint(WorldLocation, ESplineCoordinateSpace::World);
		}

		for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
		{
			SplineComponent->SetSplinePointType(i, ESplinePointType::Curve, false);
		}
		index++;
		SplineComponent->ComponentTags.Add(FName(*Terrain.TerrainType));
		Result.Add(SplineComponent);
	}
	return Result;
}
