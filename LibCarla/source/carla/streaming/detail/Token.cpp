// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Token.h"

#include "carla/Exception.h"

#include <cstring>
#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  void token_type::set_address(const boost::asio::ip::address &addr) {
    if (addr.is_v4()) {
      _token.address_type = token_data::address::ip_v4;
      _token.address.v4 = addr.to_v4().to_bytes();
    } else if (addr.is_v6()) {
      _token.address_type = token_data::address::ip_v6;
      _token.address.v6 = addr.to_v6().to_bytes();
    } else {
      throw_exception(std::invalid_argument("invalid ip address!"));
    }
  }

  token_type::token_type(const Token &rhs) {
    std::memcpy(&_token, &rhs.data[0u], sizeof(_token));
  }

  token_type::operator Token() const {
    Token token;
    std::memcpy(&token.data[0u], &_token, token.data.size());
    return token;
  }

  boost::asio::ip::address token_type::get_address() const {
    if (_token.address_type == token_data::address::ip_v4) {
      return boost::asio::ip::address_v4(_token.address.v4);
    }
    return boost::asio::ip::address_v6(_token.address.v6);
  }

} // namespace detail
} // namespace streaming
} // namespace carla
