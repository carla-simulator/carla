// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "setup_wizard/SetupWizardDialog.h"

#include "setup_wizard/PrereqProbe.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTime>
#include <QVBoxLayout>

namespace carla_studio::setup_wizard {

namespace {

QString stamp() { return QTime::currentTime().toString("hh:mm:ss"); }

QString defaultClonePath() {
  const QString envSrc  = QString::fromLocal8Bit(qgetenv("CARLA_SRC_ROOT")).trimmed();
  if (!envSrc.isEmpty()) return envSrc;
  const QString envRoot = QString::fromLocal8Bit(qgetenv("CARLA_ROOT")).trimmed();
  if (!envRoot.isEmpty()) return envRoot;
  return QDir::homePath() + "/carla/CARLA_src";
}

QString defaultUePath() {
  const QString env = QString::fromLocal8Bit(qgetenv("CARLA_UNREAL_ENGINE_PATH")).trimmed();
  if (!env.isEmpty()) return env;
  return QDir::homePath() + "/carla/Unreal5";
}

constexpr const char *kReleaseTags[] = {
  "v0.9.16", "v0.9.15", "v0.9.14", "v0.9.13", "v0.9.12", "v0.9.11",
  "v0.9.10.1", "v0.9.10", "v0.9.9", "v0.9.8", "v0.9.7", "v0.9.6",
  "v0.9.5", "v0.9.4",
};

constexpr const char *kCommunityMaps[][2] = {
  { "Mcity Digital Twin",          "https://mcity.umich.edu/" },
  { "Apollo HD Maps (Town01–05)",  "https://github.com/ApolloAuto/apollo" },
};

void styleDot(QLabel *dot, bool ok) {
  dot->setText(ok ? "✓" : "·");
  dot->setStyleSheet(ok ? "color:#33AA55; font-weight:600;"
                        : "color:#888;");
}

}  // namespace

SetupWizardDialog::SetupWizardDialog(SetCarlaRoot setCarlaRoot,
                                     int initialTab,
                                     QWidget *parent)
    : QDialog(parent),
      mSetCarlaRoot(std::move(setCarlaRoot)) {
  setWindowTitle("CARLA · Setup Wizard — CARLA Studio");
  resize(720, 640);

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);
  root->setSpacing(8);

  mTabs = new QTabWidget(this);
  mTabs->addTab(buildBinaryTab(), "Binary Package");
  mTabs->addTab(buildSourceTab(), "Build from Source");
  mTabs->addTab(buildMapsTab(),   "Maps");
  root->addWidget(mTabs, 1);

  mTabs->setCurrentIndex(std::clamp(initialTab, 0, 2));

  refreshPrereqs();
}

QWidget *SetupWizardDialog::buildBinaryTab() {
  auto *w = new QWidget();
  auto *l = new QVBoxLayout(w);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(8);
  auto *intro = new QLabel(
    "Download a pre-built CARLA release tarball and extract it to a target "
    "directory. Studio sets <code>CARLA_ROOT</code> automatically.");
  intro->setWordWrap(true);
  l->addWidget(intro);

  auto *useExisting = new QLabel(
    "<small>For now, use <b>Tools → Install / Update CARLA…</b> to manage "
    "binary releases. The full release picker will land here in a follow-up.</small>");
  useExisting->setWordWrap(true);
  l->addWidget(useExisting);
  l->addStretch();
  return w;
}

QWidget *SetupWizardDialog::buildSourceTab() {
  auto *w = new QWidget();
  auto *l = new QVBoxLayout(w);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(8);

  auto *cloneRow = new QHBoxLayout();
  cloneRow->addWidget(new QLabel("Clone to:"));
  mClonePath = new QLineEdit(defaultClonePath());
  cloneRow->addWidget(mClonePath, 1);
  auto *browseClone = new QPushButton("Browse…");
  cloneRow->addWidget(browseClone);
  l->addLayout(cloneRow);
  connect(browseClone, &QPushButton::clicked, this, &SetupWizardDialog::browseClonePath);

  auto *introBox = new QLabel(
    "<b>Build CARLA from Source</b> — clones the CARLA GitHub repo and "
    "compiles the Unreal Engine plugin. Required for Vehicle Model Import.");
  introBox->setWordWrap(true);
  introBox->setStyleSheet(
    "background:#FFF8E1; color:#5C4400; padding:6px; border:1px solid #E5C070;");
  l->addWidget(introBox);

  auto *fullBuild = new QLabel(
    "<small><b>Full build requires:</b> CARLA's fork of UE5 (clone + build, "
    "~3 h, ~225 GB), CARLA source clone, and CARLA content clone (Bitbucket). "
    "Steps 1–3 below guide you through each stage. "
    "<a href='https://carla-ue5.readthedocs.io/en/latest/build_linux_ue5/'>UE5 build docs ↗</a></small>");
  fullBuild->setOpenExternalLinks(true);
  fullBuild->setWordWrap(true);
  l->addWidget(fullBuild);

  auto *branchRow = new QHBoxLayout();
  branchRow->addWidget(new QLabel("Branch:"));
  mBranchCombo = new QComboBox();
  mBranchCombo->addItems({"ue5-dev", "master", "dev"});
  branchRow->addWidget(mBranchCombo);
  branchRow->addStretch();
  l->addLayout(branchRow);

  auto *prereqGroup = new QGroupBox("Prerequisites");
  auto *prereqLayout = new QVBoxLayout(prereqGroup);
  auto *prereqHeader = new QHBoxLayout();
  prereqHeader->addStretch();
  mInstallPrereqBtn = new QPushButton("Install Prerequisites…");
  prereqHeader->addWidget(mInstallPrereqBtn);
  prereqLayout->addLayout(prereqHeader);

  auto addRow = [&](QLabel *&dot, QLabel *&detail, const QString &name) {
    auto *row = new QHBoxLayout();
    dot = new QLabel("·");
    dot->setFixedWidth(16);
    auto *toolLabel = new QLabel(name);
    toolLabel->setStyleSheet("font-family: monospace;");
    detail = new QLabel("checking…");
    detail->setStyleSheet("color:#666;");
    row->addWidget(dot);
    row->addWidget(toolLabel);
    row->addWidget(detail, 1);
    prereqLayout->addLayout(row);
  };
  addRow(mGitDot,   mGitDetail,   "git — version control");
  addRow(mLfsDot,   mLfsDetail,   "git-lfs — large-file content submodule");
  addRow(mNinjaDot, mNinjaDetail, "ninja — build executor");
  addRow(mCmakeDot, mCmakeDetail, "cmake");
  l->addWidget(prereqGroup);

  auto *ueRow = new QHBoxLayout();
  ueRow->addWidget(new QLabel("UE5 path:"));
  mUePath = new QLineEdit(defaultUePath());
  ueRow->addWidget(mUePath, 1);
  auto *browseUe = new QPushButton("Browse…");
  ueRow->addWidget(browseUe);
  mUeReadyLabel = new QLabel("·");
  mUeReadyLabel->setFixedWidth(56);
  ueRow->addWidget(mUeReadyLabel);
  l->addLayout(ueRow);
  connect(browseUe, &QPushButton::clicked, this, &SetupWizardDialog::browseUePath);
  connect(mUePath, &QLineEdit::textChanged, this, [this](const QString &) { refreshPrereqs(); });

  mLog = new QPlainTextEdit();
  mLog->setReadOnly(true);
  mLog->setMinimumHeight(100);
  mLog->setPlaceholderText("Build log will appear here…");
  l->addWidget(mLog, 1);

  auto *btnRow = new QHBoxLayout();
  mReCloneBtn      = new QPushButton("Re-clone");
  mCloneContentBtn = new QPushButton("Clone Content");
  mBuildBtn        = new QPushButton("Build  (cmake + ninja)");
  mSetAsRootBtn    = new QPushButton("Set as CARLA_ROOT");
  mCopyLogBtn      = new QPushButton("Copy Log");
  auto *closeBtn   = new QPushButton("Close");
  btnRow->addWidget(mReCloneBtn);
  btnRow->addWidget(mCloneContentBtn);
  btnRow->addWidget(mBuildBtn);
  btnRow->addWidget(mSetAsRootBtn);
  btnRow->addStretch();
  btnRow->addWidget(mCopyLogBtn);
  btnRow->addWidget(closeBtn);
  l->addLayout(btnRow);

  connect(mReCloneBtn,      &QPushButton::clicked, this, &SetupWizardDialog::onCloneRepo);
  connect(mCloneContentBtn, &QPushButton::clicked, this, &SetupWizardDialog::onCloneContent);
  connect(mBuildBtn,        &QPushButton::clicked, this, &SetupWizardDialog::onBuild);
  connect(mSetAsRootBtn,    &QPushButton::clicked, this, &SetupWizardDialog::onSetAsRoot);
  connect(mCopyLogBtn,      &QPushButton::clicked, this, &SetupWizardDialog::onCopyLog);
  connect(closeBtn,         &QPushButton::clicked, this, &QDialog::close);
  connect(mInstallPrereqBtn, &QPushButton::clicked, this, [this]() {
    appendLog("Run: sudo apt update && sudo apt install -y git git-lfs ninja-build cmake build-essential");
    QProcess::execute("/bin/bash", QStringList() << "-lc"
      << "x-terminal-emulator -e 'sudo apt update && sudo apt install -y git git-lfs ninja-build cmake build-essential' "
         "2>/dev/null || gnome-terminal -- bash -c 'sudo apt update && sudo apt install -y git git-lfs ninja-build cmake build-essential; read'");
  });

  return w;
}

QWidget *SetupWizardDialog::buildMapsTab() {
  auto *w = new QWidget();
  auto *l = new QVBoxLayout(w);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(8);

  auto *targetRow = new QHBoxLayout();
  targetRow->addWidget(new QLabel("Target root:"));
  mMapsTargetRoot = new QLineEdit(defaultClonePath());
  targetRow->addWidget(mMapsTargetRoot, 1);
  l->addLayout(targetRow);

  auto *intro = new QLabel(
    "<b>Maps</b> Official additional map packs and community-contributed maps — select one to install.");
  intro->setStyleSheet("color:#444;");
  l->addWidget(intro);

  mMapsList = new QListWidget();
  auto *headerOfficial = new QListWidgetItem("— Official Additional Maps —");
  headerOfficial->setFlags(Qt::NoItemFlags);
  headerOfficial->setForeground(Qt::gray);
  mMapsList->addItem(headerOfficial);
  for (const char *tag : kReleaseTags) {
    auto *it = new QListWidgetItem(QString("%1 AdditionalMaps  (0 MB)").arg(tag));
    it->setData(Qt::UserRole, QString::fromLatin1(tag));
    mMapsList->addItem(it);
  }
  auto *headerComm = new QListWidgetItem("— Community Maps —");
  headerComm->setFlags(Qt::NoItemFlags);
  headerComm->setForeground(Qt::gray);
  mMapsList->addItem(headerComm);
  for (const auto &m : kCommunityMaps) {
    auto *it = new QListWidgetItem(QString::fromLatin1(m[0]));
    it->setData(Qt::UserRole, QString::fromLatin1(m[1]));
    mMapsList->addItem(it);
  }
  l->addWidget(mMapsList, 1);

  auto *btnRow = new QHBoxLayout();
  mMapsRefreshBtn = new QPushButton("Refresh list");
  mMapsInstallBtn = new QPushButton("Install Selected Map");
  auto *closeBtn  = new QPushButton("Close");
  btnRow->addWidget(mMapsRefreshBtn);
  btnRow->addStretch();
  btnRow->addWidget(mMapsInstallBtn);
  btnRow->addWidget(closeBtn);
  l->addLayout(btnRow);

  mMapsLog = new QPlainTextEdit();
  mMapsLog->setReadOnly(true);
  mMapsLog->setMaximumHeight(80);
  mMapsLog->setPlainText("Using cached release list.");
  l->addWidget(mMapsLog);

  connect(mMapsRefreshBtn, &QPushButton::clicked, this, [this]() {
    mMapsLog->appendPlainText(QString("[%1]  Refresh: cached release list still in effect.").arg(stamp()));
  });
  connect(mMapsInstallBtn, &QPushButton::clicked, this, [this]() {
    auto *it = mMapsList->currentItem();
    if (!it || !(it->flags() & Qt::ItemIsSelectable)) return;
    const QString tag = it->data(Qt::UserRole).toString();
    mMapsLog->appendPlainText(QString("[%1]  Install %2 → %3 (download flow not yet wired).")
                                .arg(stamp(), tag, mMapsTargetRoot->text().trimmed()));
  });
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

  return w;
}

void SetupWizardDialog::refreshPrereqs() {
  const auto g = probeGit();
  styleDot(mGitDot, g.ok); mGitDetail->setText(g.detail);
  const auto lfs = probeGitLfs();
  styleDot(mLfsDot, lfs.ok); mLfsDetail->setText(lfs.detail);
  const auto n = probeNinja();
  styleDot(mNinjaDot, n.ok); mNinjaDetail->setText(n.detail);
  const auto c = probeCmake();
  styleDot(mCmakeDot, c.ok); mCmakeDetail->setText(c.detail);

  const bool ueOk = pathHasUnrealEditor(mUePath->text().trimmed());
  mUeReadyLabel->setText(ueOk ? "✓ ready" : "✗ missing");
  mUeReadyLabel->setStyleSheet(ueOk ? "color:#33AA55;" : "color:#CC4444;");
}

void SetupWizardDialog::browseClonePath() {
  const QString d = QFileDialog::getExistingDirectory(
      this, "Choose clone directory", mClonePath->text());
  if (!d.isEmpty()) mClonePath->setText(d);
}

void SetupWizardDialog::browseUePath() {
  const QString d = QFileDialog::getExistingDirectory(
      this, "Choose UE5 source directory", mUePath->text());
  if (!d.isEmpty()) mUePath->setText(d);
}

void SetupWizardDialog::appendLog(const QString &line) {
  if (mLog) mLog->appendPlainText(QString("[%1]  %2").arg(stamp(), line));
}

void SetupWizardDialog::onCloneRepo() {
  const QString dest   = mClonePath->text().trimmed();
  const QString branch = mBranchCombo->currentText();
  if (dest.isEmpty()) { appendLog("Clone: target path is empty."); return; }
  appendLog(QString("Cloning carla-simulator/carla (%1) into %2 …").arg(branch, dest));
  const QString cmd = QString("git clone --depth 1 -b %1 https://github.com/carla-simulator/carla.git %2 2>&1")
                        .arg(branch, dest);
  QProcess::startDetached("/bin/bash", QStringList() << "-lc" << cmd);
}

void SetupWizardDialog::onCloneContent() {
  const QString dest = mClonePath->text().trimmed();
  if (dest.isEmpty()) { appendLog("Content clone: target path is empty."); return; }
  const QString contentDest = dest + "/Unreal/CarlaUnreal/Content/Carla";
  appendLog(QString("Cloning content into %1 …").arg(contentDest));
  const QString cmd = QString("git clone --depth 1 https://bitbucket.org/carla-simulator/carla-content.git %1 2>&1")
                        .arg(contentDest);
  QProcess::startDetached("/bin/bash", QStringList() << "-lc" << cmd);
}

void SetupWizardDialog::onBuild() {
  const QString dest = mClonePath->text().trimmed();
  if (dest.isEmpty()) { appendLog("Build: target path is empty."); return; }
  const QString uePath = mUePath->text().trimmed();
  appendLog("Configuring + building (cmake + ninja) — see terminal for live output.");
  const QString cmd = QString(
      "cd %1 && CARLA_UNREAL_ENGINE_PATH=%2 cmake -G Ninja -S . -B Build/Development "
      "--toolchain=$PWD/CMake/Toolchain.cmake "
      "-DCMAKE_BUILD_TYPE=Development -DBUILD_CARLA_UNREAL=ON && "
      "cmake --build Build/Development --target carla-unreal 2>&1")
        .arg(dest, uePath);
  QProcess::startDetached("/bin/bash", QStringList() << "-lc"
      << QString("x-terminal-emulator -e 'bash -c \"%1; read\"' || gnome-terminal -- bash -c '%1; read'")
           .arg(cmd));
}

void SetupWizardDialog::onSetAsRoot() {
  const QString dest = mClonePath->text().trimmed();
  if (dest.isEmpty()) { appendLog("Set as CARLA_ROOT: target path is empty."); return; }
  if (mSetCarlaRoot) mSetCarlaRoot(dest);
  appendLog(QString("CARLA_ROOT set to: %1").arg(dest));
  qputenv("CARLA_ROOT", dest.toLocal8Bit());
}

void SetupWizardDialog::onCopyLog() {
  if (mLog) QApplication::clipboard()->setText(mLog->toPlainText());
}

}  // namespace carla_studio::setup_wizard
