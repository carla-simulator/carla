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

class StudioAppContext {
public:
  StudioAppContext();

  void SetWorkspaceRoot(const QString &workspace_root);
  const QString &GetWorkspaceRoot() const;
  bool HasWorkspaceRoot() const;

  void SetUiOnlyMode(bool ui_only_mode);
  bool IsUiOnlyMode() const;

private:
  QString _workspace_root;
  bool _ui_only_mode{false};
};

}
}
}
