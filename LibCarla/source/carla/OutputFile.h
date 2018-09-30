// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>
#include <string>

namespace carla {

  /// Convenient wrapper around ofstream.
  ///
  ///   * Ensures all the parent directories are created if missing.
  ///   * A default extension is appended if the path is missing the extension.
  ///
  /// @warning Using this file requires linking against boost_filesystem.
  class OutputFile {
  public:

    explicit OutputFile(
        const std::string &path,
        const std::string &default_extension = "",
        std::ios_base::openmode mode = std::ios_base::out);

    explicit OutputFile(
        const std::string &path,
        std::ios_base::openmode mode = std::ios_base::out)
      : OutputFile(path, "", mode) {}

    template <typename T>
    OutputFile &operator<<(const T &value) {
      _ofstream << value;
      return *this;
    }

    typedef std::ostream& (*ostream_manipulator)(std::ostream&);
    OutputFile &operator<<(ostream_manipulator pf) {
      return operator<< <ostream_manipulator> (pf);
    }

  private:

    std::ofstream _ofstream;
  };

} // namespace carla
