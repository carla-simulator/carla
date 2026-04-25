// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "core/StudioAppContext.h"

namespace carla {
namespace studio {
namespace core {

StudioAppContext::StudioAppContext() = default;

void StudioAppContext::SetWorkspaceRoot(const QString &workspace_root) {
  _workspace_root = workspace_root;
}

const QString &StudioAppContext::GetWorkspaceRoot() const {
  return _workspace_root;
}

bool StudioAppContext::HasWorkspaceRoot() const {
  return !_workspace_root.isEmpty();
}

void StudioAppContext::SetUiOnlyMode(bool ui_only_mode) {
  _ui_only_mode = ui_only_mode;
}

bool StudioAppContext::IsUiOnlyMode() const {
  return _ui_only_mode;
}

}
}
}
