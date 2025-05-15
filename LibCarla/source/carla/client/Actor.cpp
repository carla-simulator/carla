// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  geom::Location Actor::GetLocation() const {
    return GetEpisode().Lock()->GetActorLocation(*this);
  }

  geom::Transform Actor::GetTransform() const {
    return GetEpisode().Lock()->GetActorTransform(*this);
  }

  geom::Vector3D Actor::GetVelocity() const {
    return GetEpisode().Lock()->GetActorVelocity(*this);
  }

  geom::Vector3D Actor::GetAngularVelocity() const {
    return GetEpisode().Lock()->GetActorAngularVelocity(*this);
  }

  geom::Vector3D Actor::GetAcceleration() const {
    return GetEpisode().Lock()->GetActorAcceleration(*this);
  }

  geom::BoundingBox Actor::GetBoundingBox() const {
    geom::BoundingBox bounding_box = GetEpisode().Lock()->GetActorBoundingBox(*this); 
    if (bounding_box.extent.x == 0.0f && bounding_box.extent.y == 0.0f && bounding_box.extent.z == 0.0f) {
      return bounding_box;
    }
    else{
      return Super::GetBoundingBox();
    }
  }

  geom::Transform Actor::GetComponentWorldTransform(const std::string componentName) const {
    return GetEpisode().Lock()->GetActorComponentWorldTransform(*this, componentName);
  }

  geom::Transform Actor::GetComponentRelativeTransform(const std::string componentName) const {
    return GetEpisode().Lock()->GetActorComponentRelativeTransform(*this, componentName);
  }

  std::vector<geom::Transform> Actor::GetBoneWorldTransforms() const {
    return GetEpisode().Lock()->GetActorBoneWorldTransforms(*this);
  }

  std::vector<geom::Transform> Actor::GetBoneRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorBoneRelativeTransforms(*this);
  }

  std::vector<std::string> Actor::GetComponentNames() const {
    return GetEpisode().Lock()->GetActorComponentNames(*this);
  }

  std::vector<std::string> Actor::GetBoneNames() const {
    return GetEpisode().Lock()->GetActorBoneNames(*this);
  } 

  std::vector<geom::Transform> Actor::GetSocketWorldTransforms() const {
    return GetEpisode().Lock()->GetActorSocketWorldTransforms(*this);
  }

  std::vector<geom::Transform> Actor::GetSocketRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorSocketRelativeTransforms(*this);
  }

  std::vector<std::string> Actor::GetSocketNames() const {
    return GetEpisode().Lock()->GetActorSocketNames(*this);
  }  

  void Actor::SetLocation(const geom::Location &location) {
    GetEpisode().Lock()->SetActorLocation(*this, location);
  }

  void Actor::SetTransform(const geom::Transform &transform) {
    GetEpisode().Lock()->SetActorTransform(*this, transform);
  }

  void Actor::SetTargetVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetVelocity(*this, vector);
  }

  void Actor::SetTargetAngularVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetAngularVelocity(*this, vector);
  }

  void Actor::EnableConstantVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->EnableActorConstantVelocity(*this, vector);
  }

  void Actor::DisableConstantVelocity() {
    GetEpisode().Lock()->DisableActorConstantVelocity(*this);
  }

  void Actor::AddImpulse(const geom::Vector3D &impulse) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse);
  }

  void Actor::AddImpulse(const geom::Vector3D &impulse, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse, location);
  }

  void Actor::AddForce(const geom::Vector3D &force) {
    GetEpisode().Lock()->AddActorForce(*this, force);
  }

  void Actor::AddForce(const geom::Vector3D &force, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorForce(*this, force, location);
  }

  void Actor::AddAngularImpulse(const geom::Vector3D &vector) {
    GetEpisode().Lock()->AddActorAngularImpulse(*this, vector);
  }

  void Actor::AddTorque(const geom::Vector3D &torque) {
    GetEpisode().Lock()->AddActorTorque(*this, torque);
  }

  void Actor::SetSimulatePhysics(const bool enabled) {
    GetEpisode().Lock()->SetActorSimulatePhysics(*this, enabled);
  }

  void Actor::SetCollisions(const bool enabled) {
    GetEpisode().Lock()->SetActorCollisions(*this, enabled);
  }

  void Actor::SetActorDead() {
    GetEpisode().Lock()->SetActorDead(*this);
  }

  void Actor::SetEnableGravity(const bool enabled) {
    GetEpisode().Lock()->SetActorEnableGravity(*this, enabled);
  }

  rpc::ActorState Actor::GetActorState() const {
    return GetEpisode().Lock()->GetActorState(*this);
  }

  bool Actor::Destroy() {
    rpc::ActorState actor_state = GetActorState();
    bool result = false;
    if (actor_state != rpc::ActorState::Invalid) {
      result = GetEpisode().Lock()->DestroyActor(*this);
    } else {
      log_warning(
          "attempting to destroy an actor that is already dead:",
          GetDisplayId());
    }
    return result;
  }

} // namespace client
} // namespace carla
