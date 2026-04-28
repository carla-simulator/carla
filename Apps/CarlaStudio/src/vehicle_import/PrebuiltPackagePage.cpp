// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/PrebuiltPackagePage.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QTime>
#include <QVBoxLayout>

namespace carla_studio::vehicle_import {

namespace {
QString stamp() { return QTime::currentTime().toString("hh:mm:ss"); }
}  // namespace

PrebuiltPackagePage::PrebuiltPackagePage(CarlaRootResolver findCarlaRoot,
                                         QWidget *parent)
    : QWidget(parent), mFindCarlaRoot(std::move(findCarlaRoot)) {
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(12, 12, 12, 12);
  root->setSpacing(10);

  auto *banner = new QLabel(
    "Installs a pre-cooked vehicle package (.uasset + .uexp) directly into your CARLA binary. "
    "No Unreal Editor required — point to a community or marketplace folder and Studio "
    "copies it into the right place.");
  banner->setWordWrap(true);
  banner->setStyleSheet("color: #555; font-size: 11px;");
  root->addWidget(banner);

  auto *srcGroup = new QGroupBox("Package Folder");
  auto *srcRow = new QHBoxLayout();
  mFolderEdit = new QLineEdit();
  mFolderEdit->setPlaceholderText("Folder containing .uasset and .uexp files…");
  mFolderEdit->setReadOnly(true);
  mBrowseBtn = new QPushButton("Browse…");
  srcRow->addWidget(mFolderEdit, 1);
  srcRow->addWidget(mBrowseBtn);
  auto *srcLayout = new QVBoxLayout(srcGroup);
  srcLayout->addLayout(srcRow);
  root->addWidget(srcGroup);

  auto *filesGroup = new QGroupBox("Files in package");
  mFileList = new QListWidget();
  auto *filesLayout = new QVBoxLayout(filesGroup);
  filesLayout->addWidget(mFileList);
  root->addWidget(filesGroup, 1);

  auto *dstGroup = new QGroupBox("Install destination");
  auto *dstForm = new QVBoxLayout(dstGroup);
  auto *nameRow = new QHBoxLayout();
  nameRow->addWidget(new QLabel("Vehicle folder name:"));
  mNameEdit = new QLineEdit();
  mNameEdit->setPlaceholderText("Auto-detected from BP_ filename…");
  nameRow->addWidget(mNameEdit, 1);
  dstForm->addLayout(nameRow);

  auto *dstRow = new QHBoxLayout();
  dstRow->addWidget(new QLabel("Destination:"));
  mDstLabel = new QLabel("Set CARLA_ROOT and browse to a package folder.");
  mDstLabel->setWordWrap(true);
  mDstLabel->setStyleSheet("color: gray; font-size: 11px;");
  dstRow->addWidget(mDstLabel, 1);
  dstForm->addLayout(dstRow);
  root->addWidget(dstGroup);

  auto *btnRow = new QHBoxLayout();
  btnRow->addStretch();
  mInstallBtn = new QPushButton("Install Package");
  mInstallBtn->setEnabled(false);
  btnRow->addWidget(mInstallBtn);
  root->addLayout(btnRow);

  mStatusLabel = new QLabel();
  mStatusLabel->setStyleSheet("color: gray; font-size: 11px;");
  root->addWidget(mStatusLabel);

  mLog = new QPlainTextEdit();
  mLog->setReadOnly(true);
  mLog->setMaximumHeight(110);
  mLog->setPlaceholderText("Install log will appear here…");
  root->addWidget(mLog);

  connect(mBrowseBtn,  &QPushButton::clicked, this, &PrebuiltPackagePage::onBrowse);
  connect(mInstallBtn, &QPushButton::clicked, this, &PrebuiltPackagePage::onInstall);
  connect(mNameEdit,   &QLineEdit::textChanged, this,
          [this](const QString &) { refreshDestination(); });

  refreshDestination();
}

QString PrebuiltPackagePage::resolveVehiclesDir() const {
  const QString root = mFindCarlaRoot ? mFindCarlaRoot() : QString();
  if (root.isEmpty()) return QString();
  QDir d(root);
  const QStringList shippingDirs = d.entryList(
      QStringList() << "Carla-*-Linux-Shipping" << "Carla-*-Shipping",
      QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString &sub : shippingDirs) {
    const QString p = root + "/" + sub +
                      "/CarlaUnreal/Content/Carla/Blueprints/Vehicles";
    if (QDir(p).exists()) return p;
  }
  const QString src = root + "/Unreal/CarlaUnreal/Content/Carla/Blueprints/Vehicles";
  if (QDir(src).exists()) return src;
  return QString();
}

void PrebuiltPackagePage::refreshDestination() {
  const QString name = mNameEdit->text().trimmed();
  const QString vehiclesDir = resolveVehiclesDir();
  if (vehiclesDir.isEmpty()) {
    mDstLabel->setText("CARLA_ROOT not set or Vehicles/ directory not found.");
    mDstLabel->setStyleSheet("color: #CC4444; font-size: 11px;");
    mInstallBtn->setEnabled(false);
    return;
  }
  if (name.isEmpty()) {
    mDstLabel->setText(vehiclesDir + "/<vehicle name>/");
    mDstLabel->setStyleSheet("color: gray; font-size: 11px;");
    mInstallBtn->setEnabled(false);
    return;
  }
  mDstLabel->setText(vehiclesDir + "/" + name + "/");
  mDstLabel->setStyleSheet("color: #44AA66; font-size: 11px;");
  mInstallBtn->setEnabled(!mFolderEdit->text().isEmpty());
}

void PrebuiltPackagePage::onBrowse() {
  const QString folder = QFileDialog::getExistingDirectory(
      this, "Select vehicle package folder",
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  if (folder.isEmpty()) return;

  mFolderEdit->setText(folder);
  mFileList->clear();
  mInstallBtn->setEnabled(false);

  const QDir d(folder);
  const QStringList uassets = d.entryList({"*.uasset"}, QDir::Files);
  const QStringList uexps   = d.entryList({"*.uexp"},   QDir::Files);

  if (uassets.isEmpty()) {
    mFileList->addItem("⚠  No .uasset files found in this folder.");
    mStatusLabel->setText("Not a valid package folder.");
    mStatusLabel->setStyleSheet("color: #CC4444; font-size: 11px;");
    return;
  }

  for (const QString &f : uassets) {
    auto *it = new QListWidgetItem("● " + f);
    it->setForeground(QColor("#44AAFF"));
    mFileList->addItem(it);
  }
  for (const QString &f : uexps) {
    auto *it = new QListWidgetItem("  " + f);
    it->setForeground(QColor("#88BBDD"));
    mFileList->addItem(it);
  }

  if (mNameEdit->text().trimmed().isEmpty()) {
    QString guess;
    for (const QString &a : uassets) {
      if (a.startsWith("BP_", Qt::CaseInsensitive)) {
        guess = QFileInfo(a).completeBaseName().mid(3);
        break;
      }
    }
    if (guess.isEmpty()) guess = QFileInfo(uassets.first()).completeBaseName();
    static const QStringList kSuffixes = { "_FLW", "_FRW", "_RLW", "_RRW", "_Wheel" };
    for (const QString &s : kSuffixes) {
      if (guess.endsWith(s, Qt::CaseInsensitive)) { guess.chop(s.size()); break; }
    }
    mNameEdit->setText(guess);
  }

  mStatusLabel->setText(QString("%1 file(s) found  ·  %2 .uasset  ·  %3 .uexp")
                          .arg(uassets.size() + uexps.size())
                          .arg(uassets.size()).arg(uexps.size()));
  mStatusLabel->setStyleSheet("color: #44AA66; font-size: 11px;");
  refreshDestination();
}

void PrebuiltPackagePage::onInstall() {
  const QString src  = mFolderEdit->text().trimmed();
  const QString name = mNameEdit->text().trimmed();
  const QString vehiclesDir = resolveVehiclesDir();
  if (src.isEmpty() || name.isEmpty() || vehiclesDir.isEmpty()) {
    mLog->appendPlainText(QString("[%1]  Cannot install: source / name / CARLA_ROOT missing.").arg(stamp()));
    return;
  }
  const QString dstFolder = vehiclesDir + "/" + name;
  QDir().mkpath(dstFolder);

  const QDir d(src);
  const QStringList files = d.entryList({"*.uasset", "*.uexp"}, QDir::Files);
  int ok = 0, fail = 0;
  for (const QString &f : files) {
    const QString s = src + "/" + f;
    const QString t = dstFolder + "/" + f;
    if (QFile::exists(t)) QFile::remove(t);
    if (QFile::copy(s, t)) {
      ++ok;
      mLog->appendPlainText("  ✓ " + f);
    } else {
      ++fail;
      mLog->appendPlainText("  ✗ " + f + " (copy failed)");
    }
  }
  if (fail == 0) {
    mLog->appendPlainText(QString("[%1]  Installed %2 file(s) → %3").arg(stamp()).arg(ok).arg(dstFolder));
    mStatusLabel->setText(QString("Installed %1 file(s) → %2").arg(ok).arg(dstFolder));
    mStatusLabel->setStyleSheet("color: #44AA66; font-size: 11px;");
  } else {
    mLog->appendPlainText(QString("[%1]  Done with errors: %2 ok, %3 failed.").arg(stamp()).arg(ok).arg(fail));
    mStatusLabel->setText(QString("Done with errors: %1 ok, %2 failed.").arg(ok).arg(fail));
    mStatusLabel->setStyleSheet("color: #CC4444; font-size: 11px;");
  }
}

}  // namespace carla_studio::vehicle_import
