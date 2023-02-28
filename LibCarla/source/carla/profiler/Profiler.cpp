// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_ENABLE_PROFILER
#  define LIBCARLA_ENABLE_PROFILER
#endif // LIBCARLA_ENABLE_PROFILER

#include "carla/Logging.h"
#include "carla/Version.h"
#include "carla/profiler/Profiler.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace carla {
namespace profiler {
namespace detail {

  template <typename Arg, typename ... Args>
  static void write_csv_to_stream(std::ostream &out, Arg &&arg, Args &&... args) {
    out << std::boolalpha
        << std::left << std::setw(44)
        << std::forward<Arg>(arg)
        << std::right
        << std::fixed << std::setprecision(2);
    using expander = int[];
    (void)expander{0, (void(out << ", " << std::setw(10) << std::forward<Args>(args)),0)...};
  }

  class StaticProfiler {
  public:

    StaticProfiler(std::string filename)
      : _filename(std::move(filename)) {
      logging::log("PROFILER: writing profiling data to", _filename);
      std::string header = "# LibCarla Profiler ";
      header += carla::version();
#ifdef NDEBUG
      header += " (release)";
#else
      header += " (debug)";
#endif // NDEBUG
      write_to_file(std::ios_base::out, header);
      write_line("# context", "average", "maximum", "minimum", "units", "times");
    }

    template <typename ... Args>
    void write_line(Args &&... args) {
      write_to_file(std::ios_base::app|std::ios_base::out, std::forward<Args>(args)...);
    }

  private:

    template <typename ... Args>
    void write_to_file(std::ios_base::openmode mode, Args &&... args) {
      if (!_filename.empty()) {
        static std::mutex MUTEX;
        std::lock_guard<std::mutex> guard(MUTEX);
        std::ofstream file(_filename, mode);
        write_csv_to_stream(file, std::forward<Args>(args)...);
        file << std::endl;
      }
    }

    const std::string _filename;
  };

  ProfilerData::~ProfilerData() {
    static StaticProfiler PROFILER{"profiler.csv"};
    if (_count > 0u) {
      if (_print_fps) {
        PROFILER.write_line(_name, fps(average()), fps(minimum()), fps(maximum()), "FPS", _count);
      } else {
        PROFILER.write_line(_name, average(), maximum(), minimum(), "ms", _count);
      }
    } else {
      log_error("profiler", _name, " was never run!");
    }
  }

} // namespace detail
} // namespace profiler
} // namespace carla
