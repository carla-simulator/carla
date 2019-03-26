// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#include "Signal.h"

namespace carla {
namespace road {
namespace signal {

void Signal::AddValidity(general::Validity &&validity) {
  _validities.push_back(std::move(validity));
}

void Signal::AddDependency(SignalDependency &&dependency) {
  _dependencies.push_back(std::move(dependency));
}

}
}
}