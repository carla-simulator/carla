#include "Sensor.h"

#include <gtest/gtest.h>

namespace test {

  static uint32_t ID_COUNT = 0u;

  Sensor::Sensor() : Sensor(++ID_COUNT) {}

  Sensor::Sensor(const uint32_t id)
    : _name(std::string("Sensor") + std::to_string(id)),
      _definition({id, 0u, _name.c_str()}),
      _data({id, nullptr, 0u, nullptr, 0u}) {}

  carla_sensor_data Sensor::MakeRandomData() {
    std::lock_guard<std::mutex> lock(_mutex);

    const struct {
      uint64_t FrameNumber;
      uint32_t Width;
      uint32_t Height;
      uint32_t Type;
      float FOV;
    } ImageHeader = {++_frame_number, 300u, 200u, 1u, 90.0f};

    _data.header_size = sizeof(ImageHeader);
    auto header = std::make_unique<unsigned char[]>(_data.header_size);
    std::memcpy(
        reinterpret_cast<void *>(header.get()),
        reinterpret_cast<const void *>(&ImageHeader),
        _data.header_size);
    _header = std::move(header);

    _data.data_size = 300u * 200u;
    _buffer = std::make_unique<const unsigned char[]>(_data.data_size);

    _data.header = _header.get();
    _data.data = _buffer.get();

    return _data;
  }

  void Sensor::CheckData(boost::asio::const_buffer buffer) const {
    std::lock_guard<std::mutex> lock(_mutex);
    const auto size = boost::asio::buffer_size(buffer);
    const auto begin = boost::asio::buffer_cast<const unsigned char *>(buffer);
    const auto header_begin = begin + 2u * sizeof(uint32_t);
    const auto data_begin = header_begin + _data.header_size;

    const auto expected_size = 2u * sizeof(uint32_t) + _data.header_size + _data.data_size;
    ASSERT_EQ(size, expected_size);
    ASSERT_EQ(0, std::memcmp(header_begin, _header.get(), _data.header_size));
    ASSERT_EQ(0, std::memcmp(data_begin, _buffer.get(), _data.data_size));
  }

} // test
