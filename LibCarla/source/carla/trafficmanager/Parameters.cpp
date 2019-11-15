#include "Parameters.h"

namespace traffic_manager {

    Parameters::Parameters() {}

    Parameters::~Parameters() {}

    void Parameters::SetPercentageSpeedBelowLimit(const ActorPtr &actor, const float percentage) {

        percentage_decrease_from_speed_limit.AddEntry({actor->GetId(), percentage});
    }

    void Parameters::SetGlobalPercentageBelowLimit(float percentage_below_limit) {

        global_percentage_decrease_from_limit = percentage_below_limit;
    }

    void Parameters::SetCollisionDetection(
        const ActorPtr &reference_actor,
        const ActorPtr &other_actor,
        const bool detect_collision) {

        ActorId reference_id = reference_actor->GetId();
        ActorId other_id = other_actor->GetId();

        if (detect_collision) {

            if (ignore_collision.Contains(reference_id)) {
                std::shared_ptr<AtomicActorSet> actor_set = ignore_collision.GetValue(reference_id);
                if (actor_set->Contains(other_id)) {
                actor_set->Remove({other_actor});
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

        ChangeLaneInfo lane_change_info = {true, direction};
        auto entry = std::make_pair(actor->GetId(), lane_change_info);
        force_lane_change.AddEntry(entry);
    }

    void Parameters::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {

        auto entry = std::make_pair(actor->GetId(), enable);
        auto_lane_change.AddEntry(entry);
    }

    void Parameters::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {

        auto entry = std::make_pair(actor->GetId(), distance);
        distance_to_leading_vehicle.AddEntry(entry);
    }

    float Parameters::GetVehicleTargetVelocity(const ActorPtr &actor) {

        ActorId actor_id = actor->GetId();
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
        float speed_limit = vehicle->GetSpeedLimit();
        float percentage_decrease = global_percentage_decrease_from_limit;

        if (percentage_decrease_from_speed_limit.Contains(actor_id)) {
            percentage_decrease = percentage_decrease_from_speed_limit.GetValue(actor_id);
        }

        return speed_limit * (1 - percentage_decrease/100);
    }

    bool Parameters::GetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor) {

        ActorId reference_actor_id = reference_actor->GetId();
        ActorId other_actor_id = other_actor->GetId();
        bool ignore = true;

        if (ignore_collision.Contains(reference_actor_id) &&
            ignore_collision.GetValue(reference_actor_id)->Contains(other_actor_id)) {
            ignore = false;
        }

        return ignore;
    }

    ChangeLaneInfo Parameters::GetForceLaneChange(const ActorPtr &actor) {

        ActorId actor_id = actor->GetId();
        ChangeLaneInfo change_lane_info;

        if (force_lane_change.Contains(actor_id)) {
            change_lane_info = force_lane_change.GetValue(actor_id);
        }

        force_lane_change.RemoveEntry(actor_id);

        return change_lane_info;
    }

    bool Parameters::GetAutoLaneChange(const ActorPtr &actor) {

        ActorId actor_id = actor->GetId();
        bool auto_lane_change_policy = true;

        if (auto_lane_change.Contains(actor_id)) {
            auto_lane_change_policy = auto_lane_change.GetValue(actor_id);
        }

        return auto_lane_change_policy;
    }

    float Parameters::GetDistanceToLeadingVehicle(const ActorPtr &actor) {

        ActorId actor_id = actor->GetId();
        float distance_margin = -1;

        if (distance_to_leading_vehicle.Contains(actor_id)) {
            distance_margin = distance_to_leading_vehicle.GetValue(actor_id);
        }

        return distance_margin;
    }

}