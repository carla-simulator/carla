// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

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
      const ACarlaPlayerState &PlayerState);

private:

  const uint32 WorldPort;

  const uint32 TimeOut;

  void* Server;
};
