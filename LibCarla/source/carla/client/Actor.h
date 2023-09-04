// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/detail/ActorState.h"
#include "carla/profiler/LifetimeProfiled.h"

namespace carla {
namespace client {

  /// Represents an actor in the simulation.
  class Actor
    : public EnableSharedFromThis<Actor>,
      private profiler::LifetimeProfiled,
      public detail::ActorState {
    using Super = detail::ActorState;
  public:

    explicit Actor(ActorInitializer init)
      : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(init.GetDisplayId()),
        Super(std::move(init)) {}

    using ActorState::GetBoundingBox;

    virtual ~Actor() = default;

    /// Return the current location of the actor.
    ///
    /// @note This function does not call the simulator, it returns the location
    /// received in the last tick.
    geom::Location GetLocation() const;

    /// Return the current transform of the actor.
    ///
    /// @note This function does not call the simulator, it returns the
    /// transform received in the last tick.
    geom::Transform GetTransform() const;

    /// Return the current 3D velocity of the actor.
    ///
    /// @note This function does not call the simulator, it returns the
    /// velocity received in the last tick.
    geom::Vector3D GetVelocity() const;

    /// Return the current 3D angular velocity of the actor.
    ///
    /// @note This function does not call the simulator, it returns the
    /// angular velocity received in the last tick.
    geom::Vector3D GetAngularVelocity() const;

    /// Return the current 3D acceleration of the actor.
    ///
    /// @note This function does not call the simulator, it returns the
    /// acceleration calculated after the actor's velocity.
    geom::Vector3D GetAcceleration() const;

    /// Teleport the actor to @a location.
    void SetLocation(const geom::Location &location);

    /// Teleport and rotate the actor to @a transform.
    void SetTransform(const geom::Transform &transform);

    /// Set the actor velocity before applying physics.
    void SetTargetVelocity(const geom::Vector3D &vector);

    /// Set the angular velocity of the actor before applying physics.
    void SetTargetAngularVelocity(const geom::Vector3D &vector);

    /// Enable a constant velocity mode
    void EnableConstantVelocity(const geom::Vector3D &vector);

    /// Disable the constant velocity mode
    void DisableConstantVelocity();

    /// Add impulse to the actor at its center of mass.
    void AddImpulse(const geom::Vector3D &vector);

    /// Add impulse to the actor at certain location.
    void AddImpulse(const geom::Vector3D &impulse, const geom::Vector3D &location);

    /// Add force to the actor at its center of mass.
    void AddForce(const geom::Vector3D &force);

    /// Add force to the actor at certain location.
    void AddForce(const geom::Vector3D &force, const geom::Vector3D &location);

    /// Add angular impulse to the actor.
    void AddAngularImpulse(const geom::Vector3D &vector);

    /// Add a torque to the actor.
    void AddTorque(const geom::Vector3D &vector);

    /// Enable or disable physics simulation on this actor.
    void SetSimulatePhysics(bool enabled = true);

    /// Enable or disable collisions on this actor.
    void SetCollisions(bool enabled = true);

    /// Set actor as dead and starts his life span
    void SetActorDead();

    /// Enable or disable gravity on this actor.
    void SetEnableGravity(bool enabled = true);

    rpc::ActorState GetActorState() const;

    bool IsAlive() const {
      return GetEpisode().IsValid() && (GetActorState() != rpc::ActorState::PendingKill && GetActorState() != rpc::ActorState::Invalid) ;
    }

    bool IsDormant() const {
      return GetEpisode().IsValid() && GetActorState() == rpc::ActorState::Dormant;
    }

    bool IsActive() const {
      return GetEpisode().IsValid() && GetActorState() == rpc::ActorState::Active;
    }

    /// Tell the simulator to destroy this Actor, and return whether the actor
    /// was successfully destroyed.
    ///
    /// @note It has no effect if the Actor was already successfully destroyed.
    ///
    /// @warning This function blocks until the destruction operation is
    /// completed by the simulator.
    virtual bool Destroy();

    const auto &Serialize() const {
      return Super::GetActorDescription();
    }

  };

} // namespace client
} // namespace carla
