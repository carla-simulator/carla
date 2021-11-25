// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/LidarShaderSensor.h"

#include "ConstructorHelpers.h"

bool ALidarShaderSensor::LoadPostProcessingMaterial(const FString &Path, const float Weight)
{
	ConstructorHelpers::FObjectFinder<UMaterial> Loader(*Path);
	if (Loader.Succeeded())
	{
		AddShader({ Loader.Object, Weight });
	}
	return Loader.Succeeded();
}

void ALidarShaderSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
	for (const auto &Shader : Shaders)
	{
		SceneCapture.PostProcessSettings.AddBlendable(Shader.PostProcessMaterial, Shader.Weight);
	}
}
