// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include <carla/geom/Mesh.h>
#include <carla/road/Road.h>
#include <carla/road/LaneSection.h>
#include <carla/road/Lane.h>
#include <carla/rpc/OpendriveGenerationParameters.h>

namespace carla {
namespace geom {
namespace deformation {
  inline float GetZPosInDeformation(float posx, float posy){
    // Amplitud
    const float A1 = 0.6f;
    const float A2 = 1.1f;
    // Fases
    const float F1 = 1000.0;
    const float F2 = -1500.0;
    // Modifiers
    const float Kx1 = 0.035f;
    const float Kx2 = 0.02f;

    const float Ky1 = -0.08f;
    const float Ky2 = 0.05f;

    return A1 * sin((Kx1 * posx + Ky1 * posy + F1)) +
      A2 * sin((Kx2 * posx + Ky2 * posy + F2));
  }

  inline float GetBumpDeformation(float posx, float posy){
    const float A3 = 0.15f;
    float bumpsoffset = 0;

    const float constraintX = 15.0f;
    const float constraintY = 15.0f;

    float BumpX = std::round(posx / constraintX);
    float BumpY = std::round(posy / constraintX);

    BumpX *= constraintX;
    BumpY *= constraintY;

    float DistanceToBumpOrigin = sqrt(pow(BumpX - posx, 2) + pow(BumpY - posy, 2) );
    float MaxDistance = 2.5;

    if (DistanceToBumpOrigin <= MaxDistance) {
      bumpsoffset = abs((1.0f / MaxDistance) * DistanceToBumpOrigin * DistanceToBumpOrigin - MaxDistance);
    }

    return A3 * bumpsoffset;
  }



} // namespace deformation
} // namespace geom
} // namespace carla
