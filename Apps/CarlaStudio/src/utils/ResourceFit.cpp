// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "ResourceFit.h"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QSysInfo>
#include <QTextStream>

#include <algorithm>

namespace carla_studio::utils {

namespace detail {

QString runCmd(const QString &cmd, const QStringList &args, int timeoutMs) {
  QProcess p;
  p.setProcessChannelMode(QProcess::SeparateChannels);
  p.start(cmd, args);
  bool finished = p.waitForFinished(timeoutMs);
  if (!finished) {
    p.terminate();
    if (!p.waitForFinished(1500)) {
      p.kill();
      p.waitForFinished(-1);
    }
    return QString();
  }
  if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) return QString();
  return QString::fromLocal8Bit(p.readAllStandardOutput());
}

QString readFile(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
  return QString::fromLocal8Bit(f.readAll());
}

QString detectOsPretty() {
  const QString text = readFile("/etc/os-release");
  for (const QString &line : text.split('\n')) {
    if (!line.startsWith("PRETTY_NAME=")) continue;
    QString v = line.mid(QString("PRETTY_NAME=").size()).trimmed();
    if (v.startsWith('"') && v.endsWith('"')) v = v.mid(1, v.size() - 2);
    return v;
  }
  return QSysInfo::prettyProductName();
}

void detectCpu(QString *model, int *logical, int *physical) {
  const QString text = readFile("/proc/cpuinfo");
  QStringList lines = text.split('\n');
  *logical = 0;
  for (const QString &line : lines) {
    if (line.startsWith("processor")) (*logical)++;
    if (model->isEmpty() && line.startsWith("model name")) {
      const int colon = line.indexOf(':');
      if (colon > 0) *model = line.mid(colon + 1).trimmed();
    }
  }
  for (const QString &line : lines) {
    if (line.startsWith("cpu cores")) {
      const int colon = line.indexOf(':');
      if (colon > 0) {
        bool ok = false;
        const int n = line.mid(colon + 1).trimmed().toInt(&ok);
        if (ok) { *physical = n; break; }
      }
    }
  }
  if (*physical == 0) *physical = *logical;
}

void detectRam(std::int64_t *total, std::int64_t *free) {
  const QString text = readFile("/proc/meminfo");
  for (const QString &line : text.split('\n')) {
    QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 2) continue;
    if (parts[0] == "MemTotal:")
      *total = static_cast<std::int64_t>(parts[1].toLongLong()) * 1024;
    else if (parts[0] == "MemAvailable:")
      *free  = static_cast<std::int64_t>(parts[1].toLongLong()) * 1024;
  }
}

}

namespace {

using detail::runCmd;
using detail::readFile;
using detail::detectOsPretty;
using detail::detectCpu;
using detail::detectRam;

std::vector<GpuInfo> detectGpus(QString *driverOut, QString *cudaOut) {
  std::vector<GpuInfo> out;

  const QString nvidia = runCmd("nvidia-smi", {
    "--query-gpu=name,memory.total,compute_cap,driver_version",
    "--format=csv,noheader,nounits",
  });
  if (!nvidia.isEmpty()) {
    for (const QString &line : nvidia.split('\n')) {
      if (line.trimmed().isEmpty()) continue;
      QStringList parts = line.split(',');
      if (parts.size() < 2) continue;
      GpuInfo g;
      g.name        = parts[0].trimmed();
      g.vramBytes   = static_cast<std::int64_t>(parts[1].trimmed().toLongLong()) * 1024 * 1024;
      g.cudaCapability = parts.size() > 2 ? parts[2].trimmed() : QString();
      g.driverVersion  = parts.size() > 3 ? parts[3].trimmed() : QString();
      g.isNvidia    = true;
      out.push_back(g);
      if (driverOut->isEmpty()) *driverOut = g.driverVersion;
    }
  }

  const QString nvcc = runCmd("nvcc", {"--version"});
  if (!nvcc.isEmpty()) {
    const QRegularExpression re("release\\s+([0-9]+\\.[0-9]+)");
    const auto m = re.match(nvcc);
    if (m.hasMatch()) *cudaOut = m.captured(1);
  }

  if (out.empty()) {
    const QString lspci = runCmd("lspci", {});
    for (const QString &line : lspci.split('\n')) {
      const QString lower = line.toLower();
      if (lower.contains("vga") || lower.contains("3d controller")) {
        const int colon = line.indexOf(':', line.indexOf(':') + 1);
        if (colon > 0) {
          GpuInfo g;
          g.name = line.mid(colon + 1).trimmed();
          out.push_back(g);
        }
      }
    }
  }
  return out;
}

std::vector<VolumeInfo> detectVolumes() {
  std::vector<VolumeInfo> out;
  for (const QStorageInfo &si : QStorageInfo::mountedVolumes()) {
    if (!si.isValid() || !si.isReady()) continue;
    VolumeInfo v;
    v.mountPoint  = si.rootPath();
    v.label       = si.displayName();
    v.fileSystem  = QString::fromLocal8Bit(si.fileSystemType());
    v.totalBytes  = si.bytesTotal();
    v.freeBytes   = si.bytesAvailable();
    v.isReadOnly  = si.isReadOnly();
    const QString fs = v.fileSystem.toLower();
    v.isExternal =
        v.mountPoint != "/" &&
        fs != "tmpfs" && fs != "devtmpfs" && fs != "proc" &&
        fs != "sysfs" && fs != "cgroup" && fs != "cgroup2";
    out.push_back(v);
  }
  std::sort(out.begin(), out.end(), [](const VolumeInfo &a, const VolumeInfo &b) {
    if ((a.mountPoint == "/") != (b.mountPoint == "/")) return a.mountPoint == "/";
    if (a.isExternal != b.isExternal) return a.isExternal > b.isExternal;
    return a.mountPoint < b.mountPoint;
  });
  return out;
}

}

SystemProbe::SystemProbe() {
  osPretty_ = detectOsPretty();
  kernel_   = QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
  detectCpu(&cpuModel_, &cpuLogicalCores_, &cpuPhysicalCores_);
  detectRam(&ramTotalBytes_, &ramFreeBytes_);
  gpus_ = detectGpus(&nvidiaDriverVersion_, &cudaToolkitVersion_);
  volumes_ = detectVolumes();
}

const GpuInfo *SystemProbe::primaryGpu() const {
  if (gpus_.empty()) return nullptr;
  for (const auto &g : gpus_)
    if (g.isNvidia) return &g;
  return &gpus_.front();
}

std::int64_t SystemProbe::bestVramBytes() const {
  std::int64_t best = 0;
  for (const auto &g : gpus_) best = std::max(best, g.vramBytes);
  return best;
}

std::int64_t SystemProbe::freeBytesAt(const QString &path) const {
  QStorageInfo si(path);
  if (!si.isValid() || !si.isReady()) return 0;
  return si.bytesAvailable();
}

namespace {

constexpr std::int64_t GiB = 1024LL * 1024 * 1024;

bool cudaAtLeast(const QString &cuda, const QString &min) {
  auto parts = [](const QString &s) -> QPair<int, int> {
    const QStringList p = s.split('.');
    if (p.isEmpty()) return {0, 0};
    return {p.value(0).toInt(), p.size() > 1 ? p.value(1).toInt() : 0};
  };
  const auto a = parts(cuda);
  const auto b = parts(min);
  if (a.first != b.first) return a.first > b.first;
  return a.second >= b.second;
}

}

FitResult evaluate(Tool tool, const SystemProbe &probe) {
  const std::int64_t vram = probe.bestVramBytes();
  const std::int64_t ram  = probe.ramTotalBytes();
  const QString      cuda = probe.cudaToolkitVersion();
  const bool nvidiaPresent = probe.primaryGpu() && probe.primaryGpu()->isNvidia;

  FitResult r;

  switch (tool) {
    case Tool::CarlaServer:
      r.minimumSpecPretty = "≥ 4 GB VRAM at Low quality";
      if (vram < 4 * GiB) {
        r.verdict = Eligibility::SoftWarn;
        r.reasons << QString("Detected VRAM: %1; CARLA recommends ≥ 4 GB at Low quality.")
                       .arg(humanBytes(vram));
        r.suggestion = "Pick the Low quality preset under Cfg → Render.";
      }
      return r;

    case Tool::NuRec:
      r.minimumSpecPretty = "≥ 24 GB VRAM, CUDA ≥ 12.8, NVIDIA GPU";
      if (!nvidiaPresent) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << "No NVIDIA GPU detected; NuRec runs only on CUDA hardware.";
        r.suggestion = "Switch Compute → Cloud (Brev) for an A100 / L40S / H100 instance.";
        return r;
      }
      if (vram < 24 * GiB) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << QString("Detected VRAM: %1; NuRec needs ≥ 24 GB.").arg(humanBytes(vram));
        r.suggestion = "Switch Compute → Cloud (Brev) for an A100 / L40S / H100 instance.";
        return r;
      }
      if (!cuda.isEmpty() && !cudaAtLeast(cuda, "12.8")) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << QString("CUDA toolkit %1 detected; NuRec requires ≥ 12.8.").arg(cuda);
        r.suggestion = "Install CUDA 12.8+ or switch Compute → Cloud (Brev).";
      }
      return r;

    case Tool::AlpamayoR1:
      r.minimumSpecPretty = "≥ 24 GB VRAM (model load) + ≥ 4 GB headroom (inference)";
      if (!nvidiaPresent || vram < 24 * GiB) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << QString("Alpamayo R1 (10B params) needs ≥ 24 GB VRAM; detected: %1.")
                       .arg(humanBytes(vram));
        r.suggestion = "Switch Compute → Cloud (Brev). Local inference isn't viable at this size.";
      }
      return r;

    case Tool::AlpaSimRenderer:
      r.minimumSpecPretty = "≥ 16 GB VRAM";
      if (vram < 16 * GiB) {
        r.verdict = Eligibility::SoftWarn;
        r.reasons << QString("Detected VRAM: %1; AlpaSim works at smaller scenes but rendering "
                             "may stall on larger reconstructed scenes.").arg(humanBytes(vram));
        r.suggestion = "Use scenes ≤ 20 s or switch Compute → Cloud (Brev).";
      }
      return r;

    case Tool::Lyra2Generation:
      r.minimumSpecPretty = "≥ 24 GB VRAM (generation pipeline)";
      if (!nvidiaPresent || vram < 24 * GiB) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << QString("Lyra 2.0 generation needs ≥ 24 GB VRAM; detected: %1.")
                       .arg(humanBytes(vram));
        r.suggestion = "Switch Compute → Cloud (Brev) for generation; playback works with less.";
      }
      return r;

    case Tool::Lyra2Playback:
      r.minimumSpecPretty = "≥ 12 GB VRAM (playback only)";
      if (vram < 12 * GiB) {
        r.verdict = Eligibility::SoftWarn;
        r.reasons << QString("Detected VRAM: %1; playback is supported but expect frame drops.")
                       .arg(humanBytes(vram));
      }
      return r;

    case Tool::AssetHarvester:
      r.minimumSpecPretty = "≥ 16 GB VRAM";
      if (vram < 16 * GiB) {
        r.verdict = Eligibility::SoftWarn;
        r.reasons << QString("Detected VRAM: %1; Asset-Harvester runs but expect long iteration "
                             "times on heavy scenes.").arg(humanBytes(vram));
      }
      return r;

    case Tool::AutowareBuild: {
      r.minimumSpecPretty = "Ubuntu 20.04 / 22.04, ≥ 16 GB RAM, ≥ 50 GB free disk";
      const QString os = probe.osPretty().toLower();
      const bool ubuntu2004 = os.contains("ubuntu 20.04") || os.contains("ubuntu 22.04");
      if (!ubuntu2004) {
        r.verdict = Eligibility::HardGrey;
        r.reasons << QString("Autoware source build is gated to Ubuntu 20.04 / 22.04. Detected: %1.")
                       .arg(probe.osPretty());
        r.suggestion = "Use the prebuilt apt packages toggle, or build inside an Ubuntu container.";
        return r;
      }
      if (ram < 16 * GiB) {
        r.verdict = Eligibility::SoftWarn;
        r.reasons << QString("Detected RAM: %1; Autoware's colcon build commonly OOMs under 16 GB.")
                       .arg(humanBytes(ram));
        r.suggestion = "Lower MAKEFLAGS=-j2 or use the prebuilt apt packages toggle.";
      }
      return r;
    }

    case Tool::TeraSim:
      r.minimumSpecPretty = "Redis available locally";
      return r;
  }

  return r;
}

QString humanBytes(std::int64_t bytes) {
  if (bytes <= 0) return QStringLiteral("—");
  static const char *units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
  double v = static_cast<double>(bytes);
  int idx = 0;
  while (v >= 1024.0 && idx < 5) { v /= 1024.0; ++idx; }
  if (idx == 0) return QString::number(bytes) + " B";
  return QString::number(v, 'f', v < 10.0 ? 2 : (v < 100.0 ? 1 : 0)) + " " + units[idx];
}

}
