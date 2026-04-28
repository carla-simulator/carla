// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

namespace carla_studio::setup_wizard {

struct PrereqResult {
  QString tool;
  QString detail;
  bool    ok = false;
};

PrereqResult probeGit();
PrereqResult probeGitLfs();
PrereqResult probeNinja();
PrereqResult probeCmake();
QString      runShell(const QString &cmd, int timeoutMs = 1500);
bool         pathHasUnrealEditor(const QString &uePath);

}  // namespace carla_studio::setup_wizard
