// Copyright 2017 Mike Fricker. All Rights Reserved.

#pragma once

#include "StreetMapActor.generated.h"

/** An actor that renders a street map mesh component */
UCLASS( BlueprintType, Blueprintable ) // Physics category in detail panel is hidden. Our component/Actor is not simulated !
class STREETMAPRUNTIME_API AStreetMapActor : public AActor
{
	GENERATED_BODY()

public:
  AStreetMapActor(const FObjectInitializer& ObjectInitializer);

	/**  Component that represents a section of street map roads and buildings */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "StreetMap")
		class UStreetMapComponent* StreetMapComponent;

	FORCEINLINE class UStreetMapComponent* GetStreetMapComponent() { return StreetMapComponent; }

	UFUNCTION(BlueprintCallable)
	void SetNewStreetMap(UStreetMap* NewMap);

  UFUNCTION(BlueprintCallable)
	TArray<AActor*> GenerateTopOfBuildings(FString MapName, UMaterialInstance* MaterialInstance);
  UFUNCTION(BlueprintCallable)
	AActor* GenerateTopOfBuilding(int Index, FString MapName, UMaterialInstance* MaterialInstance);
};
