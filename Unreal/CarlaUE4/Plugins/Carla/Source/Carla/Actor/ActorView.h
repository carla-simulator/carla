// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

class AActor;

/// A view over an actor and its properties.
class FActorView
{
public:

  using IdType = uint32;

  FActorView() = default;
  FActorView(const FActorView &) = default;

  bool IsValid() const
  {
    return TheActor != nullptr;
  }

  IdType GetActorId() const
  {
    return Id;
  }

  AActor *GetActor()
  {
    return TheActor;
  }

  const AActor *GetActor() const
  {
    return TheActor;
  }

private:

  friend class FActorRegistry;

  FActorView(IdType ActorId, AActor &Actor)
    : Id(ActorId),
      TheActor(&Actor) {}

  IdType Id = 0u;

  AActor *TheActor = nullptr;
};
