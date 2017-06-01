// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#define LOG_LEVEL_DEBUG      10
#define LOG_LEVEL_INFO       20
#define LOG_LEVEL_ERROR      30
#define LOG_LEVEL_CRITICAL   40
#define LOG_LEVEL_NONE      100

// Choose here the log level.
#define LOG_LEVEL LOG_LEVEL_ERROR


#include <iostream>

namespace carla {

namespace detail {

  // https://stackoverflow.com/a/27375675
  template <typename Arg, typename ... Args>
  void print_args(std::ostream &out, Arg &&arg, Args &&... args) {
    out << std::forward<Arg>(arg);
    using expander = int[];
    (void)expander{0, (void(out << ' ' << std::forward<Args>(args)),0)...};
  }

} // namespace detail

#if LOG_LEVEL <= LOG_LEVEL_DEBUG

  template <typename ... Args>
  static inline void log_debug(Args &&... args) {
    detail::print_args(std::cout, "DEBUG:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_debug(Args &&...) {}

#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO

  template <typename ... Args>
  static inline void log_info(Args &&... args) {
    detail::print_args(std::cout, "INFO:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_info(Args &&...) {}

#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR

  template <typename ... Args>
  static inline void log_error(Args &&... args) {
    detail::print_args(std::cerr, "ERROR:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_error(Args &&...) {}

#endif

#if LOG_LEVEL <= LOG_LEVEL_CRITICAL

  template <typename ... Args>
  static inline void log_critical(Args &&... args) {
    detail::print_args(std::cerr, "CRITICAL:", std::forward<Args>(args)..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_critical(Args &&...) {}

#endif

} // namespace carla
