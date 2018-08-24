// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER
#  define LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER
#else
#  error disable-ue4-macros.h should only be included once!
#endif // LIBCARLA_INCLUDED_DISABLE_UE4_MACROS_HEADER

#pragma push_macro("check")
#undef check

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"

#define LIBCARLA_INCLUDED_FROM_UE4

#ifndef BOOST_ERROR_CODE_HEADER_ONLY
#  define BOOST_ERROR_CODE_HEADER_ONLY
#endif // BOOST_ERROR_CODE_HEADER_ONLY

#ifndef BOOST_NO_EXCEPTIONS
#  define BOOST_NO_EXCEPTIONS
#endif // BOOST_NO_EXCEPTIONS

// Suppress clang warning.
#if defined(__clang__)
#  ifndef __cpp_coroutines
#    define __cpp_coroutines 0
#  endif // __cpp_coroutines
#endif // defined(__clang__)

namespace boost {

  static inline void throw_exception(const std::exception &e) {
    UE_LOG(LogCarla, Fatal, TEXT("Exception thrown on Boost libraries: %s"), UTF8_TO_TCHAR(e.what()));
  }

} // namespace boost
