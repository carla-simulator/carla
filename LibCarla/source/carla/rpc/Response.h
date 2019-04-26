// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <string>

namespace carla {
namespace rpc {

  class ResponseError {
  public:

    ResponseError() = default;

    explicit ResponseError(std::string message)
      : _what(std::move(message)) {}

    const std::string &What() const {
      return _what;
    }

    MSGPACK_DEFINE_ARRAY(_what)

  private:

    std::string _what;
  };

  template <typename T>
  class Response {
  public:

    using value_type = T;

    using error_type = ResponseError;

    Response() = default;

    template <typename TValue>
    Response(TValue &&value) : _data(std::forward<TValue>(value)) {}

    template <typename TValue>
    void Reset(TValue &&value) {
      _data = std::forward<TValue>(value);
    }

    bool HasError() const {
      return _data.which() == 0;
    }

    template <typename... Ts>
    void SetError(Ts &&... args) {
      _data = error_type(std::forward<Ts>(args)...);
    }

    const error_type &GetError() const {
      DEBUG_ASSERT(HasError());
      return boost::get<error_type>(_data);
    }

    value_type &Get() {
      DEBUG_ASSERT(!HasError());
      return boost::get<value_type>(_data);
    }

    const value_type &Get() const {
      DEBUG_ASSERT(!HasError());
      return boost::get<value_type>(_data);
    }

    operator bool() const {
      return !HasError();
    }

    MSGPACK_DEFINE_ARRAY(_data)

  private:

    boost::variant<error_type, value_type> _data;
  };

  template <>
  class Response<void> {
  public:

    using value_type = void;

    using error_type = ResponseError;

    static Response Success() {
      return success_flag{};
    }

    Response() : _data(error_type{}) {}

    Response(ResponseError error) : _data(std::move(error)) {}

    bool HasError() const {
      return _data.has_value();
    }

    template <typename... Ts>
    void SetError(Ts &&... args) {
      _data = error_type(std::forward<Ts>(args)...);
    }

    const error_type &GetError() const {
      DEBUG_ASSERT(HasError());
      return *_data;
    }

    operator bool() const {
      return !HasError();
    }

    MSGPACK_DEFINE_ARRAY(_data)

  private:

    struct success_flag {};

    Response(success_flag) {}

    boost::optional<error_type> _data;
  };

} // namespace rpc
} // namespace carla
