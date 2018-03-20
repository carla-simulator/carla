// Fill out your copyright notice in the Description page of Project Settings.

#include "Carla.h"
#include "LidarDescription.h"

#include "Util/IniFile.h"

void ULidarDescription::Load(const FIniFile &Config, const FString &Section)
{
  Super::Load(Config, Section);
  Config.GetInt(*Section, TEXT("Channels"), Channels);
  Config.GetFloat(*Section, TEXT("Range"), Range, 1e2);
  Config.GetInt(*Section, TEXT("PointsPerSecond"), PointsPerSecond);
  Config.GetFloat(*Section, TEXT("RotationFrequency"), RotationFrequency);
  Config.GetFloat(*Section, TEXT("UpperFovLimit"), UpperFovLimit);
  Config.GetFloat(*Section, TEXT("LowerFovLimit"), LowerFovLimit);
  Config.GetBool(*Section, TEXT("ShowDebugPoints"), ShowDebugPoints);
}

void ULidarDescription::Validate()
{
  Channels = (Channels == 0u ? 32u : Channels);
  FMath::Clamp(Range, 0.10f, 50000.0f);
  PointsPerSecond = (PointsPerSecond == 0u ? 56000u : PointsPerSecond);
  FMath::Clamp(RotationFrequency, 0.001f, 50000.0f);
  FMath::Clamp(UpperFovLimit, -89.9f, 90.0f);
  FMath::Clamp(LowerFovLimit, -90.0f, UpperFovLimit);
}

void ULidarDescription::Log() const
{
  Super::Log();
  UE_LOG(LogCarla, Log, TEXT("Channels = %d"), Channels);
  UE_LOG(LogCarla, Log, TEXT("Range = %f"), Range);
  UE_LOG(LogCarla, Log, TEXT("PointsPerSecond = %d"), PointsPerSecond);
  UE_LOG(LogCarla, Log, TEXT("RotationFrequency = %f"), RotationFrequency);
  UE_LOG(LogCarla, Log, TEXT("UpperFovLimit = %f"), UpperFovLimit);
  UE_LOG(LogCarla, Log, TEXT("LowerFovLimit = %f"), LowerFovLimit);
}
