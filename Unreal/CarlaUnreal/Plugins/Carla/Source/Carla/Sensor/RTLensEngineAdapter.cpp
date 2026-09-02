// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/RTLensEngineAdapter.h"
#include "Carla.h"

namespace RTLensEngineAdapter
{

void ApplyLensModel(FPostProcessSettings &PostProcessSettings, const FLensModelDescriptor &Descriptor)
{
  PostProcessSettings.bOverride_PathTracingLensModel = true;
  PostProcessSettings.bOverride_PathTracingLensFocal = true;
  PostProcessSettings.bOverride_PathTracingLensCoeffs01 = true;
  PostProcessSettings.bOverride_PathTracingLensCoeffs23 = true;
  PostProcessSettings.bOverride_PathTracingLensThetaMax = true;
  PostProcessSettings.bOverride_PathTracingLensCAScale = true;

  PostProcessSettings.PathTracingLensModel = (int32)Descriptor.Model;

  PostProcessSettings.PathTracingLensFocal = FVector4(
      Descriptor.FocalX, Descriptor.FocalY, Descriptor.CenterX, Descriptor.CenterY);

  // Model-specific packing (see CameraModelUtil.h): KannalaBrandt uses
  // Coeffs[0..3], BrownConrady uses Coeffs[0..4] (p2 lands in Coeffs23.X, see
  // the Scene.h field comment). Unused trailing slots are zero-filled.
  const auto CoeffAt = [&Descriptor](int32 i) -> float
  {
    return Descriptor.Coeffs.IsValidIndex(i) ? Descriptor.Coeffs[i] : 0.0f;
  };
  PostProcessSettings.PathTracingLensCoeffs01 = FVector4(
      CoeffAt(0), CoeffAt(1), CoeffAt(2), CoeffAt(3));
  PostProcessSettings.PathTracingLensCoeffs23 = FVector4(
      CoeffAt(4), CoeffAt(5), CoeffAt(6), CoeffAt(7));

  PostProcessSettings.PathTracingLensThetaMax = Descriptor.ThetaMax;
  PostProcessSettings.PathTracingLensCAScale = FVector2f(Descriptor.CAScaleR, Descriptor.CAScaleB);

  if (Descriptor.Model == ECameraModel::LUT1D && Descriptor.LUT.Num() > 0)
  {
    // TODO(rt-lens-engine): FPostProcessSettings has no per-view LUT resource
    // slot yet (a buffer, not a handful of scalars -- doesn't fit the
    // PathTracingLens* field pattern). LUT1D cameras currently render with
    // whatever LensModels.ush falls back to when no LUT is bound.
    UE_LOG(LogCarla, Warning,
        TEXT("RTLensEngineAdapter: camera_model=lut has no per-view upload path for its LUT ")
        TEXT("samples yet; the path tracer will not see them."));
  }
}

} // namespace RTLensEngineAdapter
