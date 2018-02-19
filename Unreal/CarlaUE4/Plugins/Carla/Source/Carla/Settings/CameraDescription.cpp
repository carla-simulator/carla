// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CameraDescription.h"

#include "Util/IniFile.h"

static EPostProcessEffect GetPostProcessEffect(const FString &SensorType)
{
  if (SensorType == "CAMERA_NONE") {
    return EPostProcessEffect::None;
  } else if (SensorType == "CAMERA_SCENE_FINAL") {
    return EPostProcessEffect::SceneFinal;
  } else if (SensorType == "CAMERA_DEPTH") {
    return EPostProcessEffect::Depth;
  } else if (SensorType == "CAMERA_SEMANTIC_SEGMENTATION") {
    return EPostProcessEffect::SemanticSegmentation;
  } else {
    UE_LOG(LogCarla, Error, TEXT("Invalid sensor type \"%s\" for a camera"), *SensorType);
    return EPostProcessEffect::INVALID;
  }
}

void UCameraDescription::Load(const FIniFile &Config, const FString &Section)
{
  Super::Load(Config, Section);
  Config.GetInt(*Section, TEXT("ImageSizeX"), ImageSizeX);
  Config.GetInt(*Section, TEXT("ImageSizeY"), ImageSizeY);
  Config.GetFloat(*Section, TEXT("CameraFOV"), FOVAngle);
}

void UCameraDescription::Validate()
{
  FMath::Clamp(FOVAngle, 0.001f, 360.0f);
  ImageSizeX = (ImageSizeX == 0u ? 720u : ImageSizeX);
  ImageSizeY = (ImageSizeY == 0u ? 512u : ImageSizeY);
  PostProcessEffect = GetPostProcessEffect(Type);
}

void UCameraDescription::Log() const
{
  Super::Log();
  UE_LOG(LogCarla, Log, TEXT("Image Size = %dx%d"), ImageSizeX, ImageSizeY);
  UE_LOG(LogCarla, Log, TEXT("Post-Processing = %s"), *PostProcessEffect::ToString(PostProcessEffect));
  UE_LOG(LogCarla, Log, TEXT("FOV = %f"), FOVAngle);
}
