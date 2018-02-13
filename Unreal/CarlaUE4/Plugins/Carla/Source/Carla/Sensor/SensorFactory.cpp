// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "SensorFactory.h"

#include "Lidar.h"
#include "SceneCaptureCamera.h"
#include "Settings/CameraDescription.h"
#include "Settings/LidarDescription.h"

ASensor *FSensorFactory::Make(
    const FString &Name,
    const USensorDescription &Description,
    UWorld &World)
{
  FSensorFactory Visitor(World);
  Description.AcceptVisitor(Visitor);
  check(Visitor.Sensor != nullptr);
  Visitor.Sensor->SetName(Name);
  return Visitor.Sensor;
}

FSensorFactory::FSensorFactory(UWorld &World) : World(World) {}

void FSensorFactory::Visit(const UCameraDescription &Description)
{
  auto Camera = World.SpawnActor<ASceneCaptureCamera>(Description.Position, Description.Rotation);
  Camera->Set(Description);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Created Capture Camera %d with postprocess \"%s\""),
      Camera->GetId(),
      *PostProcessEffect::ToString(Camera->GetPostProcessEffect()));
  Sensor = Camera;
}

void FSensorFactory::Visit(const ULidarDescription &Description)
{
  auto Lidar = World.SpawnActor<ALidar>(Description.Position, Description.Rotation);
  Lidar->Set(Description);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Created LiDAR %d"),
      Lidar->GetId());
  Sensor = Lidar;
}
