// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/router.h"

namespace carla {
namespace multigpu {

// using session = std::shared_ptr<Primary>;
// using callback_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;

class PrimaryCommands {
  public:
  
    PrimaryCommands();
    PrimaryCommands(std::shared_ptr<Router> router);

    // broadcast to all secondary servers the frame data
    void SendFrameData(carla::Buffer buffer);
    
    // broadcast to all secondary servers the map to load
    void SendLoadMap(std::string map);

    // send to who the router wants the request for a token
    void SendGetToken(carla::streaming::detail::stream_id_type sensor_id);
    
    // send to know if a connection is alive
    void SendIsAlive();

    void set_router(std::shared_ptr<Router> router);

  private:

    std::shared_ptr<Router> _router;
};

} // namespace multigpu
} // namespace carla
