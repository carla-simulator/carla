// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QVector3D>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

namespace Qt3DCore   { class QEntity; class QTransform; }
namespace Qt3DExtras { class Qt3DWindow; }
namespace Qt3DRender { class QPickEvent; }

namespace carla_studio::calibration {

enum class TargetType {
  Checkerboard = 0,
  AprilTag,
  ColorChecker,
  ConeSquare,
  ConeLine,
  PylonWall,
};

struct PlacedTarget {
  TargetType type = TargetType::Checkerboard;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double yawDeg = 0.0;
  int    cols = 8;
  int    rows = 6;
  double sizeM = 1.2;
  int    aprilId = 0;

  QJsonObject toJson() const;
  static PlacedTarget fromJson(const QJsonObject &);
};

QString labelFor(TargetType t);

QList<PlacedTarget> loadPersisted(const QString &scenarioId,
                                  const QString &sensorName);
void                savePersisted(const QString &scenarioId,
                                  const QString &sensorName,
                                  const QList<PlacedTarget> &targets);
QStringList         listPersistedSensors(const QString &scenarioId);

class CalibrationScene : public QWidget {
  Q_OBJECT
 public:
  explicit CalibrationScene(QWidget *parent = nullptr);
  ~CalibrationScene() override;

  void setHeroPose(double xMeters, double yMeters, double yawDeg);
  void setSensorPose(double xMeters, double yMeters, double zMeters,
                     double yawDeg);
  void setTargets(const QList<PlacedTarget> &);
  QList<PlacedTarget> targets() const { return mTargets; }
  int  selectedIndex() const { return mSelected; }

 signals:
  void targetsChanged(const QList<PlacedTarget> &);
  void selectionChanged(int index);

 public slots:
  void setNextDropType(TargetType t) { mNextDropType = t; }
  void addTargetAtCenter(TargetType t);
  void addTargetAt(TargetType t, double xWorld, double yWorld);
  void removeAt(int index);
  void clearAll();
  void updateTargetPose(int index, double x, double y, double z, double yawDeg);

 private:
  void rebuildTargetEntities();
  void onGroundClicked(Qt3DRender::QPickEvent *);
  void onTargetClicked(int index, Qt3DRender::QPickEvent *);

  Qt3DExtras::Qt3DWindow *mView         = nullptr;
  Qt3DCore::QEntity      *mRoot         = nullptr;
  Qt3DCore::QEntity      *mGround       = nullptr;
  Qt3DCore::QEntity      *mHero         = nullptr;
  Qt3DCore::QTransform   *mHeroXform    = nullptr;
  Qt3DCore::QEntity      *mSensorMarker = nullptr;
  Qt3DCore::QTransform   *mSensorXform  = nullptr;
  Qt3DCore::QEntity      *mTargetsRoot  = nullptr;

  QList<PlacedTarget>     mTargets;
  int                     mSelected = -1;
  TargetType              mNextDropType = TargetType::Checkerboard;
};

}  // namespace carla_studio::calibration
