// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/ActorAttributeType.h"
#include "carla/rpc/String.h"

#include <vector>

MSGPACK_ADD_ENUM(carla::rpc::ActorAttributeType);

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Actor/ActorAttribute.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class ActorAttribute {
  public:

    ActorAttribute() = default;

    std::string id;

    ActorAttributeType type = ActorAttributeType::Int;

    std::string value;

    std::vector<std::string> recommended_values;

    bool is_modifiable = true;

    bool restrict_to_recommended = false;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorAttribute(const FActorAttribute &Attribute)
      : id(FromFString(Attribute.Id)),
        type(static_cast<ActorAttributeType>(Attribute.Type)),
        value(FromFString(Attribute.Value)),
        is_modifiable(false) {}

    ActorAttribute(const FActorVariation &Variation)
      : id(FromFString(Variation.Id)),
        type(static_cast<ActorAttributeType>(Variation.Type)),
        is_modifiable(true),
        restrict_to_recommended(Variation.bRestrictToRecommended) {
      recommended_values.reserve(Variation.RecommendedValues.Num());
      for (const auto &Item : Variation.RecommendedValues) {
        recommended_values.push_back(FromFString(Item));
      }
      if (!recommended_values.empty()) {
        value = recommended_values[0u];
      }
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(id, type, value, recommended_values, is_modifiable, restrict_to_recommended);
  };

  class ActorAttributeValue {
  public:

    ActorAttributeValue() = default;

    ActorAttributeValue(const ActorAttribute &attribute)
      : id(attribute.id),
        type(attribute.type),
        value(attribute.value) {}

    std::string id;

    ActorAttributeType type = ActorAttributeType::Int;

    std::string value;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorAttributeValue(const FActorAttribute &Attribute)
      : ActorAttributeValue(ActorAttribute(Attribute)) {}

    operator FActorAttribute() const {
      FActorAttribute Attribute;
      Attribute.Id = ToFString(id);
      Attribute.Type = static_cast<EActorAttributeType>(type);
      Attribute.Value = ToFString(value);
      return Attribute;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(id, type, value);
  };

} // namespace rpc
} // namespace carla
