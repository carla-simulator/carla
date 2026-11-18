// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Client.h"

#include "carla/client/Actor.h"
#include "carla/client/ActorList.h"

#include <map>

namespace carla {
namespace client {

  std::vector<rpc::CommandResponse> Client::ApplyBatchSync(
      std::vector<rpc::Command> commands,
      bool do_tick_cue) const {
    struct AutopilotAction {
      size_t index;
      uint16_t port;
      bool enabled;
    };
    std::vector<AutopilotAction> autopilot_actions;
    const auto remember_autopilot = [&autopilot_actions](size_t index,
        const rpc::Command &command) {
      if (const auto *autopilot = std::get_if<rpc::Command::SetAutopilot>(&command.command)) {
        autopilot_actions.push_back({index, autopilot->tm_port, autopilot->enabled});
      }
    };
    for (size_t index = 0u; index < commands.size(); ++index) {
      if (const auto *spawn = std::get_if<rpc::Command::SpawnActor>(&commands[index].command)) {
        for (const auto &followup : spawn->do_after) {
          remember_autopilot(index, followup);
        }
      } else {
        remember_autopilot(index, commands[index]);
      }
    }

    auto responses = _simulator->ApplyBatchSync(std::move(commands), false);
    if (do_tick_cue)
      _simulator->Tick(_simulator->GetNetworkingTimeout());

    // SetAutopilot's TM registration is client-side, for C++ and Python alike.
    // Keep the final requested state independently for each actor and TM port.
    std::map<uint16_t, std::map<ActorId, bool>> autopilot_by_port;
    std::map<ActorId, SharedPtr<Actor>> actors;
    for (const auto &action : autopilot_actions) {
      if (action.index < responses.size() && !responses[action.index].HasError()) {
        const auto id = responses[action.index].Get();
        autopilot_by_port[action.port][id] = action.enabled;
        actors.emplace(id, nullptr);
      }
    }
    if (!actors.empty()) {
      std::vector<ActorId> ids;
      ids.reserve(actors.size());
      for (const auto &entry : actors) {
        ids.push_back(entry.first);
      }
      // Fetch all missing actor descriptions in one RPC instead of creating
      // worker threads that race on a shared TM port and query one at a time.
      const auto actor_list = GetWorld().GetActors(ids);
      for (const auto &actor : *actor_list) {
        actors[actor->GetId()] = actor;
      }
      for (const auto &port : autopilot_by_port) {
        std::vector<SharedPtr<Actor>> enable, disable;
        for (const auto &entry : port.second) {
          const auto &actor = actors.at(entry.first);
          if (actor != nullptr) {
            (entry.second ? enable : disable).push_back(actor);
          }
        }
        if (!enable.empty() || !disable.empty()) {
          auto tm = GetInstanceTM(port.first);
          if (!enable.empty()) tm.RegisterVehicles(enable);
          if (!disable.empty()) tm.UnregisterVehicles(disable);
        }
      }
    }
    return responses;
  }

} // namespace client
} // namespace carla
