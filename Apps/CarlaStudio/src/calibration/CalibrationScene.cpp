// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "calibration/CalibrationScene.h"

#include <QColor>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWidget>
#include <QWindow>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QRenderSettings>

namespace carla_studio::calibration {

QString labelFor(TargetType t) {
  switch (t) {
    case TargetType::Checkerboard: return QStringLiteral("Checkerboard");
    case TargetType::AprilTag:     return QStringLiteral("April Tag");
    case TargetType::ColorChecker: return QStringLiteral("Color Checker");
    case TargetType::ConeSquare:   return QStringLiteral("Cone Square");
    case TargetType::ConeLine:     return QStringLiteral("Cone Line");
    case TargetType::PylonWall:    return QStringLiteral("Pylon Wall");
  }
  return QStringLiteral("Target");
}

QJsonObject PlacedTarget::toJson() const {
  QJsonObject o;
  o["type"]    = static_cast<int>(type);
  o["x"]       = x;
  o["y"]       = y;
  o["z"]       = z;
  o["yaw"]     = yawDeg;
  o["cols"]    = cols;
  o["rows"]    = rows;
  o["size"]    = sizeM;
  o["aprilId"] = aprilId;
  return o;
}

PlacedTarget PlacedTarget::fromJson(const QJsonObject &o) {
  PlacedTarget t;
  t.type    = static_cast<TargetType>(o["type"].toInt(0));
  t.x       = o["x"].toDouble(0.0);
  t.y       = o["y"].toDouble(0.0);
  t.z       = o["z"].toDouble(0.0);
  t.yawDeg  = o["yaw"].toDouble(0.0);
  t.cols    = o["cols"].toInt(8);
  t.rows    = o["rows"].toInt(6);
  t.sizeM   = o["size"].toDouble(1.2);
  t.aprilId = o["aprilId"].toInt(0);
  return t;
}

static QString safeKey(const QString &raw) {
  QString out;
  out.reserve(raw.size());
  for (QChar ch : raw) {
    if (ch.isLetterOrNumber() || ch == '-' || ch == '_') out.append(ch);
    else                                                  out.append('_');
  }
  if (out.isEmpty()) out = QStringLiteral("default");
  return out;
}

static QString persistDir(const QString &scenarioId) {
  QString base = qEnvironmentVariable("CARLA_STUDIO_CALIBRATION_DIR");
  if (base.isEmpty()) {
    base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (base.isEmpty()) base = QDir::homePath() + "/.config/carla-studio";
    base += "/calibration";
  }
  QDir d(base);
  if (!d.exists()) d.mkpath(".");
  return d.filePath(safeKey(scenarioId));
}

static QString persistFile(const QString &scenarioId,
                           const QString &sensorName) {
  QDir d(persistDir(scenarioId));
  if (!d.exists()) d.mkpath(".");
  return d.filePath(safeKey(sensorName) + ".json");
}

QList<PlacedTarget> loadPersisted(const QString &scenarioId,
                                  const QString &sensorName) {
  QList<PlacedTarget> out;
  QFile f(persistFile(scenarioId, sensorName));
  if (!f.open(QIODevice::ReadOnly)) return out;
  const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
  if (!doc.isObject()) return out;
  const QJsonArray arr = doc.object()["targets"].toArray();
  for (const QJsonValue &v : arr)
    if (v.isObject()) out.append(PlacedTarget::fromJson(v.toObject()));
  return out;
}

void savePersisted(const QString &scenarioId,
                   const QString &sensorName,
                   const QList<PlacedTarget> &targets) {
  QJsonArray arr;
  for (const PlacedTarget &t : targets) arr.append(t.toJson());
  QJsonObject root;
  root["scenario"] = scenarioId;
  root["sensor"]   = sensorName;
  root["targets"]  = arr;
  QFile f(persistFile(scenarioId, sensorName));
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
  f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QStringList listPersistedSensors(const QString &scenarioId) {
  QDir d(persistDir(scenarioId));
  if (!d.exists()) return {};
  QStringList out;
  for (const QString &name : d.entryList(QStringList() << "*.json", QDir::Files))
    out.append(name.left(name.size() - 5));
  return out;
}

static QColor colorForType(TargetType t) {
  switch (t) {
    case TargetType::Checkerboard: return QColor("#f0f0f0");
    case TargetType::AprilTag:     return QColor("#202020");
    case TargetType::ColorChecker: return QColor("#a3be8c");
    case TargetType::ConeSquare:   return QColor("#d08770");
    case TargetType::ConeLine:     return QColor("#d08770");
    case TargetType::PylonWall:    return QColor("#bf616a");
  }
  return QColor("#888888");
}

static QVector3D scaleForType(TargetType t, const PlacedTarget &p) {
  const float s = static_cast<float>(p.sizeM);
  switch (t) {
    case TargetType::Checkerboard:
    case TargetType::AprilTag:
    case TargetType::ColorChecker:
      return {s, s * 0.66f, 0.05f};
    case TargetType::ConeSquare:
      return {1.0f, 1.0f, 0.6f};
    case TargetType::ConeLine:
      return {3.0f, 0.3f, 0.6f};
    case TargetType::PylonWall:
      return {2.5f, 1.0f, 1.2f};
  }
  return {1.0f, 1.0f, 1.0f};
}

CalibrationScene::CalibrationScene(QWidget *parent)
  : QWidget(parent) {
  setAcceptDrops(true);

  mView = new Qt3DExtras::Qt3DWindow();
  mView->defaultFrameGraph()->setClearColor(QColor("#1a1f2c"));

  QWidget *container = QWidget::createWindowContainer(mView, this);
  container->setMinimumSize(420, 320);
  container->setFocusPolicy(Qt::StrongFocus);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(container);

  mRoot = new Qt3DCore::QEntity();
  mView->setRootEntity(mRoot);

  Qt3DRender::QCamera *cam = mView->camera();
  cam->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 200.0f);
  cam->setPosition(QVector3D(0.0f, 12.0f, 18.0f));
  cam->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
  cam->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

  auto *camController = new Qt3DExtras::QOrbitCameraController(mRoot);
  camController->setCamera(cam);
  camController->setLinearSpeed(40.0f);
  camController->setLookSpeed(180.0f);

  auto *lightEntity = new Qt3DCore::QEntity(mRoot);
  auto *light = new Qt3DRender::QDirectionalLight(lightEntity);
  light->setColor(QColor(255, 255, 255));
  light->setIntensity(1.0f);
  light->setWorldDirection(QVector3D(-0.4f, -1.0f, -0.3f).normalized());
  lightEntity->addComponent(light);

  auto *fillLightEntity = new Qt3DCore::QEntity(mRoot);
  auto *fillLight = new Qt3DRender::QDirectionalLight(fillLightEntity);
  fillLight->setColor(QColor(180, 200, 255));
  fillLight->setIntensity(0.4f);
  fillLight->setWorldDirection(QVector3D(0.6f, -0.3f, 0.7f).normalized());
  fillLightEntity->addComponent(fillLight);

  mGround = new Qt3DCore::QEntity(mRoot);
  auto *groundMesh = new Qt3DExtras::QPlaneMesh();
  groundMesh->setWidth(40.0f);
  groundMesh->setHeight(40.0f);
  auto *groundMat = new Qt3DExtras::QPhongMaterial();
  groundMat->setDiffuse(QColor("#3b4252"));
  groundMat->setAmbient(QColor("#5a6478"));
  groundMat->setSpecular(QColor("#000000"));
  groundMat->setShininess(0.0f);
  auto *groundXform = new Qt3DCore::QTransform();
  mGround->addComponent(groundMesh);
  mGround->addComponent(groundMat);
  mGround->addComponent(groundXform);

  auto *groundPicker = new Qt3DRender::QObjectPicker(mGround);
  groundPicker->setHoverEnabled(false);
  mGround->addComponent(groundPicker);
  connect(groundPicker, &Qt3DRender::QObjectPicker::clicked,
          this, &CalibrationScene::onGroundClicked);

  for (int i = -20; i <= 20; i += 5) {
    auto *line = new Qt3DCore::QEntity(mRoot);
    auto *lineMesh = new Qt3DExtras::QCuboidMesh();
    lineMesh->setXExtent(40.0f);
    lineMesh->setYExtent(0.05f);
    lineMesh->setZExtent(0.08f);
    auto *lineMat = new Qt3DExtras::QPhongMaterial();
    QColor lc = (i == 0 ? QColor("#88c0d0") : QColor("#7a89a4"));
    lineMat->setDiffuse(lc);
    lineMat->setAmbient(lc);
    lineMat->setSpecular(QColor("#000000"));
    auto *lineXform = new Qt3DCore::QTransform();
    lineXform->setTranslation(QVector3D(0.0f, 0.04f, static_cast<float>(i)));
    line->addComponent(lineMesh);
    line->addComponent(lineMat);
    line->addComponent(lineXform);
  }
  for (int i = -20; i <= 20; i += 5) {
    auto *line = new Qt3DCore::QEntity(mRoot);
    auto *lineMesh = new Qt3DExtras::QCuboidMesh();
    lineMesh->setXExtent(0.08f);
    lineMesh->setYExtent(0.05f);
    lineMesh->setZExtent(40.0f);
    auto *lineMat = new Qt3DExtras::QPhongMaterial();
    QColor lc = (i == 0 ? QColor("#88c0d0") : QColor("#7a89a4"));
    lineMat->setDiffuse(lc);
    lineMat->setAmbient(lc);
    lineMat->setSpecular(QColor("#000000"));
    auto *lineXform = new Qt3DCore::QTransform();
    lineXform->setTranslation(QVector3D(static_cast<float>(i), 0.04f, 0.0f));
    line->addComponent(lineMesh);
    line->addComponent(lineMat);
    line->addComponent(lineXform);
  }

  mHero = new Qt3DCore::QEntity(mRoot);
  mHeroXform = new Qt3DCore::QTransform();
  mHeroXform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
  mHero->addComponent(mHeroXform);

  auto makeMat = [](const QColor &c, const QColor &amb) {
    auto *m = new Qt3DExtras::QPhongMaterial();
    m->setDiffuse(c);
    m->setAmbient(amb);
    m->setSpecular(QColor(0, 0, 0));
    m->setShininess(0.0f);
    return m;
  };

  const QColor kBodyDiffuse("#cdd9ee");
  const QColor kBodyAmbient("#cdd9ee");
  const QColor kRoofDiffuse("#a4b8d8");
  const QColor kRoofAmbient("#a4b8d8");
  const QColor kWheelDiffuse("#2c3540");
  const QColor kWheelAmbient("#3c4858");
  const QColor kAccentDiffuse("#5e81ac");
  const QColor kAccentAmbient("#7a9ec5");

  {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QCuboidMesh();
    m->setXExtent(1.85f);
    m->setYExtent(0.55f);
    m->setZExtent(4.30f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(0.0f, 0.55f, 0.0f));
    e->addComponent(m);
    e->addComponent(makeMat(kBodyDiffuse, kBodyAmbient));
    e->addComponent(xf);
  }
  {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QCuboidMesh();
    m->setXExtent(1.65f);
    m->setYExtent(0.55f);
    m->setZExtent(2.10f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(0.0f, 1.10f, -0.30f));
    e->addComponent(m);
    e->addComponent(makeMat(kRoofDiffuse, kRoofAmbient));
    e->addComponent(xf);
  }
  {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QCuboidMesh();
    m->setXExtent(1.40f);
    m->setYExtent(0.04f);
    m->setZExtent(0.60f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(0.0f, 0.85f, 1.30f));
    e->addComponent(m);
    e->addComponent(makeMat(kAccentDiffuse, kAccentAmbient));
    e->addComponent(xf);
  }
  {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QCuboidMesh();
    m->setXExtent(0.20f);
    m->setYExtent(0.05f);
    m->setZExtent(0.80f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(0.0f, 0.85f, -2.30f));
    e->addComponent(m);
    e->addComponent(makeMat(kAccentDiffuse, kAccentAmbient));
    e->addComponent(xf);
  }
  const struct { float x, z; } kWheels[4] = {
    { 0.95f,  1.40f}, {-0.95f,  1.40f},
    { 0.95f, -1.45f}, {-0.95f, -1.45f},
  };
  for (const auto &w : kWheels) {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QCylinderMesh();
    m->setRadius(0.32f);
    m->setLength(0.20f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(w.x, 0.32f, w.z));
    xf->setRotationZ(90.0f);
    e->addComponent(m);
    e->addComponent(makeMat(kWheelDiffuse, kWheelAmbient));
    e->addComponent(xf);
  }
  {
    auto *e = new Qt3DCore::QEntity(mHero);
    auto *m = new Qt3DExtras::QConeMesh();
    m->setBottomRadius(0.18f);
    m->setTopRadius(0.0f);
    m->setLength(0.45f);
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(0.0f, 0.55f, 2.50f));
    xf->setRotationX(90.0f);
    e->addComponent(m);
    e->addComponent(makeMat(QColor("#bf616a"), QColor("#bf616a")));
    e->addComponent(xf);
  }

  mSensorMarker = new Qt3DCore::QEntity(mRoot);
  auto *sMesh = new Qt3DExtras::QCuboidMesh();
  sMesh->setXExtent(0.35f);
  sMesh->setYExtent(0.35f);
  sMesh->setZExtent(0.35f);
  auto *sMat = new Qt3DExtras::QPhongMaterial();
  sMat->setDiffuse(QColor("#ebcb8b"));
  sMat->setAmbient(QColor("#ebcb8b"));
  sMat->setSpecular(QColor("#000000"));
  mSensorXform = new Qt3DCore::QTransform();
  mSensorXform->setTranslation(QVector3D(1.0f, 1.5f, 0.0f));
  mSensorMarker->addComponent(sMesh);
  mSensorMarker->addComponent(sMat);
  mSensorMarker->addComponent(mSensorXform);

  mTargetsRoot = new Qt3DCore::QEntity(mRoot);

  mView->renderSettings()->pickingSettings()->setPickMethod(
      Qt3DRender::QPickingSettings::TrianglePicking);
  mView->renderSettings()->pickingSettings()->setPickResultMode(
      Qt3DRender::QPickingSettings::NearestPick);
}

CalibrationScene::~CalibrationScene() = default;

void CalibrationScene::setHeroPose(double xMeters, double yMeters, double yawDeg) {
  mHeroXform->setTranslation(QVector3D(static_cast<float>(yMeters), 0.75f,
                                       static_cast<float>(-xMeters)));
  mHeroXform->setRotationY(static_cast<float>(-yawDeg));
}

void CalibrationScene::setSensorPose(double xMeters, double yMeters,
                                     double zMeters, double yawDeg) {
  mSensorXform->setTranslation(QVector3D(static_cast<float>(yMeters),
                                         static_cast<float>(zMeters + 0.05),
                                         static_cast<float>(-xMeters)));
  mSensorXform->setRotationY(static_cast<float>(-yawDeg));
}

void CalibrationScene::setTargets(const QList<PlacedTarget> &targets) {
  mTargets = targets;
  mSelected = -1;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
  emit selectionChanged(mSelected);
}

void CalibrationScene::addTargetAtCenter(TargetType t) {
  PlacedTarget p;
  p.type = t;
  p.x = 5.0;
  p.y = 0.0;
  p.z = 1.0;
  mTargets.append(p);
  mSelected = mTargets.size() - 1;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
  emit selectionChanged(mSelected);
}

void CalibrationScene::addTargetAt(TargetType t, double xWorld, double yWorld) {
  PlacedTarget p;
  p.type = t;
  p.x = xWorld;
  p.y = yWorld;
  p.z = 1.0;
  mTargets.append(p);
  mSelected = mTargets.size() - 1;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
  emit selectionChanged(mSelected);
}

void CalibrationScene::updateTargetPose(int index, double x, double y, double z, double yawDeg) {
  if (index < 0 || index >= mTargets.size()) return;
  mTargets[index].x      = x;
  mTargets[index].y      = y;
  mTargets[index].z      = z;
  mTargets[index].yawDeg = yawDeg;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
}

void CalibrationScene::removeAt(int index) {
  if (index < 0 || index >= mTargets.size()) return;
  mTargets.removeAt(index);
  mSelected = -1;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
  emit selectionChanged(mSelected);
}

void CalibrationScene::clearAll() {
  mTargets.clear();
  mSelected = -1;
  rebuildTargetEntities();
  emit targetsChanged(mTargets);
  emit selectionChanged(mSelected);
}

void CalibrationScene::rebuildTargetEntities() {
  const auto kids = mTargetsRoot->children();
  for (QObject *c : kids) c->deleteLater();

  for (int i = 0; i < mTargets.size(); ++i) {
    const PlacedTarget &p = mTargets[i];
    auto *ent = new Qt3DCore::QEntity(mTargetsRoot);
    auto *mesh = new Qt3DExtras::QCuboidMesh();
    QVector3D s = scaleForType(p.type, p);
    mesh->setXExtent(s.x());
    mesh->setYExtent(s.y());
    mesh->setZExtent(s.z());
    auto *mat = new Qt3DExtras::QPhongMaterial();
    QColor c = colorForType(p.type);
    if (i == mSelected) c = c.lighter(140);
    mat->setDiffuse(c);
    mat->setAmbient(c);
    mat->setSpecular(QColor("#000000"));
    auto *xf = new Qt3DCore::QTransform();
    xf->setTranslation(QVector3D(static_cast<float>(p.y),
                                 static_cast<float>(p.z),
                                 static_cast<float>(-p.x)));
    xf->setRotationY(static_cast<float>(-p.yawDeg));
    ent->addComponent(mesh);
    ent->addComponent(mat);
    ent->addComponent(xf);

    auto *pick = new Qt3DRender::QObjectPicker(ent);
    pick->setHoverEnabled(false);
    ent->addComponent(pick);
    const int captured = i;
    connect(pick, &Qt3DRender::QObjectPicker::clicked,
            this, [this, captured](Qt3DRender::QPickEvent *ev) {
              onTargetClicked(captured, ev);
            });
  }
}

void CalibrationScene::onGroundClicked(Qt3DRender::QPickEvent *ev) {
  if (!ev) return;
  const QVector3D w = ev->worldIntersection();
  const double xWorld = -w.z();
  const double yWorld =  w.x();
  addTargetAt(mNextDropType, xWorld, yWorld);
}

void CalibrationScene::onTargetClicked(int index, Qt3DRender::QPickEvent *) {
  mSelected = index;
  rebuildTargetEntities();
  emit selectionChanged(mSelected);
}

}  // namespace carla_studio::calibration
