// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDescription.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ObjectLabel.h>
#include <util/enable-ue4-macros.h>

/// A view over an actor and its properties.
struct FActorInfo
{
public:

  FActorDescription Description;

  TSet<carla::rpc::CityObjectLabel> SemanticTags;

  FBoundingBox BoundingBox;

  carla::rpc::Actor SerializedData;

  /// @todo To be used solely by the FWorldObserver.
  mutable FVector Velocity = FVector::ZeroVector;

};
