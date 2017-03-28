// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <array>
#include "GameFramework/PlayerState.h"
#include "CarlaPlayerState.generated.h"

/// Current state of the player, updated every frame by ACarlaVehicleController.
///
/// This class matches the reward that it is sent to the client over the
/// network.
UCLASS()
class CARLA_API ACarlaPlayerState : public APlayerState
{
  GENERATED_BODY()

  // ===========================================================================
  // -- Types ------------------------------------------------------------------
  // ===========================================================================
public:

  enum ImageTag {
    ImageRGB0,
    ImageRGB1,
    ImageDepth0,
    ImageDepth1,
    NUMBER_OF_IMAGES
  };

  struct Image {
    uint32 SizeX = 0u;
    uint32 SizeY = 0u;
    TArray<FColor> BitMap;
  };

  // ===========================================================================
  // -- APlayerState -----------------------------------------------------------
  // ===========================================================================
public:

  virtual void Reset() override;

  virtual void CopyProperties(APlayerState *PlayerState) override;

  // ===========================================================================
  // -- Getters ----------------------------------------------------------------
  // ===========================================================================
public:

  int32 GetTimeStamp() const
  {
    return TimeStamp;
  }

  const FVector &GetLocation() const
  {
    return Location;
  }

  const FVector &GetOrientation() const
  {
    return Orientation;
  }

  const FVector &GetAcceleration() const
  {
    return Acceleration;
  }

  float GetForwardSpeed() const
  {
    return ForwardSpeed;
  }

  int32 GetCurrentGear() const
  {
    return CurrentGear;
  }

  float GetCollisionIntensityCars() const
  {
    return CollisionIntensityCars;
  }

  float GetCollisionIntensityPedestrians() const
  {
    return CollisionIntensityPedestrians;
  }

  float GetCollisionIntensityOther() const
  {
    return CollisionIntensityOther;
  }

  float GetOtherLaneIntersectionFactor() const
  {
    return OtherLaneIntersectionFactor;
  }

  float GetOffRoadIntersectionFactor() const
  {
    return OffRoadIntersectionFactor;
  }

  const Image &GetImage(ImageTag Tag) const
  {
    return Images[Tag];
  }

  // ===========================================================================
  // -- Modifiers --------------------------------------------------------------
  // ===========================================================================
private:

  void RegisterCollision(AActor *Actor, FVector NormalImpulse);

  void UpdateTimeStamp();

  // ===========================================================================
  // -- Private members --------------------------------------------------------
  // ===========================================================================
private:

  friend class ACarlaVehicleController;

  // If you add another variable here, don't forget to copy it inside
  // CopyProperties if necessary.

  int32 TimeStamp;

  FVector Location;

  FVector Orientation;

  FVector Acceleration;

  int32 CurrentGear;

  float ForwardSpeed = 0.0f;

  float CollisionIntensityCars = 0.0f;

  float CollisionIntensityPedestrians = 0.0f;

  float CollisionIntensityOther = 0.0f;

  float OtherLaneIntersectionFactor = 0.0f;

  float OffRoadIntersectionFactor = 0.0f;

  std::array<Image, NUMBER_OF_IMAGES> Images;
};
