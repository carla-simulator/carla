// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Containers/Array.h"

class ACarlaPlayerState;
class APlayerStart;
class FSensorDataView;
class FString;
class USensorDescription;
struct FVehicleControl;

/// Wrapper around carla_server API.
class FCarlaServer
{
public:

  enum ErrorCode {
    Success,
    TryAgain,
    Error
  };

  explicit FCarlaServer(uint32 WorldPort, uint32 TimeOutInMilliseconds);

  ~FCarlaServer();

  /// Connect with the client, block until the client connects or the time-out
  /// is met.
  ErrorCode Connect();

  ErrorCode ReadNewEpisode(FString &IniFile, bool bBlocking);

  ErrorCode SendSceneDescription(
      const FString &MapName,
      const TArray<APlayerStart *> &AvailableStartSpots,
      const TArray<USensorDescription *> &SensorDescriptions,
      bool bBlocking);

  ErrorCode ReadEpisodeStart(uint32 &StartPositionIndex, bool bBlocking);

  ErrorCode SendEpisodeReady(bool bBlocking);

  ErrorCode ReadControl(FVehicleControl &Control, bool bBlocking);

  /// Enqueues sensor data to be sent to the client. It is safe to call this
  /// function from a different thread.
  ErrorCode SendSensorData(const FSensorDataView &Data);

  ErrorCode SendMeasurements(
      const ACarlaPlayerState &PlayerState,
      const TArray<const UAgentComponent *> &Agents,
      bool bSendNonPlayerAgentsInfo);

private:

  const uint32 WorldPort;

  const uint32 TimeOut;

  void* const Server;
};
