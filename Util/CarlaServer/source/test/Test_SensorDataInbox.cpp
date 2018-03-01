#include <iostream>

#include <gtest/gtest.h>

#include <carla/carla_server.h>

#include "carla/server/SensorDataInbox.h"

#include "Sensor.h"

#include <array>
#include <atomic>
#include <future>

TEST(SensorDataInbox, SyncSingleSensor) {
  using namespace carla::server;
  test::Sensor sensor0;
  SensorDataInbox::Sensors defs;
  defs.push_back(sensor0.definition());
  SensorDataInbox inbox(defs);
  for (auto j = 0u; j < 1000u; ++j) {
    auto data = sensor0.MakeRandomData();
    inbox.Write(data);
    auto buffer = (*inbox.begin()).TryMakeReader();
    ASSERT_TRUE(buffer != nullptr);
    sensor0.CheckData(buffer->buffer());
  }
}

TEST(SensorDataInbox, SyncMultipleSensors) {
  using namespace carla::server;
  std::array<test::Sensor, 50u> sensors;
  SensorDataInbox::Sensors defs;
  defs.reserve(sensors.size());
  std::for_each(sensors.begin(), sensors.end(), [&](auto &s){
    defs.push_back(s.definition());
  });
  SensorDataInbox inbox(defs);
  for (auto j = 0u; j < 1000u; ++j) {
    for (auto &sensor : sensors) {
      inbox.Write(sensor.MakeRandomData());
      auto buffer = inbox.TryMakeReader(sensor.id());
      ASSERT_TRUE(buffer != nullptr);
      sensor.CheckData(buffer->buffer());
    }
  }
}

TEST(SensorDataInbox, Async) {
  using namespace carla::server;
  std::array<test::Sensor, 50u> sensors;
  SensorDataInbox::Sensors defs;
  defs.reserve(sensors.size());
  std::for_each(sensors.begin(), sensors.end(), [&](auto &s){
    defs.push_back(s.definition());
  });
  SensorDataInbox inbox(defs);

  constexpr auto numberOfWrites = 200u;

  std::atomic_bool done{false};

  auto result_writer = std::async(std::launch::async, [&](){
    for (size_t i = 0u; i < numberOfWrites; ++i) {
      for (auto &sensor : sensors) {
        inbox.Write(sensor.MakeRandomData());
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10u));
    };
    std::this_thread::sleep_for(std::chrono::milliseconds(200u));
    done = true;
  });

  auto result_reader = std::async(std::launch::async, [&](){
    auto readings = 0u;
    while (!done && (readings < numberOfWrites * sensors.size())) {
      for (auto &sensor : sensors) {
        auto buffer = inbox.TryMakeReader(sensor.id());
        if (buffer != nullptr) {
          sensor.CheckData(buffer->buffer());
          ++readings;
        }
      }
    }
    ASSERT_EQ(numberOfWrites * sensors.size(), readings);
  });

  result_reader.get();
  result_writer.get();
}
