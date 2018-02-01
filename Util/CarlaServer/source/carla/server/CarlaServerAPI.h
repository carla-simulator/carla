// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#if defined(_MSC_VER)
#  define CARLA_SERVER_API __declspec(dllexport) extern
#elif defined(__GNUC__) || defined(__clang__)
#  define CARLA_SERVER_API __attribute__((visibility("default"))) extern
#else
#  error Compiler not supported!
#endif

#include <carla/carla_server.h>
