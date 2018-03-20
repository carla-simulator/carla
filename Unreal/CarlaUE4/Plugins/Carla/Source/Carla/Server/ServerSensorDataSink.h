// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Sensor/SensorDataSink.h"

/// Sends the sensor data to the client.
class FServerSensorDataSink : public ISensorDataSink {
public:

  void SetServer(TSharedPtr<FCarlaServer> InServer)
  {
    Server = InServer;
  }

  virtual void Write(const FSensorDataView &SensorData) final
  {
    auto Ptr = Server.Pin();
    if (Ptr.IsValid()) {
      Ptr->SendSensorData(SensorData);
    }
  }

private:

  TWeakPtr<FCarlaServer> Server;
};
