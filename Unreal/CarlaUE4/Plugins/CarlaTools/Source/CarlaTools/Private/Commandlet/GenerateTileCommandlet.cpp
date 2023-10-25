// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "GenerateTileCommandlet.h"

#include <iostream>
#include <fstream>

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerateTileCommandlet);


UGenerateTileCommandlet::UGenerateTileCommandlet()
{
#if WITH_EDITOR
  ConstructorHelpers::FClassFinder<UOpenDriveToMap> OpenDrivelassFinder(TEXT("/CarlaTools/OnroadMapGenerator/BP_OpenDriveToMap"));
  OpenDriveClass = OpenDrivelassFinder.Class;
#endif
}

UGenerateTileCommandlet::UGenerateTileCommandlet(const FObjectInitializer& Initializer)
  : Super(Initializer)
{
#if WITH_EDITOR
  ConstructorHelpers::FClassFinder<UOpenDriveToMap> OpenDrivelassFinder(TEXT("/CarlaTools/OnroadMapGenerator/BP_OpenDriveToMap"));
  OpenDriveClass = OpenDrivelassFinder.Class;
#endif
}

#if WITH_EDITORONLY_DATA

int32 UGenerateTileCommandlet::Main(const FString &Params)
{
  UE_LOG(LogCarlaToolsMapGenerateTileCommandlet, Log, TEXT("UGenerateTileCommandlet::Main Arguments %s"), *Params);
  TArray<FString> Tokens;
  TArray<FString> Switches;
  TMap<FString,FString> ParamsMap;

  ParseCommandLine(*Params, Tokens, Switches, ParamsMap );

  std::string logstr = std::string(TCHAR_TO_UTF8(*Params));
  std::ofstream file("CommandletParameters.txt", std::ios::app);
  file << logstr << std::endl;

  for( auto fstr : Tokens )
  {
    logstr = std::string(TCHAR_TO_UTF8(*fstr));
    file << " Tokens " << logstr << std::endl;
  }

  for( auto fstr : Switches )
  {
    logstr = std::string(TCHAR_TO_UTF8(*fstr));
    file << " SWITCHES " << logstr << std::endl;
  }

  for( auto PairMap : ParamsMap )
  {
    std::string key = std::string(TCHAR_TO_UTF8(*PairMap.Key));
    std::string value = std::string(TCHAR_TO_UTF8(*PairMap.Value));

    file << "Key: " << key << " Value: " << value << std::endl;
  }



  OpenDriveMap = NewObject<UOpenDriveToMap>(this, OpenDriveClass);
  OpenDriveMap->FilePath = ParamsMap["FilePath"];
  OpenDriveMap->BaseLevelName = ParamsMap["BaseLevelName"];
  OpenDriveMap->OriginGeoCoordinates = FVector2D(FCString::Atof(*ParamsMap["GeoCoordsX"]),FCString::Atof(*ParamsMap["GeoCoordsY"]));
  OpenDriveMap->CurrentTilesInXY = FIntVector(FCString::Atof(*ParamsMap["CTileX"]),FCString::Atof(*ParamsMap["CTileY"]), 0);
  // Parse Params
  OpenDriveMap->GenerateTile();
  return 0;
}

#endif
