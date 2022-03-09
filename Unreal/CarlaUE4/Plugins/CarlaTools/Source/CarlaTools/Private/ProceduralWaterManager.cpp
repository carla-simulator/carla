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

// Sets default values
UProceduralWaterManager::UProceduralWaterManager()
{
 	// Pass
}

FString UProceduralWaterManager::StartWaterGeneration(const FProceduralRiversMetaInfo metaInfo)
{
	FString errorMsg="";
	
	if(metaInfo.WaterGenerationType == EWaterGenerationType::RIVERS)
	{
		errorMsg = RiverGeneration(metaInfo);
	}
	else if(metaInfo.WaterGenerationType == EWaterGenerationType::LAKE)
	{
		errorMsg = LakeGeneration(metaInfo);
	}
	else
		errorMsg = "Error in Water Generation Type";

	return errorMsg;
}

FString UProceduralWaterManager::RiverGeneration(const FProceduralRiversMetaInfo metaInfo)
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
		else if (line == "# _L")
		{
			return "This is a LAKE file. Check the water type and the file content.";
		}
		else
		{
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

FString UProceduralWaterManager::LakeGeneration(const FProceduralRiversMetaInfo metaInfo)
{
	const FString WaterInfoPath = metaInfo.WaterInfoPath;
	if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*WaterInfoPath))
	{
		
		DEBUG_MSG("File Not Found!! :(");
		return "File Not Found!! :(";
	}

	TArray<FString> file;
	FFileHelper::LoadFileToStringArray(file, *WaterInfoPath);

	if(file.Num() == 0)
		return "Error processing file. Check file path and it content.";

	FString centerX_str, centerY_str, sizeX_str, sizeY_str, angle_str;

	AActor* lakeActor = nullptr;

	for(FString line : file)
	{
		if(line == "# _L")
		{
			lakeActor = SpawnLakeBlueprintActor();
		}
		else if(line == "# _")
		{
			return "This is a RIVER file. Check the water type and the file content.";
		}
		else
		{
			TArray<FString> lineArray;

			line.ParseIntoArray(lineArray, TEXT(" "));

			float centerX = FCString::Atof(*lineArray[0]);
			float centerY = FCString::Atof(*lineArray[1]);
			float sizeX = 	FCString::Atof(*lineArray[2]);
			float sizeY = 	FCString::Atof(*lineArray[3]);
			float angle = 	FCString::Atof(*lineArray[4]);

			float centerZ;

			if(metaInfo.CustomHeight > -100000000.0f)
				centerZ = metaInfo.CustomHeight;
			else
				centerZ = GetLandscapeSurfaceHeight(centerX, centerY, false);

			FVector location(metaInfo.CustomScaleFactor*centerX, metaInfo.CustomScaleFactor*centerY, centerZ);
			location += metaInfo.CustomLocationOffset;

			FRotator rotation(0.0f, angle,0.0f);

			FVector scale(metaInfo.CustomRiverWidth*sizeX, metaInfo.CustomRiverWidth*sizeY, 1.0f);

			lakeActor->SetActorScale3D(scale);
			lakeActor->SetActorLocationAndRotation(location, rotation, false, 0, ETeleportType::None);
			
		}
	
	}

	// Last river created must be destroyed as it is a wildcard
	if(lakeActor != nullptr)
		lakeActor->Destroy();

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

AActor* UProceduralWaterManager::SpawnLakeBlueprintActor()
{
	
	FVector location(0, 0, 0);
	FRotator rotation(0,0,0);
	FActorSpawnParameters spawnInfo;
	
	UWorld* World = GetWorld();
	AActor* riverActor =  World->SpawnActor<AActor>(LakeBlueprintClass, location, rotation, spawnInfo);

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