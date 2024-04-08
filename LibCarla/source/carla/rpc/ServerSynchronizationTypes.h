// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <locale>

namespace carla {
namespace rpc {

using synchronization_client_id_type = void*;
static constexpr carla::rpc::synchronization_client_id_type ALL_CLIENTS{nullptr};

using synchronization_participant_id_type = uint32_t;
static constexpr carla::rpc::synchronization_participant_id_type ALL_PARTICIPANTS{0};

using synchronization_target_game_time = double;
static constexpr synchronization_target_game_time NO_SYNC_TARGET_GAME_TIME{0.};
static constexpr synchronization_target_game_time BLOCKING_TARGET_GAME_TIME{1e-6};

}  // namespace rpc
}  // namespace carla
