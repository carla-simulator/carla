// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/VehicleImportPage.h"

#include "vehicle_import/BPAutopicker.h"
#include "vehicle_import/ImporterClient.h"
#include "vehicle_import/MeshAABB.h"
#include "vehicle_import/ObjSanitizer.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardPaths>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <array>
#include <cmath>

namespace carla_studio::vehicle_import {

namespace {

constexpr const char *kImportButtonStyle =
    "QPushButton { padding: 4px 14px; font-weight: 600; min-width: 80px; }"
    "QPushButton:disabled {"
    "  color: palette(mid);"
    "  background-color: palette(button);"
    "  border: 1px solid palette(mid);"
    "}";

constexpr const char *kDefaultBaseVehicleBP =
    "/Game/Carla/Blueprints/USDImportTemplates/BaseUSDImportVehicle";

QString stamp() { return QTime::currentTime().toString("hh:mm:ss"); }

struct WheelGuess { float x, y, z, radius; };

std::array<WheelGuess, 4> guessWheels(float xMin, float xMax,
                                      float yMin, float yMax,
                                      float zMin, float zMax) {
  const float cx     = (xMin + xMax) * 0.5f;
  const float halfW  = (yMax - yMin) * 0.5f;
  const float axleY  = halfW * 0.8f;
  const float radius = std::max(5.f, (zMax - zMin) * 0.18f);
  const float axleZ  = zMin + radius;
  const float frontX = cx + (xMax - cx) * 0.60f;
  const float rearX  = cx - (cx - xMin) * 0.60f;
  return {{
    { frontX,  axleY, axleZ, radius },
    { frontX, -axleY, axleZ, radius },
    { rearX,   axleY, axleZ, radius },
    { rearX,  -axleY, axleZ, radius },
  }};
}

}  // namespace

VehicleImportPage::VehicleImportPage(EditorBinaryResolver findEditor,
                                     UprojectResolver     findUproject,
                                     QWidget *parent)
    : QWidget(parent),
      mFindEditor(std::move(findEditor)),
      mFindUproject(std::move(findUproject)) {
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(12, 12, 12, 12);
  root->setSpacing(10);

  auto *meshGroup = new QGroupBox("3D Model File (*OBJ, *DAE, *glTF, *GLB)");
  auto *fileRow = new QHBoxLayout();
  mMeshPathEdit = new QLineEdit();
  mMeshPathEdit->setPlaceholderText("Path to .obj, .gltf, .glb or .dae file …");
  mMeshPathEdit->setReadOnly(true);
  mBrowseBtn = new QPushButton("Browse…");
  fileRow->addWidget(mMeshPathEdit, 1);
  fileRow->addWidget(mBrowseBtn);
  auto *meshGroupLayout = new QVBoxLayout(meshGroup);
  meshGroupLayout->addLayout(fileRow);
  mAabbLabel = new QLabel("Drop or browse to a model to auto-detect dimensions.");
  mAabbLabel->setStyleSheet("color: gray;");
  meshGroupLayout->addWidget(mAabbLabel);
  root->addWidget(meshGroup);

  auto *wheelsGroup = new QGroupBox("Wheel Positions (cm — Unreal units)");
  auto *wheelsForm  = new QFormLayout();
  wheelsForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
  wheelsForm->setVerticalSpacing(4);
  wheelsForm->setHorizontalSpacing(8);
  wheelsForm->setContentsMargins(8, 6, 8, 6);

  auto makeWheelRow = [&](const QString &label) -> WheelSpinRow {
    WheelSpinRow wr;
    auto *row = new QHBoxLayout();
    row->setSpacing(0);
    row->setContentsMargins(0, 0, 0, 0);
    auto makeSpin = [row](const QString &prefix, float lo, float hi, bool firstInRow) -> QDoubleSpinBox * {
      auto *pl = new QLabel(prefix);
      pl->setFixedWidth(14);
      pl->setAlignment(Qt::AlignCenter);
      pl->setContentsMargins(firstInRow ? 0 : 10, 0, 4, 0);
      auto *sb = new QDoubleSpinBox();
      sb->setRange(lo, hi);
      sb->setDecimals(1);
      sb->setSingleStep(0.5);
      sb->setFixedWidth(88);
      row->addWidget(pl);
      row->addWidget(sb);
      return sb;
    };
    wr.x = makeSpin("X", -5000, 5000, true);
    wr.y = makeSpin("Y", -5000, 5000, false);
    wr.z = makeSpin("Z", -5000, 5000, false);
    wr.r = makeSpin("R",      1,  200, false);
    wr.r->setToolTip("Wheel radius");
    row->addStretch();
    auto *rowW = new QWidget();
    rowW->setLayout(row);
    wheelsForm->addRow(label, rowW);
    return wr;
  };
  mRowFL = makeWheelRow("Front Left");
  mRowFR = makeWheelRow("Front Right");
  mRowRL = makeWheelRow("Rear Left");
  mRowRR = makeWheelRow("Rear Right");
  wheelsGroup->setLayout(wheelsForm);
  root->addWidget(wheelsGroup);

  auto *paramsGroup = new QGroupBox("Vehicle Parameters");
  auto *paramsForm  = new QFormLayout();
  paramsForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  paramsForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
  paramsForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
  paramsForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  paramsForm->setVerticalSpacing(4);
  paramsForm->setHorizontalSpacing(10);
  paramsForm->setContentsMargins(8, 6, 8, 6);

  mVehicleNameEdit = new QLineEdit();
  mVehicleNameEdit->setPlaceholderText("e.g. MyTruck");
  paramsForm->addRow("Vehicle Name", mVehicleNameEdit);

  mContentPathEdit = new QLineEdit("/Game/Carla/Static/Vehicles/4Wheeled");
  paramsForm->addRow("UE Content Path", mContentPathEdit);

  mBaseVehicleBPEdit = new QLineEdit(kDefaultBaseVehicleBP);
  mBaseVehicleBPEdit->setProperty("lastAutoPick", QString(kDefaultBaseVehicleBP));
  mBaseVehicleBPEdit->setToolTip(
    "Existing CARLA vehicle Blueprint to use as the rigging template.\n"
    "Defaults to BaseUSDImportVehicle — the only stock BP with a real "
    "USkeletalMesh + PhysicsAsset that the importer can consume.");
  paramsForm->addRow("Base Vehicle BP", mBaseVehicleBPEdit);

  auto makeSpin = [](double lo, double hi, double def, const QString &suffix, int decimals = 2) {
    auto *sb = new QDoubleSpinBox();
    sb->setRange(lo, hi);
    sb->setDecimals(decimals);
    sb->setValue(def);
    if (!suffix.isEmpty()) sb->setSuffix(" " + suffix);
    return sb;
  };
  mMassSpin      = makeSpin(100,  100000, 1500, "kg",   2);
  mSteerSpin     = makeSpin(0,    90,     70,   "°",    2);
  mSuspRaiseSpin = makeSpin(0,    100,    10,   "cm",   2);
  mSuspDropSpin  = makeSpin(0,    100,    10,   "cm",   2);
  mSuspDampSpin  = makeSpin(0.1,  2.0,    0.65, "",     2);
  mSuspDampSpin->setSingleStep(0.05);
  mBrakeSpin     = makeSpin(0,    10000,  1500, "Nm",   2);
  paramsForm->addRow("Vehicle Mass",          mMassSpin);
  paramsForm->addRow("Max Steer Angle",       mSteerSpin);
  paramsForm->addRow("Suspension Max Raise",  mSuspRaiseSpin);
  paramsForm->addRow("Suspension Max Drop",   mSuspDropSpin);
  paramsForm->addRow("Suspension Damping",    mSuspDampSpin);
  paramsForm->addRow("Max Brake Torque",      mBrakeSpin);
  paramsGroup->setLayout(paramsForm);
  root->addWidget(paramsGroup);

  auto *importRow = new QHBoxLayout();
  mUeStatusLabel = new QLabel("UE Editor: not connected");
  mUeStatusLabel->setStyleSheet("color: gray;");

  mImportBtn = new QPushButton("Import");
  mImportBtn->setEnabled(false);
  mImportBtn->setToolTip(
    "Run the import end-to-end:\n"
    "  • Spawn a headless UE Editor in the background.\n"
    "  • Wait for the VehicleImporter port to come up.\n"
    "  • Send the spec, import the mesh, build wheel + vehicle BPs.");

  mDropBtn = new QPushButton("Drop && Visualize");
  mDropBtn->setEnabled(false);
  mDropBtn->setToolTip(
    "Spawn the just-imported vehicle into the running UE Editor world.");

  importRow->addWidget(mUeStatusLabel, 1);
  importRow->addWidget(mImportBtn);
  importRow->addWidget(mDropBtn);
  root->addLayout(importRow);

  mProgress = new QProgressBar();
  mProgress->setRange(0, 100);
  mProgress->setValue(0);
  mProgress->setTextVisible(true);
  mProgress->setFormat("idle");
  mProgress->setVisible(false);
  root->addWidget(mProgress);

  mAssetLabel = new QLabel();
  mAssetLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  mAssetLabel->setVisible(false);
  mAssetLabel->setStyleSheet("color: #2E7D32; font-weight: 600;");
  root->addWidget(mAssetLabel);

  mLog = new QPlainTextEdit();
  mLog->setReadOnly(true);
  mLog->setMaximumHeight(110);
  mLog->setPlaceholderText("Import log will appear here…");
  root->addWidget(mLog);

  applyDisabledStateStyling();

  connect(mBrowseBtn, &QPushButton::clicked, this, &VehicleImportPage::onBrowse);
  connect(mImportBtn, &QPushButton::clicked, this, &VehicleImportPage::onImport);
  connect(mDropBtn,   &QPushButton::clicked, this, &VehicleImportPage::onDrop);
}

void VehicleImportPage::applyDisabledStateStyling() {
  mImportBtn->setStyleSheet(kImportButtonStyle);
  mDropBtn  ->setStyleSheet(kImportButtonStyle);
}

void VehicleImportPage::onBrowse() {
  const QString path = QFileDialog::getOpenFileName(
      this, "Select 3D Model",
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
      "Supported 3D Models (*.obj *.gltf *.glb *.dae);;"
      "OBJ (*.obj);;glTF (*.gltf *.glb);;Collada (*.dae);;All files (*)");
  if (path.isEmpty()) return;

  mMeshPathEdit->setText(path);
  mImportBtn->setEnabled(false);

  const QString ext = QFileInfo(path).suffix().toLower();
  MeshAABB bb;
  if      (ext == "obj")                    bb = parseOBJ(path);
  else if (ext == "gltf" || ext == "glb")   bb = parseGLTF(path);
  else if (ext == "dae")                    bb = parseDAE(path);

  if (!bb.valid) {
    mAabbLabel->setText("Could not read geometry from this file.");
    mAabbLabel->setStyleSheet("color: red;");
    *mAabbValid = false;
    return;
  }

  bb.detectConventions(ext);
  *mScaleToCm   = bb.scaleToCm;
  *mUpAxis      = bb.upAxis;
  *mForwardAxis = bb.forwardAxis;
  *mAabbValid   = true;

  float xLo, xHi, yLo, yHi, zLo, zHi;
  bb.toUE(xLo, xHi, yLo, yHi, zLo, zHi);

  mAabbLabel->setText(
      QString("Bounding box: X [%1 → %2]  Y [%3 → %4]  Z [%5 → %6] cm  (×%7, up=%8)")
        .arg(xLo, 0, 'f', 1).arg(xHi, 0, 'f', 1)
        .arg(yLo, 0, 'f', 1).arg(yHi, 0, 'f', 1)
        .arg(zLo, 0, 'f', 1).arg(zHi, 0, 'f', 1)
        .arg(bb.scaleToCm, 0, 'f', 0)
        .arg(QString("XYZ").at(bb.upAxis)));
  mAabbLabel->setStyleSheet("color: green;");

  const auto wheels = guessWheels(xLo, xHi, yLo, yHi, zLo, zHi);
  auto applyRow = [](WheelSpinRow &r, const WheelGuess &w) {
    r.x->setValue(w.x); r.y->setValue(w.y);
    r.z->setValue(w.z); r.r->setValue(w.radius);
  };
  applyRow(mRowFL, wheels[0]);
  applyRow(mRowFR, wheels[1]);
  applyRow(mRowRL, wheels[2]);
  applyRow(mRowRR, wheels[3]);

  if (mVehicleNameEdit->text().trimmed().isEmpty())
    mVehicleNameEdit->setText(QFileInfo(path).completeBaseName());

  const QString autoPicked = pickClosestBaseVehicleBP(xHi - xLo);
  const QString lastAuto   = mBaseVehicleBPEdit->property("lastAutoPick").toString();
  const QString cur        = mBaseVehicleBPEdit->text().trimmed();
  if (cur.isEmpty() || cur == lastAuto) {
    mBaseVehicleBPEdit->setText(autoPicked);
    mBaseVehicleBPEdit->setProperty("lastAutoPick", autoPicked);
  }

  mImportBtn->setEnabled(true);
}

QJsonObject VehicleImportPage::buildImportSpec(const QString &meshPathToSend) const {
  auto wheelObj = [](const WheelSpinRow &r, double steer, double brake,
                     double raise, double drop) {
    QJsonObject o;
    o["x"]                = r.x->value();
    o["y"]                = r.y->value();
    o["z"]                = r.z->value();
    o["radius"]           = r.r->value();
    o["max_steer_angle"]  = steer;
    o["max_brake_torque"] = brake;
    o["susp_max_raise"]   = raise;
    o["susp_max_drop"]    = drop;
    return o;
  };
  QJsonObject spec;
  spec["vehicle_name"]    = mVehicleNameEdit->text().trimmed();
  spec["mesh_path"]       = meshPathToSend;
  spec["content_path"]    = mContentPathEdit->text().trimmed();
  spec["base_vehicle_bp"] = mBaseVehicleBPEdit->text().trimmed();
  spec["mass"]            = mMassSpin->value();
  spec["susp_damping"]    = mSuspDampSpin->value();
  if (*mAabbValid) {
    spec["source_scale_to_cm"]  = *mScaleToCm;
    spec["source_up_axis"]      = *mUpAxis;
    spec["source_forward_axis"] = *mForwardAxis;
  }
  spec["wheel_fl"] = wheelObj(mRowFL, mSteerSpin->value(), mBrakeSpin->value(),
                              mSuspRaiseSpin->value(), mSuspDropSpin->value());
  spec["wheel_fr"] = wheelObj(mRowFR, mSteerSpin->value(), mBrakeSpin->value(),
                              mSuspRaiseSpin->value(), mSuspDropSpin->value());
  spec["wheel_rl"] = wheelObj(mRowRL, 0.0,                  mBrakeSpin->value(),
                              mSuspRaiseSpin->value(), mSuspDropSpin->value());
  spec["wheel_rr"] = wheelObj(mRowRR, 0.0,                  mBrakeSpin->value(),
                              mSuspRaiseSpin->value(), mSuspDropSpin->value());
  return spec;
}

void VehicleImportPage::onImport() {
  const QString meshPath = mMeshPathEdit->text().trimmed();
  const QString name     = mVehicleNameEdit->text().trimmed();
  if (meshPath.isEmpty() || name.isEmpty()) {
    mLog->appendPlainText("Error: vehicle name and mesh path are required.");
    return;
  }

  QString meshToSend = meshPath;
  if (QFileInfo(meshPath).suffix().toLower() == "obj") {
    const SanitizeReport rep = sanitizeOBJ(meshPath, *mScaleToCm);
    if (rep.ok) {
      meshToSend = rep.outputPath;
      mLog->appendPlainText(QString("[%1]  Sanitized OBJ → %2  (×%3 scale, skipped %4 malformed face line(s))")
                              .arg(stamp()).arg(meshToSend).arg(*mScaleToCm, 0, 'f', 0)
                              .arg(rep.skippedFaceLines));
    }
  }

  const QJsonObject spec = buildImportSpec(meshToSend);

  mImportBtn->setEnabled(false);
  mProgress->setVisible(true);
  mProgress->setRange(0, 100);
  mProgress->setValue(0);
  mProgress->setFormat("starting…");
  mAssetLabel->clear();
  mAssetLabel->setVisible(false);

  QPointer<QPushButton>     btnPtr      = mImportBtn;
  QPointer<QPushButton>     dropPtr     = mDropBtn;
  QPointer<QLabel>          statusPtr   = mUeStatusLabel;
  QPointer<QPlainTextEdit>  logPtr      = mLog;
  QPointer<QProgressBar>    progressPtr = mProgress;
  QPointer<QLabel>          assetPtr    = mAssetLabel;
  std::shared_ptr<QString>  lastAsset   = mLastAsset;

  auto sendNow = [=]() {
    if (!logPtr) return;
    logPtr->appendPlainText(QString("[%1]  Sending spec to UE Editor…").arg(stamp()));
    if (statusPtr) statusPtr->setText("UE Editor: sending …");
    if (progressPtr) {
      progressPtr->setRange(0, 0);
      progressPtr->setFormat("importing in UE Editor…");
    }
    Q_UNUSED(QtConcurrent::run([=]() {
      const QString response = sendJson(spec);
      QMetaObject::invokeMethod(qApp, [=]() {
        if (!logPtr) return;
        if (btnPtr) btnPtr->setEnabled(true);
        if (progressPtr) progressPtr->setRange(0, 100);
        if (response.isEmpty()) {
          if (statusPtr) {
            statusPtr->setText("UE Editor: connection failed");
            statusPtr->setStyleSheet("color: red;");
          }
          if (progressPtr) { progressPtr->setValue(0); progressPtr->setFormat("connection failed"); }
          logPtr->appendPlainText(QString("[%1]  Error: no response from UE Editor on port %2.")
                                    .arg(stamp()).arg(kImporterPort));
          return;
        }
        const QJsonObject resp = QJsonDocument::fromJson(response.toUtf8()).object();
        const bool        ok   = resp.value("success").toBool();
        const QString     path = resp.value("asset_path").toString();
        const QString     err  = resp.value("error").toString();
        if (ok) {
          if (statusPtr) {
            statusPtr->setText("UE Editor: import succeeded");
            statusPtr->setStyleSheet("color: green;");
          }
          if (progressPtr) { progressPtr->setValue(100); progressPtr->setFormat("done (100%)"); }
          if (assetPtr) {
            assetPtr->setText(QString("Imported asset: %1").arg(path));
            assetPtr->setVisible(true);
          }
          *lastAsset = path;
          if (dropPtr) dropPtr->setEnabled(!path.isEmpty());
          logPtr->appendPlainText(QString("[%1]  Success! Blueprint created at: %2").arg(stamp(), path));
        } else {
          if (statusPtr) {
            statusPtr->setText("UE Editor: import failed");
            statusPtr->setStyleSheet("color: red;");
          }
          if (progressPtr) { progressPtr->setValue(0); progressPtr->setFormat("failed"); }
          logPtr->appendPlainText(QString("[%1]  Failed: %2").arg(stamp(), err));
        }
      }, Qt::QueuedConnection);
    }));
  };

  if (probeImporterPort()) {
    if (progressPtr) { progressPtr->setValue(50); progressPtr->setFormat("editor already up — sending"); }
    sendNow();
    return;
  }

  const QString editor = mFindEditor ? mFindEditor() : QString();
  if (editor.isEmpty()) {
    logPtr->appendPlainText(QString("[%1]  UE Editor binary not found. Set CARLA_UNREAL_ENGINE_PATH.").arg(stamp()));
    if (statusPtr) {
      statusPtr->setText("UE Editor: not found");
      statusPtr->setStyleSheet("color: red;");
    }
    if (progressPtr) { progressPtr->setValue(0); progressPtr->setFormat("editor not found"); }
    if (btnPtr) btnPtr->setEnabled(true);
    return;
  }

  const QString uproject = mFindUproject ? mFindUproject() : QString();
  QStringList args;
  if (!uproject.isEmpty()) args << uproject;
  args << "-unattended" << "-nullrhi" << "-nosplash" << "-nosound";
  qint64 pid = 0;
  if (!QProcess::startDetached(editor, args, QString(), &pid)) {
    logPtr->appendPlainText(QString("[%1]  Could not start UE Editor: %2").arg(stamp(), editor));
    if (statusPtr) {
      statusPtr->setText("UE Editor: launch failed");
      statusPtr->setStyleSheet("color: red;");
    }
    if (progressPtr) { progressPtr->setValue(0); progressPtr->setFormat("launch failed"); }
    if (btnPtr) btnPtr->setEnabled(true);
    return;
  }
  logPtr->appendPlainText(QString("[%1]  Launched headless UE Editor (pid %2) — waiting for VehicleImporter port %3 …")
                            .arg(stamp()).arg(pid).arg(kImporterPort));
  if (statusPtr) {
    statusPtr->setText("UE Editor: launching …");
    statusPtr->setStyleSheet("color: #BB8800;");
  }
  if (progressPtr) {
    progressPtr->setValue(5);
    progressPtr->setFormat(QString("starting headless UE Editor… (0/120s)"));
  }

  auto attempts = std::make_shared<int>(0);
  constexpr int kMaxAttempts = 120;
  auto *poll = new QTimer(this);
  poll->setInterval(1000);
  connect(poll, &QTimer::timeout, this, [=]() {
    ++*attempts;
    if (probeImporterPort()) {
      poll->stop(); poll->deleteLater();
      if (logPtr) logPtr->appendPlainText(QString("[%1]  Editor ready (port %2 open).")
                                            .arg(stamp()).arg(kImporterPort));
      if (progressPtr) { progressPtr->setValue(50); progressPtr->setFormat("editor ready — sending spec"); }
      sendNow();
      return;
    }
    if (progressPtr) {
      const int pct = 5 + (*attempts * 44 / kMaxAttempts);
      progressPtr->setValue(pct);
      progressPtr->setFormat(QString("starting headless UE Editor… (%1/%2s)").arg(*attempts).arg(kMaxAttempts));
    }
    if (*attempts >= kMaxAttempts) {
      poll->stop(); poll->deleteLater();
      if (logPtr) logPtr->appendPlainText(QString("[%1]  Timed out waiting for UE Editor (%2 s).")
                                            .arg(stamp()).arg(kMaxAttempts));
      if (statusPtr) {
        statusPtr->setText("UE Editor: timeout");
        statusPtr->setStyleSheet("color: red;");
      }
      if (progressPtr) { progressPtr->setValue(0); progressPtr->setFormat("timeout"); }
      if (btnPtr) btnPtr->setEnabled(true);
    }
  });
  poll->start();
}

void VehicleImportPage::onDrop() {
  const QString assetPath = *mLastAsset;
  if (assetPath.isEmpty()) return;
  if (!probeImporterPort()) {
    mLog->appendPlainText(QString("[%1]  Drop: editor not listening on %2 — run Import first.")
                            .arg(stamp()).arg(kImporterPort));
    return;
  }
  mDropBtn->setEnabled(false);
  mLog->appendPlainText(QString("[%1]  Drop: spawning %2 in editor world…").arg(stamp(), assetPath));

  const QJsonObject spec = buildSpawnSpec(assetPath);

  QPointer<QPushButton>    btnPtr    = mDropBtn;
  QPointer<QLabel>         statusPtr = mUeStatusLabel;
  QPointer<QPlainTextEdit> logPtr    = mLog;

  Q_UNUSED(QtConcurrent::run([=]() {
    const QString resp = sendJson(spec);
    QMetaObject::invokeMethod(qApp, [=]() {
      if (!logPtr) return;
      if (btnPtr) btnPtr->setEnabled(true);
      const QJsonObject ro     = QJsonDocument::fromJson(resp.toUtf8()).object();
      const bool        ok     = ro.value("success").toBool();
      const QString     detail = ok ? ro.value("asset_path").toString()
                                    : ro.value("error").toString();
      logPtr->appendPlainText(QString("[%1]  Drop: %2 — %3")
                                .arg(stamp()).arg(ok ? "OK" : "FAIL").arg(detail));
      if (ok) {
        if (statusPtr) {
          statusPtr->setText(QString("UE Editor: %1 spawned").arg(detail));
          statusPtr->setStyleSheet("color: green;");
        }
        logPtr->appendPlainText(QString(
          "[%1]  Note: to drive in the running CARLA simulator with keyboard input, the BP needs "
          "to be registered with BP_VehicleFactory. That's the next planned feature.").arg(stamp()));
      }
    }, Qt::QueuedConnection);
  }));
}

}  // namespace carla_studio::vehicle_import
