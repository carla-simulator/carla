// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#include "Frame.h"
#include "Model.h"

namespace twinmodel {

  /// Serialise the model as an OpenDRIVE 1.6 document. The C++ port of
  /// twinmodel/export/xodr.py's export_xodr.
  std::string ExportXodr(const TwinModelData &model, const LocalFrame &frame);

} // namespace twinmodel
