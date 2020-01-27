// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerClient.h"

#include "carla/Exception.h"
#include "carla/Version.h"
#include "carla/client/TimeoutException.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/BoneTransformData.h"
#include "carla/rpc/Client.h"
#include "carla/rpc/DebugShape.h"
#include "carla/rpc/Response.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/WalkerBoneControl.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/streaming/Client.h"

#include <rpc/rpc_error.h>

#include <thread>

namespace carla {
namespace traffic_manager {
namespace client {

  template <typename T>
  static T Get(carla::rpc::Response<T> &response) {
    return response.Get();
  }

  static bool Get(carla::rpc::Response<void> &) {
    return true;
  }

  // ===========================================================================
  // -- Client::Pimpl ----------------------------------------------------------
  // ===========================================================================

  class TrafficManagerClient::Pimpl {
  public:

    Pimpl(const std::string &host, uint16_t port, size_t worker_threads)
      : endpoint(host + ":" + std::to_string(port)),
        rpc_client(host, port),
        streaming_client(host) {
      rpc_client.set_timeout(5000u);
      streaming_client.AsyncRun(
          worker_threads > 0u ? worker_threads : std::thread::hardware_concurrency());
    }

    template <typename ... Args>
    auto RawCall(const std::string &function, Args && ... args) {
      try {
        return rpc_client.call(function, std::forward<Args>(args) ...);
      } catch (const ::rpc::timeout &) {
        throw_exception(carla::client::TimeoutException(endpoint, GetTimeout()));
      }
    }

    template <typename T, typename ... Args>
    auto CallAndWait(const std::string &function, Args && ... args) {
      auto object = RawCall(function, std::forward<Args>(args) ...);
      using R = typename carla::rpc::Response<T>;
      auto response = object.template as<R>();
      if (response.HasError()) {
        throw_exception(std::runtime_error(response.GetError().What()));
      }
      return Get(response);
    }

    template <typename ... Args>
    void AsyncCall(const std::string &function, Args && ... args) {
      // Discard returned future.
      rpc_client.async_call(function, std::forward<Args>(args) ...);
    }

    time_duration GetTimeout() const {
      auto timeout = rpc_client.get_timeout();
      DEBUG_ASSERT(timeout.has_value());
      return time_duration::milliseconds(static_cast<size_t>(*timeout));
    }

    const std::string endpoint;

    rpc::Client rpc_client;

    streaming::Client streaming_client;
  };

  // ===========================================================================
  // -- Client -----------------------------------------------------------------
  // ===========================================================================

  TrafficManagerClient::TrafficManagerClient(
      const std::string &host,
      const uint16_t port,
      const size_t worker_threads)
    : _pimpl(std::make_unique<Pimpl>(host, port, worker_threads)) {}

  TrafficManagerClient::~TrafficManagerClient() = default;

  void TrafficManagerClient::SetTimeout(time_duration timeout) {
    _pimpl->rpc_client.set_timeout(static_cast<int64_t>(timeout.milliseconds()));
  }

  time_duration TrafficManagerClient::GetTimeout() const {
    return _pimpl->GetTimeout();
  }

  const std::string TrafficManagerClient::GetEndpoint() const {
    return _pimpl->endpoint;
  }

  std::string TrafficManagerClient::GetTrafficManagerClientVersion() {
    return ::carla::version();
  }

  std::string TrafficManagerClient::GetTrafficManagerServerVersion() {
    return _pimpl->CallAndWait<std::string>("version");
  }

  void TrafficManagerClient::RegisterVehicle(const std::vector<ActorPtr> &actor_list) {
    _pimpl->RawCall("register_vehicle", std::move(actor_list));
  }

  void TrafficManagerClient::UnregisterVehicle(const std::vector<ActorPtr> &actor_list) {
    _pimpl->RawCall("unregister_vehicle", std::move(actor_list));
  }
} // namespace client
} // namespace traffic_manager
} // namespace carla
