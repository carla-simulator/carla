#pragma once

#include "Carla/Weather/WeatherParameters.h"
#include "Math/UnrealMathUtility.h"

//#include "CameraPresets.generated.h"

class UCameraPresets: UObject
{
public:
	static float GetPPVExposureBiasPresetValue(ECameraPPVPreset PPVPreset)
	{
		float value = 0;

	  switch (PPVPreset)
	  {
	    case ECameraPPVPreset::Other:
	    {
	      value = 0.0f;  
	      break;
	    }
			case ECameraPPVPreset::Daylight:
			{
				value = -1.0f;
				break;
			}
			case ECameraPPVPreset::Overcast:
			{
				value = 3.0f;
				break;
			}
			default:
			{
				value = 0.0f;
				break;
			}
	  }

		return value;
	}

	static FString GetPPVExposureBiasPresetValueAsText(ECameraPPVPreset PPVPreset)
	{
		FString value = "";

	  switch (PPVPreset)
	  {
	    case ECameraPPVPreset::Other:
	    {
	      value = "0.0";  
	      break;
	    }
			case ECameraPPVPreset::Daylight:
			{
				value = "-1.0";
				break;
			}
			case ECameraPPVPreset::Overcast:
			{
				value = "3.0";
				break;
			}
	  }

		return value;
	}
};

