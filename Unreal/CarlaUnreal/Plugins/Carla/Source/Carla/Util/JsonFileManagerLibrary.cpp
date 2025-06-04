// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Util/JsonFileManagerLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Kismet/KismetStringLibrary.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY_STATIC(LogJsonFileManagerLibrary, Verbose, All);

bool UJsonFileManagerLibrary::ReadStringFromFile(const FString& FilePath, FString& DataRead)
{
  if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
  {
    DataRead = "";
    return false;
  }

  if(!FFileHelper::LoadFileToString(DataRead, *FilePath))
  {
    DataRead = "";
    return false;
  }

  return true;
}

bool UJsonFileManagerLibrary::WriteStringFromFile(const FString& FilePath, const FString& DataToWrite)
{
  if(!FFileHelper::SaveStringToFile(DataToWrite, *FilePath))
  {
    return false;
  }

  return true;
}

bool UJsonFileManagerLibrary::ReadJson(const FString& JsonFilePath, TSharedPtr<FJsonObject>& JsonObject)
{
  FString JsonString;
  const bool bReadSuccess = UJsonFileManagerLibrary::ReadStringFromFile(JsonFilePath, JsonString);
  if(!bReadSuccess)
  {
    return false;
  }

  if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), JsonObject))
  {
    return false;
  }
  
  return true;
}

bool UJsonFileManagerLibrary::WriteJson(const FString& JsonFilePath, const TSharedPtr<FJsonObject> JsonObject)
{
  if(!JsonObject.IsValid())
  {
    return false;
  }
  
  FString JsonString;
  if(!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString)))
  {
    return false;
  }
  
  const bool bWriteSuccess = UJsonFileManagerLibrary::WriteStringFromFile(JsonFilePath, JsonString);
  if(!bWriteSuccess)
  {
    return false;
  }
  
  return true;
}

void UJsonFileManagerLibrary::RoundVectorToTwoDecimals(FVector& InOutVector)
{
  InOutVector *= 100.f;
  InOutVector.X = FGenericPlatformMath::RoundToFloat(InOutVector.X);
  InOutVector.Y = FGenericPlatformMath::RoundToFloat(InOutVector.Y);
  InOutVector.Z = FGenericPlatformMath::RoundToFloat(InOutVector.Z);
  InOutVector /= 100.f;
}

FVector UJsonFileManagerLibrary::JsonObjectToVector(const TSharedPtr<FJsonObject>& JsonObject)
{
  FVector Vector = FVector::ZeroVector;
  if(!JsonObject.IsValid())
  {
    return Vector;
  }

  const FString VectorStringX = JsonObject->GetStringField(TEXT("X"));
  const FString VectorStringY = JsonObject->GetStringField(TEXT("Y"));
  const FString VectorStringZ = JsonObject->GetStringField(TEXT("Z"));
  Vector = FVector{ FCString::Atof(*VectorStringX),FCString::Atof(*VectorStringY),FCString::Atof(*VectorStringZ) };
  
  /* Same result than the previous.
  float ValueParsedX, ValueParsedY, ValueParsedZ;
  FDefaultValueHelper::ParseFloat(JsonObject->GetStringField(TEXT("X")), ValueParsedX);
  Vector.X = ValueParsedX;
  FDefaultValueHelper::ParseFloat(JsonObject->GetStringField(TEXT("Y")), ValueParsedY);
  Vector.Y = ValueParsedY;
  FDefaultValueHelper::ParseFloat(JsonObject->GetStringField(TEXT("Z")), ValueParsedZ);
  Vector.Z = ValueParsedZ;*/
  
  RoundVectorToTwoDecimals(Vector);
  
  return Vector;
}

TSharedPtr<FJsonObject> UJsonFileManagerLibrary::VectorToJsonObject(const FVector& Vector)
{
  const TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
  JsonObject->SetStringField(TEXT("X"), FString::SanitizeFloat(Vector.X));
  JsonObject->SetStringField(TEXT("Y"), FString::SanitizeFloat(Vector.Y));
  JsonObject->SetStringField(TEXT("Z"), FString::SanitizeFloat(Vector.Z));
  
  return JsonObject;
}

bool UJsonFileManagerLibrary::SaveIMUDataToJson(const FString& JsonFilePath, const FVector& Accelerometer,
                                                const FVector& Gyroscope, float Compass, const FString& FrameNumber)
{
  TArray<TSharedPtr<FJsonValue>> ArrayValue;
  TSharedPtr<FJsonObject> JsonDataObject = GetSensorJsonData(JsonFilePath, ArrayValue);

  // Accelerometer and Gyroscope
  const TSharedPtr<FJsonObject> IMUDataObject = MakeShared<FJsonObject>();
  IMUDataObject->SetStringField(TEXT("Frame"), FrameNumber);
  IMUDataObject->SetObjectField(TEXT("Accelerometer"), VectorToJsonObject(Accelerometer));
  IMUDataObject->SetObjectField( TEXT("Gyroscope"), VectorToJsonObject(Gyroscope));

  // Compass
  const TSharedPtr<FJsonObject> CompassJsonObject = MakeShared<FJsonObject>();
  FString Heading = Compass > 270.5f || Compass < 89.5f ? "N" : "";
  Heading += Compass > 90.5f && Compass < 269.5f ? "S" : "";
  Heading += Compass > 0.5f && Compass < 179.5f ? "E" : "";
  Heading += Compass > 180.5f && Compass < 359.5f ? "W" : "";
  CompassJsonObject->SetStringField(TEXT("Degrees"), FString::SanitizeFloat(Compass));
  CompassJsonObject->SetStringField(TEXT("Sign"), Heading);
  IMUDataObject->SetObjectField(TEXT("Compass"), CompassJsonObject);

  const bool bWriteSuccess = SaveSensorJsonData(JsonFilePath, IMUDataObject, JsonDataObject, ArrayValue);
  return bWriteSuccess;
}

bool UJsonFileManagerLibrary::SaveGnssDataToJson(const FString& JsonFilePath, double Altitude, double Latitude,
  double Longitude, const FString& FrameNumber)
{
  TArray<TSharedPtr<FJsonValue>> ArrayValue;
  TSharedPtr<FJsonObject> JsonDataObject = GetSensorJsonData(JsonFilePath, ArrayValue);
  
  const TSharedPtr<FJsonObject> IMUDataObject = MakeShared<FJsonObject>();
  IMUDataObject->SetStringField(TEXT("Frame"), FrameNumber);
  IMUDataObject->SetStringField(TEXT("Altitude"), FString::SanitizeFloat(Altitude));
  IMUDataObject->SetStringField( TEXT("Latitude"), FString::SanitizeFloat(Latitude));
  IMUDataObject->SetStringField(TEXT("Longitude"), FString::SanitizeFloat(Longitude));

  const bool bWriteSuccess = SaveSensorJsonData(JsonFilePath, IMUDataObject, JsonDataObject, ArrayValue);
  return bWriteSuccess;
}


bool UJsonFileManagerLibrary::SaveLidarDataToPly(const FString& FilePath, const TArray<float>& PointArray, int ArrayElementSize)
{
  FString DataToWrite;
  const bool bReadSuccess = UJsonFileManagerLibrary::ReadStringFromFile(FilePath, DataToWrite);
  if(!bReadSuccess)
  {
    DataToWrite = "";
  }

  FString LineTerminator = "\n";

#if PLATFORM_WINDOWS
  LineTerminator = LINE_TERMINATOR;
#endif  

  // Create header if no data exist
  DataToWrite += FString("ply") + LineTerminator +
    FString("format ascii 1.0") + LineTerminator +
    FString("element vertex ") + FString::FromInt(PointArray.Num()/ArrayElementSize) + LineTerminator +
    FString("property float32 x") + LineTerminator +
    FString("property float32 y") + LineTerminator +
    FString("property float32 z") + LineTerminator +
    FString("property float32 I") + LineTerminator +
    FString("end_header") + LineTerminator;
  
  for(SIZE_T i = 0; i < PointArray.Num(); i+=4)
  {
    if(PointArray.IsValidIndex(i+3))
    {
      // As Unreal is right handed, some visualizers will see the PointCloud Y axis inverted. If we want to get a left handed mesh output, it is necessary to negate the Y.
      DataToWrite += FString::SanitizeFloat(PointArray[i]) + " " + FString::SanitizeFloat(PointArray[i+1]) + " " + FString::SanitizeFloat(PointArray[i+2]) + " " + FString::SanitizeFloat(PointArray[i+3]) + LineTerminator;
    }
  }
  
  const bool bWriteSuccess = UJsonFileManagerLibrary::WriteStringFromFile(FilePath, DataToWrite);
  if(!bWriteSuccess)
  {
    return false;
  }
  
  return true;
}

TSharedPtr<FJsonObject> UJsonFileManagerLibrary::GetSensorJsonData(const FString& JsonFilePath, TArray<TSharedPtr<FJsonValue>>& ArrayValue)
{
  TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
  const bool bReadSuccess = ReadJson(JsonFilePath, JsonObject);

  if(!bReadSuccess)
  {
    UE_LOG(LogJsonFileManagerLibrary, Log, TEXT("UJsonFileManagerLibrary, file not exist will create a new one: %s"), *JsonFilePath);
  }

  if(JsonObject->HasField(TEXT("DataArray")))
  {
    ArrayValue = JsonObject->GetArrayField(TEXT("DataArray"));
  }

  return JsonObject;
}

bool UJsonFileManagerLibrary::SaveSensorJsonData(const FString& JsonFilePath,
  const TSharedPtr<FJsonObject> SensorDataToSave, TSharedPtr<FJsonObject> JsonDataObject,
  TArray<TSharedPtr<FJsonValue>>& ArrayValue)
{
  // Set array field an write Json
  const TSharedRef<FJsonValue> IMUJsonValue = MakeShareable(new FJsonValueObject(SensorDataToSave));
  ArrayValue.Add(IMUJsonValue);
  JsonDataObject->SetArrayField(TEXT("DataArray"), ArrayValue);
  const bool bWriteSuccess = WriteJson(JsonFilePath, JsonDataObject);
  
  return bWriteSuccess;
}

bool UJsonFileManagerLibrary::SaveRadarDataToJson(const FString& JsonFilePath, 
  const FRadarData& Rays, const FString& FrameNumber)
{
  TArray<TSharedPtr<FJsonValue>> ArrayValue;
  TSharedPtr<FJsonObject> JsonDataObject = GetSensorJsonData(JsonFilePath, ArrayValue);
  bool bWriteSuccess = true;
  for( auto& var : Rays.GetDetections() )
  {
    const TSharedPtr<FJsonObject> RadarDataObject = MakeShared<FJsonObject>();
    RadarDataObject->SetStringField(TEXT("Frame"), FrameNumber);
    RadarDataObject->SetStringField(TEXT("Velocity"), FString::SanitizeFloat(var.velocity));
    RadarDataObject->SetStringField(TEXT("Azimuth"), FString::SanitizeFloat(var.azimuth));
    RadarDataObject->SetStringField(TEXT("Altitude"), FString::SanitizeFloat(var.altitude));
    RadarDataObject->SetStringField(TEXT("Depth"), FString::SanitizeFloat(var.depth));
    bWriteSuccess &= SaveSensorJsonData(JsonFilePath, RadarDataObject, JsonDataObject, ArrayValue);
  }

  return bWriteSuccess;
}

