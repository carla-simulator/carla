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

  class ActorAttributeValueAccess
  {
  public:
    ActorAttributeValueAccess() = default;
    ActorAttributeValueAccess(ActorAttributeValueAccess const &) = default;
    ActorAttributeValueAccess(ActorAttributeValueAccess &&) = default;
    virtual ~ActorAttributeValueAccess() = default;

    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess const & ) = default;
    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess && ) = default;

    virtual const std::string &GetId() const = 0;

    virtual rpc::ActorAttributeType GetType() const = 0;

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

  protected:
    virtual const std::string &GetValue() const = 0;

    void Validate() const;
  };

  template <>
  bool ActorAttributeValueAccess::As<bool>() const;

  template <>
  int ActorAttributeValueAccess::As<int>() const;

  template <>
  float ActorAttributeValueAccess::As<float>() const;

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const;

  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const;

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Bool>() const {
    return As<bool>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Int>() const {
    return As<int>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Float>() const {
    return As<float>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::String>() const {
    return As<std::string>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::RGBColor>() const {
    return As<sensor::data::Color>();
  }

  template <typename T>
  inline bool ActorAttributeValueAccess::operator==(const T &rhs) const {
    return As<T>() == rhs;
  }

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValueAccess &rhs) const {
    return
        (GetType() == rhs.GetType()) &&
        (GetValue() == rhs.GetValue());
  }

  class ActorAttributeValue: public ActorAttributeValueAccess {

  public:
    ActorAttributeValue(rpc::ActorAttributeValue attribute):
      _attribute(std::move(attribute))
    {
      Validate();
    }
    ActorAttributeValue(ActorAttributeValue const &) = default;
    ActorAttributeValue(ActorAttributeValue &&) = default;
    virtual ~ActorAttributeValue() = default;

    ActorAttributeValue & operator= (ActorAttributeValue const & ) = default;
    ActorAttributeValue & operator= (ActorAttributeValue && ) = default;

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }

    virtual rpc::ActorAttributeType GetType() const override {
      return _attribute.type;
    }

    /// Serialize this object as a carla::rpc::ActorAttributeValue.
    operator rpc::ActorAttributeValue() const{
      return _attribute;
    }

    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }

  private:

    rpc::ActorAttributeValue _attribute;
  };

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValue &rhs) const {
    return rhs.operator==(*this);
  }

  /// An attribute of an ActorBlueprint.
  class ActorAttribute: public ActorAttributeValueAccess {

  public:
    ActorAttribute(rpc::ActorAttribute attribute)
      : ActorAttributeValueAccess(),
        _attribute(std::move(attribute)) {
      Validate();
    }

    ActorAttribute(ActorAttribute const &) = default;
    ActorAttribute(ActorAttribute &&) = default;
    virtual ~ActorAttribute() = default;

    ActorAttribute & operator= (ActorAttribute const & ) = default;
    ActorAttribute & operator= (ActorAttribute && ) = default;

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }

    virtual rpc::ActorAttributeType GetType() const override {
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

    /// Serialize this object as a carla::rpc::ActorAttributeValue.
    operator rpc::ActorAttributeValue() const {
      return _attribute;
    }

  protected:
    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }

  private:
    rpc::ActorAttribute _attribute;
  };

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttribute &rhs) const {
    return rhs.operator==(*this);
  }

} // namespace client
} // namespace carla
