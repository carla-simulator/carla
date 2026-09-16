// Copyright 2017 Mike Fricker. All Rights Reserved.

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "StreetMapActorFactory.h"
#include "StreetMapImporting.h"

#ifndef __has_include
#include "AssetData.h"
#else
#if __has_include("AssetData.h")
#include "AssetData.h"
#endif
#endif

#include "StreetMapActor.h"
#include "StreetMapComponent.h"
#include "StreetMap.h"


//////////////////////////////////////////////////////////////////////////
// UStreetMapActorFactory

UStreetMapActorFactory::UStreetMapActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("StreetMap", "StreetMapFactoryDisplayName", "Add StreetMap Actor");
	NewActorClass = AStreetMapActor::StaticClass();
}

void UStreetMapActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	if (UStreetMap* StreetMapAsset = Cast<UStreetMap>(Asset))
	{
		AStreetMapActor* StreetMapActor = CastChecked<AStreetMapActor>(NewActor);
		UStreetMapComponent* StreetMapComponent = StreetMapActor->GetStreetMapComponent();
		StreetMapComponent->SetStreetMap(StreetMapAsset, false, false);
	}
}

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 3
void UStreetMapActorFactory::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	unimplemented();
}
#else
void UStreetMapActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != nullptr && CDO != nullptr)
	{
		UStreetMap* StreetMapAsset = CastChecked<UStreetMap>(Asset);
		AStreetMapActor* StreetMapActor = CastChecked<AStreetMapActor>(CDO);
		UStreetMapComponent* StreetMapComponent = StreetMapActor->GetStreetMapComponent();
		StreetMapComponent->SetStreetMap(StreetMapAsset, true, false);
	}
}
#endif

bool UStreetMapActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	return (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(UStreetMap::StaticClass()));
}
