// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/Constants.h"

namespace carla {
namespace traffic_manager {

Parameters::Parameters() {

  /// Set default synchronous mode time out.
  synchronous_time_out = std::chrono::duration<int, std::milli>(10);
}

Parameters::~Parameters() {}

//////////////////////////////////// SETTERS //////////////////////////////////

void Parameters::SetHybridPhysicsMode(const bool mode_switch) {

  hybrid_physics_mode.store(mode_switch);
}

void Parameters::SetRespawnDormantVehicles(const bool mode_switch) {

  respawn_dormant_vehicles.store(mode_switch);
}

void Parameters::SetMaxBoundaries(const float lower, const float upper) {
  min_lower_bound = lower;
  max_upper_bound = upper;
}

void Parameters::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  respawn_lower_bound = min_lower_bound > lower_bound ? min_lower_bound : lower_bound;
  respawn_upper_bound = max_upper_bound < upper_bound ? max_upper_bound : upper_bound;
}

void Parameters::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {

  float new_percentage = std::min(100.0f, percentage);
  percentage_difference_from_speed_limit.AddEntry({actor->GetId(), new_percentage});
}

void Parameters::SetGlobalPercentageSpeedDifference(const float percentage) {
  float new_percentage = std::min(100.0f, percentage);
  global_percentage_difference_from_limit = new_percentage;
}

void Parameters::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
  const ActorId reference_id = reference_actor->GetId();
  const ActorId other_id = other_actor->GetId();

  if (detect_collision) {
    if (ignore_collision.Contains(reference_id)) {
      std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);
      if (actor_set->Contains(other_id)) {
        actor_set->Remove({other_id});
      }
    }
  } else {
    if (ignore_collision.Contains(reference_id)) {
      std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);
      if (!actor_set->Contains(other_id)) {
        actor_set->Insert({other_actor});
      }
    } else {
      std::shared_ptr<AtomicActorSet> actor_set = std::make_shared<AtomicActorSet>();
      actor_set->Insert({other_actor});
      auto entry = std::make_pair(reference_id, actor_set);
      ignore_collision.AddEntry(entry);
    }
  }
}

void Parameters::SetForceLaneChange(const ActorPtr &actor, const bool direction) {

  const ChangeLaneInfo lane_change_info = {true, direction};
  const auto entry = std::make_pair(actor->GetId(), lane_change_info);
  force_lane_change.AddEntry(entry);
}

void Parameters::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {

  const auto entry = std::make_pair(actor->GetId(), percentage);
  perc_keep_right.AddEntry(entry);
}

void Parameters::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {

  const auto entry = std::make_pair(actor->GetId(), enable);
  auto_lane_change.AddEntry(entry);
}

void Parameters::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {

  float new_distance = std::max(0.0f, distance);
  const auto entry = std::make_pair(actor->GetId(), new_distance);
  distance_to_leading_vehicle.AddEntry(entry);
}

void Parameters::SetSynchronousMode(const bool mode_switch) {
  synchronous_mode.store(mode_switch);
}

void Parameters::SetSynchronousModeTimeOutInMiliSecond(const double time) {
  synchronous_time_out = std::chrono::duration<double, std::milli>(time);
}

void Parameters::SetGlobalDistanceToLeadingVehicle(const float dist) {

  distance_margin.store(dist);
}

void Parameters::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {

  float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
  const auto entry = std::make_pair(actor->GetId(), new_perc);
  perc_run_traffic_light.AddEntry(entry);
}

void Parameters::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {

  float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
  const auto entry = std::make_pair(actor->GetId(), new_perc);
  perc_run_traffic_sign.AddEntry(entry);
}

void Parameters::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {

  float new_perc = cg::Math::Clamp(perc, 0.0f, 100.0f);
  const auto entry = std::make_pair(actor->GetId(), new_perc);
  perc_ignore_vehicles.AddEntry(entry);
}

void Parameters::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {

  float new_perc = cg::Math::Clamp(perc,0.0f,100.0f);
  const auto entry = std::make_pair(actor->GetId(), new_perc);
  perc_ignore_walkers.AddEntry(entry);
}

void Parameters::SetHybridPhysicsRadius(const float radius) {
  float new_radius = std::max(radius, 0.0f);
  hybrid_physics_radius.store(new_radius);
}

void Parameters::SetOSMMode(const bool mode_switch) {
  osm_mode.store(mode_switch);
}

//////////////////////////////////// GETTERS //////////////////////////////////

float Parameters::GetHybridPhysicsRadius() const {

  return hybrid_physics_radius.load();
}

bool Parameters::GetSynchronousMode() const {
  return synchronous_mode.load();
}

double Parameters::GetSynchronousModeTimeOutInMiliSecond() const {
  return synchronous_time_out.count();
}

float Parameters::GetVehicleTargetVelocity(const ActorId &actor_id, const float speed_limit) const {

  float percentage_difference = global_percentage_difference_from_limit;

  if (percentage_difference_from_speed_limit.Contains(actor_id)) {
    percentage_difference = percentage_difference_from_speed_limit.GetValue(actor_id);
  }

  return speed_limit * (1.0f - percentage_difference / 100.0f);
}

bool Parameters::GetCollisionDetection(const ActorId &reference_actor_id, const ActorId &other_actor_id) const {

  bool avoid_collision = true;

  if (ignore_collision.Contains(reference_actor_id) &&
    ignore_collision.GetValue(reference_actor_id)->Contains(other_actor_id)) {
    avoid_collision = false;
  }

  return avoid_collision;
}

ChangeLaneInfo Parameters::GetForceLaneChange(const ActorId &actor_id) {

  ChangeLaneInfo change_lane_info {false, false};

  if (force_lane_change.Contains(actor_id)) {
    change_lane_info = force_lane_change.GetValue(actor_id);
  }

  force_lane_change.RemoveEntry(actor_id);

  return change_lane_info;
}

float Parameters::GetKeepRightPercentage(const ActorId &actor_id) {

  float percentage = -1.0f;

  if (perc_keep_right.Contains(actor_id)) {
    percentage = perc_keep_right.GetValue(actor_id);
  }

  perc_keep_right.RemoveEntry(actor_id);

  return percentage;
}

bool Parameters::GetAutoLaneChange(const ActorId &actor_id) const {

  bool auto_lane_change_policy = true;

  if (auto_lane_change.Contains(actor_id)) {
    auto_lane_change_policy = auto_lane_change.GetValue(actor_id);
  }

  return auto_lane_change_policy;
}

float Parameters::GetDistanceToLeadingVehicle(const ActorId &actor_id) const {

  float specific_distance_margin = 0.0f;
  if (distance_to_leading_vehicle.Contains(actor_id)) {
    specific_distance_margin = distance_to_leading_vehicle.GetValue(actor_id);
  } else {
    specific_distance_margin = distance_margin;
  }

  return specific_distance_margin;
}

float Parameters::GetPercentageRunningLight(const ActorId &actor_id) const {

  float percentage = 0.0f;

  if (perc_run_traffic_light.Contains(actor_id)) {
    percentage = perc_run_traffic_light.GetValue(actor_id);
  }

  return percentage;
}

float Parameters::GetPercentageRunningSign(const ActorId &actor_id) const {

  float percentage = 0.0f;

  if (perc_run_traffic_sign.Contains(actor_id)) {
    percentage = perc_run_traffic_sign.GetValue(actor_id);
  }

  return percentage;
}

float Parameters::GetPercentageIgnoreWalkers(const ActorId &actor_id) const {

  float percentage = 0.0f;

  if (perc_ignore_walkers.Contains(actor_id)) {
    percentage = perc_ignore_walkers.GetValue(actor_id);
  }

  return percentage;
}

float Parameters::GetPercentageIgnoreVehicles(const ActorId &actor_id) const {

  float percentage = 0.0f;

  if (perc_ignore_vehicles.Contains(actor_id)) {
    percentage = perc_ignore_vehicles.GetValue(actor_id);
  }

  return percentage;
}

bool Parameters::GetHybridPhysicsMode() const {

  return hybrid_physics_mode.load();
}

bool Parameters::GetRespawnDormantVehicles() const {

  return respawn_dormant_vehicles.load();
}

float Parameters::GetLowerBoundaryRespawnDormantVehicles() const {

  return respawn_lower_bound.load();
}

float Parameters::GetUpperBoundaryRespawnDormantVehicles() const {

  return respawn_upper_bound.load();
}


bool Parameters::GetOSMMode() const {

  return osm_mode.load();
}

} // namespace traffic_manager
} // namespace carla
