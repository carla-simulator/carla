// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

#include <cstdint>
#include <optional>
#include <vector>

namespace carla_studio::utils {

namespace detail {

QString runCmd(const QString &cmd, const QStringList &args, int timeoutMs = 3000);
QString readFile(const QString &path);
QString detectOsPretty();
void detectCpu(QString *model, int *logical, int *physical);
void detectRam(std::int64_t *total, std::int64_t *free);

}

struct GpuInfo {
  QString name;
  std::int64_t vramBytes = 0;
  QString cudaCapability;
  QString driverVersion;
  bool isNvidia = false;
};

struct VolumeInfo {
  QString mountPoint;
  QString label;
  QString fileSystem;
  std::int64_t totalBytes = 0;
  std::int64_t freeBytes = 0;
  bool isReadOnly = false;
  bool isExternal = false;
};

class SystemProbe {
public:
  SystemProbe();

  const QString &osPretty() const { return osPretty_; }
  const QString &kernel() const { return kernel_; }

  const QString &cpuModel() const { return cpuModel_; }
  int cpuLogicalCores() const { return cpuLogicalCores_; }
  int cpuPhysicalCores() const { return cpuPhysicalCores_; }

  std::int64_t ramTotalBytes() const { return ramTotalBytes_; }
  std::int64_t ramFreeBytes() const { return ramFreeBytes_; }

  const std::vector<GpuInfo> &gpus() const { return gpus_; }
  const GpuInfo *primaryGpu() const;
  std::int64_t bestVramBytes() const;

  const QString &cudaToolkitVersion() const { return cudaToolkitVersion_; }
  const QString &nvidiaDriverVersion() const { return nvidiaDriverVersion_; }

  const std::vector<VolumeInfo> &volumes() const { return volumes_; }
  std::int64_t freeBytesAt(const QString &path) const;

private:
  QString osPretty_;
  QString kernel_;
  QString cpuModel_;
  int     cpuLogicalCores_  = 0;
  int     cpuPhysicalCores_ = 0;
  std::int64_t ramTotalBytes_ = 0;
  std::int64_t ramFreeBytes_  = 0;
  std::vector<GpuInfo>    gpus_;
  QString cudaToolkitVersion_;
  QString nvidiaDriverVersion_;
  std::vector<VolumeInfo> volumes_;
};

enum class Tool {
  CarlaServer,
  NuRec,
  AlpamayoR1,
  AlpaSimRenderer,
  Lyra2Generation,
  Lyra2Playback,
  AssetHarvester,
  AutowareBuild,
  TeraSim,
};

enum class Eligibility { Eligible, SoftWarn, HardGrey };

struct FitResult {
  Eligibility verdict = Eligibility::Eligible;
  QStringList reasons;
  QString     suggestion;
  QString     minimumSpecPretty;
};

FitResult evaluate(Tool tool, const SystemProbe &probe);

QString humanBytes(std::int64_t bytes);

}
