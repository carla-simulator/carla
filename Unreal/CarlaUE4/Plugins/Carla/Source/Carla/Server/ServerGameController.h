// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Game/CarlaGameControllerBase.h"

class FCarlaServer;
class FServerSensorDataSink;

/// Implements remote control of game and player.
class FServerGameController : public ICarlaGameControllerBase
{
public:

  FServerGameController(FDataRouter &DataRouter);

  ~FServerGameController();

  virtual void Initialize(UCarlaSettings &CarlaSettings) final;

  virtual APlayerStart *ChoosePlayerStart(const TArray<APlayerStart *> &AvailableStartSpots) final;

  virtual void RegisterPlayer(AController &NewPlayer) final;

  virtual void BeginPlay() final;

    
  virtual void Tick(float DeltaSeconds) final;
  /** @param mapname Use full path name or filename without the extension .
  * @param refreshmapfiles updates the list
  */
  bool MapExists(const FString& mapname, bool refreshmapfiles = false);

private:
  /** Returns true if the current level name (map) is the same as the one in the current settings */
  bool IsTheSameLevel();

  /** Using the world. It will just open the mapname */
  void ChangeLevel(UWorld* world, const FString& mapname);

  /** Reload the current level */
  void RestartLevel();

  const TSharedPtr<FServerSensorDataSink> DataSink;

  TSharedPtr<FCarlaServer> Server;

  UCarlaSettings* CarlaSettings = nullptr;
};
