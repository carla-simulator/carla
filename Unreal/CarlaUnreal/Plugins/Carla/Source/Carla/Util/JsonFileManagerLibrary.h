// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Sensor/Radar.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "JsonFileManagerLibrary.generated.h"

class FJsonValue;
class FJsonObject;

UCLASS()
class CARLA_API UJsonFileManagerLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  using FRadarData = carla::sensor::data::RadarData;

public:
  static bool ReadStringFromFile(const FString& FilePath, FString& DataRead);

  static bool WriteStringFromFile(const FString& FilePath, const FString& DataToWrite);

  static bool ReadJson(const FString& JsonFilePath, TSharedPtr<FJsonObject>& JsonObject);

  static bool WriteJson(const FString& JsonFilePath, const TSharedPtr<FJsonObject> JsonObject);
  
  static void RoundVectorToTwoDecimals(FVector& InOutVector);

  static FVector JsonObjectToVector(const TSharedPtr<FJsonObject>& JsonObject);

  static TSharedPtr<FJsonObject> VectorToJsonObject(const FVector& Vector);

  static bool SaveIMUDataToJson(const FString& JsonFilePath, const FVector& Accelerometer, const FVector& Gyroscope, float Compass, const FString& FrameNumber);

  static bool SaveGnssDataToJson(const FString& JsonFilePath, double Altitude, double Latitude, double Longitude, const FString& FrameNumber);

  // ToDo: Modify the parameter and add a reference to an FVector of a struct that contains the Velocity, Azimuth, Altitude and Depth.
  static bool SaveRadarDataToJson(const FString& JsonFilePath, const FRadarData& Rays, const FString& FrameNumber);

  static bool SaveLidarDataToPly(const FString& FilePath, const TArray<float>& PointArray, int ArrayElementSize);


protected:
  static TSharedPtr<FJsonObject> GetSensorJsonData(const FString& JsonFilePath, TArray<TSharedPtr<FJsonValue>>& ArrayValue);

  static bool SaveSensorJsonData(const FString& JsonFilePath, const TSharedPtr<FJsonObject> SensorDataToSave,
    TSharedPtr<FJsonObject> JsonDataObject, TArray<TSharedPtr<FJsonValue>>& ArrayValue);
};
