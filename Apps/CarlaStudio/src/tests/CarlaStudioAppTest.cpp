// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QtTest/QtTest>

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>

#include "core/PlayerSlots.h"
#include "core/SensorMountKey.h"
#include "core/StudioAppContext.h"
#include "utils/ResourceFit.h"

class CarlaStudioAppTest : public QObject {
  Q_OBJECT

private:
  QString sourcePath;

  QString readSource() const {
    QFile file(sourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return QString();
    }
    QTextStream in(&file);
    return in.readAll();
  }

private slots:
  void initTestCase() {
#ifdef CARLA_STUDIO_SOURCE_PATH
    sourcePath = QStringLiteral(CARLA_STUDIO_SOURCE_PATH);
#else
  sourcePath = QStringLiteral("../../CarlaStudio/src/app/carla_studio.cpp");
#endif

    if (!QFile::exists(sourcePath)) {
      QSKIP("app carla_studio.cpp not found; set CARLA_STUDIO_SOURCE_PATH or run tests from Apps/CarlaStudio.", SkipSingle);
    }
  }

  void startup_status_states_present() {
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), "Unable to read app carla_studio.cpp");

    QVERIFY2(source.contains("setSimulationStatus(\"Idle\")"), "Missing Idle status state");
    QVERIFY2(source.contains("setSimulationStatus(\"Running\")"), "Missing Running status state");
    QVERIFY2(source.contains("setSimulationStatus(\"Stopped\")"), "Missing Stopped status state");
    QVERIFY2(source.contains("setSimulationStatus(\"Initializing\")"), "Missing Initializing status state");
  }

  void stop_timeout_force_exit_present() {
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), "Unable to read app carla_studio.cpp");

    QVERIFY2(source.contains("forceStopTimer->start(60000)"), "Missing 60-second force-stop timeout");
    QVERIFY2(source.contains("pkill -TERM -f 'CarlaUE4|CarlaUE5|CarlaUE'"), "Missing fallback kill-all command");
  }

  void live_process_table_ui_present() {
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), "Unable to read app carla_studio.cpp");

    QVERIFY2(source.contains("carlaProcessTable"), "Missing live process table widget");
    QVERIFY2(source.contains("\"PID\" << \"Process\" << \"CPU\" << \"Memory\" << \"GPU\""),
             "Live process table header columns changed");
    QVERIFY2(source.contains("trackedCarlaPids"), "Missing tracked PID store");
    QVERIFY2(source.contains("echo $!"), "START path does not capture child PID");
  }

  void sensor_assembly_rotation_range_present() {
    const QString source = readSource();
    QVERIFY2(!source.isEmpty(), "Unable to read app carla_studio.cpp");

    QVERIFY2(source.contains("rx->setRange(0.0, 359.0)"), "rx range is not 0..359");
    QVERIFY2(source.contains("ry->setRange(0.0, 359.0)"), "ry range is not 0..359");
    QVERIFY2(source.contains("rz->setRange(0.0, 359.0)"), "rz range is not 0..359");
  }

  void resource_fit_run_cmd_timeout_returns_empty() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString scriptPath = dir.path() + "/sleeper.sh";
    {
      QFile f(scriptPath);
      QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
      QTextStream(&f) << "#!/bin/sh\nsleep 5\n";
    }
    QFile::setPermissions(scriptPath,
        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
        QFile::ReadUser  | QFile::WriteUser  | QFile::ExeUser);

    QElapsedTimer t;
    t.start();
    const QString out = carla_studio::utils::detail::runCmd(
        "/bin/sh", QStringList() << scriptPath, 200);
    const qint64 elapsed = t.elapsed();
    QVERIFY2(out.isEmpty(), "runCmd should return empty QString on timeout");
    QVERIFY2(elapsed < 5000,
        qPrintable(QString("runCmd should escalate terminate->kill, elapsed=%1ms").arg(elapsed)));
  }

  void resource_fit_run_cmd_normal_completion() {
    const QString out = carla_studio::utils::detail::runCmd(
        "/bin/sh", QStringList() << "-c" << "printf hello", 3000);
    QCOMPARE(out, QStringLiteral("hello"));
  }

  void resource_fit_detect_os_pretty_non_empty() {
    const QString os = carla_studio::utils::detail::detectOsPretty();
    QVERIFY2(!os.isEmpty(), "detectOsPretty returned empty string on Linux host");
  }

  void resource_fit_detect_cpu_non_zero() {
    QString model;
    int logical = 0;
    int physical = 0;
    carla_studio::utils::detail::detectCpu(&model, &logical, &physical);
    QVERIFY2(logical > 0,  qPrintable(QString("logical=%1").arg(logical)));
    QVERIFY2(physical > 0, qPrintable(QString("physical=%1").arg(physical)));
  }

  void resource_fit_detect_ram_non_zero() {
    std::int64_t total = 0;
    std::int64_t free = 0;
    carla_studio::utils::detail::detectRam(&total, &free);
    QVERIFY2(total > 0, qPrintable(QString("ram total bytes=%1").arg(total)));
  }

  void studio_app_context_workspace_round_trip() {
    carla::studio::core::StudioAppContext ctx;
    QVERIFY(!ctx.HasWorkspaceRoot());
    QVERIFY(ctx.GetWorkspaceRoot().isEmpty());
    ctx.SetWorkspaceRoot(QStringLiteral("/tmp/ws-root"));
    QCOMPARE(ctx.GetWorkspaceRoot(), QStringLiteral("/tmp/ws-root"));
    QVERIFY(ctx.HasWorkspaceRoot());
    ctx.SetWorkspaceRoot(QString());
    QVERIFY(!ctx.HasWorkspaceRoot());
  }

  void studio_app_context_ui_only_round_trip() {
    carla::studio::core::StudioAppContext ctx;
    QCOMPARE(ctx.IsUiOnlyMode(), false);
    ctx.SetUiOnlyMode(true);
    QCOMPARE(ctx.IsUiOnlyMode(), true);
    ctx.SetUiOnlyMode(false);
    QCOMPARE(ctx.IsUiOnlyMode(), false);
  }

  void sensor_mount_key_instance_one_is_bare() {
    using carla::studio::core::sensorMountKey;
    QCOMPARE(sensorMountKey("Fisheye", 1), QStringLiteral("Fisheye"));
  }

  void sensor_mount_key_instance_n_appends_hash() {
    using carla::studio::core::sensorMountKey;
    QCOMPARE(sensorMountKey("Fisheye", 4), QStringLiteral("Fisheye#4"));
    QCOMPARE(sensorMountKey("Lidar",   2), QStringLiteral("Lidar#2"));
  }

  void sensor_mount_key_non_positive_clamps_to_bare() {
    using carla::studio::core::sensorMountKey;
    QCOMPARE(sensorMountKey("Fisheye",  0), QStringLiteral("Fisheye"));
    QCOMPARE(sensorMountKey("Fisheye", -1), QStringLiteral("Fisheye"));
  }

  void player_slots_default_names_full_range() {
    using carla::studio::core::defaultPlayerName;
    QCOMPARE(defaultPlayerName(0),  QStringLiteral("EGO"));
    QCOMPARE(defaultPlayerName(1),  QStringLiteral("POV.01"));
    QCOMPARE(defaultPlayerName(2),  QStringLiteral("POV.02"));
    QCOMPARE(defaultPlayerName(3),  QStringLiteral("POV.03"));
    QCOMPARE(defaultPlayerName(4),  QStringLiteral("POV.04"));
    QCOMPARE(defaultPlayerName(5),  QStringLiteral("POV.05"));
    QCOMPARE(defaultPlayerName(6),  QStringLiteral("POV.06"));
    QCOMPARE(defaultPlayerName(7),  QStringLiteral("POV.07"));
    QCOMPARE(defaultPlayerName(8),  QStringLiteral("POV.08"));
    QCOMPARE(defaultPlayerName(9),  QStringLiteral("POV.09"));
    QCOMPARE(defaultPlayerName(10), QStringLiteral("POV.10"));
    QCOMPARE(defaultPlayerName(11), QStringLiteral("V2X.01"));
    QCOMPARE(defaultPlayerName(12), QStringLiteral("V2X.02"));
    QCOMPARE(defaultPlayerName(13), QStringLiteral("V2X.03"));
    QCOMPARE(defaultPlayerName(14), QStringLiteral("V2X.04"));
    QCOMPARE(defaultPlayerName(15), QStringLiteral("V2X.05"));
    QCOMPARE(defaultPlayerName(16), QStringLiteral("V2X.06"));
  }

  void player_slots_out_of_range_returns_empty() {
    using carla::studio::core::defaultPlayerName;
    QVERIFY(defaultPlayerName(-1).isEmpty());
    QVERIFY(defaultPlayerName(17).isEmpty());
    QVERIFY(defaultPlayerName(99).isEmpty());
  }

  void binary_smoke_startup_offscreen() {
    const QString binaryPath = qEnvironmentVariable("CARLA_STUDIO_BIN", QStringLiteral("./carla-studio"));
    if (!QFile::exists(binaryPath)) {
      QSKIP("carla-studio binary not found; set CARLA_STUDIO_BIN to run smoke test.", SkipSingle);
    }

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_QPA_PLATFORM", "offscreen");
    process.setProcessEnvironment(env);
    process.start(binaryPath, QStringList());

    QVERIFY2(process.waitForStarted(5000), "carla-studio failed to start");
    QTest::qWait(1200);
    QVERIFY2(process.state() == QProcess::Running, "carla-studio exited unexpectedly during smoke run");

    process.terminate();
    if (!process.waitForFinished(3000)) {
      process.kill();
      process.waitForFinished(3000);
    }
  }
};

QTEST_MAIN(CarlaStudioAppTest)
#include "CarlaStudioAppTest.moc"
