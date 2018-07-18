// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Server.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Version.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Server.h>
#include <carla/rpc/Transform.h>
#include <carla/rpc/VehicleControl.h>
#include <compiler/enable-ue4-macros.h>

static FVehicleControl MakeControl(const carla::rpc::VehicleControl &cControl)
{
  FVehicleControl Control;
  Control.Throttle = cControl.throttle;
  Control.Steer = cControl.steer;
  Control.Brake = cControl.brake;
  Control.bHandBrake = cControl.hand_brake;
  Control.bReverse = cControl.reverse;
  return Control;
}

class FRPCServer::Pimpl
{
public:
  Pimpl(uint16_t port) : Server(port) {}
  carla::rpc::Server Server;
};

FRPCServer::FRPCServer() : _Pimpl(nullptr) {}

FRPCServer::~FRPCServer() {}

void FRPCServer::Initialize(AServer &Server, uint16_t Port)
{
  UE_LOG(LogTemp, Error, TEXT("Initializing rpc-server at port %d"), Port);

  _Pimpl = std::make_unique<Pimpl>(Port);

  namespace cr = carla::rpc;

  auto &srv = _Pimpl->Server;

  srv.BindAsync("ping", []() { return true; });

  srv.BindAsync("version", []() { return std::string(carla::version()); });

  // srv.BindAsync("get_blueprints", []() {
  //   return std::vector<cr::ActorBlueprint>{
  //     cr::ActorBlueprint{"vehicle.mustang.red"},
  //     cr::ActorBlueprint{"vehicle.mustang.also_red"},
  //     cr::ActorBlueprint{"vehicle.mustang.still_red"}
  //   };
  // });

  // srv.BindSync("spawn_actor", [&](
  //     const cr::ActorBlueprint &blueprint,
  //     const cr::Transform &transform) {
  //   auto id = Server.SpawnAgent(transform);
  //   return cr::Actor{static_cast<cr::Actor::id_type>(id), blueprint};
  // });

  srv.BindSync("apply_control_to_actor", [&](
      const cr::Actor &actor,
      const cr::VehicleControl &control) {
    Server.ApplyControl(actor.id, MakeControl(control));
  });
}

void FRPCServer::Run()
{
  _Pimpl->Server.AsyncRun(4);
}

void FRPCServer::RunSome()
{
  using namespace std::chrono_literals;
  _Pimpl->Server.SyncRunFor(20ms);
}

void FRPCServer::Stop()
{
  _Pimpl->Server.Stop();
}
