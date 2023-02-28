// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// Macro utilities for assertions and debug-only code.
///
/// Defines three levels of assertions: debug, development, and release. By
/// default, if NDEBUG is defined the level is set to development, otherwise is
/// set to debug.
///
/// The following macros are defined here:
///
///   * DEBUG_ONLY(code) - Code is evaluated only if assertion level is set to
///     debug.
///
///   * DEVELOPMENT_ONLY(code) - Code is evaluated only if assertion level is
///     set to development or higher.
///
///   * DEBUG_ASSERT(predicate) - Effectively calls assert(predicate).
///
///   * DEVELOPMENT_ASSERT(predicate) - Throws an exception if assertion is
///     set to development or higher, and predicate evaluates to false.
///
///   * RELEASE_ASSERT(predicate) - Throws an exception if predicate evaluates
///     to false.

#pragma once

#include "carla/Exception.h"

#include <stdexcept>

#define LIBCARLA_ASSERT_LEVEL_DEBUG        30
#define LIBCARLA_ASSERT_LEVEL_DEVELOPMENT  20
#define LIBCARLA_ASSERT_LEVEL_RELEASE      10

#ifndef LIBCARLA_ASSERT_LEVEL
#  ifdef NDEBUG
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEVELOPMENT
#  else
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEBUG
#  endif // NDEBUG
#endif // LIBCARLA_ASSERT_LEVEL

#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEBUG)
#  ifdef NDEBUG
#    error Cannot have debug asserts with NDEBUG enabled.
#  endif
#  include <cassert>
#endif // NDEBUG

#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEBUG)
#  define DEBUG_ONLY(code) code
#else
#  define DEBUG_ONLY(code)
#endif

#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEVELOPMENT)
#  define DEVELOPMENT_ONLY(code) code
#else
#  define DEVELOPMENT_ONLY(code)
#endif

#define DEBUG_ASSERT(predicate) DEBUG_ONLY(assert(predicate));

#define DEBUG_ERROR DEBUG_ASSERT(false);

#ifdef LIBCARLA_WITH_GTEST
#  include <gtest/gtest.h>

#  define DEBUG_ASSERT_EQ(lhs, rhs) DEBUG_ONLY(EXPECT_EQ(lhs, rhs));DEBUG_ASSERT(lhs == rhs);
#  define DEBUG_ASSERT_NE(lhs, rhs) DEBUG_ONLY(EXPECT_NE(lhs, rhs));DEBUG_ASSERT(lhs != rhs);
#else
#  define DEBUG_ASSERT_EQ(lhs, rhs) DEBUG_ASSERT((lhs) == (rhs))
#  define DEBUG_ASSERT_NE(lhs, rhs) DEBUG_ASSERT((lhs) != (rhs))
#endif // LIBCARLA_WITH_GTEST

#define LIBCARLA_ASSERT_THROW__(pred, msg)  if (!(pred)) { ::carla::throw_exception(std::runtime_error(msg)); }

#define DEVELOPMENT_ASSERT(pred) DEVELOPMENT_ONLY(LIBCARLA_ASSERT_THROW__(pred, #pred))

#define RELEASE_ASSERT(pred) LIBCARLA_ASSERT_THROW__(pred, #pred)
