// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
    const float A3 = 0.10f;
    float bumpsoffset = 0;

    float constraintX = 17.0f;
    float constraintY = 12.0f;

    float BumpX = std::ceil(posx / constraintX);
    float BumpY = std::floor(posy / constraintY);

    BumpX *= constraintX;
    BumpY *= constraintY;

    float DistanceToBumpOrigin = sqrt(pow(BumpX - posx, 2) + pow(BumpY - posy, 2) );
    float MaxDistance = 2.0;

    if (DistanceToBumpOrigin <= MaxDistance) {
      bumpsoffset = sin(DistanceToBumpOrigin);
    }

    return A3 * bumpsoffset;
  }



} // namespace deformation
} // namespace geom
} // namespace carla
