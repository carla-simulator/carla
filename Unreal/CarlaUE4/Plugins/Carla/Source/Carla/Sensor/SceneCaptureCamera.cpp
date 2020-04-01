// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include "Components/SceneCaptureComponent2D.h"

FActorDefinition ASceneCaptureCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
      TEXT("rgb"),
      bEnableModifyingPostProcessEffects);
}

void ASceneCaptureCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  SceneCapture.bCaptureEveryFrame = false;
  SceneCapture.bCaptureOnMovement = false;
}

void ASceneCaptureCamera::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (!bToken)
    return;

  // Suprisingly, originally RenderThread execute FWritePixels_SendPixelsInRenderThread
  // before CaptureCommand (analyzed using Optick), which means the client always see the previous frame.
  switch (CaptureMode) {
    case ECaptureMode::Always:
      CaptureScene();
      FPixelReader::SendPixelsInRenderThread(*this);
      break;
    case ECaptureMode::Step:
      if (bCapture)
      {
          CaptureScene();
          FPixelReader::SendPixelsInRenderThread(*this);
      }
      bCapture = !bCapture;
      break;
    case ECaptureMode::Never:
    default:
      break;
  }
}
