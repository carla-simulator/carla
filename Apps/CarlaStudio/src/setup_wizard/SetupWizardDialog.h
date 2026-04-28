// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QString>
#include <functional>

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QProcess;
class QPushButton;
class QTabWidget;

namespace carla_studio::setup_wizard {

class SetupWizardDialog : public QDialog {
  Q_OBJECT
 public:
  using SetCarlaRoot = std::function<void(const QString &)>;

  explicit SetupWizardDialog(SetCarlaRoot setCarlaRoot,
                             int initialTab = 1,
                             QWidget *parent = nullptr);

 private:
  QWidget *buildBinaryTab();
  QWidget *buildSourceTab();
  QWidget *buildMapsTab();

  void refreshPrereqs();
  void browseClonePath();
  void browseUePath();
  void onCloneRepo();
  void onCloneContent();
  void onBuild();
  void onSetAsRoot();
  void onCopyLog();
  void appendLog(const QString &line);

  SetCarlaRoot mSetCarlaRoot;

  QTabWidget    *mTabs            = nullptr;

  QLineEdit     *mClonePath       = nullptr;
  QComboBox     *mBranchCombo     = nullptr;
  QLabel        *mGitDot          = nullptr;
  QLabel        *mGitDetail       = nullptr;
  QLabel        *mLfsDot          = nullptr;
  QLabel        *mLfsDetail       = nullptr;
  QLabel        *mNinjaDot        = nullptr;
  QLabel        *mNinjaDetail     = nullptr;
  QLabel        *mCmakeDot        = nullptr;
  QLabel        *mCmakeDetail     = nullptr;
  QLineEdit     *mUePath          = nullptr;
  QLabel        *mUeReadyLabel    = nullptr;
  QPushButton   *mInstallPrereqBtn = nullptr;
  QPushButton   *mReCloneBtn      = nullptr;
  QPushButton   *mCloneContentBtn = nullptr;
  QPushButton   *mBuildBtn        = nullptr;
  QPushButton   *mSetAsRootBtn    = nullptr;
  QPushButton   *mCopyLogBtn      = nullptr;
  QPlainTextEdit *mLog            = nullptr;

  QListWidget   *mMapsList        = nullptr;
  QPushButton   *mMapsRefreshBtn  = nullptr;
  QPushButton   *mMapsInstallBtn  = nullptr;
  QLineEdit     *mMapsTargetRoot  = nullptr;
  QPlainTextEdit *mMapsLog        = nullptr;
};

}  // namespace carla_studio::setup_wizard
