// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/LibITS.h"

#include <cstdint>
#include <vector>

namespace carla {
namespace sensor {

namespace s11n {

  class CAMDataSerializer;
  class CustomV2XDataSerializer;

} // namespace s11n

namespace data {

  class CAMData {
  public:

    float Power;
    CAM_t Message;
  };

  class CustomV2XData {
  public:

    float Power;
    CustomV2XM Message;
  };

  class CAMDataS {
  public:

    /// Returns the number of currently received messages.
    size_t GetMessageCount() const {
      return MessageList.size();
    }

    /// Deletes the current messages.
    void Reset() {
      MessageList.clear();
    }

    /// Adds a new message.
    void WriteMessage(const CAMData &message) {
      MessageList.push_back(message);
    }

  private:

    std::vector<CAMData> MessageList;

    friend class s11n::CAMDataSerializer;
  };

  class CustomV2XDataS {
  public:

    /// Returns the number of currently received messages.
    size_t GetMessageCount() const {
      return MessageList.size();
    }

    /// Deletes the current messages.
    void Reset() {
      MessageList.clear();
    }

    /// Adds a new message.
    void WriteMessage(const CustomV2XData &message) {
      MessageList.push_back(message);
    }

  private:

    std::vector<CustomV2XData> MessageList;

    friend class s11n::CustomV2XDataSerializer;
  };

} // namespace data
} // namespace sensor
} // namespace carla
