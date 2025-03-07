// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CameraControlROS2Handler.h"
#include "CameraController/CameraControl.h"

void CameraControlROS2Handler::operator()(carla::ros2::CameraControl &Source)
{
  if (!_Actor) return;

  ACameraControllerActor *CameraControl = Cast<ACameraControllerActor>(_Actor);
  if (!CameraControl) return;

  // setup control values
  FCameraControl NewControl;
  NewControl.Pan = Source.pan;
  NewControl.Tilt = Source.tilt;
  NewControl.Zoom = Source.zoom;

  CameraControl->ApplyCameraControl(NewControl);
}
