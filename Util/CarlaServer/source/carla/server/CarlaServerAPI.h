// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#pragma once

#if defined(_MSC_VER)
#  define CARLA_SERVER_API __declspec(dllexport) extern
#elif defined(__GNUC__) || defined(__clang__)
#  define CARLA_SERVER_API __attribute__((visibility("default"))) extern
#else
#  error Compiler not supported!
#endif

#include <carla/carla_server.h>
