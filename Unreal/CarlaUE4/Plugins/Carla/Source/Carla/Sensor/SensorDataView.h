// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Sensor/ReadOnlyBufferView.h"

/// A view over a sensor's output data. Does not own the data.
class FSensorDataView {
public:

  FSensorDataView(
      uint32 InSensorId,
      FReadOnlyBufferView InHeader,
      FReadOnlyBufferView InData)
    : SensorId(InSensorId),
      Header(InHeader),
      Data(InData) {}

  uint32 GetSensorId() const
  {
    return SensorId;
  }

  FReadOnlyBufferView GetHeader() const
  {
    return Header;
  }

  FReadOnlyBufferView GetData() const
  {
    return Data;
  }

private:

  uint32 SensorId;

  FReadOnlyBufferView Header;

  FReadOnlyBufferView Data;
};
