// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "PostProcessEffect.h"
#include "SensorDescription.h"
#include "CameraDescription.generated.h"

UCLASS()
class UCameraDescription : public USensorDescription
{
  GENERATED_BODY()

public:

  virtual void AcceptVisitor(ISensorDescriptionVisitor &Visitor) const final
  {
    Visitor.Visit(*this);
  }

  virtual void Load(const FIniFile &Config, const FString &Section) final;

  virtual void Validate() final;

  virtual bool RequiresSemanticSegmentation() const final
  {
    return PostProcessEffect == EPostProcessEffect::SemanticSegmentation;
  }

  virtual void Log() const final;

  /** X size in pixels of the captured image. */
  UPROPERTY(Category = "Camera Description", EditDefaultsOnly, meta=(ClampMin = "1"))
  uint32 ImageSizeX = 720u;

  /** Y size in pixels of the captured image. */
  UPROPERTY(Category = "Camera Description", EditDefaultsOnly, meta=(ClampMin = "1"))
  uint32 ImageSizeY = 512u;

  /** Post-process effect to be applied to the captured image. */
  UPROPERTY(Category = "Camera Description", EditDefaultsOnly)
  EPostProcessEffect PostProcessEffect = EPostProcessEffect::SceneFinal;

  /** Camera field of view (in degrees). */
  UPROPERTY(Category = "Camera Description", EditDefaultsOnly, meta=(DisplayName = "Field of View", ClampMin = "0.001", ClampMax = "360.0"))
  float FOVAngle = 90.0f;
};
