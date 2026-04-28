// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QString>

namespace carla_studio::vehicle_import {

struct MeshAABB {
  float xMin =  1e9f, xMax = -1e9f;
  float yMin =  1e9f, yMax = -1e9f;
  float zMin =  1e9f, zMax = -1e9f;
  bool  valid = false;

  float scaleToCm   = 1.0f;
  int   upAxis      = 2;
  int   forwardAxis = 0;

  void feed(float x, float y, float z);
  void detectConventions(const QString &ext);
  void toUE(float &xLo, float &xHi,
            float &yLo, float &yHi,
            float &zLo, float &zHi) const;
};

MeshAABB parseOBJ (const QString &path);
MeshAABB parseGLTF(const QString &path);
MeshAABB parseDAE (const QString &path);

}  // namespace carla_studio::vehicle_import
