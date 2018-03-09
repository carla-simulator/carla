// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class FSensorDataView;

/// Interface for sensor data sinks.
class ISensorDataSink {
public:

  virtual ~ISensorDataSink() {}

  virtual void Write(const FSensorDataView &SensorData) = 0;
};
