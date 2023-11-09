#include "Carla.h"
#include "Carla/Sensor/OpticalFlowCamera.h"
#include "HAL/IConsoleManager.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition AOpticalFlowCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("optical_flow"));
}

AOpticalFlowCamera::AOpticalFlowCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Enable16BitFormat(true);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
          TEXT("Material'/Carla/PostProcessingMaterials/VelocityMaterial.VelocityMaterial'"));
}

void AOpticalFlowCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  auto CVarForceOutputsVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BasePassForceOutputsVelocity"));
  int32 OldValue = CVarForceOutputsVelocity->GetInt();
  CVarForceOutputsVelocity->Set(1);

  std::function<TArray<float>(void *, uint32)> Conversor = [](void *Data, uint32 Size)
  {
    TArray<float> IntermediateBuffer;
    int32 Count = Size / sizeof(FFloat16Color);
    DEBUG_ASSERT(Count * sizeof(FFloat16Color) == Size);
    FFloat16Color *Buf = reinterpret_cast<FFloat16Color *>(Data);
    IntermediateBuffer.Reserve(Count * 2);
    for (int i=0; i<Count; ++i)
    {
      float x = (Buf->R.GetFloat() - 0.5f) * 4.f;
      float y = (Buf->G.GetFloat() - 0.5f) * 4.f;
      IntermediateBuffer.Add(x);
      IntermediateBuffer.Add(y);
      ++Buf;
    }
    return IntermediateBuffer;
  };
  FPixelReader::SendPixelsInRenderThread<AOpticalFlowCamera, float>(*this, true, Conversor);
  
  CVarForceOutputsVelocity->Set(OldValue);
}
