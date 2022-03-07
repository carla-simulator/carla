// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralWaterManager.h"

#include "Components/SplineComponent.h"
#include "FileHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/CString.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "UnrealString.h"

#include "Actor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

// #define DEBUG_MSG(x, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT(x), __VA_ARGS__));}

// Sets default values
UProceduralWaterManager::UProceduralWaterManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	// static ConstructorHelpers::FObjectFinder<UBlueprint> DummyBlueprint(TEXT("Blueprint'/CarlaTools/ProceduralWaterManager/DummyActor.DummyActor'"));
    // if (DummyBlueprint.Object){
    //     MyDummyBlueprint = (UClass*)DummyBlueprint.Object->GeneratedClass;
    // }
	static ConstructorHelpers::FObjectFinder<UBlueprint> RiverBlueprint(TEXT("Blueprint'/Game/racer-sim/Blueprints/Water/BP_River.BP_River'"));
    if (RiverBlueprint.Object){
        RiverBlueprintClass = (UClass*)RiverBlueprint.Object->GeneratedClass;
    }

}

FString UProceduralWaterManager::StartWaterGeneration(const FProceduralRiversMetaInfo metaInfo)
{
	FString errorMsg;

	const FString WaterInfoPath = metaInfo.WaterInfoPath;
	if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*WaterInfoPath))
	{
		errorMsg = "File Not Found!! :(";
		DEBUG_MSG("File Not Found!! :(");
		return errorMsg;
	}

	TArray<FString> file;
	FFileHelper::LoadFileToStringArray(file, *WaterInfoPath);

	TArray<FSplinePoint> riverPoints;

	float inputKeyCount = 0.0f;
	int iterationNumber = 0;

	AActor* riverActor = nullptr;

	FString x_str, y_str;
	FVector previusPosition(NAN, NAN, NAN);

	float scaleFactor = metaInfo.CustomScaleFactor;
	FVector locationOffset(0,0,0);

	if(file.Num() == 0)
		return "Error processing file. Check file path and it content.";

	for(FString line : file)
	{
		if(line == "# _")
		{
			// Important IF to add last point for every spline
			// Uses data from previus iteration
			if(iterationNumber != 0 && iterationNumber != -1)
			{
				// Add Last point to river spline
				FSplinePoint location(inputKeyCount, previusPosition);
				if(riverActor != nullptr)
					AddRiverPointFromCode(riverActor, location); // Last Point
				CheckAndReverseWaterFlow(riverActor);
			}

			riverActor = SpawnRiverBlueprintActor();
			inputKeyCount = 0.0f;
			iterationNumber = -1;	// Wildcard value used for headers
		}
		else{
			if(!line.Split(TEXT(" "), &y_str, &x_str))
			{
				return "ERROR: Coordinated cannot be proccess... Check file format";
			}	
			
			float positionX = scaleFactor*FCString::Atof(*x_str);
			float positionY = scaleFactor*FCString::Atof(*y_str);
			float positionZ;

			if(metaInfo.CustomHeight > -100000000.0f)
				positionZ = metaInfo.CustomHeight;
			else
				positionZ = GetLandscapeSurfaceHeight(positionX, positionY, false);

			FVector position(positionX, positionY, positionZ);
			

			position += metaInfo.CustomLocationOffset;

			if((iterationNumber % metaInfo.CustomSampling) == 0){
				FSplinePoint newPoint(inputKeyCount, position);
				float width = (metaInfo.CustomRiverWidth > 0.0f) ? metaInfo.CustomRiverWidth : 2.5f;
				newPoint.Scale = FVector(1.0f, width, 1.0f);
				if(riverActor != nullptr)
					AddRiverPointFromCode(riverActor, newPoint);
				inputKeyCount += 1.0f;	
			}
			previusPosition = position;
		}
		iterationNumber++;
	}

	// Last river created must be destroyed as it is a wildcard
	if(riverActor != nullptr)
		riverActor->Destroy();

	return "Successfully processed";
}

AActor* UProceduralWaterManager::SpawnRiverBlueprintActor()
{
	
	FVector location(0, 0, 0);
	FRotator rotation(0,0,0);
	FActorSpawnParameters spawnInfo;
	
	UWorld* World = GetWorld();
	AActor* riverActor =  World->SpawnActor<AActor>(RiverBlueprintClass, location, rotation, spawnInfo);

	return riverActor;
}

float UProceduralWaterManager::GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines)
{
	UWorld* world = GetWorld();

	if(world)
	{
		FVector RayStartingPoint(x, y, 999999);
		FVector RayEndPoint(x, y, 0);

		// Raytrace
		FHitResult HitResult;
		world->LineTraceSingleByObjectType(
			OUT HitResult,
			RayStartingPoint,
			RayEndPoint,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
			FCollisionQueryParams()
		);

		// Draw debug line.
		if (bDrawDebugLines)
		{
			FColor LineColor;

			if (HitResult.GetActor()) LineColor = FColor::Red;
			else LineColor = FColor::Green;

			DrawDebugLine(
				world,
				RayStartingPoint,
				RayEndPoint,
				LineColor,
				true,
				5.f,
				0.f,
				10.f
			);
		}

		// Return Z location.
		if (HitResult.GetActor()) return HitResult.ImpactPoint.Z;
	}
	return 0.0f;
}