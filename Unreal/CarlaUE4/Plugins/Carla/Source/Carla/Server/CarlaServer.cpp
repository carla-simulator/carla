// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaServer.h"

#include "Server/CarlaEncoder.h"

#include <carla/carla_server.h>

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

static FCarlaServer::ErrorCode ParseErrorCode(const uint32 ErrorCode)
{
  if (ErrorCode == CARLA_SERVER_SUCCESS) {
    return FCarlaServer::Success;
  } else if (ErrorCode == CARLA_SERVER_TRY_AGAIN) {
    return FCarlaServer::TryAgain;
  } else {
    return FCarlaServer::Error;
  }
}

static int32 GetTimeOut(uint32 TimeOut, const bool bBlocking)
{
  return (bBlocking ? TimeOut : 0u);
}

// =============================================================================
// -- CarlaServer --------------------------------------------------------------
// =============================================================================

FCarlaServer::FCarlaServer(const uint32 InWorldPort, const uint32 InTimeOut) :
  WorldPort(InWorldPort),
  TimeOut(InTimeOut),
  Server(carla_make_server()) {
  check(Server != nullptr);
}

FCarlaServer::~FCarlaServer()
{
#ifdef CARLA_SERVER_EXTRA_LOG
  UE_LOG(LogCarlaServer, Warning, TEXT("Destroying CarlaServer"));
#endif // CARLA_SERVER_EXTRA_LOG
  carla_free_server(Server);
}

FCarlaServer::ErrorCode FCarlaServer::Connect()
{
  UE_LOG(LogCarlaServer, Log, TEXT("Waiting for the client to connect..."));
  return ParseErrorCode(carla_server_connect(Server, WorldPort, TimeOut));
}

FCarlaServer::ErrorCode FCarlaServer::ReadNewEpisode(FString &IniFile, const bool bBlocking)
{
  carla_request_new_episode values;
  auto ec = ParseErrorCode(carla_read_request_new_episode(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
    FCarlaEncoder::Decode(values, IniFile);
    UE_LOG(LogCarlaServer, Log, TEXT("Received new episode"));
#ifdef CARLA_SERVER_EXTRA_LOG
    UE_LOG(LogCarlaServer, Log, TEXT("Received CarlaSettings.ini:\n%s"), *IniFile);
#endif // CARLA_SERVER_EXTRA_LOG
  }
  return ec;
}

FCarlaServer::ErrorCode FCarlaServer::SendSceneDescription(
    const FString &MapName,
    const TArray<APlayerStart *> &AvailableStartSpots,
    const TArray<USensorDescription *> &SensorDescriptions,
    const bool bBlocking)
{
  carla_scene_description scene;
  // Encode map name.
  const auto MapNameBuffer = FCarlaEncoder::Encode(MapName);
  scene.map_name = MapNameBuffer.Get();
  // Encode start spots.
  TArray<carla_transform> Transforms;
  FCarlaEncoder::Encode(AvailableStartSpots, Transforms);
  scene.player_start_spots = (Transforms.Num() > 0 ? Transforms.GetData() : nullptr);;
  scene.number_of_player_start_spots = Transforms.Num();
  // Encode sensors.
  TArray<TUniquePtr<const char[]>> SensorNames; // This holds the memory while we send it.
  TArray<carla_sensor_definition> Sensors;
  FCarlaEncoder::Encode(SensorDescriptions, Sensors, SensorNames);
  scene.sensors = (Sensors.Num() > 0 ? Sensors.GetData() : nullptr);;
  scene.number_of_sensors = Sensors.Num();
  // Send scene description.
  UE_LOG(LogCarlaServer, Log, TEXT("Sending %d available start positions"), scene.number_of_player_start_spots);
  UE_LOG(LogCarlaServer, Log, TEXT("Sending %d sensor descriptions"), scene.number_of_sensors);
  return ParseErrorCode(carla_write_scene_description(Server, scene, GetTimeOut(TimeOut, bBlocking)));
}

FCarlaServer::ErrorCode FCarlaServer::ReadEpisodeStart(uint32 &StartPositionIndex, const bool bBlocking)
{
  carla_episode_start values;
  auto ec = ParseErrorCode(carla_read_episode_start(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
    StartPositionIndex = values.player_start_spot_index;
    UE_LOG(LogCarlaServer, Log, TEXT("Episode start received: { StartIndex = %d }"), StartPositionIndex);
  }
  return ec;
}

FCarlaServer::ErrorCode FCarlaServer::SendEpisodeReady(const bool bBlocking)
{
  UE_LOG(LogCarlaServer, Log, TEXT("Ready to play, notifying client"));
  const carla_episode_ready values = {true};
  return ParseErrorCode(carla_write_episode_ready(Server, values, GetTimeOut(TimeOut, bBlocking)));
}

FCarlaServer::ErrorCode FCarlaServer::ReadControl(FVehicleControl &Control, const bool bBlocking)
{
  carla_control values;
  auto ec = ParseErrorCode(carla_read_control(Server, values, GetTimeOut(TimeOut, bBlocking)));
  if (Success == ec) {
#ifdef CARLA_SERVER_EXTRA_LOG
    UE_LOG(
        LogCarlaServer,
        Log,
        TEXT("Read control (%s): { Steer = %f, Throttle = %f, Brake = %f, Handbrake = %s, Reverse = %s }"),
        (bBlocking ? TEXT("Sync") : TEXT("Async")),
        values.steer,
        values.throttle,
        values.brake,
        (values.hand_brake ? TEXT("True") : TEXT("False")),
        (values.reverse ? TEXT("True") : TEXT("False")));
#endif // CARLA_SERVER_EXTRA_LOG
    FCarlaEncoder::Decode(values, Control);
  } else if ((!bBlocking) && (TryAgain == ec)) {
    UE_LOG(LogCarlaServer, Warning, TEXT("No control received from the client this frame!"));
  }
  return ec;
}

FCarlaServer::ErrorCode FCarlaServer::SendSensorData(const FSensorDataView &Data)
{
  carla_sensor_data values;
  FCarlaEncoder::Encode(Data, values);
  return ParseErrorCode(carla_write_sensor_data(Server, values));
}

FCarlaServer::ErrorCode FCarlaServer::SendMeasurements(
    const ACarlaPlayerState &PlayerState,
    const TArray<const UAgentComponent *> &Agents,
    const bool bSendNonPlayerAgentsInfo)
{
  // Encode measurements.
  carla_measurements values;
  FCarlaEncoder::Encode(PlayerState, values);
  // Encode agents.
  TArray<carla_agent> AgentsData;
  if (bSendNonPlayerAgentsInfo) {
    FCarlaEncoder::Encode(Agents, AgentsData);
  }
  values.non_player_agents = (AgentsData.Num() > 0 ? AgentsData.GetData() : nullptr);;
  values.number_of_non_player_agents = AgentsData.Num();
  // Send measurements.
#ifdef CARLA_SERVER_EXTRA_LOG
  UE_LOG(LogCarlaServer, Log, TEXT("Sending data of %d agents"), values.number_of_non_player_agents);
#endif // CARLA_SERVER_EXTRA_LOG
  return ParseErrorCode(carla_write_measurements(Server, values));
}
