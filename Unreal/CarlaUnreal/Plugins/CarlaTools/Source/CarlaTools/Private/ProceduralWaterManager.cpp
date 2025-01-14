// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ProceduralWaterManager.h"

#include <util/ue-header-guard-begin.h>
#include "Components/SplineComponent.h"
#include "FileHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/CString.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include <Containers/UnrealString.h>
#include <util/ue-header-guard-end.h>

// Sets default values
UProceduralWaterManager::UProceduralWaterManager()
{
   // Pass
}

FString UProceduralWaterManager::StartWaterGeneration(const FProceduralRiversMetaInfo MetaInfo)
{
  FString ErrorMsg="";

  if(MetaInfo.WaterGenerationType == EWaterGenerationType::RIVERS)
  {
    if(RiverBlueprintClass)
      ErrorMsg = RiverGeneration(MetaInfo);
    else
      ErrorMsg = "ERROR: River class not assigned";
  }
  else if(MetaInfo.WaterGenerationType == EWaterGenerationType::LAKE)
  {
    ErrorMsg = LakeGeneration(MetaInfo);
  }
  else
    ErrorMsg = "Error in Water Generation Type";

  return ErrorMsg;
}

FString UProceduralWaterManager::RiverGeneration(const FProceduralRiversMetaInfo MetaInfo)
{
  FString ErrorMsg;

  const FString WaterInfoPath = MetaInfo.WaterInfoPath;
  if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*WaterInfoPath))
  {
    ErrorMsg = "File Not Found!! :(";
    return ErrorMsg;
  }

  TArray<FString> File;
  FFileHelper::LoadFileToStringArray(File, *WaterInfoPath);

  float InputKeyCount = 0.0f;
  int IterationNumber = 0;

  AActor* RiverActor = nullptr;

  FString StrX, StrY;
  FVector PreviusPosition(NAN, NAN, NAN);

  float ScaleFactor = MetaInfo.CustomScaleFactor;

  if(File.Num() == 0)
    return "Error processing file. Check file path and it content.";

  for(FString Line : File)
  {
    if(Line == "# _")
    {
      // Important IF to add last point for every spline
      // Uses data from previus iteration
      if(IterationNumber != 0 && IterationNumber != -1)
      {
        // Add Last point to river spline
        FSplinePoint Location(InputKeyCount, PreviusPosition);
        if(RiverActor != nullptr)
          AddRiverPointFromCode(RiverActor, Location); // Last Point
        CheckAndReverseWaterFlow(RiverActor);
      }

      RiverActor = SpawnRiverBlueprintActor();
      InputKeyCount = 0.0f;
      IterationNumber = -1;  // Wildcard value used for headers
    }
    else if (Line == "# _L")
    {
      return "This is a LAKE file. Check the water type and the file content.";
    }
    else
    {
      if(!Line.Split(TEXT(" "), &StrY, &StrX))
      {
        return "ERROR: Coordinated cannot be proccess... Check file format";
      }  
      
      float PositionX = ScaleFactor*FCString::Atof(*StrX);
      float PositionY = ScaleFactor*FCString::Atof(*StrY);
      float PositionZ;

      if(MetaInfo.CustomHeight > -100000000.0f)
        PositionZ = MetaInfo.CustomHeight;
      else
        PositionZ = GetLandscapeSurfaceHeight(PositionX, PositionY, false);

      FVector Position(PositionX, PositionY, PositionZ);
      

      Position += MetaInfo.CustomLocationOffset;

      if((IterationNumber % MetaInfo.CustomSampling) == 0){
        FSplinePoint NewPoint(InputKeyCount, Position);
        float Width = (MetaInfo.CustomRiverWidth > 0.0f) ? 
            MetaInfo.CustomRiverWidth : 2.5f;
        NewPoint.Scale = FVector(1.0f, Width, 1.0f);
        if(RiverActor != nullptr)
          AddRiverPointFromCode(RiverActor, NewPoint);
        InputKeyCount += 1.0f;  
      }
      PreviusPosition = Position;
    }
    IterationNumber++;
  }

  // Last river created must be destroyed as it is a wildcard
  if(RiverActor != nullptr)
    RiverActor->Destroy();

  return "Successfully processed";
}

FString UProceduralWaterManager::LakeGeneration(const FProceduralRiversMetaInfo MetaInfo)
{
  const FString WaterInfoPath = MetaInfo.WaterInfoPath;
  if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*WaterInfoPath))
  {
    return "File Not Found!! :(";
  }

  TArray<FString> File;
  FFileHelper::LoadFileToStringArray(File, *WaterInfoPath);

  if(File.Num() == 0)
    return "Error processing file. Check file path and it content.";

  AActor* LakeActor = nullptr;

  for(FString Line : File)
  {
    if(Line == "# _L")
    {
      LakeActor = SpawnLakeBlueprintActor();
    }
    else if(Line == "# _")
    {
      return "This is a RIVER file. Check the water type and the file content.";
    }
    else
    {
      TArray<FString> LineArray;

      Line.ParseIntoArray(LineArray, TEXT(" "));

      float CenterX = FCString::Atof(*LineArray[0]);
      float CenterY = FCString::Atof(*LineArray[1]);
      float SizeX =   FCString::Atof(*LineArray[2]);
      float SizeY =   FCString::Atof(*LineArray[3]);
      float Angle =   FCString::Atof(*LineArray[4]);

      float CenterZ;

      if(MetaInfo.CustomHeight > -100000000.0f)
        CenterZ = MetaInfo.CustomHeight;
      else
        CenterZ = GetLandscapeSurfaceHeight(CenterX, CenterY, false);

      FVector Location(
          MetaInfo.CustomScaleFactor*CenterX, 
          MetaInfo.CustomScaleFactor*CenterY, 
          CenterZ);

      Location += MetaInfo.CustomLocationOffset;

      FRotator Rotation(0.0f, Angle, 0.0f);

      FVector Scale(
          MetaInfo.CustomRiverWidth * SizeX, 
          MetaInfo.CustomRiverWidth * SizeY, 
          1.0f);

      LakeActor->SetActorScale3D(Scale);
      LakeActor->SetActorLocationAndRotation(
          Location, 
          Rotation, 
          false, 
          0, 
          ETeleportType::None);
      
    }
  
  }

  // Last river created must be destroyed as it is a wildcard
  if(LakeActor != nullptr)
    LakeActor->Destroy();

  return "Successfully processed";
}

AActor* UProceduralWaterManager::SpawnRiverBlueprintActor()
{
  
  FVector Location(0, 0, 0);
  FRotator Rotation(0,0,0);
  FActorSpawnParameters SpawnInfo;
  
  UWorld* World = GetWorld();
  AActor* RiverActor =  World->SpawnActor<AActor>(
      RiverBlueprintClass, 
      Location, 
      Rotation, 
      SpawnInfo);

  return RiverActor;
}

AActor* UProceduralWaterManager::SpawnLakeBlueprintActor()
{
  
  FVector Location(0, 0, 0);
  FRotator Rotation(0,0,0);
  FActorSpawnParameters SpawnInfo;
  
  UWorld* World = GetWorld();
  AActor* LakeActor =  World->SpawnActor<AActor>(
      LakeBlueprintClass, 
      Location, 
      Rotation, 
      SpawnInfo);

  return LakeActor;
}

float UProceduralWaterManager::GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines)
{
  UWorld* World = GetWorld();

  if(World)
  {
    FVector RayStartingPoint(x, y, 9999999);
    FVector RayEndPoint(x, y, -9999999);

    // Raytrace
    FHitResult HitResult;
    World->LineTraceSingleByObjectType(
        OUT HitResult,
        RayStartingPoint,
        RayEndPoint,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
        FCollisionQueryParams());

    // Draw debug line.
    if (bDrawDebugLines)
    {
      FColor LineColor;

      if (HitResult.GetActor()) LineColor = FColor::Red;
      else LineColor = FColor::Green;

      DrawDebugLine(
          World,
          RayStartingPoint,
          RayEndPoint,
          LineColor,
          true,
          5.f,
          0.f,
          10.f);
    }

    // Return Z Location.
    if (HitResult.GetActor()) return HitResult.ImpactPoint.Z;
  }
  return 0.0f;
}


bool UProceduralWaterManager::CreateRiverPresetFiles(TSubclassOf<AActor> RiverParentClass)
{
  // TODO
  return true;
}
