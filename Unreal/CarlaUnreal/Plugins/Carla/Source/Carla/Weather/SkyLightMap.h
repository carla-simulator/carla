// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

class UTextureCube;

/// Environment-map support for the sky light (the `set_sky_light_map` RPC).
///
/// Convention of the input panorama (equirectangular, CARLA world frame,
/// which is Unreal's left-handed X forward / Y right / Z up):
///   column u in [0, W): azimuth phi = 2*pi * (u + 0.5) / W, measured from +X
///     toward +Y (the same sense as a CARLA yaw: 0 = +X, 90 deg = +Y).
///   row v in [0, H): elevation theta = pi/2 - pi * (v + 0.5) / H
///     (row 0 = zenith, last row = nadir).
///   direction d = (cos(theta) cos(phi), cos(theta) sin(phi), sin(theta)).
/// Pixels are linear RGBA float, row-major, top row first. Values are scene
/// radiance in whatever unit the caller wants; the sky light's Intensity is
/// the multiplier on top.
namespace CarlaSkyLightMap
{
  /// Resamples the panorama into a transient 6-face HDR cubemap
  /// (PF_FloatRGBA, FaceSize x FaceSize per face) laid out in Unreal's
  /// cubemap face convention (+X, -X, +Y, -Y, +Z, -Z, see GetCubemapVector in
  /// ReflectionEnvironmentShaders.usf), so that sampling the cube with a
  /// world direction returns the panorama value for that direction.
  /// Returns nullptr on bad input. The texture is NOT rooted: the caller owns
  /// keeping it alive.
  CARLA_API UTextureCube* CreateCubemapFromEquirect(
      const float* RGBA, int32 Width, int32 Height, int32 FaceSize = 512);
}
