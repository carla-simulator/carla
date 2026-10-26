// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/client/TimeoutException.h>
#include <carla/client/detail/Client.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>
#include <carla/trafficmanager/WorkerException.h>

#include <rpc/server.h>

#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace {

template <typename Function>
std::exception_ptr RunInWorker(Function &&function) {
  std::exception_ptr error;
  std::thread worker([&]() {
    error = carla::traffic_manager::detail::CaptureWorkerException(function);
  });
  worker.join();
  return error;
}

void BindVersion(::rpc::server &server) {
  server.bind("version", [](carla::rpc::Metadata) {
    return carla::rpc::Response<std::string>{std::string("worker-test")};
  });
  server.async_run(1u);
}

} // namespace

TEST(traffic_manager_worker, successful_worker_control) {
  int steps = 0;
  const auto error = RunInWorker([&]() {
    while (steps < 3) {
      ++steps;
    }
  });
  EXPECT_FALSE(error);
  EXPECT_EQ(steps, 3);
}

TEST(traffic_manager_worker, timeout_exception_does_not_escape_thread) {
  const auto error = RunInWorker([]() {
    throw carla::client::TimeoutException("127.0.0.1:2000", carla::time_duration::milliseconds(20));
  });
  ASSERT_TRUE(error);
  EXPECT_THROW(std::rethrow_exception(error), carla::client::TimeoutException);
}

TEST(traffic_manager_worker, stopped_rpc_server_timeout_preserves_error_and_releases_lock) {
  ::rpc::server server(0u);
  BindVersion(server);
  carla::client::detail::Client client("127.0.0.1", server.port(), 1u);
  client.SetTimeout(carla::time_duration::milliseconds(500));
  ASSERT_EQ(client.GetServerVersion(), "worker-test");
  server.stop();
  client.SetTimeout(carla::time_duration::milliseconds(100));
  std::mutex step_lock;
  const auto error = RunInWorker([&]() {
    std::lock_guard<std::mutex> lock(step_lock);
    client.GetServerVersion();
  });
  ASSERT_TRUE(error);
  ASSERT_TRUE(step_lock.try_lock());
  step_lock.unlock();
  try {
    std::rethrow_exception(error);
    FAIL() << "Expected the real CARLA RPC timeout";
  } catch (const carla::client::TimeoutException &exception) {
    const std::string message(exception.what());
    EXPECT_NE(message.find("127.0.0.1:"), std::string::npos);
    EXPECT_NE(message.find("100ms"), std::string::npos);
  }
}

TEST(traffic_manager_worker, caller_thread_timeout_still_propagates_control) {
  ::rpc::server server(0u);
  BindVersion(server);
  carla::client::detail::Client client("127.0.0.1", server.port(), 1u);
  client.SetTimeout(carla::time_duration::milliseconds(500));
  ASSERT_EQ(client.GetServerVersion(), "worker-test");
  server.stop();
  client.SetTimeout(carla::time_duration::milliseconds(100));
  EXPECT_THROW(client.GetServerVersion(), carla::client::TimeoutException);
}

TEST(traffic_manager_worker, reporting_failure_does_not_escape_thread) {
  const auto error = RunInWorker([]() {
    const auto original = carla::traffic_manager::detail::CaptureWorkerException([]() {
      throw std::runtime_error("RPC connection failed");
    });
    if (original) {
      // The same boundary protects forwarding to an episode being destroyed.
      throw std::runtime_error("episode expired while forwarding");
    }
  });
  ASSERT_TRUE(error);
  EXPECT_THROW(std::rethrow_exception(error), std::runtime_error);
}

TEST(traffic_manager_worker, nonstandard_exception_is_preserved) {
  const auto error = RunInWorker([]() { throw 42; });
  ASSERT_TRUE(error);
  try {
    std::rethrow_exception(error);
    FAIL() << "Expected the original exception";
  } catch (int value) {
    EXPECT_EQ(value, 42);
  }
}
