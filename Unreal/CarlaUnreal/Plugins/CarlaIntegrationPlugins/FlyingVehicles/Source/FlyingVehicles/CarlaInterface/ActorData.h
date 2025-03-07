// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorData.h"
#include "FlyingVehicles/MultirotorControl.h"
#include "FlyingVehicles/MultirotorPhysicsControl.h"


class FMultirotorData : public FActorData
{
public:

    FMultirotorPhysicsControl PhysicsControl;

    FMultirotorControl Control;

    virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

    virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};