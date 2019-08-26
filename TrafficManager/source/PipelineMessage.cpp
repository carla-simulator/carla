// Member definitions for class PipelineMessage

#include "PipelineMessage.h"

namespace traffic_manager {

  PipelineMessage::PipelineMessage() {}
  PipelineMessage::~PipelineMessage() {}

  int PipelineMessage::getActorID() const {
    return actor_id;
  }
  void PipelineMessage::setActor(carla::SharedPtr<carla::client::Actor> actor) {
    this->actor = actor;
    this->actor_id = actor->GetId();
  }
  carla::SharedPtr<carla::client::Actor> PipelineMessage::getActor() const {
    return actor;
  }
  void PipelineMessage::setAttribute(std::string name, float value) {
    attributes[name] = value;
  }
  float PipelineMessage::getAttribute(const std::string& name) {
    return attributes[name];
  }
  bool PipelineMessage::hasAttribute(std::string name)const {
    if (attributes.find(name) != attributes.end()) {
      return true;
    } else {
      return false;
    }
  }
  void PipelineMessage::removeAttribute(std::string name) {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
      attributes.erase(it);
    }
  }
}
