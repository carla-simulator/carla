// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "StreetMapActor.h"
#include "StreetMapRuntime.h"
#include "StreetMapComponent.h"


AStreetMapActor::AStreetMapActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StreetMapComponent = CreateDefaultSubobject<UStreetMapComponent>(TEXT("StreetMapComp"));
	RootComponent = StreetMapComponent;
}

void AStreetMapActor::SetNewStreetMap(UStreetMap* NewMap){
	StreetMapComponent->SetStreetMap(NewMap);
}

TArray<UStaticMesh*> AStreetMapActor::GenerateTopOfBuildings(FString MapName, UMaterialInstance* MaterialInstance)
{
	return StreetMapComponent->GenerateTopsOfBuildings(MapName, MaterialInstance);
}

UStaticMesh* AStreetMapActor::GenerateTopOfBuilding(int Index, FString MapName, UMaterialInstance* MaterialInstance)
{
	return StreetMapComponent->GenerateTopOfBuilding(Index, MapName, MaterialInstance);
}

TArray<USplineComponent*> AStreetMapActor::SpawnTaggedTerrainSplines()
{
	if(StreetMapComponent == nullptr || StreetMapComponent->GetStreetMap() == nullptr)
	{
		UE_LOG(LogStreetMapObject, Warning, TEXT("StreetMapComponent or StreetMap is not set. Cannot spawn tagged terrain splines."));
		return TArray<USplineComponent*>();
	}
	return StreetMapComponent->GetStreetMap()->SpawnTaggedTerrainSplines(GetWorld());
}
