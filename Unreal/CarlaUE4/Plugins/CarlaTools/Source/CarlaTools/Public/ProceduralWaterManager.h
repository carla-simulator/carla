// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

#include "Components/SplineComponent.h"
#include "EditorUtilityWidget.h"
#include "Math/Vector.h"

#include "ProceduralWaterManager.generated.h"

USTRUCT(BlueprintType)
struct CARLATOOLS_API FProceduralRiversMetaInfo
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
	FString WaterInfoPath;

	UPROPERTY(BlueprintReadWrite)
	int CustomSampling;

	UPROPERTY(BlueprintReadWrite)
	float CustomScaleFactor;

	UPROPERTY(BlueprintReadWrite)
	float CustomRiverWidth;

	UPROPERTY(BlueprintReadWrite)
	FVector CustomLocationOffset;

	UPROPERTY(BlueprintReadWrite)
	float CustomHeight;

	UPROPERTY(BlueprintReadWrite)
	int SizeOfLandscape;

	UPROPERTY(BlueprintReadWrite)
	int SizeOfTexture;
};

UCLASS(BlueprintType)
class CARLATOOLS_API UProceduralWaterManager : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UProceduralWaterManager();

public:	
	/// Main function to be called from the widget to start all the generation process
	/// @a metaInfo is the input data for this process
	UFUNCTION(BlueprintCallable)
	FString StartWaterGeneration(const FProceduralRiversMetaInfo metaInfo);

	/// Add river @a riverActor spline point @a splinePoint to SplinePoint  
	/// collection to be added in later processes to the spline component.
	/// This is implemented in blueprint code.
	UFUNCTION(BlueprintImplementableEvent)
	void AddRiverPointFromCode(AActor* riverActor, FSplinePoint splinePoint);

	/// It checks which is the direction the flow of the river depending on the
	/// altitude of the start and the end of the river @a riverActor
	UFUNCTION(BlueprintImplementableEvent)
	void CheckAndReverseWaterFlow(AActor* riverActor);

private:
	TSubclassOf<class AActor> RiverBlueprintClass;

	/// Instantiate a new actor of type RiverBlueprintClass
	/// Returns the the actor created
	UFUNCTION()
	AActor* SpawnRiverBlueprintActor();

	/// Calculates the height of the landscape in an specific 2D coordinate ( @a x, @a y)
	/// throwing rays and detecting the hit point. @a bDrawDebugLines allows to visualize
	/// the rays in the viewport, only for debug purposes.
	/// Return the Z value
	UFUNCTION()
	float GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines = false);
};
