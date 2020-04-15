// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace client {

  class Timestamp {
  public:

    Timestamp() = default;

    Timestamp(
        std::size_t in_frame,
        double in_elapsed_seconds,
        double in_delta_seconds,
        double in_platform_timestamp)
      : frame(in_frame),
        elapsed_seconds(in_elapsed_seconds),
        delta_seconds(in_delta_seconds),
        platform_timestamp(in_platform_timestamp) {}

    /// Number of frames elapsed since the simulator was launched.
    std::size_t frame = 0u;

    /// Simulated seconds elapsed since the beginning of the current episode.
    double elapsed_seconds = 0.0;

    /// Simulated seconds elapsed since previous frame.
    double delta_seconds = 0.0;

    /// Time-stamp of the frame at which this measurement was taken, in seconds
    /// as given by the OS.
    double platform_timestamp = 0.0;

    bool operator==(const Timestamp &rhs) const {
      return frame == rhs.frame;
    }

    bool operator!=(const Timestamp &rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace client
} // namespace carla


namespace std {
/**
 * \brief standard ostream operator
 *
 * \param[in/out] out The output stream to write to
 * \param[in] timestamp the timestamp to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out, const ::carla::client::Timestamp &timestamp) {
  out << "Timestamp(frame=" << std::to_string(timestamp.frame)
      << ",elapsed_seconds=" << std::to_string(timestamp.elapsed_seconds)
      << ",delta_seconds=" << std::to_string(timestamp.delta_seconds)
      << ",platform_timestamp=" << std::to_string(timestamp.platform_timestamp) << ')';
  return out;
}
} // namespace std
