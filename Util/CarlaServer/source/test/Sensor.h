#pragma once

#include <carla/carla_server.h>

#include "carla/server/ServerTraits.h"

#include <boost/asio/buffer.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace test {

  /// A class for testing usage of sensor data.
  class Sensor {
  public:

    Sensor();

    uint32_t id() const {
      return _definition.id;
    }

    const carla_sensor_definition &definition() const {
      return _definition;
    }

    carla_sensor_data MakeRandomData();

    void CheckData(boost::asio::const_buffer buffer) const;

  private:

    Sensor(uint32_t id);

    uint64_t _frame_number = 0u;

    mutable std::mutex _mutex;

    const std::string _name;

    const carla_sensor_definition _definition;

    std::unique_ptr<const unsigned char[]> _header;

    std::unique_ptr<const unsigned char[]> _buffer;

    carla_sensor_data _data;
  };

} // test
