// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "TheNewCarlaServer.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Server.h>
#include <compiler/enable-ue4-macros.h>

class FTheNewCarlaServer::FPimpl
{
public:

  FPimpl(uint16_t port) : Server(port) {}

  carla::rpc::Server Server;

  UCarlaEpisode *CurrentEpisode = nullptr;
};

FTheNewCarlaServer::FTheNewCarlaServer() : Pimpl(nullptr) {}

FTheNewCarlaServer::~FTheNewCarlaServer() {}

void FTheNewCarlaServer::Start(uint16_t Port)
{
  UE_LOG(LogCarlaServer, Log, TEXT("Initializing rpc-server at port %d"), Port);

  Pimpl = MakeUnique<FPimpl>(Port);

  namespace cr = carla::rpc;

  auto &srv = Pimpl->Server;

  srv.BindAsync("ping", []() { return true; });

  srv.BindAsync("version", []() { return std::string(carla::version()); });
}

void FTheNewCarlaServer::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  UE_LOG(LogCarlaServer, Log, TEXT("New episode '%s' started"), *Episode.GetMapName());
  Pimpl->CurrentEpisode = &Episode;
}

void FTheNewCarlaServer::NotifyEndEpisode()
{
  Pimpl->CurrentEpisode = nullptr;
}

void FTheNewCarlaServer::AsyncRun(uint32 NumberOfWorkerThreads)
{
  Pimpl->Server.AsyncRun(NumberOfWorkerThreads);
}

void FTheNewCarlaServer::RunSome(uint32 Milliseconds)
{
  Pimpl->Server.SyncRunFor(carla::time_duration::milliseconds(Milliseconds));
}

void FTheNewCarlaServer::Stop()
{
  Pimpl->Server.Stop();
}
