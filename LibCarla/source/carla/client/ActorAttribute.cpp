// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorAttribute.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"

namespace carla {
namespace client {

#define LIBCARLA_THROW_INVALID_VALUE(message) throw InvalidAttributeValue(_attribute.id + ": " + message);
#define LIBCARLA_THROW_BAD_VALUE_CAST(type) \
    if (GetType() != rpc::ActorAttributeType:: type) { \
      throw BadAttributeCast(_attribute.id + ": bad attribute cast: cannot convert to " #type); \
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
  bool ActorAttribute::As<bool>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Bool);
    auto value = StringUtil::ToLowerCopy(_attribute.value);
    if (value == "true") {
      return true;
    } else if (value == "false") {
      return false;
    }
    LIBCARLA_THROW_INVALID_VALUE("invalid bool: " + _attribute.value);
  }

  template<>
  int ActorAttribute::As<int>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Int);
    return std::atoi(_attribute.value.c_str());
  }

  template<>
  float ActorAttribute::As<float>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Float);
    double x = std::atof(_attribute.value.c_str());
    if ((x > std::numeric_limits<float>::max()) ||
        (x < std::numeric_limits<float>::lowest())) {
      LIBCARLA_THROW_INVALID_VALUE("float overflow");
    }
    return x;
  }

  template <>
  std::string ActorAttribute::As<std::string>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(String);
    return _attribute.value;
  }

  template <>
  sensor::data::Color ActorAttribute::As<sensor::data::Color>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(RGBColor);

    std::vector<std::string> channels;
    StringUtil::Split(channels, _attribute.value, ",");
    if (channels.size() != 3u) {
      log_error("invalid color", _attribute.value);
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

  void ActorAttribute::Validate() const {
    switch (_attribute.type) {
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
