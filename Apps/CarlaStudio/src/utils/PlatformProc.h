// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QString>
#include <QThread>

#include <cerrno>

#if defined(Q_OS_MAC) || defined(__APPLE__)
#include <csignal>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(Q_OS_LINUX) || defined(__linux__)
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace carla_studio_proc {

inline qint64 systemTotalRamMiB() {
#if defined(Q_OS_MAC) || defined(__APPLE__)
  uint64_t bytes = 0;
  size_t   len   = sizeof(bytes);
  if (sysctlbyname("hw.memsize", &bytes, &len, nullptr, 0) == 0)
    return static_cast<qint64>(bytes / (1024ULL * 1024ULL));
  return 0;
#else
  QFile f(QStringLiteral("/proc/meminfo"));
  if (!f.open(QIODevice::ReadOnly)) return 0;
  while (!f.atEnd()) {
    const QByteArray line = f.readLine();
    if (line.startsWith("MemTotal:")) {
      const QList<QByteArray> p = line.simplified().split(' ');
      if (p.size() >= 2) return p[1].toLongLong() / 1024;
    }
  }
  return 0;
#endif
}

inline QString cpuModelString() {
#if defined(Q_OS_MAC) || defined(__APPLE__)
  char   buf[256] = {0};
  size_t len      = sizeof(buf);
  if (sysctlbyname("machdep.cpu.brand_string", buf, &len, nullptr, 0) == 0)
    return QString::fromLatin1(buf);
  return {};
#else
  QFile f(QStringLiteral("/proc/cpuinfo"));
  if (!f.open(QIODevice::ReadOnly)) return {};
  while (!f.atEnd()) {
    const QByteArray line = f.readLine();
    const int colon = line.indexOf(':');
    if (colon < 0) continue;
    const QByteArray key = line.left(colon).trimmed();
    if (key == "model name")
      return QString::fromLocal8Bit(line.mid(colon + 1).trimmed());
  }
  return {};
#endif
}

inline int cpuLogicalCount() {
#if defined(Q_OS_MAC) || defined(__APPLE__)
  int n = 0;
  size_t len = sizeof(n);
  if (sysctlbyname("hw.logicalcpu", &n, &len, nullptr, 0) == 0 && n > 0)
    return n;
#endif
  return QThread::idealThreadCount();
}

inline double cpuClockGHz() {
#if defined(Q_OS_MAC) || defined(__APPLE__)
  uint64_t hz = 0;
  size_t   sz = sizeof(hz);
  if (sysctlbyname("hw.cpufrequency", &hz, &sz, nullptr, 0) == 0 && hz > 0)
    return static_cast<double>(hz) / 1.0e9;
  return 0.0;
#else
  QFile f(QStringLiteral("/proc/cpuinfo"));
  if (!f.open(QIODevice::ReadOnly)) return 0.0;
  while (!f.atEnd()) {
    const QByteArray line = f.readLine();
    if (line.startsWith("cpu MHz")) {
      const int colon = line.indexOf(':');
      if (colon < 0) continue;
      bool ok = false;
      const double mhz = QByteArray(line.mid(colon + 1)).trimmed().toDouble(&ok);
      if (ok && mhz > 0.0) return mhz / 1000.0;
    }
  }
  return 0.0;
#endif
}

inline bool pidIsAlive(qint64 pid) {
  if (pid <= 0) return false;
#if defined(Q_OS_LINUX) || defined(__linux__)
  return QFileInfo(QStringLiteral("/proc/%1").arg(pid)).exists();
#elif defined(Q_OS_MAC) || defined(__APPLE__) || defined(Q_OS_UNIX)
  if (::kill(static_cast<pid_t>(pid), 0) == 0) return true;
  return errno != ESRCH;
#else
  Q_UNUSED(pid);
  return true;
#endif
}

inline bool platformHasNvidiaSmi() {
#if defined(Q_OS_MAC) || defined(__APPLE__)
  return false;
#else
  return true;
#endif
}

}  // namespace carla_studio_proc
