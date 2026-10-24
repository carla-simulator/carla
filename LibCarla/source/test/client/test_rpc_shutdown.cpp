// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/rpc/Client.h>
#include <carla/rpc/Server.h>

#include <boost/asio/ip/tcp.hpp>
#include <rpc/rpc_error.h>

#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <thread>

namespace {

using namespace std::chrono_literals;

struct PendingCall {
  std::atomic_bool armed{false};
  std::atomic_bool observed{false};
  std::atomic_bool invoked{false};
  std::promise<void> entered;
};

// The wrapper finishes copying its binding before this observer is armed. Its
// next copy is made inside WrapSyncCall when it constructs the packaged task,
// proving that the real RPC worker reached the synchronous dispatch path.
struct ObserveSyncDispatch {
  explicit ObserveSyncDispatch(std::shared_ptr<PendingCall> value) : state(std::move(value)) {}

  ObserveSyncDispatch(const ObserveSyncDispatch &other) : state(other.state) {
    if (state->armed && !state->observed.exchange(true)) {
      state->entered.set_value();
    }
  }

  ObserveSyncDispatch(ObserveSyncDispatch &&) = default;

  int operator()() const {
    state->invoked = true;
    return 42;
  }

  std::shared_ptr<PendingCall> state;
};

struct CallResult {
  bool timed_out = false;
  bool returned = false;
  int value = 0;
  std::string error;
};

uint16_t AvailableLoopbackPort() {
  boost::asio::io_context io;
  boost::asio::ip::tcp::acceptor socket(
      io, {boost::asio::ip::address_v4::loopback(), 0u});
  return socket.local_endpoint().port();
}

} // namespace

TEST(rpc, sync_wrapper_reports_shutdown_without_running_handler) {
  struct State {
    std::atomic_bool shutting_down{false};
    boost::asio::io_context io;
  };
  auto state = std::make_shared<State>();
  auto pending = std::make_shared<PendingCall>();
  auto entered = pending->entered.get_future();
  auto wrapped = carla::rpc::detail::FunctionWrapper<ObserveSyncDispatch>::WrapSyncCall(
      state->shutting_down, state->io, ObserveSyncDispatch(pending));
  auto completed = std::make_shared<std::promise<CallResult>>();
  auto completion = completed->get_future();
  pending->armed = true;
  std::thread worker([state, wrapped=std::move(wrapped), completed]() {
    CallResult result;
    try {
      result.value = wrapped(carla::rpc::Metadata::MakeSync());
      result.returned = true;
    } catch (const std::exception &error) {
      result.error = error.what();
    }
    completed->set_value(std::move(result));
  });

  const bool dispatched = entered.wait_for(5s) == std::future_status::ready;
  EXPECT_TRUE(dispatched);
  if (dispatched) {
    EXPECT_EQ(completion.wait_for(0s), std::future_status::timeout);
  }
  state->shutting_down = true;
  if (completion.wait_for(5s) != std::future_status::ready) {
    worker.detach(); // Worker owns the io_context and flag it still references.
    ADD_FAILURE() << "Synchronous wrapper ignored shutdown";
    return;
  }
  worker.join();
  const auto result = completion.get();
  EXPECT_FALSE(result.returned);
  EXPECT_EQ(result.error, "RPC server is shutting down");
  EXPECT_FALSE(pending->invoked);
}

TEST(rpc, stop_completes_with_pending_sync_call) {
  const auto port = AvailableLoopbackPort();
  auto server = std::make_shared<carla::rpc::Server>("127.0.0.1", port);
  auto pending = std::make_shared<PendingCall>();
  auto entered = pending->entered.get_future();
  server->BindSync("pending_sync", ObserveSyncDispatch(pending));
  pending->armed = true;
  server->AsyncRun(1u);

  auto completed = std::make_shared<std::promise<CallResult>>();
  auto completion = completed->get_future().share();
  std::thread caller([completed, port]() {
    CallResult result;
    try {
      carla::rpc::Client client("127.0.0.1", port);
      client.set_timeout(3000); // Bounded cleanup, not a successful shutdown response.
      result.value = client.call("pending_sync").as<int>();
      result.returned = true;
    } catch (const ::rpc::timeout &error) {
      result.timed_out = true;
      result.error = error.what();
    } catch (const std::exception &error) {
      result.error = error.what();
    }
    completed->set_value(std::move(result));
  });

  const bool dispatched = entered.wait_for(5s) == std::future_status::ready;
  EXPECT_TRUE(dispatched) << "RPC worker did not reach WrapSyncCall";
  if (dispatched) {
    EXPECT_EQ(completion.wait_for(0s), std::future_status::timeout);
    EXPECT_FALSE(pending->invoked); // No SyncRunFor: the handler cannot run.
  }

  auto stopped = std::make_shared<std::promise<void>>();
  auto shutdown = stopped->get_future();
  auto stop_returned = std::make_shared<std::atomic_bool>(false);
  std::thread stopper([server=std::move(server), stopped, stop_returned]() mutable {
    server->Stop();
    *stop_returned = true;
    // Release the stopped server here so its destruction is bounded as well.
    server.reset();
    stopped->set_value();
  });

  if (shutdown.wait_for(5s) != std::future_status::ready) {
    // A regression must report failure instead of hanging in thread/server
    // destructors. Detached workers own all state they can still access.
    stopper.detach();
    caller.detach();
    ADD_FAILURE() << "Server shutdown blocked on a pending synchronous RPC; Stop returned="
                  << stop_returned->load();
    return;
  }
  stopper.join();
  EXPECT_TRUE(*stop_returned);
  if (completion.wait_for(5s) != std::future_status::ready) {
    caller.detach();
    ADD_FAILURE() << "Synchronous client did not finish after server shutdown";
    return;
  }
  caller.join();
  // rpclib may leave the client waiting until its timeout. This regression
  // covers Stop(), not successful delivery of a shutdown response.
  const auto result = completion.get();
  RecordProperty("client_cleanup", result.timed_out ? "timeout" :
      (result.returned ? "returned" : "exception"));
  EXPECT_FALSE(pending->invoked);
}
