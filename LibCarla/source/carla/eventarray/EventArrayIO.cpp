// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/eventarray/EventArrayIO.h"

#include <iomanip>

namespace carla {
namespace eventarray {

  void EventArrayIO::WriteHeader(std::ostream &out, size_t number_of_events) {
    out << "W: H: \n"
           "Num events: " << std::to_string(number_of_events) << "\n";
    out << std::fixed << std::setprecision(4u);
  }

} // namespace eventarray
} // namespace carla
