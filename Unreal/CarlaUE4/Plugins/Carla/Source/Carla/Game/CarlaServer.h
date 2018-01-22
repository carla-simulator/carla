// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class ACarlaGameState;
class ACarlaVehicleController;
class APlayerStart;
class UCarlaSettings;

/// Wrapper around carla_server API.
class CARLA_API CarlaServer
{
public:

  enum ErrorCode {
    Success,
    TryAgain,
    Error
  };

  explicit CarlaServer(uint32 WorldPort, uint32 TimeOutInMilliseconds);

  ~CarlaServer();

  /// Connect with the client, block until the client connects or the time-out
  /// is met.
  ErrorCode Connect();

  ErrorCode ReadNewEpisode(UCarlaSettings &Settings, bool bBlocking);

  ErrorCode SendSceneDescription(
      const TArray<APlayerStart *> &AvailableStartSpots,
      bool bBlocking);

  ErrorCode ReadEpisodeStart(uint32 &StartPositionIndex, bool bBlocking);

  ErrorCode SendEpisodeReady(bool bBlocking);

  ErrorCode ReadControl(ACarlaVehicleController &Player, bool bBlocking);

  ErrorCode SendMeasurements(
      const ACarlaGameState &GameState,
      const ACarlaPlayerState &PlayerState,
      bool bSendNonPlayerAgentsInfo);

private:

  const uint32 WorldPort;

  const uint32 TimeOut;

  void* const Server;
};
