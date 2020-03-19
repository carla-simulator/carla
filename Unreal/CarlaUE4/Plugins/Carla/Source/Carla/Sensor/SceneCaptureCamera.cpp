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

  // Suprisingly, originally RenderThread execute FWritePixels_SendPixelsInRenderThread
  // before CaptureCommand (analyzed using Optick), which means the client always see the previous frame.
  switch (captureMode) {
    case CaptureMode::Always:
      CaptureScene();
      FPixelReader::SendPixelsInRenderThread(*this);
      break;
    case CaptureMode::Step:
      if (bCapture)
      {
          CaptureScene();
          FPixelReader::SendPixelsInRenderThread(*this);
      }
      bCapture = !bCapture;
      break;
    default:
      break;
  }
}
