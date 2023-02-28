// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#if defined(_MSC_VER)
#  define LIBCARLA_FORCEINLINE __forceinline
#  define LIBCARLA_NOINLINE __declspec(noinline)
#elif defined(__clang__) || defined(__GNUC__)
#  if defined(NDEBUG)
#    define LIBCARLA_FORCEINLINE inline __attribute__((always_inline))
#  else
#    define LIBCARLA_FORCEINLINE inline
#  endif // NDEBUG
#  define LIBCARLA_NOINLINE __attribute__((noinline))
#else
#  warning Compiler not supported.
#  define LIBCARLA_NOINLINE
#endif
