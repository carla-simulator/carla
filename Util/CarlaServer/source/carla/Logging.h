// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#define CARLA_SERVER_LOG_LEVEL_DEBUG        10
#define CARLA_SERVER_LOG_LEVEL_INFO         20
#define CARLA_SERVER_LOG_LEVEL_WARNING      30
#define CARLA_SERVER_LOG_LEVEL_ERROR        40
#define CARLA_SERVER_LOG_LEVEL_CRITICAL     50
#define CARLA_SERVER_LOG_LEVEL_NONE        100

#ifndef CARLA_SERVER_LOG_LEVEL
#  ifdef NDEBUG
#    define CARLA_SERVER_LOG_LEVEL CARLA_SERVER_LOG_LEVEL_WARNING
#  else
#    define CARLA_SERVER_LOG_LEVEL CARLA_SERVER_LOG_LEVEL_INFO
#  endif // NDEBUG
#endif // CARLA_SERVER_LOG_LEVEL

// The following log functions are available, they are only active if
// CARLA_SERVER_LOG_LEVEL is greater equal the function's log level.
//
//  * log_debug
//  * log_info
//  * log_error
//  * log_critical
//
// And macros
//
//  * LOG_DEBUG_ONLY(/* code here */)
//  * LOG_INFO_ONLY(/* code here */)

// =============================================================================
// -- Implementation of log functions ------------------------------------------
// =============================================================================

#include <iostream>

namespace carla {

namespace logging {

  // https://stackoverflow.com/a/27375675
  template <typename Arg, typename ... Args>
  static void print(std::ostream &out, Arg &&arg, Args &&... args) {
    out << std::boolalpha << std::forward<Arg>(arg);
    using expander = int[];
    (void)expander{0, (void(out << ' ' << std::forward<Args>(args)),0)...};
  }

} // namespace logging

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_DEBUG

  template <typename ... Args>
  static inline void log_debug(Args &&... args) {
    logging::print(std::cout, "DEBUG:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_debug(Args &&...) {}

#endif

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_INFO

  template <typename ... Args>
  static inline void log_info(Args &&... args) {
    logging::print(std::cout, "INFO: ", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_info(Args &&...) {}

#endif

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_WARNING

  template <typename ... Args>
  static inline void log_warning(Args &&... args) {
    logging::print(std::cerr, "WARNING:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_warning(Args &&...) {}

#endif

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_ERROR

  template <typename ... Args>
  static inline void log_error(Args &&... args) {
    logging::print(std::cerr, "ERROR:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_error(Args &&...) {}

#endif

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_CRITICAL

  template <typename ... Args>
  static inline void log_critical(Args &&... args) {
    logging::print(std::cerr, "CRITICAL:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_critical(Args &&...) {}

#endif

} // namespace carla

// =============================================================================
// -- Implementation of macros -------------------------------------------------
// =============================================================================

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_DEBUG
#  define LOG_DEBUG_ONLY(code) code
#else
#  define LOG_DEBUG_ONLY(code)
#endif

#if CARLA_SERVER_LOG_LEVEL <= CARLA_SERVER_LOG_LEVEL_INFO
#  define LOG_INFO_ONLY(code) code
#else
#  define LOG_INFO_ONLY(code)
#endif
