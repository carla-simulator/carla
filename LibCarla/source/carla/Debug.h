// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifndef NDEBUG
#  include <cassert>
#endif // NDEBUG

#ifdef NDEBUG
#  define DEBUG_ONLY(code)
#else
#  define DEBUG_ONLY(code) code
#endif // NDEBUG

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
