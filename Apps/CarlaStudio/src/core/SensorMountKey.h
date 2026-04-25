// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QString>

namespace carla {
namespace studio {
namespace core {

inline QString sensorMountKey(const QString &name, int instance) {
  if (instance <= 1) return name;
  return QString("%1#%2").arg(name).arg(instance);
}

}
}
}
