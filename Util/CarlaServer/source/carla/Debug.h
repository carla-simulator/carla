// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _DEBUG
#include <cassert>
#endif // _DEBUG

#ifdef _DEBUG
#  define DEBUG_ONLY(code) code
#else
#  define DEBUG_ONLY(code)
#endif // _DEBUG

#define DEBUG_ASSERT(predicate) DEBUG_ONLY(assert(predicate))
