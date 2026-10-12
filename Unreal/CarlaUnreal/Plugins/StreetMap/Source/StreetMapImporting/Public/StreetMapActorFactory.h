// Copyright 2017 Mike Fricker. All Rights Reserved.

#pragma once
#include "ActorFactories/ActorFactory.h"
#include "StreetMapActorFactory.generated.h"

UCLASS()
class UStreetMapActorFactory : public UActorFactory
{
	GENERATED_UCLASS_BODY()

		//~ Begin UActorFactory Interface
		virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 3
		virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#else
		virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
#endif
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory Interface
};
