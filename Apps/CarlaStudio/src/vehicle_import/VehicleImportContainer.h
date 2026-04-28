// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>
#include <functional>

class QTabWidget;

namespace carla_studio::vehicle_import {

class VehicleImportPage;
class PrebuiltPackagePage;

class VehicleImportContainer : public QWidget {
  Q_OBJECT
 public:
  using EditorBinaryResolver = std::function<QString()>;
  using UprojectResolver     = std::function<QString()>;
  using CarlaRootResolver    = std::function<QString()>;

  explicit VehicleImportContainer(EditorBinaryResolver findEditor,
                                  UprojectResolver     findUproject,
                                  CarlaRootResolver    findCarlaRoot,
                                  QWidget *parent = nullptr);

  PrebuiltPackagePage *prebuiltPage() const { return mPrebuilt; }

 private:
  QTabWidget          *mTabs     = nullptr;
  VehicleImportPage   *mFromMesh = nullptr;
  PrebuiltPackagePage *mPrebuilt = nullptr;
};

}  // namespace carla_studio::vehicle_import
