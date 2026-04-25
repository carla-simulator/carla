// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QChar>
#include <QString>

namespace carla {
namespace studio {
namespace core {

inline QString defaultPlayerName(int idx) {
  if (idx == 0) return QStringLiteral("EGO");
  if (idx >= 1 && idx <= 10) return QString("POV.%1").arg(idx, 2, 10, QChar('0'));
  if (idx >= 11 && idx <= 16) return QString("V2X.%1").arg(idx - 10, 2, 10, QChar('0'));
  return QString();
}

}
}
}
