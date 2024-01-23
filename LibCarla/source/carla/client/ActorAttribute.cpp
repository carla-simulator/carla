// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorAttribute.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/StringUtil.h"

namespace carla {
namespace client {

#define LIBCARLA_THROW_INVALID_VALUE(message) throw_exception(InvalidAttributeValue(GetId() + ": " + message));
#define LIBCARLA_THROW_BAD_VALUE_CAST(type) \
    if (GetType() != rpc::ActorAttributeType:: type) { \
      throw_exception(BadAttributeCast(GetId() + ": bad attribute cast: cannot convert to " #type)); \
    }

  void ActorAttribute::Set(std::string value) {
    if (!_attribute.is_modifiable) {
      LIBCARLA_THROW_INVALID_VALUE("read-only attribute");
    }
    if (GetType() == rpc::ActorAttributeType::Bool) {
      StringUtil::ToLower(value);
    }
    _attribute.value = std::move(value);
    Validate();
  }


  template <>
  bool ActorAttributeValueAccess::As<bool>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Bool);
    auto value = StringUtil::ToLowerCopy(GetValue());
    if (value == "true") {
      return true;
    } else if (value == "false") {
      return false;
    }
    LIBCARLA_THROW_INVALID_VALUE("invalid bool: " + GetValue());
  }

  template<>
  int ActorAttributeValueAccess::As<int>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Int);
    return std::atoi(GetValue().c_str());
  }

  template<>
  float ActorAttributeValueAccess::As<float>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Float);
    double x = std::atof(GetValue().c_str());
    if ((x > std::numeric_limits<float>::max()) ||
        (x < std::numeric_limits<float>::lowest())) {
      LIBCARLA_THROW_INVALID_VALUE("float overflow");
    }
    return static_cast<float>(x);
  }

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(String);
    return GetValue();
  }

  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(RGBColor);

    std::vector<std::string> channels;
    StringUtil::Split(channels, GetValue(), ",");
    if (channels.size() != 3u) {
      log_error("invalid color", GetValue());
      LIBCARLA_THROW_INVALID_VALUE("colors must have 3 channels (R,G,B)");
    }

    auto to_int = [this](const std::string &str) {
      int i = std::atoi(str.c_str());
      if (i > std::numeric_limits<uint8_t>::max()) {
        LIBCARLA_THROW_INVALID_VALUE("integer overflow in color channel");
      }
      return static_cast<uint8_t>(i);
    };

    return {to_int(channels[0u]), to_int(channels[1u]), to_int(channels[2u])};
  }

  void ActorAttributeValueAccess::Validate() const {
    switch (GetType()) {
      case rpc::ActorAttributeType::Bool:     As<rpc::ActorAttributeType::Bool>();     break;
      case rpc::ActorAttributeType::Int:      As<rpc::ActorAttributeType::Int>();      break;
      case rpc::ActorAttributeType::Float:    As<rpc::ActorAttributeType::Float>();    break;
      case rpc::ActorAttributeType::String:   As<rpc::ActorAttributeType::String>();   break;
      case rpc::ActorAttributeType::RGBColor: As<rpc::ActorAttributeType::RGBColor>(); break;
      default:
        LIBCARLA_THROW_INVALID_VALUE("invalid value type");
    }
  }

#undef LIBCARLA_THROW_BAD_VALUE_CAST
#undef LIBCARLA_THROW_INVALID_VALUE

} // namespace client
} // namespace carla
