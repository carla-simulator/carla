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
#include <memory>

class QDoubleSpinBox;
class QJsonObject;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;

namespace carla_studio::vehicle_import {

struct WheelSpinRow {
  QDoubleSpinBox *x = nullptr;
  QDoubleSpinBox *y = nullptr;
  QDoubleSpinBox *z = nullptr;
  QDoubleSpinBox *r = nullptr;
};

class VehicleImportPage : public QWidget {
  Q_OBJECT
 public:
  using EditorBinaryResolver = std::function<QString()>;
  using UprojectResolver     = std::function<QString()>;

  explicit VehicleImportPage(EditorBinaryResolver findEditor,
                             UprojectResolver     findUproject,
                             QWidget *parent = nullptr);

 private:
  void onBrowse();
  void onImport();
  void onDrop();
  void applyDisabledStateStyling();
  QJsonObject buildImportSpec(const QString &meshPathToSend) const;

  EditorBinaryResolver mFindEditor;
  UprojectResolver     mFindUproject;

  QLineEdit       *mMeshPathEdit       = nullptr;
  QPushButton     *mBrowseBtn          = nullptr;
  QLabel          *mAabbLabel          = nullptr;
  WheelSpinRow     mRowFL, mRowFR, mRowRL, mRowRR;
  QLineEdit       *mVehicleNameEdit    = nullptr;
  QLineEdit       *mContentPathEdit    = nullptr;
  QLineEdit       *mBaseVehicleBPEdit  = nullptr;
  QDoubleSpinBox  *mMassSpin           = nullptr;
  QDoubleSpinBox  *mSteerSpin          = nullptr;
  QDoubleSpinBox  *mSuspRaiseSpin      = nullptr;
  QDoubleSpinBox  *mSuspDropSpin       = nullptr;
  QDoubleSpinBox  *mSuspDampSpin       = nullptr;
  QDoubleSpinBox  *mBrakeSpin          = nullptr;
  QLabel          *mUeStatusLabel      = nullptr;
  QPushButton     *mImportBtn          = nullptr;
  QPushButton     *mDropBtn            = nullptr;
  QProgressBar    *mProgress           = nullptr;
  QLabel          *mAssetLabel         = nullptr;
  QPlainTextEdit  *mLog                = nullptr;

  std::shared_ptr<float>   mScaleToCm   = std::make_shared<float>(1.0f);
  std::shared_ptr<int>     mUpAxis      = std::make_shared<int>(2);
  std::shared_ptr<int>     mForwardAxis = std::make_shared<int>(0);
  std::shared_ptr<bool>    mAabbValid   = std::make_shared<bool>(false);
  std::shared_ptr<QString> mLastAsset   = std::make_shared<QString>();
};

}  // namespace carla_studio::vehicle_import
