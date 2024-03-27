// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"
#include <boost/assert/source_location.hpp>

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS

namespace boost {

void throw_exception(const std::exception &e) {
  carla::throw_exception(e);
}

void throw_exception(const std::exception &e, boost::source_location const &loc) {
  (void)loc;
  throw_exception(e);
}

}  // namespace boost

#endif  // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- Workaround for Boost.Asio bundled with rpclib ----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS

#include <exception>
#include <system_error>
#include <typeinfo>

namespace clmdep_asio {
namespace detail {

template <typename Exception>
void throw_exception(const Exception &e) {
  carla::throw_exception(e);
}

template void throw_exception<std::bad_cast>(const std::bad_cast &e);
template void throw_exception<std::exception>(const std::exception &e);
template void throw_exception<std::system_error>(const std::system_error &e);

}  // namespace detail
}  // namespace clmdep_asio

namespace asio {
namespace detail {

template <typename Exception>
void throw_exception(const Exception &e) {
  carla::throw_exception(e);
}

template void throw_exception<std::bad_cast>(const std::bad_cast &e);
template void throw_exception<std::exception>(const std::exception &e);
template void throw_exception<std::system_error>(const std::system_error &e);

}  // namespace detail
}  // namespace asio

#endif  // ASIO_NO_EXCEPTIONS

#ifndef LIBCARLA_NO_EXCEPTIONS

namespace carla {
template
#ifndef __clang__
    // clang doesn't support C++11 attributes in template explicit instantiation, since attributes in each case cannot
    // be changed here MSVC requires it (might be a bit too conservative and requires the attributes also in explicit
    // instanciation)
    [[noreturn]]
#endif
    void
    throw_exception<std::exception>(const std::exception &);
}  // namespace carla

#endif
