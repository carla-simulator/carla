#include "Carla.h"
#include "Carla/Sensor/OpticalFlowCamera.h"

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
//  AddPostProcessingMaterial(
//#if PLATFORM_LINUX
//      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
//#else
//      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
//#endif
//  );
  AddPostProcessingMaterial(
          TEXT("Material'/Carla/PostProcessingMaterials/VelocityMaterial.VelocityMaterial'"));
}

void AOpticalFlowCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  FPixelReader::SendPixelsInRenderThread(*this, true);
}
