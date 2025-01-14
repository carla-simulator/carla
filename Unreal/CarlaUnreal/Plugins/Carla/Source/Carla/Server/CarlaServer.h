// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActor.h"
#include "Carla/Sensor/DataStream.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/multigpu/router.h>
#include <carla/streaming/Server.h>
#include <util/enable-ue4-macros.h>


class UCarlaEpisode;

class FCarlaServer
{
public:

  FCarlaServer();

  ~FCarlaServer();

  FDataMultiStream Start(uint16_t RPCPort, uint16_t StreamingPort, uint16_t SecondaryPort);

  void NotifyBeginEpisode(UCarlaEpisode &Episode);

  void NotifyEndEpisode();

  void AsyncRun(uint32 NumberOfWorkerThreads);

  void RunSome(uint32 Milliseconds);

  void Tick();
  
  bool TickCueReceived();

  void Stop();

  FDataStream OpenStream() const;

  std::shared_ptr<carla::multigpu::Router> GetSecondaryServer();

  carla::streaming::Server &GetStreamingServer();

private:

  class FPimpl;
  TUniquePtr<FPimpl> Pimpl;
};
