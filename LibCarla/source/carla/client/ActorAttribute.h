// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorAttribute.h"
#include "carla/sensor/data/Color.h"

#include <cstdlib>
#include <exception>
#include <type_traits>

namespace carla {
namespace client {

  // ===========================================================================
  // -- InvalidAttributeValue --------------------------------------------------
  // ===========================================================================

  /// Exception thrown when the value given to an ActorAttribute cannot be
  /// converted to its type.
  class InvalidAttributeValue : public std::invalid_argument {
  public:

    InvalidAttributeValue(const std::string &what) : std::invalid_argument(what) {}
  };

  // ===========================================================================
  // -- BadAttributeCast -------------------------------------------------------
  // ===========================================================================

  /// Exception thrown when the value of an ActorAttribute cannot be cast to the
  /// requested type.
  class BadAttributeCast : public std::logic_error {
  public:

    BadAttributeCast(const std::string &what) : std::logic_error(what) {}
  };

  // ===========================================================================
  // -- ActorAttribute ---------------------------------------------------------
  // ===========================================================================

  /// An attribute of an ActorBlueprint.
  class ActorAttribute {
  public:

    ActorAttribute(rpc::ActorAttribute attribute)
      : _attribute(std::move(attribute)) {
      Validate();
    }

    const std::string &GetId() const {
      return _attribute.id;
    }

    rpc::ActorAttributeType GetType() const {
      return _attribute.type;
    }

    const std::vector<std::string> &GetRecommendedValues() const {
      return _attribute.recommended_values;
    }

    bool IsModifiable() const {
      return _attribute.is_modifiable;
    }

    /// Set the value of this attribute.
    ///
    /// @throw InvalidAttributeValue if attribute is not modifiable.
    /// @throw InvalidAttributeValue if format does not match this type.
    void Set(std::string value);

    /// Cast the value to the given type.
    ///
    /// @throw BadAttributeCast if the cast fails.
    template <typename T>
    T As() const;

    /// Cast the value to the type specified by the enum
    /// carla::rpc::ActorAttributeType.
    ///
    /// @throw BadAttributeCast if the cast fails.
    template <rpc::ActorAttributeType Type>
    auto As() const;

    template <typename T>
    bool operator==(const T &rhs) const;

    template <typename T>
    bool operator!=(const T &rhs) const {
      return !(*this == rhs);
    }

    /// Serialize this object as a carla::rpc::ActorAttributeValue.
    operator rpc::ActorAttributeValue() const {
      return _attribute;
    }

  private:

    void Validate() const;

    rpc::ActorAttribute _attribute;
  };

  template <>
  bool ActorAttribute::As<bool>() const;

  template <>
  int ActorAttribute::As<int>() const;

  template <>
  float ActorAttribute::As<float>() const;

  template <>
  std::string ActorAttribute::As<std::string>() const;

  template <>
  sensor::data::Color ActorAttribute::As<sensor::data::Color>() const;

  template <>
  inline auto ActorAttribute::As<rpc::ActorAttributeType::Bool>() const {
    return As<bool>();
  }

  template <>
  inline auto ActorAttribute::As<rpc::ActorAttributeType::Int>() const {
    return As<int>();
  }

  template <>
  inline auto ActorAttribute::As<rpc::ActorAttributeType::Float>() const {
    return As<float>();
  }

  template <>
  inline auto ActorAttribute::As<rpc::ActorAttributeType::String>() const {
    return As<std::string>();
  }

  template <>
  inline auto ActorAttribute::As<rpc::ActorAttributeType::RGBColor>() const {
    return As<sensor::data::Color>();
  }

  template <typename T>
  inline bool ActorAttribute::operator==(const T &rhs) const {
    return As<T>() == rhs;
  }

  template <>
  inline bool ActorAttribute::operator==(const ActorAttribute &rhs) const {
    return
        (_attribute.type == rhs._attribute.type) &&
        (_attribute.value == rhs._attribute.value);
  }

} // namespace client
} // namespace carla
