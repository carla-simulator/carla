// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/V2XSerializer.h"

#include "carla/sensor/data/V2XEvent.h"

namespace carla
{
  namespace sensor
  {
    namespace s11n
    {

      SharedPtr<SensorData> CAMDataSerializer::Deserialize(RawData &&data)
      {
        return SharedPtr<data::CAMEvent>(new data::CAMEvent(std::move(data)));
      }

      SharedPtr<SensorData> CustomV2XDataSerializer::Deserialize(RawData &&data)
      {
        return SharedPtr<data::CustomV2XEvent>(new data::CustomV2XEvent(std::move(data)));
      }

    } // namespace s11n
  }   // namespace sensor
} // namespace carla
