// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/VehicleImportContainer.h"

#include "vehicle_import/PrebuiltPackagePage.h"
#include "vehicle_import/VehicleImportPage.h"

#include <QTabWidget>
#include <QVBoxLayout>

namespace carla_studio::vehicle_import {

VehicleImportContainer::VehicleImportContainer(EditorBinaryResolver findEditor,
                                               UprojectResolver     findUproject,
                                               CarlaRootResolver    findCarlaRoot,
                                               QWidget *parent)
    : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(6, 6, 6, 6);
  layout->setSpacing(0);

  mTabs = new QTabWidget(this);
  mTabs->setDocumentMode(true);
  mFromMesh = new VehicleImportPage(std::move(findEditor),
                                    std::move(findUproject), this);
  mPrebuilt = new PrebuiltPackagePage(std::move(findCarlaRoot), this);
  mTabs->addTab(mFromMesh, "From 3D Model");
  mTabs->addTab(mPrebuilt, "Pre-built Package");
  layout->addWidget(mTabs);
}

}  // namespace carla_studio::vehicle_import
