// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Settings/SensorDescriptionVisitor.h"

class ASensor;
class UWorld;

class FSensorFactory : private ISensorDescriptionVisitor
{
public:

  static ASensor *Make(
      const FString &Name,
      const USensorDescription &Description,
      UWorld &World);

private:

  FSensorFactory(UWorld &World);

  virtual void Visit(const UCameraDescription &) final;

  virtual void Visit(const ULidarDescription &) final;

  UWorld &World;

  ASensor *Sensor = nullptr;
};
