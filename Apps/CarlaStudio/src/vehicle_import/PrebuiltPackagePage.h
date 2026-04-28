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

class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

namespace carla_studio::vehicle_import {

class PrebuiltPackagePage : public QWidget {
  Q_OBJECT
 public:
  using CarlaRootResolver = std::function<QString()>;

  explicit PrebuiltPackagePage(CarlaRootResolver findCarlaRoot,
                               QWidget *parent = nullptr);

 public slots:
  void refreshDestination();

 private:
  void onBrowse();
  void onInstall();
  QString resolveVehiclesDir() const;

  CarlaRootResolver mFindCarlaRoot;

  QLineEdit      *mFolderEdit  = nullptr;
  QPushButton    *mBrowseBtn   = nullptr;
  QListWidget    *mFileList    = nullptr;
  QLabel         *mStatusLabel = nullptr;
  QLineEdit      *mNameEdit    = nullptr;
  QLabel         *mDstLabel    = nullptr;
  QPushButton    *mInstallBtn  = nullptr;
  QPlainTextEdit *mLog         = nullptr;
};

}  // namespace carla_studio::vehicle_import
