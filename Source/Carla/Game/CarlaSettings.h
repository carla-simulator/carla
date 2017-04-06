// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "UObject/NoExportTypes.h"
#include "CameraDescription.h"
#include "CarlaSettings.generated.h"

/// Global settings for CARLA.
UCLASS()
class CARLA_API UCarlaSettings : public UObject
{
  GENERATED_BODY()

public:

  /** Load the settings based on the command-line arguments and the INI file if provided. */
  void LoadSettings();

  /** Log settings values. */
  void LogSettings();

private:

  /** File name of the settings file used to load this settings. Empty if none used. */
  UPROPERTY(Category = "CARLA Settings|Debug", VisibleAnywhere)
  FString CurrentFileName;

  // ===========================================================================
  /// @name CARLA Server
  // ===========================================================================
  /// @{
public:

  /** If active, wait for the client to connect and control the pawn. */
  UPROPERTY(Category = "CARLA Server", EditDefaultsOnly)
  bool bUseNetworking = true;

  /** World port to listen for client connections. */
  UPROPERTY(Category = "CARLA Server", EditDefaultsOnly, meta = (EditCondition = bUseNetworking))
  uint32 WorldPort = 2000u;

  /** If networking is active, rewards are sent to this port. */
  UPROPERTY(Category = "CARLA Server", EditDefaultsOnly, meta = (EditCondition = bUseNetworking))
  uint32 WritePort = 2001u;

  /** If networking is active, controls are read from this port. */
  UPROPERTY(Category = "CARLA Server", EditDefaultsOnly, meta = (EditCondition = bUseNetworking))
  uint32 ReadPort = 2002u;

  /// @}
  // ===========================================================================
  /// @name Scene Capture
  // ===========================================================================
  /// @{
public:

  /** Descriptions of the cameras to be attached to the player. */
  UPROPERTY(Category = "Scene Capture", EditDefaultsOnly)
  TMap<FString, FCameraDescription> CameraDescriptions;

  /// @}
};
