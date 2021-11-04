// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/InstanceSegmentationCamera.h"
#include "Carla/Game/TaggedComponent.h"

#include "Carla/Sensor/PixelReader.h"

#include "Components/SceneCaptureComponent2D.h"

FActorDefinition AInstanceSegmentationCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("instance_segmentation"));
}

AInstanceSegmentationCamera::AInstanceSegmentationCamera(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &AInstanceSegmentationCamera::OnActorSpawned));
}

void AInstanceSegmentationCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);

  ApplyViewMode(VMI_Unlit, true, SceneCapture.ShowFlags);

  SceneCapture.ShowFlags.SetNotDrawTaggedComponents(false); // TaggedComponent detects this and sets view relevance for proxy material
  //SceneCapture.ShowFlags.SetDecals(false);
  //SceneCapture.ShowFlags.SetPostProcessing(false);
  //SceneCapture.ShowFlags.SetAmbientCubemap(false);
  //SceneCapture.ShowFlags.SetVignette(false);
  //SceneCapture.ShowFlags.SetRectLights(false);
  //SceneCapture.ShowFlags.SetToneCurve(false);
  //SceneCapture.ShowFlags.SetScreenPercentage(false);
  //SceneCapture.ShowFlags.SetReflectionEnvironment(false);
  //SceneCapture.ShowFlags.SetSpecular(false);
  //SceneCapture.ShowFlags.SetContactShadows(false);
  //SceneCapture.ShowFlags.SetRayTracedDistanceFieldShadows(false);
  //SceneCapture.ShowFlags.SetCapsuleShadows(false);
  //SceneCapture.ShowFlags.SetVolumetricLightmap(false);
  //SceneCapture.ShowFlags.SetIndirectLightingCache(false);
  //SceneCapture.ShowFlags.SetTexturedLightProfiles(false);
  //SceneCapture.ShowFlags.SetDeferredLighting(false);
  //SceneCapture.ShowFlags.SetTranslucency(false);
  //SceneCapture.ShowFlags.SetBillboardSprites(false);
  //SceneCapture.ShowFlags.SetBSPTriangles(false); // hmm
  //SceneCapture.ShowFlags.SetLandscape(false); // hmm
  //SceneCapture.ShowFlags.SetBSP(false); // hmm
  //SceneCapture.ShowFlags.SetPostProcessMaterial(false); // hmm
  SceneCapture.ShowFlags.SetAtmosphere(false);
  //SceneCapture.ShowFlags.SetPrecomputedVisibility(false);
  //SceneCapture.ShowFlags.SetPaper2DSprites(false);
  //SceneCapture.ShowFlags.SetDistanceFieldAO(false);
  //SceneCapture.ShowFlags.SetWidgetComponents(false);
  //SceneCapture.ShowFlags.SetMediaPlanes(false);

  //SceneCapture.PostProcessSettings.AutoExposureBias = 0;

  SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

  TArray<UObject *> TaggedComponents;
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  UE_LOG(LogCarla, Warning, TEXT("AInstanceSegmentationCamera::SetUpSceneCaptureComponent with %d tagged components!"), TaggedComponents.Num());

  TArray<UPrimitiveComponent *> ShowOnlyComponents;
  for (UObject *Object : TaggedComponents) {
    UPrimitiveComponent *Component = Cast<UPrimitiveComponent>(Object);
    SceneCapture.ShowOnlyComponents.Emplace(Component);
  }
}

void AInstanceSegmentationCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera::PostPhysTick);

  USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();
  TArray<UObject *> TaggedComponents;
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  // UE_LOG(LogCarla, Warning, TEXT("AInstanceSegmentationCamera::SetUpSceneCaptureComponent with %d tagged components!"), TaggedComponents.Num());
  SceneCapture->ClearShowOnlyComponents();
  for (UObject *Object : TaggedComponents) {
    UPrimitiveComponent *Component = Cast<UPrimitiveComponent>(Object);
    SceneCapture->ShowOnlyComponents.Emplace(Component);
  }

  FPixelReader::SendPixelsInRenderThread(*this);

}
