// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDCustomLayerData.h"
#include "USDConverter.h"
#include "Engine/Scene.h"
#include "SimReadyPathHelper.h"

const std::string FUSDCustomLayerData::MutenessCustomKey = "omni_layer:muteness";
const std::string FUSDCustomLayerData::LockedCustomKey = "omni_layer:locked";
const std::string FUSDCustomLayerData::AuthoringLayerCustomKey = "omni_layer:authoring_layer";

void FUSDCustomLayerData::ResetMaxRoughness(const pxr::SdfLayerRefPtr& RootLayer)
{
    pxr::VtDictionary CustomLayerData = RootLayer->GetCustomLayerData();
    pxr::VtDictionary RenderSettingsDict;
    auto RenderSettingsEntry = CustomLayerData.find(std::string("renderSettings"));
    if (RenderSettingsEntry != CustomLayerData.end())
    {
        RenderSettingsDict = *&(RenderSettingsEntry->second.Get<pxr::VtDictionary>());
    }
    // Reflections max roughness setting to 0.3 by default in Create is making those areas not get any reflections, while UE needs 1.0
    RenderSettingsDict["rtx:reflections:maxRoughness"] = 1.0f;
    CustomLayerData[std::string("renderSettings")] = RenderSettingsDict;
    RootLayer->SetCustomLayerData(CustomLayerData);
}

void FUSDCustomLayerData::SetEditorCamera(const pxr::UsdStageRefPtr& USDStage, const FVector& CamPosition, const FVector& CamTarget)
{
    auto RootLayer = USDStage->GetRootLayer();
    pxr::VtDictionary CustomLayerData = RootLayer->GetCustomLayerData();
    pxr::VtDictionary CameraSettingsDict;

    // Just export perspective now
    pxr::VtDictionary ViewportLayer;
    ViewportLayer["position"] = USDConvertD(USDConvertPosition(USDStage, CamPosition));
    ViewportLayer["target"] = USDConvertD(USDConvertPosition(USDStage, CamTarget));
    CameraSettingsDict["Perspective"] = ViewportLayer;

    pxr::SdfPath CameraShotPath;
    auto DefaultPrim = USDStage->GetDefaultPrim();
    if (DefaultPrim)
    {
        CameraShotPath = DefaultPrim.GetPath();
    }
    else
    {
        CameraShotPath = pxr::SdfPath::AbsoluteRootPath();
    }
    CameraShotPath = CameraShotPath.AppendElementString(TCHAR_TO_ANSI(*SimReadyCameraShot));
    if (pxr::UsdGeomCamera::Get(USDStage, CameraShotPath))
    {
        CameraSettingsDict["boundCamera"] = CameraShotPath.GetString();
    }

    CustomLayerData[std::string("cameraSettings")] = CameraSettingsDict;
    RootLayer->SetCustomLayerData(CustomLayerData);
}

FORCEINLINE float LuminanceToEV100(float Luminance)
{
    // if Luminance was 0, reset EV100 to 0
    return Luminance > 0.0f ? FMath::Log2(Luminance / 1.2f) : 0.0f;
}

FORCEINLINE float EV100ToHistogramEV(float EV100)
{
    return FMath::Pow(2.0f, EV100 - 3) * 1000.0f;
}

void FUSDCustomLayerData::SetRenderSettings(const pxr::SdfLayerRefPtr& RootLayer, const FPostProcessSettings& Settings)
{
    pxr::VtDictionary CustomLayerData = RootLayer->GetCustomLayerData();
    pxr::VtDictionary RenderSettingsDict;
    auto RenderSettingsEntry = CustomLayerData.find(std::string("renderSettings"));
    if (RenderSettingsEntry != CustomLayerData.end())
    {
        RenderSettingsDict = *&(RenderSettingsEntry->second.Get<pxr::VtDictionary>());
    }

    EAutoExposureMethod AutoExposureMethod = Settings.AutoExposureMethod;
    if (AutoExposureMethod == EAutoExposureMethod::AEM_Manual)
    {
        if (Settings.AutoExposureApplyPhysicalCameraExposure)
        {
            //const float FixedEV100 = FMath::Log2(FMath::Square(Settings.DepthOfFieldFstop) * Settings.CameraShutterSpeed * 100 / FMath::Max(1.f, Settings.CameraISO));
            RenderSettingsDict["rtx:post:tonemap:cameraShutter"] = static_cast<double>(Settings.CameraShutterSpeed * 1000.0f); // ms in kit
            RenderSettingsDict["rtx:post:tonemap:fNumber"] = static_cast<double>(Settings.DepthOfFieldFstop);
            RenderSettingsDict["rtx:post:tonemap:filmIso"] = static_cast<double>(FMath::Max(1.f, Settings.CameraISO));
        }
        else
        {	// EV100 is always 0 if AutoExposureApplyPhysicalCameraExposure is disabled
            RenderSettingsDict["rtx:post:tonemap:cameraShutter"] = static_cast<double>(1.0f / FMath::Square(4.0f) * 1000.0f); // ms in kit
            RenderSettingsDict["rtx:post:tonemap:fNumber"] = 4.0;
            RenderSettingsDict["rtx:post:tonemap:filmIso"] = 100.0;
        }
        RenderSettingsDict["rtx:post:tonemap:cm2Factor"] = static_cast<double>(FMath::Pow(2.0f, Settings.AutoExposureBias));
    }
    else // Auto Exposure
    {
        RenderSettingsDict["rtx:post:histogram:enabled"] = true;
        float AdaptSpeed = (Settings.AutoExposureSpeedUp + Settings.AutoExposureSpeedDown) * 0.5f;
        RenderSettingsDict["rtx:post:histogram:tau"] = static_cast<double>(AdaptSpeed);
        RenderSettingsDict["rtx:post:histogram:whiteScale"] = static_cast<double>(10.0f / FMath::Pow(2.0f, Settings.AutoExposureBias));

        static const auto VarDefaultAutoExposureExtendDefaultLuminanceRange = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange"));
        const bool bExtendedLuminanceRange = VarDefaultAutoExposureExtendDefaultLuminanceRange->GetValueOnGameThread() == 1;
        if (bExtendedLuminanceRange)
        {
            float MinAverageEV100 = Settings.AutoExposureMinBrightness;
            float MaxAverageEV100 = Settings.AutoExposureMaxBrightness;
            MinAverageEV100 = FMath::Min(MinAverageEV100, MaxAverageEV100);

            // NOTE: minEV and maxEV is Luminance
            RenderSettingsDict["rtx:post:histogram:minEV"] = static_cast<double>(EV100ToHistogramEV(MinAverageEV100));
            RenderSettingsDict["rtx:post:histogram:maxEV"] = static_cast<double>(EV100ToHistogramEV(MaxAverageEV100));
        }
        else
        {
            float MinAverageLuminance = Settings.AutoExposureMinBrightness;
            float MaxAverageLuminance = Settings.AutoExposureMaxBrightness;
            MinAverageLuminance = FMath::Min(MinAverageLuminance, MaxAverageLuminance);
            
            // NOTE: minEV and maxEV is USD Luminance
            RenderSettingsDict["rtx:post:histogram:minEV"] = static_cast<double>(EV100ToHistogramEV(LuminanceToEV100(MinAverageLuminance)));
            RenderSettingsDict["rtx:post:histogram:maxEV"] = static_cast<double>(EV100ToHistogramEV(LuminanceToEV100(MaxAverageLuminance)));
        }
    }

    CustomLayerData[std::string("renderSettings")] = RenderSettingsDict;
    RootLayer->SetCustomLayerData(CustomLayerData);
}

bool FUSDCustomLayerData::GetLayerCustomField(pxr::SdfLayerRefPtr RootLayer, const std::string& LayerIdentifier, const std::string& Key)
{
    // By default, the layer is not muted.
    pxr::VtDictionary ValueMap;
    const PXR_NS::VtDictionary& CustomLayerData = RootLayer->GetCustomLayerData();
    auto Value = CustomLayerData.GetValueAtPath(Key);
    if (Value && !Value->IsEmpty())
    {
        ValueMap = Value->Get<pxr::VtDictionary>();
    }

    std::string LayerAbsolutePath = RootLayer->ComputeAbsolutePath(LayerIdentifier);
    FString LayerAbsolutePathFString(UTF8_TO_TCHAR(LayerAbsolutePath.c_str()));
    FPaths::NormalizeFilename(LayerAbsolutePathFString);
    for (auto Iter = ValueMap.begin(); Iter != ValueMap.end(); Iter++)
    {
        std::string AbsolutePath = RootLayer->ComputeAbsolutePath(Iter->first);
        FString AbsolutePathFString(UTF8_TO_TCHAR(AbsolutePath.c_str()));
        FPaths::NormalizeFilename(AbsolutePathFString);
        if (AbsolutePathFString == LayerAbsolutePathFString)
        {
            return Iter->second.Get<bool>();
        }
    }

    return false;
}

void FUSDCustomLayerData::SetLayerCustomField(const pxr::SdfLayerRefPtr& RootLayer, const std::string& LayerIdentifier, const std::string& Key, bool Value)
{
    pxr::VtDictionary ValueMap;
    pxr::VtDictionary CustomLayerData = RootLayer->GetCustomLayerData();
    auto OldValue = CustomLayerData.GetValueAtPath(Key);
    if (OldValue && !OldValue->IsEmpty())
    {
        ValueMap = OldValue->Get<pxr::VtDictionary>();
    }

    std::string LayerAbsolutePath = RootLayer->ComputeAbsolutePath(LayerIdentifier);
    FString LayerAbsolutePathFString(UTF8_TO_TCHAR(LayerAbsolutePath.c_str()));
    FPaths::NormalizeFilename(LayerAbsolutePathFString);
    for (auto Iter = ValueMap.begin(); Iter != ValueMap.end(); Iter++)
    {
        std::string AbsolutePath = RootLayer->ComputeAbsolutePath(Iter->first);
        FString AbsolutePathFString(UTF8_TO_TCHAR(AbsolutePath.c_str()));
        FPaths::NormalizeFilename(AbsolutePathFString);
        if (AbsolutePathFString == LayerAbsolutePathFString)
        {
            ValueMap.erase(Iter);
            break;
        }
    }

    ValueMap[LayerIdentifier] = pxr::VtValue(Value);
    CustomLayerData.SetValueAtPath(Key, pxr::VtValue(ValueMap));
    RootLayer->SetCustomLayerData(CustomLayerData);
}

void FUSDCustomLayerData::SetAuthoringLayerCustomField(const pxr::SdfLayerRefPtr& RootLayer, const std::string& LayerIdentifier)
{
    pxr::VtDictionary CustomLayerData = RootLayer->GetCustomLayerData();
    CustomLayerData.SetValueAtPath(AuthoringLayerCustomKey, pxr::VtValue(LayerIdentifier));
    RootLayer->SetCustomLayerData(CustomLayerData);
}