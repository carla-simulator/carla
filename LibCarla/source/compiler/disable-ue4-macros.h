// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma push_macro("check")
#undef check

#define LIBCARLA_INCLUDED_FROM_UE4

#ifndef BOOST_ERROR_CODE_HEADER_ONLY
#  define BOOST_ERROR_CODE_HEADER_ONLY
#endif // BOOST_ERROR_CODE_HEADER_ONLY

#ifndef BOOST_COROUTINES_NO_DEPRECATION_WARNING
#  define BOOST_COROUTINES_NO_DEPRECATION_WARNING
#endif // BOOST_COROUTINES_NO_DEPRECATION_WARNING

#ifndef BOOST_NO_EXCEPTIONS
#  define BOOST_NO_EXCEPTIONS
#endif // BOOST_NO_EXCEPTIONS

namespace boost {

  static inline void throw_exception(const std::exception &) {}

} // namespace boost
