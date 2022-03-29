// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDescription.h"
#include "Carla/Game/Tagger.h"

#include "Math/DVector.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/ObjectLabel.h>
#include <compiler/enable-ue4-macros.h>

namespace crp = carla::rpc;

/// A view over an actor and its properties.
struct FActorInfo
{
public:

  FActorDescription Description;

  TSet<crp::CityObjectLabel> SemanticTags;

  FBoundingBox BoundingBox;

  crp::Actor SerializedData;

  /// @todo To be used solely by the FWorldObserver.
  mutable FVector Velocity = {0.0f, 0.0f, 0.0f};

};
