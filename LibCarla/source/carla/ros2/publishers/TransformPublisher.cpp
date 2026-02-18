// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _USE_MATH_DEFINES
#include <cmath>

#include "TransformPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

TransformPublisher::TransformPublisher()
  : PublisherBase(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("tf")),
    _impl_tf(std::make_shared<TransformPublisherImpl>()),
    _impl_tf_static(std::make_shared<TransformPublisherImpl>()) {}

bool TransformPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl_tf->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, "rt/tf", get_topic_qos())
    && _impl_tf_static->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, "rt/tf_static", get_topic_qos());
}

bool TransformPublisher::Publish() {
  auto success = _impl_tf->Publish();
  success &= _impl_tf_static->Publish();
  // after every frame clear the dynamic tf tree
  _impl_tf->Message().transforms().clear();
  return success;
}

bool TransformPublisher::SubscribersConnected() const {
  return _impl_tf->SubscribersConnected() || _impl_tf_static->SubscribersConnected();
}

void TransformPublisher::AddTransform(const builtin_interfaces::msg::Time &stamp, const std::string &name, const std::string &parent,
                                      geometry_msgs::msg::Transform const &transform, TransformPublisher::TransformPublisherMode const mode) {
  
  geometry_msgs::msg::TransformStamped ts;
  ts.header().frame_id(parent);
  if ( name == parent ) {
    // the child frame cannot be its own parent in ROS TF, so just ignore
    return;
  }
  else {
    ts.child_frame_id(name);
  }
  ts.header().stamp(stamp);
  ts.transform(transform);

  if ( mode == TransformPublisherMode::MODE_STATIC ) {
    bool found = false;
    for (auto & t : _impl_tf_static->Message().transforms()) {
      if (t.child_frame_id() == ts.child_frame_id()) {
        // the child frame already exists in the static tf tree, so republish it only if necessary
        // either the transform or the parent frame has changed
        if (t.transform() != ts.transform() || t.header().frame_id() != ts.header().frame_id()) {
          t = ts;
          _impl_tf_static->SetMessageUpdated();
        }
        found = true;
        break;
      }
    }
    if ( !found ) {
      _impl_tf_static->Message().transforms().push_back(ts);
      _impl_tf_static->SetMessageUpdated();
    }
  }
  else {
    _impl_tf->Message().transforms().push_back(ts);
    _impl_tf->SetMessageUpdated();
  }
}

void TransformPublisher::RemoveTransform(const std::string &name) {
  // remove the transform from the static tf tree if it exists there
  for (auto it = _impl_tf_static->Message().transforms().begin(); it != _impl_tf_static->Message().transforms().end(); ++it) {
    if (it->child_frame_id() == name) {
      _impl_tf_static->Message().transforms().erase(it);
      _impl_tf_static->SetMessageUpdated();
      return;
    }
 }
}

}  // namespace ros2
}  // namespace carla
