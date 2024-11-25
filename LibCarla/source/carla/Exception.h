// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <exception>



// =============================================================================
// -- Define carla::throw_exception --------------------------------------------
// =============================================================================

#ifdef LIBCARLA_NO_EXCEPTIONS

namespace carla
{
  /// User defined function, similar to Boost throw_exception.
  ///
  /// @important Boost exceptions are also routed to this function.
  ///
  /// When compiled with LIBCARLA_NO_EXCEPTIONS, this function is left undefined
  /// in LibCarla, and the modules using LibCarla are expected to supply an
  /// appropriate definition. Callers of throw_exception are allowed to assume
  /// that the function never returns; therefore, if the user-defined
  /// throw_exception returns, the behavior is undefined.
  [[noreturn]]
  void throw_exception(const std::exception &e);

} // namespace carla

#else

namespace carla
{
  [[noreturn]]
  inline static void throw_exception(const std::exception &e)
  {
    throw e;
  }
} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS



// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/assert/source_location.hpp>

namespace boost
{
  BOOST_NORETURN
  inline void throw_exception(const std::exception &e)
  {
    carla::throw_exception(e);
  }

  BOOST_NORETURN
  inline void throw_exception(const std::exception &e, boost::source_location const& loc)
  {
    throw_exception(e);
  }
} // namespace boost
#endif // BOOST_NO_EXCEPTIONS



// =============================================================================
// -- Workaround for Boost.Asio bundled with rpclib ----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS
namespace clmdep_asio
{
  namespace detail
  {
    [[noreturn]]
    inline void throw_exception(const std::exception& e)
    {
      carla::throw_exception(e);
    }
  } // namespace detail
} // namespace clmdep_asio
#endif // ASIO_NO_EXCEPTIONS
