// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/RTLensEngineAdapter.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "HAL/IConsoleManager.h"
#include <util/ue-header-guard-end.h>

namespace RTLensEngineAdapter
{
namespace
{
  // CVars are looked up by name every call rather than cached: IConsoleVariable
  // pointers are only guaranteed valid once the owning module has registered
  // them, and this adapter must not crash on an engine build that predates
  // rt-lens-engine's PathTracing.cpp change (it just warns and no-ops).
  IConsoleVariable *FindCVarOrWarn(const TCHAR *Name)
  {
    IConsoleVariable *CVar = IConsoleManager::Get().FindConsoleVariable(Name);
    if (CVar == nullptr)
    {
      UE_LOG(LogCarla, Warning,
          TEXT("RTLensEngineAdapter: CVar '%s' not found -- this engine build may ")
          TEXT("predate rt-lens-engine's path-tracer lens model. sensor.camera.rt_lens ")
          TEXT("will render with the stock pinhole projection."),
          Name);
    }
    return CVar;
  }
} // namespace

void ApplyLensModel(const FLensModelDescriptor &Descriptor)
{
  if (IConsoleVariable *CVar = FindCVarOrWarn(TEXT("r.PathTracing.LensModel")))
  {
    CVar->Set(*FString::Printf(TEXT("%d"), (int32)Descriptor.Model), ECVF_SetByCode);
  }

  if (IConsoleVariable *CVar = FindCVarOrWarn(TEXT("r.PathTracing.LensFocal")))
  {
    CVar->Set(*FString::Printf(TEXT("%f,%f,%f,%f"),
        Descriptor.FocalX, Descriptor.FocalY, Descriptor.CenterX, Descriptor.CenterY),
        ECVF_SetByCode);
  }

  if (IConsoleVariable *CVar = FindCVarOrWarn(TEXT("r.PathTracing.LensThetaMax")))
  {
    CVar->Set(*FString::SanitizeFloat(Descriptor.ThetaMax), ECVF_SetByCode);
  }

  if (IConsoleVariable *CVar = FindCVarOrWarn(TEXT("r.PathTracing.LensCAScale")))
  {
    CVar->Set(*FString::Printf(TEXT("%f,%f"), Descriptor.CAScaleR, Descriptor.CAScaleB), ECVF_SetByCode);
  }

  if (IConsoleVariable *CVar = FindCVarOrWarn(TEXT("r.PathTracing.LensCoeffs")))
  {
    // Model-specific packing (see CameraModelUtil.h): KannalaBrandt uses
    // Coeffs[0..3], BrownConrady uses Coeffs[0..4]. The CVar always carries 8
    // slots; unused trailing slots are zero-filled.
    FString CSV;
    for (int32 i = 0; i != 8; ++i)
    {
      if (i != 0)
        CSV += TEXT(",");
      CSV += FString::SanitizeFloat(Descriptor.Coeffs.IsValidIndex(i) ? Descriptor.Coeffs[i] : 0.0f);
    }
    CVar->Set(*CSV, ECVF_SetByCode);
  }

  if (Descriptor.Model == ECameraModel::LUT1D && Descriptor.LUT.Num() > 0)
  {
    // TODO(rt-lens-engine): there is no CVar-based upload path for
    // FLensModelDescriptor::LUT yet (a resource, not a handful of scalars --
    // doesn't fit the r.PathTracing.Lens* CVar pattern). LUT1D cameras
    // currently render with whatever LensModels.ush falls back to when no LUT
    // is bound; wire this up once the engine side exposes a per-view LUT
    // resource (e.g. a texture/buffer on FSceneView) instead of a CVar.
    UE_LOG(LogCarla, Warning,
        TEXT("RTLensEngineAdapter: camera_model=lut has no CVar upload path for its LUT ")
        TEXT("samples yet; the path tracer will not see them."));
  }
}

} // namespace RTLensEngineAdapter
