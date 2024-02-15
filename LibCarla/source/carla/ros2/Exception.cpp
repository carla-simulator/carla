// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"
#include <asio.hpp>

#ifdef ASIO_NO_EXCEPTIONS

namespace asio {
namespace detail {

  template <typename Exception>
  void throw_exception(const Exception& e) {
    carla::throw_exception(e);
  }

  template void throw_exception<std::length_error>(const std::length_error &);
  template void throw_exception<asio::service_already_exists>(const asio::service_already_exists &);
  template void throw_exception<asio::invalid_service_owner>(const asio::invalid_service_owner &);
  template void throw_exception<asio::execution::bad_executor>(const asio::execution::bad_executor &);
  template void throw_exception<asio::ip::bad_address_cast>(const asio::ip::bad_address_cast &);
  template void throw_exception<std::out_of_range>(const std::out_of_range &);

} // namespace detail
} // namespace asio

#endif // ASIO_NO_EXCEPTIONS
