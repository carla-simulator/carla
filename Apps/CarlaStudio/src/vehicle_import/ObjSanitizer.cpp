// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/ObjSanitizer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace carla_studio::vehicle_import {

SanitizeReport sanitizeOBJ(const QString &inputPath, float scaleToCm) {
  SanitizeReport rep;
  QFile in(inputPath);
  if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) return rep;

  rep.outputPath = QDir::tempPath() + "/" +
                   QFileInfo(inputPath).completeBaseName() + "_sanitized.obj";
  QFile out(rep.outputPath);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return rep;

  QTextStream sIn(&in), sOut(&out);
  sOut.setRealNumberPrecision(7);

  while (!sIn.atEnd()) {
    const QString line = sIn.readLine();
    if (line.startsWith("v ") || line.startsWith("v\t")) {
      const QStringList tok = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (tok.size() >= 4) {
        bool okx, oky, okz;
        const float x = tok[1].toFloat(&okx);
        const float y = tok[2].toFloat(&oky);
        const float z = tok[3].toFloat(&okz);
        if (okx && oky && okz) {
          sOut << "v " << (x * scaleToCm) << ' '
                       << (y * scaleToCm) << ' '
                       << (z * scaleToCm) << '\n';
          continue;
        }
      }
    }
    if (line.startsWith("f ") || line.startsWith("f\t")) {
      const QStringList tok = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (tok.size() < 4) { ++rep.skippedFaceLines; continue; }
    }
    sOut << line << '\n';
  }

  rep.ok = true;
  return rep;
}

}  // namespace carla_studio::vehicle_import
