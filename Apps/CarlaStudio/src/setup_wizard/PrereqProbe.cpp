// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "setup_wizard/PrereqProbe.h"

#include <QFileInfo>
#include <QProcess>

namespace carla_studio::setup_wizard {

QString runShell(const QString &cmd, int timeoutMs) {
  QProcess p;
  p.start("/bin/bash", QStringList() << "-lc" << cmd);
  if (!p.waitForFinished(timeoutMs)) {
    p.kill();
    p.waitForFinished(500);
    return QString();
  }
  return QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
}

PrereqResult probeGit() {
  PrereqResult r; r.tool = "git";
  const QString out = runShell("git --version 2>/dev/null");
  r.ok = !out.isEmpty();
  r.detail = r.ok ? out : QStringLiteral("not found — sudo apt install git");
  return r;
}

PrereqResult probeGitLfs() {
  PrereqResult r; r.tool = "git-lfs";
  const QString out = runShell("git lfs --version 2>/dev/null");
  r.ok = !out.isEmpty();
  r.detail = r.ok ? out : QStringLiteral("not found — sudo apt install git-lfs");
  return r;
}

PrereqResult probeNinja() {
  PrereqResult r; r.tool = "ninja";
  const QString out = runShell("ninja --version 2>/dev/null");
  r.ok = !out.isEmpty();
  r.detail = r.ok ? QStringLiteral("ninja %1").arg(out)
                  : QStringLiteral("not found — sudo apt install ninja-build");
  return r;
}

PrereqResult probeCmake() {
  PrereqResult r; r.tool = "cmake";
  const QString out = runShell("cmake --version 2>/dev/null | head -1");
  r.ok = !out.isEmpty();
  r.detail = r.ok ? out : QStringLiteral("not found — sudo apt install cmake");
  return r;
}

bool pathHasUnrealEditor(const QString &uePath) {
  if (uePath.isEmpty()) return false;
  return QFileInfo(uePath + "/Engine/Binaries/Linux/UnrealEditor").isExecutable();
}

}  // namespace carla_studio::setup_wizard
