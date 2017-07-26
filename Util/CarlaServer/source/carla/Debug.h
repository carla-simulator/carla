// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
