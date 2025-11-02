// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDConverter.h"
#include "Algo/Accumulate.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "MeshAttributes.h"
#include "MeshDescriptionOperations.h"
#include "SkeletalRenderPublic.h"
#include "ReferenceSkeleton.h"
#include "Model.h"
#include "Engine/StaticMesh.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureCube.h"
#include "Misc/ConfigCacheIni.h"
#include "CineCameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"
#include "Particles/TypeData/ParticleModuleTypeDataRibbon.h"
#include "ParticleEmitterInstances.h"
#include "Templates/Casts.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "StaticMeshResources.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "Components/CapsuleComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Exporters/Exporter.h"
#include "BlueprintGraph/Classes/K2Node_CallFunction.h"
#include "BlueprintGraph/Classes/K2Node_VariableGet.h"
#include "BlueprintGraph/Classes/K2Node_Knot.h"
#include "SimReadySettings.h"
#include "SimReadyStageActor.h"
#include "SimReadyMDL.h"
#include "SimReadyPxr.h"
#include "SimReadyPathHelper.h"
#include "SimReadyUSDLog.h"
#include "SimReadyUsdLuxLightCompat.h"
#include "SimReadyUSDTokens.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadyMaterialReparentUtility.h"

#if WITH_EDITOR
#include "Materials/MaterialInstanceConstant.h"
#include "MeshUtilities.h"
#endif
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionConstant.h"

#include <geometry/PxConvexMesh.h>

#include <foundation/PxVec3.h>

#include "Runtime/Launch/Resources/Version.h"

#include "Extractors/TimeSamplesData.h"
#include "Extractors/SequenceExtractor.h"

#include "Animation/DebugSkelMeshComponent.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimationAsset.h"
#include "AnimationRuntime.h"
#include "USDHashGenerator.h"
#include "USDDerivedDataCache.h"
#include "SimReadyCarlaWrapper.h"
#include "USDCARLAVehicleTools.h"

#define CUSTOM_KEY_UE4_CINE_CAMERA_MIN_FSTOP "omni:cine_camera_min_fstop"
#define CUSTOM_KEY_UE4_CINE_CAMERA_MAX_FSTOP "omni:cine_camera_max_fstop"
#define CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY "omni:cine_camera_chrom_abb_intens"
#define CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET "omni:cine_camera_chrom_abb_start_off"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD "omni:cine_camera_bloom_method"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY "omni:cine_camera_bloom_intensity"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD "omni:cine_camera_bloom_threshold"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED "omni:cine_camera_lens_camera_shutter_speed"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO "omni:cine_camera_lens_camera_iso"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE "omni:cine_camera_lens_camera_aperture"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE "omni:cine_camera_lens_camera_max_aperture"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES "omni:cine_camera_lens_camera_num_dia_blades"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN "omni:cine_camera_lens_camera_expo_compen"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE "omni:cine_camera_exposure_metering_mode"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS "omni:cine_camera_exposure_min_brightness"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS "omni:cine_camera_exposure_max_brightness"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP "omni:cine_camera_exposure_speed_up"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN "omni:cine_camera_exposure_speed_down"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY "omni:cine_camera_image_eff_veg_intens"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER "omni:cine_camera_image_eff_grain_jitter"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY "omni:cine_camera_image_eff_grain_intens"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD "omni:cine_camera_dof_method"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF "omni:cine_camera_dof_high_quality_gaus"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE "omni:cine_camera_dof_focal_distance"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM "omni:cine_camera_dof_blur_km"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS "omni:cine_camera_dof_blur_radius"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION "omni:cine_camera_dof_focal_region"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION "omni:cine_camera_dof_near_trans_region"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION "omni:cine_camera_dof_far_trans_region"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE "omni:cine_camera_dof_depth_scale"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE "omni:cine_camera_dof_max_bokeh_size"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE "omni:cine_camera_dof_near_blur_size"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE "omni:cine_camera_dof_far_blur_size"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION "omni:cine_camera_cg_global_saturation"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST "omni:cine_camera_cg_global_contrast"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA "omni:cine_camera_cg_global_gamma"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN "omni:cine_camera_cg_global_gain"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET "omni:cine_camera_cg_global_offset"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION "omni:cine_camera_sh_global_saturation"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST "omni:cine_camera_sh_global_contrast"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA "omni:cine_camera_sh_global_gamma"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN "omni:cine_camera_sh_global_gain"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET "omni:cine_camera_sh_global_offset"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX "omni:cine_camera_sh_max"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION "omni:cine_camera_gm_global_saturation"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST "omni:cine_camera_gm_global_contrast"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA "omni:cine_camera_gm_global_gamma"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN "omni:cine_camera_gm_global_gain"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET "omni:cine_camera_gm_global_offset"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION "omni:cine_camera_gh_global_saturation"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST "omni:cine_camera_gh_global_contrast"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA "omni:cine_camera_gh_global_gamma"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN "omni:cine_camera_gh_global_gain"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET "omni:cine_camera_gh_global_offset"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN "omni:cine_camera_gh_min"

#define CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY_TOGGLE "omni:cine_camera_chrom_abb_intens_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET_TOGGLE "omni:cine_camera_chrom_abb_start_off_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD_TOGGLE "omni:cine_camera_bloom_method_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY_TOGGLE "omni:cine_camera_bloom_intensity_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD_TOGGLE "omni:cine_camera_bloom_threshold_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED_TOGGLE "omni:cine_camera_lens_camera_shutter_speed_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO_TOGGLE "omni:cine_camera_lens_camera_iso_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE_TOGGLE "omni:cine_camera_lens_camera_aperture_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE_TOGGLE "omni:cine_camera_lens_camera_max_aperture_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES_TOGGLE "omni:cine_camera_lens_camera_num_dia_blades_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN_TOGGLE "omni:cine_camera_lens_camera_expo_compen_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE_TOGGLE "omni:cine_camera_exposure_metering_mode_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS_TOGGLE "omni:cine_camera_exposure_min_brightness_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS_TOGGLE "omni:cine_camera_exposure_max_brightness_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP_TOGGLE "omni:cine_camera_exposure_speed_up_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN_TOGGLE "omni:cine_camera_exposure_speed_down_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY_TOGGLE "omni:cine_camera_image_eff_veg_intens_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER_TOGGLE "omni:cine_camera_image_eff_grain_jitter_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY_TOGGLE "omni:cine_camera_image_eff_grain_intens_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD_TOGGLE "omni:cine_camera_dof_method_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF_TOGGLE "omni:cine_camera_dof_high_quality_gaus_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE_TOGGLE "omni:cine_camera_dof_focal_distance_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM_TOGGLE "omni:cine_camera_dof_blur_km_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS_TOGGLE "omni:cine_camera_dof_blur_radius_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION_TOGGLE "omni:cine_camera_dof_focal_region_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION_TOGGLE "omni:cine_camera_dof_near_trans_region_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION_TOGGLE "omni:cine_camera_dof_far_trans_region_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE_TOGGLE "omni:cine_camera_dof_depth_scale_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE_TOGGLE "omni:cine_camera_dof_max_bokeh_size_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE_TOGGLE "omni:cine_camera_dof_near_blur_size_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE_TOGGLE "omni:cine_camera_dof_far_blur_size_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION_TOGGLE "omni:cine_camera_cg_global_saturation_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST_TOGGLE "omni:cine_camera_cg_global_contrast_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA_TOGGLE "omni:cine_camera_cg_global_gamma_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN_TOGGLE "omni:cine_camera_cg_global_gain_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET_TOGGLE "omni:cine_camera_cg_global_offset_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION_TOGGLE "omni:cine_camera_sh_global_saturation_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST_TOGGLE "omni:cine_camera_sh_global_contrast_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA_TOGGLE "omni:cine_camera_sh_global_gamma_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN_TOGGLE "omni:cine_camera_sh_global_gain_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET_TOGGLE "omni:cine_camera_sh_global_offset_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX_TOGGLE "omni:cine_camera_sh_max_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION_TOGGLE "omni:cine_camera_gm_global_saturation_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST_TOGGLE "omni:cine_camera_gm_global_contrast_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA_TOGGLE "omni:cine_camera_gm_global_gamma_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN_TOGGLE "omni:cine_camera_gm_global_gain_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET_TOGGLE "omni:cine_camera_gm_global_offset_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION_TOGGLE "omni:cine_camera_gh_global_saturation_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST_TOGGLE "omni:cine_camera_gh_global_contrast_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA_TOGGLE "omni:cine_camera_gh_global_gamma_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN_TOGGLE "omni:cine_camera_gh_global_gain_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET_TOGGLE "omni:cine_camera_gh_global_offset_toggle"
#define CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN_TOGGLE "omni:cine_camera_gh_min_toggle"

#define CUSTOM_KEY_UE4_LIGHT_INTENSITY_UNITS "omni:light_intensity_units"
#define CUSTOM_KEY_UE4_LIGHT_INTENSITY_UNITLESS "Unitless"
#define CUSTOM_KEY_UE4_LIGHT_INTENSITY_CANDES "Candelas"
#define CUSTOM_KEY_UE4_LIGHT_INTENSITY_LUMENS "Lumens"
#define CUSTOM_KEY_UE4_LIGHT_ATTENUATE_DISTANCE "omni:light_attenuate_distance"
#define CUSTOM_KEY_UE4_LIGHT_CAST_SHADOWS "omni:light_cast_shadows"

#define UNIT_PER_SQUARE_METRE (0.0001f)

#define USD_CAMERA_CLIPPING_RANGE_MIN (1.0f)
#define USD_CAMERA_CLIPPING_RANGE_MAX (10000000.0f)

#define USD_LIGHT_MIN_RADIUS (8.0f)

const FMatrix SimReadyZUpConversion{
    FPlane(1.0f, 0.0f, 0.0f, 0.0f),
    FPlane(0.0f, -1.0f, 0.0f, 0.0f),
    FPlane(0.0f, 0.0f, 1.0f, 0.0f),
    FPlane(0.0f, 0.0f, 0.0f, 1.0f)
};

const FMatrix SimReadyYUpConversion{
    FPlane(1.0f, 0.0f, 0.0f, 0.0f),
    FPlane(0.0f, 0.0f, 1.0f, 0.0f),
    FPlane(0.0f, 1.0f, 0.0f, 0.0f),
    FPlane(0.0f, 0.0f, 0.0f, 1.0f)
};

// The fallback axis for UE4 is z-up
pxr::TfToken GetUSDStageAxis(const pxr::UsdStageRefPtr& Stage)
{
    return pxr::UsdGeomGetStageUpAxis(Stage);
}

float UnitScaleFromUSDToUE(const pxr::UsdStageRefPtr& Stage)
{
    const double USDMetersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(Stage);
    const double UEMetersPerUnit = 0.01;
    return USDMetersPerUnit / UEMetersPerUnit;
}

float UnitScaleFromUEToUSD(const pxr::UsdStageRefPtr& Stage)
{
    const double USDMetersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(Stage);
    const double UEMetersPerUnit = 0.01;
    return UEMetersPerUnit / USDMetersPerUnit;
}

void FixPrimKind(const pxr::UsdPrim& Prim)
{
    // check parent kind
    auto ParentPrim = Prim.GetParent();
    if (ParentPrim && !ParentPrim.IsPseudoRoot())
    {
        pxr::TfToken PrimKind;
        if (pxr::UsdModelAPI(Prim).GetKind(&PrimKind))
        {
            pxr::TfToken ParentKind;
            if (pxr::UsdModelAPI(ParentPrim).GetKind(&ParentKind))
            {
                if (ParentKind == pxr::KindTokens->component || ParentKind == pxr::KindTokens->subcomponent)
                {
                    pxr::UsdModelAPI(Prim).SetKind(pxr::KindTokens->subcomponent);
                }
                else if (PrimKind == pxr::KindTokens->group && ParentKind != pxr::KindTokens->group && ParentKind != pxr::KindTokens->assembly)
                {
                    // If parent is not 'assembly' and 'group', clean 'group' Kind
                    pxr::UsdModelAPI(Prim).SetKind(pxr::TfToken());
                }
            }
            else // No Kind for parent
            {
                if (PrimKind == pxr::KindTokens->group)
                {
                    // If parent has no Kind, clean 'group' Kind
                    pxr::UsdModelAPI(Prim).SetKind(pxr::TfToken());
                }
            }
        }
    }
}

void AddReferenceOrPayload(bool bPayload, const pxr::UsdPrim& Prim, const FString& RelativePath)
{
    if (bPayload)
    {
        Prim.GetPayloads().AddPayload(TCHAR_TO_UTF8(*RelativePath));
    }
    else
    {
        Prim.GetReferences().AddReference(TCHAR_TO_UTF8(*RelativePath));
    }
    FixPrimKind(Prim);
}

bool IsInRange(int32 Time, const TArray<FTimeSampleRange>* Ranges)
{
    if (Ranges)
    {
        for (auto Range : *Ranges)
        {
            if (Range.IsInRange(Time))
            {
                return true;
            }
        }

        return false;
    }

    // No defined range, always return true
    return true;
}

TArray<int32> GetTimeCodesFromRange(const TArray<FTimeSampleRange>* Ranges)
{
    TArray<int32> TimeCodes;
    if (Ranges)
    {
        for (auto Range : *Ranges)
        {
            TimeCodes.Add(Range.StartTimeCode);
            TimeCodes.Add(Range.EndTimeCode);
        }
    }
    return TimeCodes;
}

namespace SimReadyTokens
{
    extern const pxr::TfToken Behaviors("omni:simready:behaviors");
    extern const pxr::TfToken LightIntensityDomain("omni:simready:light:intensityDomain");
    extern const pxr::TfToken TimeOfDay("TimeOfDay");
    extern const pxr::TfToken SemanticsApiWikidataQcode("SemanticsAPI:wikidata_qcode");
    extern const pxr::TfToken SemanticsLabelsApiWikidataQcode("SemanticsLabelsAPI:wikidata_qcode");
    extern const pxr::TfToken WikiDataQCodeSemanticType("semantic:wikidata_qcode:params:semanticType");
    extern const pxr::TfToken WikiDataQCodeSemanticData("semantic:wikidata_qcode:params:semanticData");
    extern const pxr::TfToken WikiDataQCode("semantics:labels:wikidata_qcode");
    extern const pxr::TfToken SignalId("omni:simready:signalID");
    extern const pxr::TfToken SignalType("omni:simready:signalType");
    extern const pxr::TfToken TrafficLight("trafficLight");
    extern const pxr::TfToken Signal("omni:simready:signal");
    extern const pxr::TfToken SignalIntensityDomain("omni:simready:signal:intensityDomain");
    extern const pxr::TfToken SignalOrder("omni:simready:signalOrder");
    extern const pxr::TfToken Red("red");
    extern const pxr::TfToken Green("green");
    extern const pxr::TfToken Amber("amber");
    extern const pxr::TfToken Skin("skin");
    extern const pxr::TfToken Fabric("fabric");
    extern const pxr::TfToken FurHair("fur_hair");
    // Non-Visual
    extern const pxr::TfToken NonVisualAttributes("omni:simready:nonvisual:attributes");
    extern const pxr::TfToken NonVisualBase("omni:simready:nonvisual:base");
    extern const pxr::TfToken NonVisualCoating("omni:simready:nonvisual:coating");
    extern const pxr::TfToken None("none");
    extern const pxr::TfToken Asphalt("asphalt");
    extern const pxr::TfToken Concrete("concrete");
    extern const pxr::TfToken Brick("brick");
    extern const pxr::TfToken Wood("wood");
    extern const pxr::TfToken Aluminum("aluminum");
    extern const pxr::TfToken Plexiglass("plexiglass");
    extern const pxr::TfToken Bark("bark");
    extern const pxr::TfToken Carbon_fiber("carbon_fiber");
    extern const pxr::TfToken Plastic("plastic");
    extern const pxr::TfToken Water("water");
    extern const pxr::TfToken Gravel("gravel");
    extern const pxr::TfToken Steel("steel");
    extern const pxr::TfToken Dirt("dirt");
    extern const pxr::TfToken Stone("stone");

    // coating
    extern const pxr::TfToken Paint("paint");
    extern const pxr::TfToken Clearcoat("clearcoat");
    extern const pxr::TfToken PaintClearcoat("paint_clearcoat");
    // attributes
    extern const pxr::TfToken Emissive("emissive");
    extern const pxr::TfToken Retroreflective("retroreflective");
    extern const pxr::TfToken SingleSided("single_sided");
    extern const pxr::TfToken VisuallyTransparent("visually_transparent");
}

void CreateSimReadyTimeOfDayAttributes(const pxr::UsdPrim& Prim, float LightIntensity)
{
    auto BehaviorsAttr = Prim.CreateAttribute(SimReadyTokens::Behaviors, pxr::SdfValueTypeNames->TokenArray);
    BehaviorsAttr.Set(pxr::VtTokenArray{ SimReadyTokens::TimeOfDay });

    auto IntensityDomainAttr = Prim.CreateAttribute(SimReadyTokens::LightIntensityDomain, pxr::SdfValueTypeNames->Float2);
    IntensityDomainAttr.Set(pxr::GfVec2f(0, LightIntensity));
}

// Add time of day information for lights and meshes
void ExportSimReadyTimeOfDayInfo(const pxr::UsdPrim& Prim, const USceneComponent& SceneComp)
{
    if (!SceneComp.IsA<UStaticMeshComponent>() && !SceneComp.IsA<ULightComponentBase>())
    {
        return;
    }

    auto CarlaLights = SimReadyCarlaWrapper::FindCarlaLights(*SceneComp.GetOwner());
    if (CarlaLights.Num() == 0)
    {
        return;
    }

    auto Blueprint = Cast<UBlueprint>(SceneComp.GetOwner()->GetClass()->ClassGeneratedBy);
    if (!Blueprint)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Actor %s is not created from Blueprint. Couldn't export Sim Ready time of day data for this actor."), *SceneComp.GetOwner()->GetName());
        return;
    }

    TArray<UBlueprint*> OutBlueprintParents;
    UBlueprint::GetBlueprintHierarchyFromClass(Blueprint->GeneratedClass, OutBlueprintParents);

    // Collect SetLight() and SetEmisive() from graphs. They are used to update lights and meshes.
    TArray<const UK2Node_CallFunction*> SetLightCalls;
    TArray<const UK2Node_CallFunction*> SetEmissiveCalls;

    for (auto OutBlueprintParent : OutBlueprintParents)
    {
        for (auto Graph : OutBlueprintParent->UbergraphPages)
        {
            for (auto Node : Graph->Nodes)
            {
                auto CallFunNode = Cast<UK2Node_CallFunction>(Node);
                if (!CallFunNode)
                {
                    continue;
                }

                auto MemberParentClass = CallFunNode->FunctionReference.GetMemberParentClass();
                if (!MemberParentClass)
                {
                    continue;
                }

                if (MemberParentClass->GetPathName() != "/Game/Carla/Blueprints/Lights/BP_Lights.BP_Lights_C")
                {
                    continue;
                }

                auto MemberName = CallFunNode->FunctionReference.GetMemberName();
                if (MemberName == "SetLight")
                {
                    SetLightCalls.Add(CallFunNode);
                }
                else if (MemberName == "SetEmissive")
                {
                    SetEmissiveCalls.Add(CallFunNode);
                }
            }
        }
    }

    // Function to collect time of day components
    auto FindTimeOfDayComponents = [&SceneComp](const TArray<const UK2Node_CallFunction*>& CallNodes, const FString& PinName, const UClass& Class)
        {
            // For each call node, find linked light or mesh components and CARLA light components
            TMap<const UActorComponent*, const UActorComponent*> ComponentMap;

            for (auto CallNode : CallNodes)
            {
                // Function to find the component that links to this node
                auto FindLinkedComponent = [&](const FString& PinName, const UClass& Class)
                    {
                        TSet<const UActorComponent*> Components;

                        for (auto Pin : CallNode->Pins)
                        {
                            if (Pin->GetName() != PinName)
                            {
                                continue;
                            }

                            // Find the final varable nodes bypassing reroute nodes
                            static TFunction<TSet<const UK2Node_VariableGet*>(const UEdGraphPin& Pin)> FindVarGetNodes = [](const UEdGraphPin& Pin) {
                                TSet<const UK2Node_VariableGet*> VarGetNodes;

                                for (auto LinkedPin : Pin.LinkedTo)
                                {
                                    auto LinkedNode = LinkedPin->GetOwningNode();
                                    if (auto VarGetNode = Cast<UK2Node_VariableGet>(LinkedNode))
                                    {
                                        VarGetNodes.Add(VarGetNode);
                                    }
                                    else if (auto Knot = Cast<UK2Node_Knot>(LinkedNode))
                                    {
                                        VarGetNodes.Append(FindVarGetNodes(*Knot->GetInputPin()));
                                    }
                                }

                                return VarGetNodes;
                                };

                            auto VarGetNodes = FindVarGetNodes(*Pin);

                            // Get components that are represented by variable nodes.
                            for (auto VarGetNode : VarGetNodes)
                            {
                                auto Component = Cast<UActorComponent>(SceneComp.GetOwner()->GetDefaultSubobjectByName(VarGetNode->GetVarName()));
                                if (Component && Component->GetClass()->IsChildOf(&Class))
                                {
                                    Components.Add(Component);
                                }
                            }
                        }

                        return Components;
                    };

                // Get a CARLA light component that is set as "Target" of the node
                auto LinkedCarlaLights = FindLinkedComponent("self", *UActorComponent::StaticClass());
                if (LinkedCarlaLights.Num() != 1)
                {
                    continue;
                }

                // Get the light or mesh components that are controlled by the CARLA light
                auto LinkedComponents = FindLinkedComponent(PinName, Class);
                for (auto Component : LinkedComponents)
                {
                    ComponentMap.Add(Component) = *LinkedCarlaLights.begin();
                }
            }

            return ComponentMap;
        };

    // Collect lights of time of day
    auto LightMap = FindTimeOfDayComponents(SetLightCalls, "Light", *ULightComponentBase::StaticClass());
    auto MeshMap = FindTimeOfDayComponents(SetEmissiveCalls, "Mesh", *UStaticMeshComponent::StaticClass());

    // Export time of day data
    const UActorComponent** CarlaLight = nullptr;
    if (SceneComp.IsA<ULightComponentBase>())
    {
        CarlaLight = LightMap.Find(&SceneComp);
    }
    else if (SceneComp.IsA<UStaticMeshComponent>())
    {
        CarlaLight = MeshMap.Find(&SceneComp);
    }

    if (!CarlaLight)
    {
        return;
    }

    auto LightData = CarlaLights.Find(*CarlaLight);
    if (!LightData)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Couldn't find light intensity for CARLA light %s for component %s"), *(*CarlaLight)->GetFullName(), *SceneComp.GetFullName());
        return;
    }

    if (auto LightComponent = Cast<ULightComponentBase>(&SceneComp))
    {
        extern float ConvertUSDLightIntensity(const ULightComponentBase& LightComponent, float Intensity);
        auto LightIntensity = ConvertUSDLightIntensity(*LightComponent, LightData->Intensity);
        if (SceneComp.IsA<URectLightComponent>())
        {
            LightIntensity *= 0.03f;
        }

        CreateSimReadyTimeOfDayAttributes(Prim, LightIntensity);

        if (auto UsdLight = pxr::UsdLuxLight(Prim))
        {
            UsdLight.GetVisibilityAttr().Clear();

            Prim.CreateAttribute(pxr::TfToken("inputs:intensity"), pxr::SdfValueTypeNames->Float).Set(LightIntensity);

            auto& Color = LightData->Color;
            UsdLight.CreateColorAttr(pxr::VtValue(pxr::GfVec3f(Color.R, Color.G, Color.B)));
        }
    }
    else if (SceneComp.IsA<UStaticMeshComponent>())
    {
        auto StaticMeshComp = Cast<UStaticMeshComponent>(&SceneComp);
        auto MeshName = StaticMeshComp->GetStaticMesh()->GetName();
        auto MeshPrim = Prim.GetChild(pxr::TfToken(TCHAR_TO_ANSI(*MeshName)));

        if (!MeshPrim)
        {
            UE_LOG(LogSimReadyUsd, Error, TEXT("Can't find mesh prim of name %s under prim %s"), *MeshName, *FString(Prim.GetPath().GetText()));
            return;
        }

        for (auto SubPrim : pxr::UsdPrimRange(MeshPrim))
        {
            if (!pxr::UsdGeomMesh(SubPrim))
            {
                continue;
            }

            CreateSimReadyTimeOfDayAttributes(SubPrim, 1.f);

            auto Material = pxr::UsdShadeMaterialBindingAPI(SubPrim).ComputeBoundMaterial();
            if (!Material)
            {
                continue;
            }

            auto SurfaceAttr = Material.GetPrim().GetAttribute(pxr::TfToken("outputs:mdl:surface"));
            if (!SurfaceAttr)
            {
                continue;
            }

            pxr::SdfPathVector Paths;
            SurfaceAttr.GetConnections(&Paths);
            for (auto Path : Paths)
            {
                auto Shader = pxr::UsdShadeShader::Get(Prim.GetStage(), Path.GetPrimPath());
                if (!Shader)
                {
                    continue;
                }

                extern const float GetLightIntensityScale();

                auto IntensityInput = Shader.GetInput(pxr::TfToken("Intensity"));
                if (IntensityInput)
                {
                    float Intensity = 0;
                    IntensityInput.Get(&Intensity);
                    IntensityInput.Set(Intensity * GetLightIntensityScale());
                }

                auto EmissiveIntensityInput = Shader.GetInput(pxr::TfToken("EmissiveIntensity"));
                if (EmissiveIntensityInput)
                {
                    EmissiveIntensityInput.Set(LightData->Intensity * GetLightIntensityScale());
                }

                auto EmissiveColorInput = Shader.GetInput(pxr::TfToken("EmissiveColor"));
                if (EmissiveColorInput)
                {
                    auto& Color = LightData->Color;
                    EmissiveColorInput.Set(pxr::GfVec4f(Color.R, Color.G, Color.B, 1));
                }

                auto OnOffInput = Shader.GetInput(pxr::TfToken("On_Off"));
                if (OnOffInput)
                {
                    OnOffInput.Set(1.f);
                }
            }
        }
    }
}

void ExportSimReadyTrafficLightColor(const pxr::UsdPrim& Prim, const UStaticMeshComponent& MeshComp)
{
    if (!SimReadyCarlaWrapper::IsTrafficLight(*MeshComp.GetOwner()))
    {
        return;
    }

    // Function to check if the component has the material of a specific light color
    auto HasLightColorMaterial = [&](const FName& ColorName) {
        auto Class = MeshComp.GetOwner()->GetClass();

        auto RedLightsProp = CastField<FArrayProperty>(Class->FindPropertyByName(ColorName));
        if (!RedLightsProp)
        {
            return false;
        }

        auto ElementProp = CastField<FObjectProperty>(RedLightsProp->Inner);
        if (!ElementProp)
        {
            return false;
        }

        FScriptArrayHelper_InContainer Array(RedLightsProp, MeshComp.GetOwner());
        for (int Index = 0; Index < Array.Num(); ++Index)
        {
            auto MaterialInstance = ElementProp->GetObjectPropertyValue(Array.GetRawPtr(Index));
            for (int MaterialIndex = 0; MaterialIndex < MeshComp.GetNumMaterials(); ++MaterialIndex)
            {
                if (MeshComp.GetMaterial(MaterialIndex) == MaterialInstance)
                {
                    return true;
                }
            }
        }

        return false;
        };

    // Add traffic light color attribute
    pxr::TfToken Color;
    if (HasLightColorMaterial("RedLights"))
    {
        Color = SimReadyTokens::Red;
    }
    else if (HasLightColorMaterial("GreenLights"))
    {
        Color = SimReadyTokens::Green;
    }
    else if (HasLightColorMaterial("YellowLights"))
    {
        Color = SimReadyTokens::Amber;
    }
    else
    {
        return;
    }

    for (auto SubPrim : pxr::UsdPrimRange(Prim))
    {
        if (pxr::UsdGeomMesh(SubPrim))
        {
            auto SignalAttr = SubPrim.CreateAttribute(SimReadyTokens::Signal, pxr::SdfValueTypeNames->Token, true);
            SignalAttr.Set(Color);

            auto IntensityDomainAttr = SubPrim.CreateAttribute(SimReadyTokens::SignalIntensityDomain, pxr::SdfValueTypeNames->Float2, true);
            IntensityDomainAttr.Set(pxr::GfVec2f(0, 1));
        }
    }
}

template<typename From, typename To>
pxr::VtArray<To> USDConvert(const FMeshDescription& MeshDesc, const FName& AttributeName, int32 Index = 0)
{
    return USDConvert<From, To>(MeshDesc, AttributeName, Index,
        [](const From& Value)
        {
            return USDConvert(Value);
        }
        );
}

template<typename From, typename To>
pxr::VtArray<To> USDConvert(const FMeshDescription& MeshDesc, const FName& AttributeName, int32 Index, TFunction<To(const From&)> Convert)
{
    pxr::VtArray<To> USDValues;

    if(!MeshDesc.VertexInstanceAttributes().HasAttribute(AttributeName))
    {
        return USDValues;
    }

    if(Index >= MeshDesc.VertexInstanceAttributes().GetAttributeIndexCount<From>(AttributeName))
    {
        return USDValues;
    }

    TVertexInstanceAttributesConstRef<From> Attributes = MeshDesc.VertexInstanceAttributes().GetAttributesRef<From>(AttributeName);
    USDValues.reserve(Attributes.GetNumElements());

    for(FPolygonID PolygonID : MeshDesc.Polygons().GetElementIDs())
    {
        TArray<FVertexID> Vertices;
        MeshDesc.GetPolygonVertices(PolygonID, Vertices);

        for(FVertexID VertexID : Vertices)
        {
            FVertexInstanceID VertexInstID = MeshDesc.GetVertexInstanceForPolygonVertex(PolygonID, VertexID);
            USDValues.push_back(Convert(Attributes.Get(VertexInstID, Index)));
        }
    }

    return USDValues;
}

template<typename From, typename To>
pxr::VtArray<To> USDConvert(const FMeshDescription& MeshDesc, const FName& AttributeName, const TArray<FPolygonID>& PolygonIDs, int32 Index = 0)
{
    return USDConvert<From, To>(MeshDesc, AttributeName, PolygonIDs, Index,
        [](const From& Value)
    {
        return USDConvert(Value);
    }
    );
}

template<typename From, typename To>
pxr::VtArray<To> USDConvert(const FMeshDescription& MeshDesc, const FName& AttributeName, const TArray<FPolygonID>& PolygonIDs, int32 Index, TFunction<To(const From&)> Convert)
{
    pxr::VtArray<To> USDValues;

    if (!MeshDesc.VertexInstanceAttributes().HasAttribute(AttributeName))
    {
        return USDValues;
    }

    if (Index >= MeshDesc.VertexInstanceAttributes().GetAttributeIndexCount<From>(AttributeName))
    {
        return USDValues;
    }

    TVertexInstanceAttributesConstRef<From> Attributes = MeshDesc.VertexInstanceAttributes().GetAttributesRef<From>(AttributeName);
    USDValues.reserve(Attributes.GetNumElements());

    for (FPolygonID PolygonID : PolygonIDs)
    {
        TArray<FVertexID> Vertices;
        MeshDesc.GetPolygonVertices(PolygonID, Vertices);

        for (FVertexID VertexID : Vertices)
        {
            FVertexInstanceID VertexInstID = MeshDesc.GetVertexInstanceForPolygonVertex(PolygonID, VertexID);
            USDValues.push_back(Convert(Attributes.Get(VertexInstID, Index)));
        }
    }

    return USDValues;
}

void WeldVertex(TArray<int32>& VertexRemap, TArray<int32>& UniqueVertex, const FPositionVertexBuffer& PositionVertexBuffer, const TArray<int32>* UsedVertex = nullptr)
{
    const int32 VertexCount = PositionVertexBuffer.GetNumVertices();
    VertexRemap.Empty(VertexCount);
    VertexRemap.AddUninitialized(VertexCount);
    UniqueVertex.Empty(VertexCount);

    TMap<FVector, int32> HashedVertex;
    const int32 UsedVertexCount = UsedVertex ? UsedVertex->Num() : VertexCount;
    for (int32 VertexIndex = 0; VertexIndex < UsedVertexCount; ++VertexIndex)
    {
        int32 UEVertexIndex = UsedVertex ? (*UsedVertex)[VertexIndex] : VertexIndex;
        const FVector& Position = PositionVertexBuffer.VertexPosition(UEVertexIndex);
        const int32* FoundIndex = HashedVertex.Find(Position);
        if (!FoundIndex)
        {
            int32 NewIndex = UniqueVertex.Add(UEVertexIndex);
            VertexRemap[UEVertexIndex] = NewIndex;
            HashedVertex.Add(Position, NewIndex);
        }
        else
        {
            VertexRemap[UEVertexIndex] = *FoundIndex;
        }
    }
}

void FUSDExporter::FillVertexColor(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomXformable& USDMesh, const pxr::TfToken& Interpolation, const TArray<FUSDGeomMeshAttributes>& Attributes)
{
    for (int32 SectionIndex = 0; SectionIndex < Attributes.Num(); ++SectionIndex)
    {
        FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);
        auto SectionPath = USDMesh.GetPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SectionName)));
        auto SectionMesh = pxr::UsdGeomMesh::Define(Stage, SectionPath);
        FillVertexColor(Stage, SectionMesh, Interpolation, Attributes[SectionIndex]);
    }
}

const FColorVertexBuffer* GetFinalColorVertexBuffer(const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors)
{
    const FColorVertexBuffer* ColorVertexBuffer = &LODModel.VertexBuffers.ColorVertexBuffer;
    if (OverrideVertexColors)
    {
        bool bBroken = false;
        for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
        {
            const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
            if (Section.MaxVertexIndex >= OverrideVertexColors->GetNumVertices())
            {
                bBroken = true;
                break;
            }
        }

        if (!bBroken)
        {
            ColorVertexBuffer = OverrideVertexColors;
        }
    }

    return ColorVertexBuffer;
}

bool AllColorsAreEqual(const FColorVertexBuffer* ColorVertexBuffer)
{
    const int32 VertexCount = ColorVertexBuffer->GetNumVertices();
    if (VertexCount > 0)
    {
        const FColor FirstColor = ColorVertexBuffer->VertexColor(0);

        for (int32 CurVertexIndex = 0; CurVertexIndex < VertexCount; ++CurVertexIndex)
        {
            const FColor CurColor = ColorVertexBuffer->VertexColor(CurVertexIndex);

            if (CurColor.R != FirstColor.R || CurColor.G != FirstColor.G || CurColor.B != FirstColor.B || CurColor.A != FirstColor.A)
            {
                return false;
            }
        }

        return true;
    }

    return true;
}

void OverridePostProcessSettings(const FPostProcessSettings& Src, float Weight, FPostProcessSettings& FinalPostProcessSettings)
{
    if (Weight <= 0.0f)
    {
        // no need to blend anything
        return;
    }

    if (Weight > 1.0f)
    {
        Weight = 1.0f;
    }

    {
        FPostProcessSettings& Dest = FinalPostProcessSettings;

        if (Src.bOverride_TranslucencyType)
        {
            Dest.TranslucencyType = Src.TranslucencyType;
        }

        if (Src.bOverride_RayTracingTranslucencyRefraction)
        {
            Dest.RayTracingTranslucencyRefraction = Src.RayTracingTranslucencyRefraction;
        }

#define LERP_PP(NAME) if(Src.bOverride_ ## NAME)	Dest . NAME = FMath::Lerp(Dest . NAME, Src . NAME, Weight);
        LERP_PP(CameraShutterSpeed);
        LERP_PP(CameraISO);
        LERP_PP(AutoExposureMinBrightness);
        LERP_PP(AutoExposureMaxBrightness);
        LERP_PP(AutoExposureBias);
        LERP_PP(DepthOfFieldFstop);
        LERP_PP(DepthOfFieldFocalDistance);
        LERP_PP(DepthOfFieldSensorWidth);
#undef LERP_PP

        if (Src.bOverride_AutoExposureMethod)
        {
            Dest.AutoExposureMethod = Src.AutoExposureMethod;
        }

        if (Src.bOverride_AutoExposureApplyPhysicalCameraExposure)
        {
            Dest.AutoExposureApplyPhysicalCameraExposure = Src.AutoExposureApplyPhysicalCameraExposure;
        }
    }
}

void GetMeshFromSkelRoot(const pxr::UsdSkelRoot& USDSkelRoot, TArray<pxr::UsdPrim>& MeshPrims)
{
    if (USDSkelRoot)
    {
        pxr::UsdSkelCache USDSkelCache;
        USDSkelCache.Populate(USDSkelRoot);

        // exported skeletal mesh should be super-simple: 1 skinnable mesh under the SkelRoot
        std::vector<pxr::UsdSkelBinding> Bindings;
        USDSkelCache.ComputeSkelBindings(USDSkelRoot, &Bindings);
        if (Bindings.size() > 0)
        {
            const pxr::UsdSkelBinding& Binding = Bindings[0];
            pxr::UsdSkelSkeletonQuery SkelQuery = USDSkelCache.GetSkelQuery(Binding.GetSkeleton());
            for (int32 SkinTargetIndex = 0; SkinTargetIndex < Binding.GetSkinningTargets().size(); ++SkinTargetIndex)
            {
                const pxr::UsdSkelSkinningQuery& SkinningQuery = Binding.GetSkinningTargets()[SkinTargetIndex];
                MeshPrims.Add(SkinningQuery.GetPrim());
            }
        }
        else
        {
            // NOTE: If skeleton binding can't be found, checking children of SkelRoot
            FindAllPrims<pxr::UsdGeomMesh>(USDSkelRoot.GetPrim(), MeshPrims);
        }
    }
}

void FUSDExporter::FillVertexColor(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomMesh& USDMesh, const pxr::TfToken& Interpolation, const FUSDGeomMeshAttributes& Attributes)
{
    // Vertex color
    if (!Attributes.Colors.empty())
    {
        auto Primvar = USDMesh.CreateDisplayColorPrimvar(Attributes.Colors.size() == 1 ? pxr::UsdGeomTokens->constant : Interpolation);
        Primvar.Set(Attributes.Colors);
    }
    else
    {
        pxr::VtArray<pxr::GfVec3f> Color;
        Color.push_back(pxr::GfVec3f(1.0f));
        auto Primvar = USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->constant);
        Primvar.Set(Color);
    }

    // Vertex color opacity
    if (!Attributes.Opacities.empty())
    {
        auto Primvar = USDMesh.CreateDisplayOpacityPrimvar(Attributes.Opacities.size() == 1 ? pxr::UsdGeomTokens->constant : Interpolation);
        Primvar.Set(Attributes.Opacities);
    }
    else
    {
        pxr::VtArray<float> Opacity;
        Opacity.push_back(1.0f);
        auto Primvar = USDMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->constant);
        Primvar.Set(Opacity);
    }

#if 0 // Workaround with UVSet
    if (Attributes.UVs.size() <= 2) // Kit supports 4 uvs for now
    {
        pxr::VtArray<pxr::GfVec2f> RG;
        pxr::VtArray<pxr::GfVec2f> BA;
        // Breaking display color/opacity to UVs
        if (!Attributes.Colors.empty() || !Attributes.Opacities.empty())
        {
            int32 ColorNum = Attributes.Colors.empty() ? Attributes.Opacities.size() : Attributes.Colors.size();
            RG.reserve(ColorNum);
            BA.reserve(ColorNum);
            for (int32 ColorIndex = 0; ColorIndex < ColorNum; ++ColorIndex)
            {
                if (Attributes.Colors.empty())
                {
                    RG.push_back(pxr::GfVec2f(1.0f));
                    BA.push_back(pxr::GfVec2f(1.0f, Attributes.Opacities[ColorIndex]));
                }
                else if (Attributes.Opacities.empty())
                {
                    RG.push_back(pxr::GfVec2f(Attributes.Colors[ColorIndex][0], Attributes.Colors[ColorIndex][1]));
                    BA.push_back(pxr::GfVec2f(Attributes.Colors[ColorIndex][2], 1.0f));
                }
                else
                {
                    RG.push_back(pxr::GfVec2f(Attributes.Colors[ColorIndex][0], Attributes.Colors[ColorIndex][1]));
                    BA.push_back(pxr::GfVec2f(Attributes.Colors[ColorIndex][2], Attributes.Opacities[ColorIndex]));
                }
            }
        }
        else // still need to break into coordinate even if there's no display color
        {
            int32 ColorNum = Attributes.Normals.size();
            RG.reserve(ColorNum);
            BA.reserve(ColorNum);
            for (int32 ColorIndex = 0; ColorIndex < ColorNum; ++ColorIndex)
            {
                RG.push_back(pxr::GfVec2f(1.0f));
                BA.push_back(pxr::GfVec2f(1.0f));			
            }
        }

        auto PrimvarVC0 = USDMesh.CreatePrimvar(pxr::TfToken("vc"), pxr::SdfValueTypeNames->TexCoord2fArray, Interpolation);
        PrimvarVC0.Set(RG);
        auto PrimvarVC1 = USDMesh.CreatePrimvar(pxr::TfToken("vc1"), pxr::SdfValueTypeNames->TexCoord2fArray, Interpolation);
        PrimvarVC1.Set(BA);
    }
#endif
}

void FUSDExporter::FillUSDMesh(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomXformable& USDMesh, const pxr::TfToken& Interpolation, const TArray<FUSDGeomMeshAttributes>& Attributes)
{
    for (int32 SectionIndex = 0; SectionIndex < Attributes.Num(); ++SectionIndex)
    {
        FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);
        auto SectionPath = USDMesh.GetPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SectionName)));
        auto SectionMesh = pxr::UsdGeomMesh::Define(Stage, SectionPath);
        FillUSDMesh(Stage, SectionMesh, Interpolation, Attributes[SectionIndex]);
    }
}

void FUSDExporter::FillUSDMesh(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomMesh& USDMesh, const pxr::TfToken& Interpolation, const FUSDGeomMeshAttributes& Attributes)
{
    USDMesh.CreatePointsAttr(pxr::VtValue(Attributes.Points));
    USDMesh.CreateFaceVertexCountsAttr(pxr::VtValue(Attributes.FaceVertexCounts));
    USDMesh.CreateFaceVertexIndicesAttr(pxr::VtValue(Attributes.FaceVertexIndices));

    pxr::VtArray<pxr::GfVec3f> Extent;
    pxr::UsdGeomPointBased::ComputeExtent(Attributes.Points, &Extent);
    USDMesh.CreateExtentAttr().Set(Extent);

    // Remove existing subsets to avoid a USD crash
    auto Subsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
    for(auto Subset : Subsets)
    {
        Stage->RemovePrim(Subset.GetPath());
    }

    // Handle subsets
    for (int i = 0; i < Attributes.FaceIndices.size(); ++i)
    {
        pxr::UsdGeomSubset::CreateUniqueGeomSubset(USDMesh, USDTokens.subset, pxr::UsdGeomTokens->face, Attributes.FaceIndices[i]);
    }

    // Normal
    if (!Attributes.Normals.empty())
    {
        USDMesh.CreateNormalsAttr(pxr::VtValue(Attributes.Normals));
        USDMesh.SetNormalsInterpolation(Interpolation);
    }

    // Texture UV
    for (int i = 0; i < Attributes.UVs.size(); ++i)
    {
        auto Primvar = USDMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + (i == 0 ? "" : std::to_string(i))), pxr::SdfValueTypeNames->TexCoord2fArray, Interpolation);
        Primvar.Set(Attributes.UVs[i]);
    }

    FillVertexColor(Stage, USDMesh, Interpolation, Attributes);

    // TangentX
    if (!Attributes.TangentX.empty())
    {
        auto Primvar = USDMesh.CreatePrimvar(USDTokens.tangentX, pxr::SdfValueTypeNames->Float3Array, Interpolation);
        Primvar.Set(Attributes.TangentX);
    }

    USDMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));
}

pxr::UsdGeomXformable FUSDExporter::ExportStaticMeshBackSide(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FStaticMeshLODResources& LODModel, int32 SectionIndex, const FColorVertexBuffer* OverrideVertexColors)
{
    auto USDMesh = pxr::UsdGeomMesh::Define(Stage, Path);
    if (!USDMesh)
    {
        return USDMesh;
    }

    if (SectionIndex < LODModel.Sections.Num())
    {
        FUSDGeomMeshAttributes Attributes;
        const FColorVertexBuffer* ColorVertexBuffer = &LODModel.VertexBuffers.ColorVertexBuffer;
        if (OverrideVertexColors)
        {
            bool bBroken = false;
            const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
            if (Section.MaxVertexIndex >= OverrideVertexColors->GetNumVertices())
            {
                bBroken = true;
            }

            if (!bBroken)
            {
                ColorVertexBuffer = OverrideVertexColors;
            }
        }

        bool bHasColor = ColorVertexBuffer->GetNumVertices() > 0;
        const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
        FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

        // Handle subsets
        const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];

        const int32 NumFaces = Section.NumTriangles;
        if (NumFaces == 0)
        {
            return USDMesh;
        }
        const int32 NumWedges = NumFaces * 3;
        Attributes.FaceVertexCounts.reserve(NumFaces);
        for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
        {
            Attributes.FaceVertexCounts.push_back(3);
        }

        TMap<int32, int32> SectionVerticesMap;
        Attributes.FaceVertexIndices.reserve(NumWedges);
        for (int32 Index = 0; Index < NumWedges; ++Index)
        {
            uint32 VertexIndex = Indices[Section.FirstIndex + Index];
            int32* FoundFaceVertexIndex = SectionVerticesMap.Find(VertexIndex);

            if (FoundFaceVertexIndex)
            {
                Attributes.FaceVertexIndices.push_back(*FoundFaceVertexIndex);
            }
            else
            {
                auto FaceVertexIndex = SectionVerticesMap.Num();
                SectionVerticesMap.Add(VertexIndex, FaceVertexIndex);
                Attributes.FaceVertexIndices.push_back(FaceVertexIndex);
            }
        }

        int32 NumVertices = SectionVerticesMap.Num();
        Attributes.Points.resize(NumVertices);
        Attributes.Normals.resize(NumVertices);
        Attributes.UVs.resize(NumTexCoord);

        for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
        {
            Attributes.UVs[TexCoordIndex].resize(NumVertices);
        }

        if (bHasColor)
        {
            Attributes.Colors.resize(NumVertices);
            Attributes.Opacities.resize(NumVertices);
        }

        int32 SectionVertexIndex = 0;
        for (auto& SectionVertex : SectionVerticesMap)
        {
            int32 VertexIndex = SectionVertex.Key;
            FVector Tangent = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
            FVector Normal = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
            FVector Point = USDConvertPosition(Stage, LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex));
            Point += -0.1f * Normal;
            Attributes.Points[SectionVertexIndex] = USDConvert(Point);
            Attributes.Normals[SectionVertexIndex] = USDConvert(USDConvertVector(Stage, Normal));

            if (bHasColor)
            {
                FLinearColor LinearColor = ColorVertexBuffer->VertexColor(VertexIndex).ReinterpretAsLinear();
                Attributes.Colors[SectionVertexIndex] = USDConvert(LinearColor);
                Attributes.Opacities[SectionVertexIndex] = LinearColor.A;
            }

            for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
            {
                FVector2D TexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, TexCoordIndex);
                TexCoord[1] = 1.f - TexCoord[1];
                Attributes.UVs[TexCoordIndex][SectionVertexIndex] = USDConvert(TexCoord);
            }

            ++SectionVertexIndex;
        }

        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->vertex, Attributes);
    }

    return USDMesh;
}

void FUSDExporter::ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, const bool bExportGeomSubset)
{
    if (bExportGeomSubset || LODModel.Sections.Num() == 1)
    {
        // Create Mesh primitive
        auto USDMesh = pxr::UsdGeomMesh::Get(Stage, Path);
        if (!USDMesh)
        {
            return;
        }
        FUSDGeomMeshAttributes Attributes;
        ExportOverrideVertexColor(Stage, LODModel, OverrideVertexColors, Attributes);
        FillVertexColor(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);
    }
    else
    {
        auto USDMesh = pxr::UsdGeomXform::Get(Stage, Path);
        if (!USDMesh)
        {
            return;
        }

        TArray<FUSDGeomMeshAttributes> Attributes;
        ExportOverrideVertexColor(Stage, LODModel, OverrideVertexColors, Attributes);
        FillVertexColor(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);
    }
}

void FUSDExporter::ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, TArray<FUSDGeomMeshAttributes>& Attributes)
{
    Attributes.AddZeroed(LODModel.Sections.Num());

    const FColorVertexBuffer* ColorVertexBuffer = GetFinalColorVertexBuffer(LODModel, OverrideVertexColors);

    if (!ColorVertexBuffer)
    {
        return;
    }

    bool bHasColor = ColorVertexBuffer->GetNumVertices() > 0;
    if (!bHasColor)
    {
        return;
    }

    bool bAllColorsAreEqual = AllColorsAreEqual(ColorVertexBuffer);
    const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

    // Handle subsets
    for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
    {
        const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
        
        const int32 NumFaces = Section.NumTriangles;
        if (NumFaces == 0)
        {
            continue;
        }
        const int32 NumWedges = NumFaces*3;

        Attributes[SectionIndex].Colors.resize(bAllColorsAreEqual ? 1 : NumWedges);
        Attributes[SectionIndex].Opacities.resize(bAllColorsAreEqual ? 1 : NumWedges);

        TMap<int32, int32> SectionVerticesMap;
        Attributes[SectionIndex].FaceVertexIndices.reserve(NumWedges);
        for (int32 Index = 0; Index < NumWedges; ++Index)
        {
            uint32 VertexIndex = Indices[Section.FirstIndex + Index];

            FLinearColor LinearColor = ColorVertexBuffer->VertexColor(VertexIndex).ReinterpretAsLinear();
            Attributes[SectionIndex].Colors[bAllColorsAreEqual ? 0 : Index] = USDConvert(LinearColor);
            Attributes[SectionIndex].Opacities[bAllColorsAreEqual ? 0 : Index] = LinearColor.A;
        }
    }
}

void FUSDExporter::ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, FUSDGeomMeshAttributes& Attributes)
{
    const int32 NumWedges = LODModel.IndexBuffer.GetNumIndices();
    const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();

    const FColorVertexBuffer* ColorVertexBuffer = GetFinalColorVertexBuffer(LODModel, OverrideVertexColors);

    if (!ColorVertexBuffer)
    {
        return;
    }

    bool bHasColor = ColorVertexBuffer->GetNumVertices() > 0;

    if (!bHasColor)
    {
        return;
    }

    bool bAllColorsAreEqual = AllColorsAreEqual(ColorVertexBuffer);
    Attributes.UVs.resize(NumTexCoord);
    Attributes.Colors.resize(bAllColorsAreEqual ? 1 : NumWedges);
    Attributes.Opacities.resize(bAllColorsAreEqual ? 1 : NumWedges);

    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

    for (int32 Index = 0; Index < NumWedges; ++Index)
    {
        int32 OriginalVertexIndex = Indices[Index];
        FLinearColor LinearColor = ColorVertexBuffer->VertexColor(OriginalVertexIndex).ReinterpretAsLinear();
        Attributes.Colors[bAllColorsAreEqual ? 0 : Index] = USDConvert(LinearColor);
        Attributes.Opacities[bAllColorsAreEqual ? 0 : Index] = LinearColor.A;
    }
}

pxr::UsdGeomXformable FUSDExporter::ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, const bool bExportTangentX, const bool bExportGeomSubset)
{
#if UE_BUILD_DEBUG
    const FString PrimPathText = Path.GetText();
    const FString StagePath = Stage->GetRootLayer()->GetIdentifier().c_str();
#endif
    if (bExportGeomSubset || LODModel.Sections.Num() == 1)
    {
        // Create Mesh primitive
        auto USDMesh = pxr::UsdGeomMesh::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }
        FUSDGeomMeshAttributes Attributes;
        ExportStaticMesh(Stage, LODModel, OverrideVertexColors, Attributes, bExportTangentX, bExportGeomSubset);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);

        return USDMesh;
    }
    else
    {
        auto USDMesh = pxr::UsdGeomXform::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }

        TArray<FUSDGeomMeshAttributes> Attributes;
        ExportStaticMesh(Stage, LODModel, OverrideVertexColors, Attributes, bExportTangentX);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);
        return USDMesh;
    }
}

void FUSDExporter::ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX)
{
    Attributes.AddZeroed(LODModel.Sections.Num());

    const FColorVertexBuffer* ColorVertexBuffer = GetFinalColorVertexBuffer(LODModel, OverrideVertexColors);

    bool bHasColor = ColorVertexBuffer->GetNumVertices() > 0;
    bool bAllColorsAreEqual = AllColorsAreEqual(ColorVertexBuffer);

    const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

    // Handle subsets
    for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
    {
        const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
        
        const int32 NumFaces = Section.NumTriangles;
        if (NumFaces == 0)
        {
            continue;
        }
        const int32 NumWedges = NumFaces*3;
        Attributes[SectionIndex].FaceVertexCounts.reserve(NumFaces);
        for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
        {
            Attributes[SectionIndex].FaceVertexCounts.push_back(3);
        }

        Attributes[SectionIndex].Normals.resize(NumWedges);
        Attributes[SectionIndex].UVs.resize(NumTexCoord);
        if (bExportTangentX)
        {
            Attributes[SectionIndex].TangentX.resize(NumWedges);
        }

        for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
        {
            Attributes[SectionIndex].UVs[TexCoordIndex].resize(NumWedges);
        }

        if (bHasColor)
        {
            Attributes[SectionIndex].Colors.resize(bAllColorsAreEqual ? 1 : NumWedges);
            Attributes[SectionIndex].Opacities.resize(bAllColorsAreEqual ? 1 : NumWedges);
        }

        TMap<int32, int32> SectionVerticesMap;
        Attributes[SectionIndex].FaceVertexIndices.reserve(NumWedges);
        for (int32 Index = 0; Index < NumWedges; ++Index)
        {
            uint32 VertexIndex = Indices[Section.FirstIndex + Index];

            FVector Tangent = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
            FVector Normal = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
            Attributes[SectionIndex].Normals[Index] = USDConvert(USDConvertVector(Stage, Normal));
            if (bExportTangentX)
            {
                Attributes[SectionIndex].TangentX[Index] = USDConvert(USDConvertVector(Stage, Tangent));
            }

            if (bHasColor)
            {
                FLinearColor LinearColor = ColorVertexBuffer->VertexColor(VertexIndex).ReinterpretAsLinear();
                Attributes[SectionIndex].Colors[bAllColorsAreEqual ? 0 : Index] = USDConvert(LinearColor);
                Attributes[SectionIndex].Opacities[bAllColorsAreEqual ? 0 : Index] = LinearColor.A;
            }

            for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
            {
                FVector2D TexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, TexCoordIndex);
                TexCoord[1] = 1.f - TexCoord[1];
                Attributes[SectionIndex].UVs[TexCoordIndex][Index] = USDConvert(TexCoord);
            }

            int32* FoundFaceVertexIndex = SectionVerticesMap.Find(VertexIndex);
            if (!FoundFaceVertexIndex)
            {
                auto FaceVertexIndex = SectionVerticesMap.Num();
                SectionVerticesMap.Add(VertexIndex, FaceVertexIndex);
            }
        }

        TArray<int32> VertexRemap;
        TArray<int32> UniqueVertex;
        TArray<int32> UsedVertex;
        SectionVerticesMap.GenerateKeyArray(UsedVertex);
        WeldVertex(VertexRemap, UniqueVertex, LODModel.VertexBuffers.PositionVertexBuffer, &UsedVertex);

        int32 NumVertices = UniqueVertex.Num();
        Attributes[SectionIndex].Points.resize(NumVertices);

        for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
        {
            FVector Point = USDConvertPosition(Stage, LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(UniqueVertex[VertexIndex]));
            Attributes[SectionIndex].Points[VertexIndex] = USDConvert(Point);
        }

        for (int32 Index = 0; Index < NumWedges; ++Index)
        {
            uint32 VertexIndex = Indices[Section.FirstIndex + Index];
            Attributes[SectionIndex].FaceVertexIndices.push_back(VertexRemap[VertexIndex]);
        }
    }
}

void FUSDExporter::ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const FStaticMeshLODResources& LODModel, const FColorVertexBuffer* OverrideVertexColors, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX, const bool bExportGeomSubset)
{
    // Create Mesh primitive
    // Right hand to left hand conversion
    // Add points
    TArray<int32> VertexRemap;
    TArray<int32> UniqueVertex;
    WeldVertex(VertexRemap, UniqueVertex, LODModel.VertexBuffers.PositionVertexBuffer);

    const int32 NumWedges = LODModel.IndexBuffer.GetNumIndices();
    const int32 NumVertices = UniqueVertex.Num();
    const int32 NumFaces = LODModel.GetNumTriangles();
    const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();

    const FColorVertexBuffer* ColorVertexBuffer = GetFinalColorVertexBuffer(LODModel, OverrideVertexColors);

    bool bHasColor = ColorVertexBuffer->GetNumVertices() > 0;
    bool bAllColorsAreEqual = AllColorsAreEqual(ColorVertexBuffer);

    Attributes.Points.resize(NumVertices);
    Attributes.Normals.resize(NumWedges);
    Attributes.UVs.resize(NumTexCoord);
    if (bExportTangentX)
    {
        Attributes.TangentX.resize(NumWedges);
    }

    for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
    {
        Attributes.UVs[TexCoordIndex].resize(NumWedges);
    }

    if (bHasColor)
    {
        Attributes.Colors.resize(bAllColorsAreEqual ? 1 : NumWedges);
        Attributes.Opacities.resize(bAllColorsAreEqual ? 1 : NumWedges);
    }

    for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
    {
        FVector Point = USDConvertPosition(Stage, LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(UniqueVertex[VertexIndex]));
        Attributes.Points[VertexIndex] = USDConvert(Point);
    }

    // Add faces
    Attributes.FaceVertexCounts.reserve(NumFaces);
    for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
    {
        Attributes.FaceVertexCounts.push_back(3);
    }
    Attributes.FaceVertexIndices.reserve(NumWedges);
    FIndexArrayView Indices = LODModel.IndexBuffer.GetArrayView();

    for (int32 Index = 0; Index < NumWedges; ++Index)
    {
        int32 OriginalVertexIndex = Indices[Index];
        Attributes.FaceVertexIndices.push_back(VertexRemap[OriginalVertexIndex]);

        FVector Tangent = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(OriginalVertexIndex);
        FVector Normal = LODModel.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(OriginalVertexIndex);
        Attributes.Normals[Index] = USDConvert(USDConvertVector(Stage, Normal));
        if (bExportTangentX)
        {
            Attributes.TangentX[Index] = USDConvert(USDConvertVector(Stage, Tangent));
        }
        if (bHasColor)
        {
            FLinearColor LinearColor = ColorVertexBuffer->VertexColor(OriginalVertexIndex).ReinterpretAsLinear();
            Attributes.Colors[bAllColorsAreEqual ? 0 : Index] = USDConvert(LinearColor);
            Attributes.Opacities[bAllColorsAreEqual ? 0 : Index] = LinearColor.A;
        }

        for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
        {
            FVector2D TexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(OriginalVertexIndex, TexCoordIndex);
            TexCoord[1] = 1.f - TexCoord[1];
            Attributes.UVs[TexCoordIndex][Index] = USDConvert(TexCoord);
        }
    }

    if (bExportGeomSubset)
    {
        // Handle subsets
        for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); ++SectionIndex)
        {
            const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];
            pxr::VtArray<int> Subset;
            Subset.reserve(Section.NumTriangles);
            for (uint32 Index = 0; Index < Section.NumTriangles; ++Index)
            {
                Subset.push_back(Section.FirstIndex / 3 + Index);
            }
            Attributes.FaceIndices.push_back(Subset);
        }
    }
}

pxr::UsdGeomXformable FUSDExporter::ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FMeshDescription& MeshDesc, const bool bExportTangentX, const bool bExportGeomSubset)
{
    if (bExportGeomSubset || MeshDesc.PolygonGroups().GetArraySize() == 1)
    {
        // Create Mesh primitive
        auto USDMesh = pxr::UsdGeomMesh::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }

        FUSDGeomMeshAttributes Attributes;
        ExportMeshDescription(Stage, MeshDesc, Attributes, bExportTangentX, bExportGeomSubset);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);

        return USDMesh;
    }
    else
    {
        auto USDMesh = pxr::UsdGeomXform::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }

        TArray<FUSDGeomMeshAttributes> Attributes;
        ExportMeshDescription(Stage, MeshDesc, Attributes, bExportTangentX);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->faceVarying, Attributes);

        return USDMesh;
    }
}

void FUSDExporter::ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const FMeshDescription& MeshDesc, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX)
{
    int32 NumSections = MeshDesc.PolygonGroups().GetArraySize();
    Attributes.AddZeroed(NumSections);

    TVertexAttributesConstRef<FVector> Positions = MeshDesc.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);

    int32 SectionIndex = 0;
    // Handle subsets
    for (const FPolygonGroupID PolygonGroupID : MeshDesc.PolygonGroups().GetElementIDs())
    {
        const TArray<FPolygonID>& PolygonIDs = MeshDesc.GetPolygonGroupPolygons(PolygonGroupID);

        if (PolygonIDs.Num() == 0)
        {
            continue;
        }

        int FaceIndexCount = Algo::Accumulate(PolygonIDs, 0,
            [&](int Result, FPolygonID PolygonID)
            {
                return Result + MeshDesc.GetPolygonVertexInstances(PolygonID).Num();
            }
        );
        Attributes[SectionIndex].FaceVertexIndices.reserve(FaceIndexCount);
        Attributes[SectionIndex].FaceVertexCounts.reserve(PolygonIDs.Num());

        TMap<FVertexID, FVector> PointIndices;
        for (FPolygonID PolygonID : PolygonIDs)
        {
            TArray<FVertexID> VertexIDs;
            MeshDesc.GetPolygonVertices(PolygonID, VertexIDs);

            Attributes[SectionIndex].FaceVertexCounts.push_back(VertexIDs.Num());
            for (auto VertexID : VertexIDs)
            {
                FVector Point = USDConvertPosition(Stage, Positions[VertexID]);
                PointIndices.FindOrAdd(VertexID, Point);
            }
        }

        Attributes[SectionIndex].Points.resize(PointIndices.Num());
        TMap<FVertexID, int> VertexIDToPointIndex;
        VertexIDToPointIndex.Reserve(PointIndices.Num());
        {
            int32 PointIndex = 0;
            for (auto& Elem : PointIndices)
            {
                Attributes[SectionIndex].Points[PointIndex] = USDConvert(Elem.Value);
                VertexIDToPointIndex.Add(Elem.Key, PointIndex++);
            }
        }

        for (FPolygonID PolygonID : PolygonIDs)
        {
            TArray<FVertexID> VertexIDs;
            MeshDesc.GetPolygonVertices(PolygonID, VertexIDs);

            for (auto VertexID : VertexIDs)
            {
                Attributes[SectionIndex].FaceVertexIndices.push_back(VertexIDToPointIndex[VertexID]);
            }
        }

        // Vertex color
        do
        {
            auto PrimvarValues = USDConvert<FVector4, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Color, PolygonIDs, 0,
                [](const FVector4& Color)
            {
                return USDConvert(FLinearColor(Color));
            }
            );

            if (PrimvarValues.empty())
            {
                break;
            }

            if (std::all_of(
                PrimvarValues.begin(),
                PrimvarValues.end(),
                [](const auto& Color) {return Color == pxr::GfVec3f(1); }
            ))
            {
                break;
            }

            Attributes[SectionIndex].Colors = PrimvarValues;
        } while (false);

        // Vertex color opacity
        do
        {
            auto PrimvarValues = USDConvert<FVector4, float>(MeshDesc, MeshAttribute::VertexInstance::Color, PolygonIDs, 0,
                [](const FVector4& Color)
            {
                return FLinearColor(Color).A;
            }
            );

            if (PrimvarValues.empty())
            {
                break;
            }

            if (std::all_of(
                PrimvarValues.begin(),
                PrimvarValues.end(),
                [](const auto& Opacity) {return Opacity == 1; }
            ))
            {
                break;
            }

            Attributes[SectionIndex].Opacities = PrimvarValues;
        } while (false);

        // Normal
        Attributes[SectionIndex].Normals = USDConvert<FVector, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Normal, PolygonIDs, 0,
            [&](auto Normal)
        {
            return USDConvert(USDConvertVector(Stage, Normal));
        }
        );

        if (bExportTangentX)
        {
            // TangentX
            Attributes[SectionIndex].TangentX = USDConvert<FVector, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Tangent, PolygonIDs, 0,
                [&](auto MeshTangentX)
            {
                return USDConvert(USDConvertVector(Stage, MeshTangentX));
            }
            );
        }

        // Texture UV
        for (int32 UVIndex = 0; true; ++UVIndex)
        {
            auto UVValues = USDConvert<FVector2D, pxr::GfVec2f>(MeshDesc, MeshAttribute::VertexInstance::TextureCoordinate, PolygonIDs, UVIndex);
            if (UVValues.empty())
            {
                break;
            }

            for (auto& UV : UVValues)
            {
                UV[1] = 1 - UV[1];
            }

            Attributes[SectionIndex].UVs.push_back(UVValues);
        }

        ++SectionIndex;
    }
}

void FUSDExporter::ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const FMeshDescription& MeshDesc, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX, const bool bExportGeomSubset)
{
    // Create Mesh primitive
    // Right hand to left hand conversion
    // Add points
    auto Vertices = MeshDesc.Vertices();
    TVertexAttributesConstRef<FVector> Positions = MeshDesc.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);

    Attributes.Points.resize(Vertices.Num());

    TMap<FVertexID, int> VertexIDToPointIndex;
    VertexIDToPointIndex.Reserve(Vertices.Num());

    int PointIndex = 0;
    for (FVertexID VertexID : Vertices.GetElementIDs())
    {
        FVector Point = USDConvertPosition(Stage, Positions[VertexID]);

        VertexIDToPointIndex.Add(VertexID, PointIndex);
        Attributes.Points[PointIndex++] = USDConvert(Point);
    }

    // Add faces
    Attributes.FaceVertexCounts.resize(MeshDesc.Polygons().Num());
    int FaceIndexCount = Algo::Accumulate(MeshDesc.Polygons().GetElementIDs(), 0,
        [&](int Result, FPolygonID PolygonID)
    {
        return Result + MeshDesc.GetPolygonVertexInstances(PolygonID).Num();
    }
    );
    Attributes.FaceVertexIndices.reserve(FaceIndexCount);

    int FaceIndex = 0;
    for (FPolygonID PolygonID : MeshDesc.Polygons().GetElementIDs())
    {
        TArray<FVertexID> VertexIDs;
        MeshDesc.GetPolygonVertices(PolygonID, VertexIDs);

        Attributes.FaceVertexCounts[FaceIndex++] = VertexIDs.Num();

        for (auto VertexID : VertexIDs)
        {
            Attributes.FaceVertexIndices.push_back(VertexIDToPointIndex[VertexID]);
        }
    }

    if (bExportGeomSubset)
    {
        // Handle subsets
        for (const FPolygonGroupID PolygonGroupID : MeshDesc.PolygonGroups().GetElementIDs())
        {
            const TArray<FPolygonID>& PolygonIDs = MeshDesc.GetPolygonGroupPolygons(PolygonGroupID);
            pxr::VtArray<int> Subset;
            Subset.reserve(PolygonIDs.Num());

            for (auto PolygonID : PolygonIDs)
            {
                Subset.push_back(PolygonID.GetValue());
            }

            Attributes.FaceIndices.push_back(Subset);
        }
    }

    // Vertex color
    do
    {
        auto PrimvarValues = USDConvert<FVector4, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Color, 0,
            [](const FVector4& Color)
        {
            return USDConvert(FLinearColor(Color));
        }
        );

        if (PrimvarValues.empty())
        {
            break;
        }

        if (std::all_of(
            PrimvarValues.begin(),
            PrimvarValues.end(),
            [](const auto& Color) {return Color == pxr::GfVec3f(1); }
        ))
        {
            break;
        }

        Attributes.Colors = PrimvarValues;
    } while (false);

    // Vertex color opacity
    do
    {
        auto PrimvarValues = USDConvert<FVector4, float>(MeshDesc, MeshAttribute::VertexInstance::Color, 0,
            [](const FVector4& Color)
        {
            return FLinearColor(Color).A;
        }
        );

        if (PrimvarValues.empty())
        {
            break;
        }

        if (std::all_of(
            PrimvarValues.begin(),
            PrimvarValues.end(),
            [](const auto& Opacity) {return Opacity == 1; }
        ))
        {
            break;
        }

        Attributes.Opacities = PrimvarValues;
    } while (false);

    // Normal
    Attributes.Normals = USDConvert<FVector, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Normal, 0,
        [&](auto Normal)
    {
        return USDConvert(USDConvertVector(Stage, Normal));
    }
    );

    if (bExportTangentX)
    {
        // TangentX
        Attributes.TangentX = USDConvert<FVector, pxr::GfVec3f>(MeshDesc, MeshAttribute::VertexInstance::Tangent, 0,
            [&](auto MeshTangentX)
        {
            return USDConvert(USDConvertVector(Stage, MeshTangentX));
        }
        );
    }

    // Texture UV
    for (int32 UVIndex = 0; true; ++UVIndex)
    {
        auto UVValues = USDConvert<FVector2D, pxr::GfVec2f>(MeshDesc, MeshAttribute::VertexInstance::TextureCoordinate, UVIndex);
        if (UVValues.empty())
        {
            break;
        }

        for (auto& UV : UVValues)
        {
            UV[1] = 1 - UV[1];
        }

        Attributes.UVs.push_back(UVValues);
    }
}

pxr::UsdGeomCamera ConvertUSDCamera(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const UCameraComponent& Camera, const TArray<FTimeSampleRange>* Ranges)
{
    const FNamedParameterTimeSamples* ParameterTimeSamples = nullptr;
    ParameterTimeSamples = FSequenceExtractor::Get().GetParameterTimeSamples(&Camera);

    auto USDCamera = pxr::UsdGeomCamera::Define(Stage, Path);

    auto static ConvertVector = [](const FVector4 & Vector) -> pxr::GfVec4f
    {
        return pxr::GfVec4f(Vector.X, Vector.Y, Vector.Z, Vector.W);
    };

    // Batch the updates
    pxr::SdfChangeBlock ChangeBlock;

    pxr::GfCamera GFCamera;
    if (auto CineCamera = Cast<UCineCameraComponent>(&Camera))
    {
        auto CameraPrim = USDCamera.GetPrim();

#define STORE_PROPERTY(PropertyName, PropertyValue) \
        CameraPrim.SetCustomDataByKey(pxr::TfToken(PropertyName), pxr::VtValue(PropertyValue));
        
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_MIN_FSTOP, CineCamera->LensSettings.MinFStop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_MAX_FSTOP, CineCamera->LensSettings.MaxFStop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY, CineCamera->PostProcessSettings.SceneFringeIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET, CineCamera->PostProcessSettings.ChromaticAberrationStartOffset);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD, (int) CineCamera->PostProcessSettings.BloomMethod.GetValue());
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY, CineCamera->PostProcessSettings.BloomIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD, CineCamera->PostProcessSettings.BloomThreshold);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED, CineCamera->PostProcessSettings.CameraShutterSpeed);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO, CineCamera->PostProcessSettings.CameraISO);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE, CineCamera->PostProcessSettings.DepthOfFieldFstop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE, CineCamera->PostProcessSettings.DepthOfFieldMinFstop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES, CineCamera->PostProcessSettings.DepthOfFieldBladeCount);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN, CineCamera->PostProcessSettings.AutoExposureBias);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE, (int) CineCamera->PostProcessSettings.AutoExposureMethod.GetValue());
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS, CineCamera->PostProcessSettings.AutoExposureMaxBrightness);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS, CineCamera->PostProcessSettings.AutoExposureMinBrightness);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP, CineCamera->PostProcessSettings.AutoExposureSpeedUp);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN, CineCamera->PostProcessSettings.AutoExposureSpeedDown);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY, CineCamera->PostProcessSettings.VignetteIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER, CineCamera->PostProcessSettings.GrainJitter);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY, CineCamera->PostProcessSettings.GrainIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM, CineCamera->PostProcessSettings.DepthOfFieldDepthBlurAmount);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS, CineCamera->PostProcessSettings.DepthOfFieldDepthBlurRadius);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE, CineCamera->PostProcessSettings.DepthOfFieldFarBlurSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION, CineCamera->PostProcessSettings.DepthOfFieldFarTransitionRegion);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION, CineCamera->PostProcessSettings.DepthOfFieldFocalRegion);
        //4.22 deprecated STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE, CineCamera->PostProcessSettings.DepthOfFieldMaxBokehSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE, CineCamera->PostProcessSettings.DepthOfFieldNearBlurSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION, CineCamera->PostProcessSettings.DepthOfFieldNearTransitionRegion);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE, CineCamera->PostProcessSettings.DepthOfFieldScale);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE, CineCamera->PostProcessSettings.DepthOfFieldFocalDistance);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF, CineCamera->PostProcessSettings.bMobileHQGaussian);
        //4.22 deprecated  STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD, (int) CineCamera->PostProcessSettings.DepthOfFieldMethod.GetValue());
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST, ConvertVector(CineCamera->PostProcessSettings.ColorContrast));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION, ConvertVector(CineCamera->PostProcessSettings.ColorSaturation));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN, ConvertVector(CineCamera->PostProcessSettings.ColorGain));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA, ConvertVector(CineCamera->PostProcessSettings.ColorGamma));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET, ConvertVector(CineCamera->PostProcessSettings.ColorOffset));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST, ConvertVector(CineCamera->PostProcessSettings.ColorContrastShadows));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION, ConvertVector(CineCamera->PostProcessSettings.ColorSaturationShadows));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN, ConvertVector(CineCamera->PostProcessSettings.ColorGainShadows));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA, ConvertVector(CineCamera->PostProcessSettings.ColorGammaShadows));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET, ConvertVector(CineCamera->PostProcessSettings.ColorOffsetShadows));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX, CineCamera->PostProcessSettings.ColorCorrectionShadowsMax);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST, ConvertVector(CineCamera->PostProcessSettings.ColorContrastMidtones));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION, ConvertVector(CineCamera->PostProcessSettings.ColorSaturationMidtones));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN, ConvertVector(CineCamera->PostProcessSettings.ColorGainMidtones));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA, ConvertVector(CineCamera->PostProcessSettings.ColorGammaMidtones));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET, ConvertVector(CineCamera->PostProcessSettings.ColorOffsetMidtones));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST, ConvertVector(CineCamera->PostProcessSettings.ColorContrastHighlights));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION, ConvertVector(CineCamera->PostProcessSettings.ColorSaturationHighlights));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN, ConvertVector(CineCamera->PostProcessSettings.ColorGainHighlights));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA, ConvertVector(CineCamera->PostProcessSettings.ColorGammaHighlights));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET, ConvertVector(CineCamera->PostProcessSettings.ColorOffsetHighlights));
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN, CineCamera->PostProcessSettings.ColorCorrectionHighlightsMin);

        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_SceneFringeIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ChromaticAberrationStartOffset);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomMethod);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomThreshold);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED_TOGGLE, CineCamera->PostProcessSettings.bOverride_CameraShutterSpeed);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO_TOGGLE, CineCamera->PostProcessSettings.bOverride_CameraISO);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFstop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMinFstop);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldBladeCount);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureBias);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMethod);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMaxBrightness);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMinBrightness);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureSpeedUp);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureSpeedDown);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_VignetteIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER_TOGGLE, CineCamera->PostProcessSettings.bOverride_GrainJitter);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_GrainIntensity);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldDepthBlurAmount);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldDepthBlurRadius);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFarBlurSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFarTransitionRegion);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFocalRegion);
        //4.22 deprecated STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMaxBokehSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldNearBlurSize);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldNearTransitionRegion);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldScale);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF_TOGGLE, CineCamera->PostProcessSettings.bOverride_MobileHQGaussian);
        //4.22 deprecated STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMethod);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrast);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturation);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGain);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGamma);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffset);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastShadows);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationShadows);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainShadows);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaShadows);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetShadows);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorCorrectionShadowsMax);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastMidtones);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationMidtones);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainMidtones);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaMidtones);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetMidtones);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastHighlights);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationHighlights);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainHighlights);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaHighlights);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetHighlights);
        STORE_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorCorrectionHighlightsMin);

        GFCamera.SetProjection(pxr::GfCamera::Perspective);
        GFCamera.SetClippingRange(pxr::GfRange1f(USD_CAMERA_CLIPPING_RANGE_MIN, USD_CAMERA_CLIPPING_RANGE_MAX));

        auto Name = GET_MEMBER_NAME_CHECKED(UCineCameraComponent, CurrentAperture);
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                GFCamera.SetFStop(TimeSample); //seq
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetFStopAttr().Set(GFCamera.GetFStop(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            GFCamera.SetFStop(CineCamera->CurrentAperture); //seq
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for(auto TimeCode : TimeCodes)
                {
                    USDCamera.GetFStopAttr().Set(GFCamera.GetFStop(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetFStopAttr().Set(GFCamera.GetFStop()); //seq
            }
        }

        Name = GET_MEMBER_NAME_CHECKED(UCineCameraComponent, CurrentFocalLength); //mm or tenths of world unit
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                GFCamera.SetFocalLength(USDConvertLength(Stage, TimeSample)); //seq
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetFocalLengthAttr().Set(GFCamera.GetFocalLength(), pxr::UsdTimeCode(TimeCode));  //seq
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            GFCamera.SetFocalLength(USDConvertLength(Stage, CineCamera->CurrentFocalLength)); //seq
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetFocalLengthAttr().Set(GFCamera.GetFocalLength(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetFocalLengthAttr().Set(GFCamera.GetFocalLength());  //seq
            }
        }

        Name = GET_MEMBER_NAME_CHECKED(UCineCameraComponent, FocusSettings.ManualFocusDistance); // cm or world unit
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                GFCamera.SetFocusDistance(USDConvertLength(Stage, TimeSample)); //seq
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetFocusDistanceAttr().Set(GFCamera.GetFocusDistance(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            GFCamera.SetFocusDistance(USDConvertLength(Stage, CineCamera->FocusSettings.ManualFocusDistance)); //seq
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetFocusDistanceAttr().Set(GFCamera.GetFocusDistance(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetFocusDistanceAttr().Set(GFCamera.GetFocusDistance()); //seq
            }
        }

        Name = GET_MEMBER_NAME_CHECKED(UCineCameraComponent, Filmback.SensorWidth); // mm or tenths of world unit
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetHorizontalApertureAttr().Set(USDConvertLength(Stage, TimeSample), pxr::UsdTimeCode(TimeCode));
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            // Set a clipping range that works better in Omni Kit
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetHorizontalApertureAttr().Set(USDConvertLength(Stage, CineCamera->Filmback.SensorWidth), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetHorizontalApertureAttr().Set(USDConvertLength(Stage, CineCamera->Filmback.SensorWidth)); //AspectRatio Horizontal / Vertical
            }
        }

        Name = GET_MEMBER_NAME_CHECKED(UCineCameraComponent, Filmback.SensorHeight); // mm or tenths of world unit
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetVerticalApertureAttr().Set(USDConvertLength(Stage, TimeSample), pxr::UsdTimeCode(TimeCode));
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            // Set a clipping range that works better in Omni Kit
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetVerticalApertureAttr().Set(USDConvertLength(Stage, CineCamera->Filmback.SensorHeight), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetVerticalApertureAttr().Set(USDConvertLength(Stage, CineCamera->Filmback.SensorHeight)); //AspectRatio Horizontal / Vertical
            }
        }
    }
    else
    {
        FPostProcessSettings PostProcessSettings;
        PostProcessSettings.SetBaseValues();
        OverridePostProcessSettings(Camera.PostProcessSettings, Camera.PostProcessBlendWeight, PostProcessSettings);

        switch (Camera.ProjectionMode)
        {
        case ECameraProjectionMode::Perspective:
            GFCamera.SetProjection(pxr::GfCamera::Perspective);
            GFCamera.SetPerspectiveFromAspectRatioAndFieldOfView(Camera.AspectRatio, Camera.FieldOfView, pxr::GfCamera::FOVDirection::FOVHorizontal); //seq
            // Set a clipping range that works better in Omni Kit
            GFCamera.SetClippingRange(pxr::GfRange1f(USD_CAMERA_CLIPPING_RANGE_MIN, USD_CAMERA_CLIPPING_RANGE_MAX));
            break;
        case ECameraProjectionMode::Orthographic:
            GFCamera.SetProjection(pxr::GfCamera::Orthographic);
            GFCamera.SetOrthographicFromAspectRatioAndSize(Camera.AspectRatio, Camera.OrthoWidth, pxr::GfCamera::FOVDirection::FOVHorizontal);
            GFCamera.SetClippingRange(pxr::GfRange1f(Camera.OrthoNearClipPlane, Camera.OrthoFarClipPlane));
            break;
        }

        auto VerticalAperture = GFCamera.GetVerticalAperture();
        auto HorizontalAperture = GFCamera.GetHorizontalAperture();

        auto Name = GET_MEMBER_NAME_CHECKED(UCameraComponent, AspectRatio);
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetVerticalApertureAttr().Set(HorizontalAperture / FMath::Max(TimeSample, 0.1f), pxr::UsdTimeCode(TimeCode));
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);	
        }
        else
        {
            // Set a clipping range that works better in Omni Kit
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetVerticalApertureAttr().Set(VerticalAperture, pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetVerticalApertureAttr().Set(VerticalAperture);
            }
        }
        USDCamera.GetHorizontalApertureAttr().Set(HorizontalAperture); //AspectRatio Horizontal / Vertical

        Name = GET_MEMBER_NAME_CHECKED(UCameraComponent, FieldOfView);
        if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
        {
            auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
            {
                if (IsInRange(TimeCode, Ranges))
                {
                    USDCamera.GetFocalLengthAttr().Set((HorizontalAperture / FMath::Tan(FMath::DegreesToRadians(FMath::Max(TimeSample, 0.1f)) * 0.5f)) * 0.5f, pxr::UsdTimeCode(TimeCode));
                }
            };
            ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
        }
        else
        {
            if (Ranges)
            {
                auto TimeCodes = GetTimeCodesFromRange(Ranges);
                for (auto TimeCode : TimeCodes)
                {
                    USDCamera.GetFocalLengthAttr().Set(GFCamera.GetFocalLength(), pxr::UsdTimeCode(TimeCode)); //seq
                }
            }
            else
            {
                USDCamera.GetFocalLengthAttr().Set(GFCamera.GetFocalLength());  //seq
            }
        }
        USDCamera.GetFStopAttr().Set(GFCamera.GetFStop()); //seq
        USDCamera.GetFocusDistanceAttr().Set(GFCamera.GetFocusDistance()); //seq
    }

    // Don't use SetCamera as it will override transform, which will cause crash for layers
    USDCamera.GetProjectionAttr().Set(pxr::UsdGeomTokens->perspective);
    USDCamera.GetHorizontalApertureOffsetAttr().Set(GFCamera.GetHorizontalApertureOffset());
    USDCamera.GetVerticalApertureOffsetAttr().Set(GFCamera.GetVerticalApertureOffset());

    auto ClippingRange = GFCamera.GetClippingRange();
    USDCamera.GetClippingRangeAttr().Set(pxr::GfVec2f(ClippingRange.GetMin(), ClippingRange.GetMax()));
    pxr::VtArray<pxr::GfVec4f> Planes;
    auto ClippingPlanes = GFCamera.GetClippingPlanes();
    Planes.assign(ClippingPlanes.begin(), ClippingPlanes.end());
    USDCamera.GetClippingPlanesAttr().Set(Planes);

    return USDCamera;
}

// hacked copy of FDynamicSpriteEmitterDataBase::CalculateParticleTransform, that is safe to run on game thread. avert your eyes.
// long term TODO: refactor FDynamicSpriteEmitterDataBase to isolate logic from render-thread data, so we can call it from here
void CalculateParticleTransform(
    UParticleModuleRequired* RequiredModule,
    const FVector& Scale,
    const FMatrix& ProxyLocalToWorld,
    const FVector& ParticleLocation, float ParticleRotation,
    const FVector& ParticleVelocity,
    const FVector& ParticleSize, const FVector& ViewOrigin,
    const FVector& ViewDirection,
    FMatrix& OutTransformMat)
{
    //const FDynamicSpriteEmitterReplayDataBase* SourceData = GetSourceData();

    FVector CameraFacingOpVector(0.0f, 0.0f, 1.0f);

    OutTransformMat = FMatrix::Identity;

    FVector ParticlePosition(ParticleLocation);
    FTranslationMatrix kTransMat(FVector::ZeroVector);
    kTransMat.M[3][0] = ParticlePosition.X;
    kTransMat.M[3][1] = ParticlePosition.Y;
    kTransMat.M[3][2] = ParticlePosition.Z;

    FVector ScaledSize = ParticleSize * Scale;
    FScaleMatrix kScaleMat(FVector(1.0f));
    kScaleMat.M[0][0] = ScaledSize.X;
    kScaleMat.M[1][1] = ScaledSize.Y;
    kScaleMat.M[2][2] = ScaledSize.Z;

    FMatrix LocalToWorld = ProxyLocalToWorld;

    FVector Location = ParticlePosition;
    FVector VelocityDirection = ParticleVelocity;

    if (RequiredModule->bUseLocalSpace)
    {
        // Transform the location to world space
        Location = LocalToWorld.TransformPosition(Location);
        VelocityDirection = LocalToWorld.InverseFast().GetTransposed().TransformVector(VelocityDirection);
        // Set the translation matrix to the location
        kTransMat.SetOrigin(Location);
        // Set Local2World to identify to remove any rotational information
        LocalToWorld.SetIdentity();
    }
    VelocityDirection.Normalize();

    FVector DirToCamera = ViewOrigin - Location;
    DirToCamera.Normalize();
    if (DirToCamera.SizeSquared() < 0.5f)
    {
        // Assert possible if DirToCamera is not normalized
        DirToCamera = FVector(1, 0, 0);
    }

    // Camera face the X-axis, and point the selected axis towards the world up
    FQuat PointTo = FQuat::FindBetweenNormals(FVector(1, 0, 0), DirToCamera);
    FVector FacingDir = DirToCamera;
    FVector DesiredDir;
    if (RequiredModule->ScreenAlignment == PSA_Velocity)
    {
        DesiredDir = VelocityDirection;
    }
    else
    {
        DesiredDir = FVector(0, 0, 1);
    }

    FVector DirToDesiredInRotationPlane = DesiredDir - ((DesiredDir | FacingDir) * FacingDir);
    DirToDesiredInRotationPlane.Normalize();
    FQuat FacingRotation =
        FQuat::FindBetweenNormals(PointTo.RotateVector(CameraFacingOpVector), DirToDesiredInRotationPlane);
    PointTo = FacingRotation * PointTo;

    // Add in additional rotation about either the directional or camera facing axis
    FQuat AddedRotation = FQuat(DirToCamera, ParticleRotation);
    FQuat kLockedAxisQuat = (AddedRotation * PointTo);

    FMatrix RotMatrix = FQuatRotationMatrix(kLockedAxisQuat);
    OutTransformMat = kScaleMat * RotMatrix * kTransMat;

    if (RequiredModule->bUseLocalSpace)
    {
        OutTransformMat *= LocalToWorld;
    }
}

// hacked copy of FDynamicMeshEmitterData::CalculateParticleTransform, that is safe to run on game thread. avert your eyes.
// long term TODO: refactor FDynamicMeshEmitterData to isolate logic from render-thread data, so we can call it from here
void CalculateParticleTransform(
    const UParticleModuleRequired* RequiredModule,
    const UParticleModuleTypeDataMesh* MeshModule,
    const FMatrix& ProxyLocalToWorld,
    const FVector& ParticleLocation,
    float    ParticleRotation,
    const FVector& ParticleVelocity,
    const FVector& ParticleSize,
    const FVector& ParticlePayloadInitialOrientation,
    const FVector& ParticlePayloadRotation,
    const FVector& ParticlePayloadCameraOffset,
    const FVector& ParticlePayloadOrbitOffset,
    const FVector& ViewOrigin,
    const FVector& ViewDirection,
    const FVector& Scale,
    FMatrix& OutTransformMat
)
{
    bool bApplyPreRotation(false);
    bool bFaceCameraDirectionRatherThanPosition(false);
    uint8 CameraFacingOption(0);
    bool bUseMeshLockedAxis(false);
    bool bUseCameraFacing(false);
    FVector LockedAxis;
    bool bApplyParticleRotationAsSpin(false);

    // adapted from FDynamicMeshEmitterData::Init
    // Setup the camera facing options
    if (MeshModule->bCameraFacing == true)
    {
        bUseCameraFacing = true;
        CameraFacingOption = MeshModule->CameraFacingOption;
        bApplyParticleRotationAsSpin = MeshModule->bApplyParticleRotationAsSpin;
        bFaceCameraDirectionRatherThanPosition = MeshModule->bFaceCameraDirectionRatherThanPosition;
    }

    // Camera facing trumps locked axis... but can still use it.
    // Setup the locked axis option
    uint8 CheckAxisLockOption = MeshModule->AxisLockOption;
    if ((CheckAxisLockOption >= EPAL_X) && (CheckAxisLockOption <= EPAL_NEGATIVE_Z))
    {
        bUseMeshLockedAxis = true;
        LockedAxis = FVector(
            (CheckAxisLockOption == EPAL_X) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_X) ? -1.0f : 0.0),
            (CheckAxisLockOption == EPAL_Y) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_Y) ? -1.0f : 0.0),
            (CheckAxisLockOption == EPAL_Z) ? 1.0f : ((CheckAxisLockOption == EPAL_NEGATIVE_Z) ? -1.0f : 0.0)
        );
    }
    else if ((CameraFacingOption >= LockedAxis_ZAxisFacing) && (CameraFacingOption <= LockedAxis_NegativeYAxisFacing))
    {
        // Catch the case where we NEED locked axis...
        bUseMeshLockedAxis = true;
        LockedAxis = FVector(1.0f, 0.0f, 0.0f);
    }



    FVector CameraFacingOpVector = FVector::ZeroVector;
    if (MeshModule->CameraFacingOption != XAxisFacing_NoUp)
    {
        switch (MeshModule->CameraFacingOption)
        {
        case XAxisFacing_ZUp:
            CameraFacingOpVector = FVector(0.0f, 0.0f, 1.0f);
            break;
        case XAxisFacing_NegativeZUp:
            CameraFacingOpVector = FVector(0.0f, 0.0f, -1.0f);
            break;
        case XAxisFacing_YUp:
            CameraFacingOpVector = FVector(0.0f, 1.0f, 0.0f);
            break;
        case XAxisFacing_NegativeYUp:
            CameraFacingOpVector = FVector(0.0f, -1.0f, 0.0f);
            break;
        case LockedAxis_YAxisFacing:
        case VelocityAligned_YAxisFacing:
            CameraFacingOpVector = FVector(0.0f, 1.0f, 0.0f);
            break;
        case LockedAxis_NegativeYAxisFacing:
        case VelocityAligned_NegativeYAxisFacing:
            CameraFacingOpVector = FVector(0.0f, -1.0f, 0.0f);
            break;
        case LockedAxis_ZAxisFacing:
        case VelocityAligned_ZAxisFacing:
            CameraFacingOpVector = FVector(0.0f, 0.0f, 1.0f);
            break;
        case LockedAxis_NegativeZAxisFacing:
        case VelocityAligned_NegativeZAxisFacing:
            CameraFacingOpVector = FVector(0.0f, 0.0f, -1.0f);
            break;
        }
    }

    FQuat PointToLockedAxis;
    if (bUseMeshLockedAxis == true)
    {
        // facing axis is taken to be the local x axis.	
        PointToLockedAxis = FQuat::FindBetweenNormals(FVector(1, 0, 0), LockedAxis);
    }

    OutTransformMat = FMatrix::Identity;

    FTranslationMatrix kTransMat(FVector::ZeroVector);
    FScaleMatrix kScaleMat(FVector(1.0f));
    FQuat kLockedAxisQuat = FQuat::Identity;

    FVector ParticlePosition(ParticleLocation + ParticlePayloadCameraOffset);
    kTransMat.M[3][0] = ParticlePosition.X;
    kTransMat.M[3][1] = ParticlePosition.Y;
    kTransMat.M[3][2] = ParticlePosition.Z;

    FVector ScaledSize = ParticleSize * Scale;
    kScaleMat.M[0][0] = ScaledSize.X;
    kScaleMat.M[1][1] = ScaledSize.Y;
    kScaleMat.M[2][2] = ScaledSize.Z;

    FMatrix kRotMat(FMatrix::Identity);
    FMatrix LocalToWorld = ProxyLocalToWorld;

    FVector	LocalSpaceFacingAxis;
    FVector	LocalSpaceUpAxis;
    FVector Location;
    FVector	DirToCamera;
    FQuat PointTo = PointToLockedAxis;

    if (bUseCameraFacing)
    {
        Location = ParticlePosition;
        FVector	VelocityDirection = ParticleVelocity;

        if (RequiredModule->bUseLocalSpace)
        {
            bool bClearLocal2World = false;

            // Transform the location to world space
            Location = LocalToWorld.TransformPosition(Location);
            if (CameraFacingOption <= XAxisFacing_NegativeYUp)
            {
                bClearLocal2World = true;
            }
            else if (CameraFacingOption >= VelocityAligned_ZAxisFacing)
            {
                bClearLocal2World = true;
                VelocityDirection = LocalToWorld.InverseFast().GetTransposed().TransformVector(VelocityDirection);
            }

            if (bClearLocal2World)
            {
                // Set the translation matrix to the location
                kTransMat.SetOrigin(Location);
                // Set Local2World to identify to remove any rotational information
                LocalToWorld.SetIdentity();
            }
        }
        VelocityDirection.Normalize();

        if (bFaceCameraDirectionRatherThanPosition)
        {
            DirToCamera = -ViewDirection;
        }
        else
        {
            DirToCamera = ViewOrigin - Location;
        }

        DirToCamera.Normalize();
        if (DirToCamera.SizeSquared() < 0.5f)
        {
            // Assert possible if DirToCamera is not normalized
            DirToCamera = FVector(1, 0, 0);
        }

        bool bFacingDirectionIsValid = true;
        if (CameraFacingOption != XAxisFacing_NoUp)
        {
            FVector FacingDir;
            FVector DesiredDir;

            if ((CameraFacingOption >= VelocityAligned_ZAxisFacing) &&
                (CameraFacingOption <= VelocityAligned_NegativeYAxisFacing))
            {
                if (VelocityDirection.IsNearlyZero())
                {
                    // We have to fudge it
                    bFacingDirectionIsValid = false;
                }

                // Velocity align the X-axis, and camera face the selected axis
                PointTo = FQuat::FindBetweenNormals(FVector(1.0f, 0.0f, 0.0f), VelocityDirection);
                FacingDir = VelocityDirection;
                DesiredDir = DirToCamera;
            }
            else if (CameraFacingOption <= XAxisFacing_NegativeYUp)
            {
                // Camera face the X-axis, and point the selected axis towards the world up
                PointTo = FQuat::FindBetweenNormals(FVector(1, 0, 0), DirToCamera);
                FacingDir = DirToCamera;
                DesiredDir = FVector(0, 0, 1);
            }
            else
            {
                // Align the X-axis with the selected LockAxis, and point the selected axis towards the camera
                // PointTo will contain quaternion for locked axis rotation.
                FacingDir = LockedAxis;

                if (RequiredModule->bUseLocalSpace)
                {
                    //Transform the direction vector into local space.
                    DesiredDir = LocalToWorld.GetTransposed().TransformVector(DirToCamera);
                }
                else
                {
                    DesiredDir = DirToCamera;
                }
            }

            FVector	DirToDesiredInRotationPlane = DesiredDir - ((DesiredDir | FacingDir) * FacingDir);
            DirToDesiredInRotationPlane.Normalize();
            FQuat FacingRotation = FQuat::FindBetweenNormals(PointTo.RotateVector(CameraFacingOpVector), DirToDesiredInRotationPlane);
            PointTo = FacingRotation * PointTo;

            // Add in additional rotation about either the directional or camera facing axis
            if (bApplyParticleRotationAsSpin)
            {
                if (bFacingDirectionIsValid)
                {
                    FQuat AddedRotation = FQuat(FacingDir, ParticleRotation);
                    kLockedAxisQuat = (AddedRotation * PointTo);
                }
            }
            else
            {
                FQuat AddedRotation = FQuat(DirToCamera, ParticleRotation);
                kLockedAxisQuat = (AddedRotation * PointTo);
            }
        }
        else
        {
            PointTo = FQuat::FindBetweenNormals(FVector(1, 0, 0), DirToCamera);
            // Add in additional rotation about facing axis
            FQuat AddedRotation = FQuat(DirToCamera, ParticleRotation);
            kLockedAxisQuat = (AddedRotation * PointTo);
        }
    }
    else if (bUseMeshLockedAxis)
    {
        // Add any 'sprite rotation' about the locked axis
        FQuat AddedRotation = FQuat(LockedAxis, ParticleRotation);
        kLockedAxisQuat = (AddedRotation * PointTo);
    }
    else if (RequiredModule->ScreenAlignment == PSA_TypeSpecific)
    {
        Location = ParticlePosition;
        if (RequiredModule->bUseLocalSpace)
        {
            // Transform the location to world space
            Location = LocalToWorld.TransformPosition(Location);
            kTransMat.SetOrigin(Location);
            LocalToWorld.SetIdentity();
        }

        DirToCamera = ViewOrigin - Location;
        DirToCamera.Normalize();
        if (DirToCamera.SizeSquared() < 0.5f)
        {
            // Assert possible if DirToCamera is not normalized
            DirToCamera = FVector(1, 0, 0);
        }

        LocalSpaceFacingAxis = FVector(1, 0, 0); // facing axis is taken to be the local x axis.	
        LocalSpaceUpAxis = FVector(0, 0, 1); // up axis is taken to be the local z axis

        if (MeshModule->MeshAlignment == PSMA_MeshFaceCameraWithLockedAxis)
        {
            // TODO: Allow an arbitrary	vector to serve	as the locked axis

            // For the locked axis behavior, only rotate to	face the camera	about the
            // locked direction, and maintain the up vector	pointing towards the locked	direction
            // Find	the	rotation that points the localupaxis towards the targetupaxis
            FQuat PointToUp = FQuat::FindBetweenNormals(LocalSpaceUpAxis, LockedAxis);

            // Add in rotation about the TargetUpAxis to point the facing vector towards the camera
            FVector	DirToCameraInRotationPlane = DirToCamera - ((DirToCamera | LockedAxis)*LockedAxis);
            DirToCameraInRotationPlane.Normalize();
            FQuat PointToCamera = FQuat::FindBetweenNormals(PointToUp.RotateVector(LocalSpaceFacingAxis), DirToCameraInRotationPlane);

            // Set kRotMat to the composed rotation
            FQuat MeshRotation = PointToCamera * PointToUp;
            kRotMat = FQuatRotationMatrix(MeshRotation);
        }
        else if (MeshModule->MeshAlignment == PSMA_MeshFaceCameraWithSpin)
        {
            // Implement a tangent-rotation	version	of point-to-camera.	 The facing	direction points to	the	camera,
            // with	no roll, and has addtional sprite-particle rotation	about the tangential axis
            // (c.f. the roll rotation is about	the	radial axis)

            // Find	the	rotation that points the facing	axis towards the camera
            FRotator PointToRotation = FRotator(FQuat::FindBetweenNormals(LocalSpaceFacingAxis, DirToCamera));

            // When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
            // otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
            PointToRotation.Roll = 0;

            // Add in the tangential rotation we do	want.
            FVector	vPositivePitch = FVector(0, 0, 1); //	this is	set	by the rotator's yaw/pitch/roll	reference frame
            FVector	vTangentAxis = vPositivePitch ^ DirToCamera;
            vTangentAxis.Normalize();
            if (vTangentAxis.SizeSquared() < 0.5f)
            {
                vTangentAxis = FVector(1, 0, 0); // assert is	possible if	FQuat axis/angle constructor is	passed zero-vector
            }

            FQuat AddedTangentialRotation = FQuat(vTangentAxis, ParticleRotation);

            // Set kRotMat to the composed rotation
            FQuat MeshRotation = AddedTangentialRotation * PointToRotation.Quaternion();
            kRotMat = FQuatRotationMatrix(MeshRotation);
        }
        else
        {
            // Implement a roll-rotation version of	point-to-camera.  The facing direction points to the camera,
            // with	no roll, and then rotates about	the	direction_to_camera	by the spriteparticle rotation.

            // Find	the	rotation that points the facing	axis towards the camera
            FRotator PointToRotation = FRotator(FQuat::FindBetweenNormals(LocalSpaceFacingAxis, DirToCamera));

            // When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
            // otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
            PointToRotation.Roll = 0;

            // Add in the roll we do want.
            FQuat AddedRollRotation = FQuat(DirToCamera, ParticleRotation);

            // Set kRotMat to the composed	rotation
            FQuat MeshRotation = AddedRollRotation * PointToRotation.Quaternion();
            kRotMat = FQuatRotationMatrix(MeshRotation);
        }
    }
    else
    {
        float fRot = ParticleRotation * 180.0f / PI;
        FVector kRotVec = FVector(fRot, fRot, fRot);
        FRotator kRotator = FRotator::MakeFromEuler(kRotVec);

        kRotator += FRotator::MakeFromEuler(ParticlePayloadRotation);

        kRotMat = FRotationMatrix(kRotator);
    }

    if (bApplyPreRotation == true)
    {
        FRotator MeshOrient = FRotator::MakeFromEuler(ParticlePayloadInitialOrientation);
        FRotationMatrix OrientMat(MeshOrient);

        if ((bUseCameraFacing == true) || (bUseMeshLockedAxis == true))
        {
            OutTransformMat = (OrientMat * kScaleMat) * FQuatRotationMatrix(kLockedAxisQuat) * kRotMat * kTransMat;
        }
        else
        {
            OutTransformMat = (OrientMat*kScaleMat) * kRotMat * kTransMat;
        }
    }
    else if ((bUseCameraFacing == true) || (bUseMeshLockedAxis == true))
    {
        OutTransformMat = kScaleMat * FQuatRotationMatrix(kLockedAxisQuat) * kRotMat * kTransMat;
    }
    else
    {
        OutTransformMat = kScaleMat * kRotMat * kTransMat;
    }

    FVector OrbitOffset = ParticlePayloadOrbitOffset;
    if (RequiredModule->bUseLocalSpace == false)
    {
        OrbitOffset = LocalToWorld.TransformVector(OrbitOffset);
    }

    FTranslationMatrix OrbitMatrix(OrbitOffset);
    OutTransformMat *= OrbitMatrix;

    if (RequiredModule->bUseLocalSpace)
    {
        OutTransformMat *= LocalToWorld;
    }
}

static void TriangleStrip2TriangleIndices(uint16* InIndex, uint16* OutIndex, int32 NumIndex, bool SingleSided)
{
    for (int32 i = 0; i < NumIndex - 2; ++i)
    {
        if (i & 1)
        {
            *(OutIndex++) = InIndex[i];
            *(OutIndex++) = InIndex[i + 2];
            *(OutIndex++) = InIndex[i + 1];
            if (SingleSided)
            {
                *(OutIndex++) = InIndex[i];
                *(OutIndex++) = InIndex[i + 1];
                *(OutIndex++) = InIndex[i + 2];
            }
        }
        else
        {
            *(OutIndex++) = InIndex[i];
            *(OutIndex++) = InIndex[i + 1];
            *(OutIndex++) = InIndex[i + 2];
            if (SingleSided)
            {
                *(OutIndex++) = InIndex[i];
                *(OutIndex++) = InIndex[i + 2];
                *(OutIndex++) = InIndex[i + 1];
            }
        }
    }
}
// FDynamicTrailsEmitterData::FillIndexData
int32 FillIndexData(const UParticleModuleRequired* RequiredModule,
                    const FParticleRibbonEmitterInstance* SourcePointer,
                    TArray<uint16>& IndexData)
{
    int32	TrianglesToRender = 0;

    // Trails polygons are packed and joined as follows:
    //
    // 1--3--5--7--9-...
    // |\ |\ |\ |\ |\...
    // | \| \| \| \| ...
    // 0--2--4--6--8-...
    //
    // (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
    //

    int32	Sheets = 1;
    int32	TessFactor = 1;//FMath::Max<int32>(Source.TessFactor, 1);

    int32	CheckCount = 0;

    uint16*	Index = (uint16*)IndexData.GetData();
    uint16	VertexIndex = 0;

    int32 CurrentTrail = 0;
    for (int32 ParticleIdx = 0; ParticleIdx < SourcePointer->ActiveParticles; ParticleIdx++)
    {
        int32 CurrentIndex = SourcePointer->ParticleIndices[ParticleIdx];
        DECLARE_PARTICLE_PTR(Particle, SourcePointer->ParticleData + SourcePointer->ParticleStride * CurrentIndex);

        FTrailsBaseTypeDataPayload* TrailPayload = (FTrailsBaseTypeDataPayload*)((uint8*)Particle + SourcePointer->TypeDataOffset);
        if (TRAIL_EMITTER_IS_HEAD(TrailPayload->Flags) == false)
        {
            continue;
        }

        int32 LocalTrianglesToRender = TrailPayload->TriangleCount;
        if (LocalTrianglesToRender == 0)
        {
            continue;
        }

        //@todo. Support clip source segment

        // For the source particle itself
        if (CurrentTrail == 0)
        {
            *(Index++) = VertexIndex++;		// The first vertex..
            *(Index++) = VertexIndex++;		// The second index..
            CheckCount += 2;
        }
        else
        {
            // Add the verts to join this trail with the previous one
            *(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
            *(Index++) = VertexIndex;		// First vertex of the next sheet
            *(Index++) = VertexIndex++;		// First vertex of the next sheet
            *(Index++) = VertexIndex++;		// Second vertex of the next sheet
            TrianglesToRender += 4;
            CheckCount += 4;
        }

        for (int32 LocalIdx = 0; LocalIdx < LocalTrianglesToRender; LocalIdx++)
        {
            *(Index++) = VertexIndex++;
            CheckCount++;
            TrianglesToRender++;
        }

        //@todo. Support sheets!

        CurrentTrail++;
    }

//#if RHI_RAYTRACING
    //TriangleStrip2TriangleIndices<uint16>((uint16*)Data.IndexData, (uint16*)Data.TriangleIndexData, Data.IndexCount, Data.SingleSidedMaterial);
//#endif
    return TrianglesToRender;
}


// FDynamicRibbonEmitterData::FillVertexData()
int32 FillVertexData(const UParticleModuleRequired* RequiredModule,
                    const FParticleRibbonEmitterInstance* SourcePointer,
                    const FMatrix& CameraToWorld,
                    const FVector& Scale,
                    const FMatrix& LocalToWorld,
                    TArray<FParticleBeamTrailVertex>& VertexData)
{
    //VertexData.SetNum()
    bool bTextureTileDistance = (SourcePointer->TrailTypeData->TilingDistance > 0.0f); // FParticleRibbonEmitterInstance::GetDynamicData

    int32	TrianglesToRender = 0;

    uint8* TempVertexData = (uint8*)VertexData.GetData();
    //uint8* TempDynamicParamData = (uint8*)Data.DynamicParameterData; // TODO: TC - all DynamicParamData disabled. needed? 
    FParticleBeamTrailVertex* Vertex;
    //FParticleBeamTrailVertexDynamicParameter* DynParamVertex;

    FVector CameraUp = CameraToWorld.TransformVector(FVector(0, 0, 1));
    FVector	ViewOrigin = CameraToWorld.GetOrigin();

    int32 MaxTessellationBetweenParticles = FMath::Max<int32>(SourcePointer->TrailTypeData->MaxTessellationBetweenParticles, 1);
    int32 Sheets = 1;

    //bool bUseDynamic = bUsesDynamicParameter && TempDynamicParamData != nullptr;

    // The distance tracking for tiling the 2nd UV set
    float CurrDistance = 0.0f;

    FBaseParticle* PackingParticle;
    const uint8* ParticleData = SourcePointer->ParticleData;
    for (int32 ParticleIdx = 0; ParticleIdx < SourcePointer->ActiveParticles; ParticleIdx++)
    {
        DECLARE_PARTICLE_PTR(Particle, ParticleData + SourcePointer->ParticleStride * SourcePointer->ParticleIndices[ParticleIdx]);
        FRibbonTypeDataPayload* TrailPayload = (FRibbonTypeDataPayload*)((uint8*)Particle + SourcePointer->TypeDataOffset);
        if (TRAIL_EMITTER_IS_HEAD(TrailPayload->Flags) == 0)
        {
            continue;
        }

        if (TRAIL_EMITTER_GET_NEXT(TrailPayload->Flags) == TRAIL_EMITTER_NULL_NEXT)
        {
            continue;
        }

        PackingParticle = Particle;
        // Pin the size to the X component
        FLinearColor CurrLinearColor = PackingParticle->Color;
        // The increment for going [0..1] along the complete trail
        float TextureIncrement = 1.0f / (TrailPayload->TriangleCount / 2.0f);
        float Tex_U = 0.0f;
        FVector CurrTilePosition = PackingParticle->Location;
        FVector PrevTilePosition = PackingParticle->Location;
        FVector PrevWorkingUp(0, 0, 1);
        int32 VertexStride = sizeof(FParticleBeamTrailVertex);
        /*int32 DynamicParameterStride = 0;
        bool bFillDynamic = false;
        if (bUseDynamic)
        {
            DynamicParameterStride = sizeof(FParticleBeamTrailVertexDynamicParameter);
            if (Source.DynamicParameterDataOffset > 0)
            {
                bFillDynamic = true;
            }
        }*/
        float CurrTileU;
        /*FEmitterDynamicParameterPayload* CurrDynPayload = NULL;
        FEmitterDynamicParameterPayload* PrevDynPayload = NULL;*/
        FBaseParticle* PrevParticle = NULL;
        FRibbonTypeDataPayload* PrevTrailPayload = NULL;

        FVector WorkingUp = TrailPayload->Up;
        if (SourcePointer->TrailTypeData->RenderAxis == Trails_CameraUp)
        {
            FVector DirToCamera = PackingParticle->Location - ViewOrigin;
            DirToCamera.Normalize();
            FVector NormailzedTangent = TrailPayload->Tangent;
            NormailzedTangent.Normalize();
            WorkingUp = NormailzedTangent ^ DirToCamera;
            if (WorkingUp.IsNearlyZero())
            {
                WorkingUp = CameraUp;
            }

            WorkingUp.Normalize();
        }

        while (TrailPayload)
        {
            float CurrSize = PackingParticle->Size.X * Scale.X;

            int32 InterpCount = TrailPayload->RenderingInterpCount;
            if (InterpCount > 1)
            {
                check(PrevParticle);
                check(TRAIL_EMITTER_IS_HEAD(TrailPayload->Flags) == 0);

                // Interpolate between current and next...
                FVector CurrPosition = PackingParticle->Location;
                FVector CurrTangent = TrailPayload->Tangent;
                FVector CurrUp = WorkingUp;
                FLinearColor CurrColor = PackingParticle->Color;

                FVector PrevPosition = PrevParticle->Location; //-V522
                FVector PrevTangent = PrevTrailPayload->Tangent; //-V522
                FVector PrevUp = PrevWorkingUp;
                FLinearColor PrevColor = PrevParticle->Color;
                float PrevSize = PrevParticle->Size.X * Scale.X;

                float InvCount = 1.0f / InterpCount;
                float Diff = PrevTrailPayload->SpawnTime - TrailPayload->SpawnTime;

                FVector4 CurrDynParam;
                FVector4 PrevDynParam;
                /*if (bFillDynamic)
                {
                    GetDynamicValueFromPayload(Source.DynamicParameterDataOffset, *PackingParticle, CurrDynParam);
                    GetDynamicValueFromPayload(Source.DynamicParameterDataOffset, *PrevParticle, PrevDynParam);
                }*/

                //FVector4 InterpDynamic(1.0f, 1.0f, 1.0f, 1.0f);
                for (int32 SpawnIdx = InterpCount - 1; SpawnIdx >= 0; SpawnIdx--)
                {
                    float TimeStep = InvCount * SpawnIdx;
                    FVector InterpPos = FMath::CubicInterp<FVector>(CurrPosition, CurrTangent, PrevPosition, PrevTangent, TimeStep);
                    FVector InterpUp = FMath::Lerp<FVector>(CurrUp, PrevUp, TimeStep);
                    FLinearColor InterpColor = FMath::Lerp<FLinearColor>(CurrColor, PrevColor, TimeStep);
                    float InterpSize = FMath::Lerp<float>(CurrSize, PrevSize, TimeStep);
                    /*if (bFillDynamic)
                    {
                        InterpDynamic = FMath::Lerp<FVector4>(CurrDynParam, PrevDynParam, TimeStep);
                    }*/

                    if (bTextureTileDistance == true)
                    {
                        CurrTileU = FMath::Lerp<float>(TrailPayload->TiledU, PrevTrailPayload->TiledU, TimeStep);
                    }
                    else
                    {
                        CurrTileU = Tex_U;
                    }

                    FVector FinalPos = InterpPos + InterpUp * InterpSize;
                    if (RequiredModule->bUseLocalSpace)
                    {
                        FinalPos += LocalToWorld.GetOrigin();
                    }
                    Vertex = (FParticleBeamTrailVertex*)(TempVertexData);
                    Vertex->Position = FinalPos;
                    Vertex->OldPosition = FinalPos;
                    Vertex->ParticleId = 0;
                    Vertex->Size.X = InterpSize;
                    Vertex->Size.Y = InterpSize;
                    Vertex->Tex_U = Tex_U;
                    Vertex->Tex_V = 0.0f;
                    Vertex->Tex_U2 = CurrTileU;
                    Vertex->Tex_V2 = 0.0f;
                    Vertex->Rotation = PackingParticle->Rotation;
                    Vertex->Color = InterpColor;
                    /*if (bUseDynamic)
                    {
                        DynParamVertex = (FParticleBeamTrailVertexDynamicParameter*)(TempDynamicParamData);
                        DynParamVertex->DynamicValue[0] = InterpDynamic.X;
                        DynParamVertex->DynamicValue[1] = InterpDynamic.Y;
                        DynParamVertex->DynamicValue[2] = InterpDynamic.Z;
                        DynParamVertex->DynamicValue[3] = InterpDynamic.W;
                        TempDynamicParamData += DynamicParameterStride;
                    }*/
                    TempVertexData += VertexStride;
                    //PackedVertexCount++;

                    FinalPos = InterpPos - InterpUp * InterpSize;
                    Vertex = (FParticleBeamTrailVertex*)(TempVertexData);
                    Vertex->Position = FinalPos;
                    Vertex->OldPosition = FinalPos;
                    Vertex->ParticleId = 0;
                    Vertex->Size.X = InterpSize;
                    Vertex->Size.Y = InterpSize;
                    Vertex->Tex_U = Tex_U;
                    Vertex->Tex_V = 1.0f;
                    Vertex->Tex_U2 = CurrTileU;
                    Vertex->Tex_V2 = 1.0f;
                    Vertex->Rotation = PackingParticle->Rotation;
                    Vertex->Color = InterpColor;
                    /*if (bUseDynamic)
                    {
                        DynParamVertex = (FParticleBeamTrailVertexDynamicParameter*)(TempDynamicParamData);
                        DynParamVertex->DynamicValue[0] = InterpDynamic.X;
                        DynParamVertex->DynamicValue[1] = InterpDynamic.Y;
                        DynParamVertex->DynamicValue[2] = InterpDynamic.Z;
                        DynParamVertex->DynamicValue[3] = InterpDynamic.W;
                        TempDynamicParamData += DynamicParameterStride;
                    }*/
                    TempVertexData += VertexStride;
                    //PackedVertexCount++;

                    Tex_U += TextureIncrement;
                }
            }
            else
            {
                /*if (bFillDynamic == true)
                {
                    CurrDynPayload = ((FEmitterDynamicParameterPayload*)((uint8*)(PackingParticle)+Source.DynamicParameterDataOffset));
                }*/

                if (bTextureTileDistance == true)
                {
                    CurrTileU = TrailPayload->TiledU;
                }
                else
                {
                    CurrTileU = Tex_U;
                }

                Vertex = (FParticleBeamTrailVertex*)(TempVertexData);
                Vertex->Position = PackingParticle->Location + WorkingUp * CurrSize;
                Vertex->OldPosition = PackingParticle->OldLocation;
                Vertex->ParticleId = 0;
                Vertex->Size.X = CurrSize;
                Vertex->Size.Y = CurrSize;
                Vertex->Tex_U = Tex_U;
                Vertex->Tex_V = 0.0f;
                Vertex->Tex_U2 = CurrTileU;
                Vertex->Tex_V2 = 0.0f;
                Vertex->Rotation = PackingParticle->Rotation;
                Vertex->Color = PackingParticle->Color;
                /*if (bUseDynamic)
                {
                    DynParamVertex = (FParticleBeamTrailVertexDynamicParameter*)(TempDynamicParamData);
                    if (CurrDynPayload != NULL)
                    {
                        DynParamVertex->DynamicValue[0] = CurrDynPayload->DynamicParameterValue[0];
                        DynParamVertex->DynamicValue[1] = CurrDynPayload->DynamicParameterValue[1];
                        DynParamVertex->DynamicValue[2] = CurrDynPayload->DynamicParameterValue[2];
                        DynParamVertex->DynamicValue[3] = CurrDynPayload->DynamicParameterValue[3];
                    }
                    else
                    {
                        DynParamVertex->DynamicValue[0] = 1.0f;
                        DynParamVertex->DynamicValue[1] = 1.0f;
                        DynParamVertex->DynamicValue[2] = 1.0f;
                        DynParamVertex->DynamicValue[3] = 1.0f;
                    }
                    TempDynamicParamData += DynamicParameterStride;
                }*/
                TempVertexData += VertexStride;
                //PackedVertexCount++;

                Vertex = (FParticleBeamTrailVertex*)(TempVertexData);
                Vertex->Position = PackingParticle->Location - WorkingUp * CurrSize;
                Vertex->OldPosition = PackingParticle->OldLocation;
                Vertex->ParticleId = 0;
                Vertex->Size.X = CurrSize;
                Vertex->Size.Y = CurrSize;
                Vertex->Tex_U = Tex_U;
                Vertex->Tex_V = 1.0f;
                Vertex->Tex_U2 = CurrTileU;
                Vertex->Tex_V2 = 1.0f;
                Vertex->Rotation = PackingParticle->Rotation;
                Vertex->Color = PackingParticle->Color;
                /*if (bUseDynamic)
                {
                    DynParamVertex = (FParticleBeamTrailVertexDynamicParameter*)(TempDynamicParamData);
                    if (CurrDynPayload != NULL)
                    {
                        DynParamVertex->DynamicValue[0] = CurrDynPayload->DynamicParameterValue[0];
                        DynParamVertex->DynamicValue[1] = CurrDynPayload->DynamicParameterValue[1];
                        DynParamVertex->DynamicValue[2] = CurrDynPayload->DynamicParameterValue[2];
                        DynParamVertex->DynamicValue[3] = CurrDynPayload->DynamicParameterValue[3];
                    }
                    else
                    {
                        DynParamVertex->DynamicValue[0] = 1.0f;
                        DynParamVertex->DynamicValue[1] = 1.0f;
                        DynParamVertex->DynamicValue[2] = 1.0f;
                        DynParamVertex->DynamicValue[3] = 1.0f;
                    }
                    TempDynamicParamData += DynamicParameterStride;
                }*/
                TempVertexData += VertexStride;
                //PackedVertexCount++;

                Tex_U += TextureIncrement;
            }

            PrevParticle = PackingParticle;
            PrevTrailPayload = TrailPayload;
            PrevWorkingUp = WorkingUp;

            int32	NextIdx = TRAIL_EMITTER_GET_NEXT(TrailPayload->Flags);
            if (NextIdx == TRAIL_EMITTER_NULL_NEXT)
            {
                TrailPayload = NULL;
                PackingParticle = NULL;
            }
            else
            {
                DECLARE_PARTICLE_PTR(TempParticle, ParticleData + SourcePointer->ParticleStride * NextIdx);
                PackingParticle = TempParticle;
                TrailPayload = (FRibbonTypeDataPayload*)((uint8*)TempParticle + SourcePointer->TypeDataOffset);
                WorkingUp = TrailPayload->Up;
                if (SourcePointer->TrailTypeData->RenderAxis == Trails_CameraUp)
                {
                    FVector DirToCamera = PackingParticle->Location - ViewOrigin;
                    DirToCamera.Normalize();
                    FVector NormailzedTangent = TrailPayload->Tangent;
                    NormailzedTangent.Normalize();
                    WorkingUp = NormailzedTangent ^ DirToCamera;
                    if (WorkingUp.IsNearlyZero())
                    {
                        WorkingUp = CameraUp;
                    }
                    WorkingUp.Normalize();
                }
            }
        }
    }

    return TrianglesToRender;
}


FVector GetCameraOffset(
    float CameraPayloadOffset,
    FVector DirToCamera
)
{
    float CheckSize = DirToCamera.SizeSquared();
    DirToCamera.Normalize();

    if (CheckSize > (CameraPayloadOffset * CameraPayloadOffset))
    {
        return DirToCamera * CameraPayloadOffset;
    }
    else
    {
        // If the offset will push the particle behind the camera, then push it 
        // WAY behind the camera. This is a hack... but in the case of 
        // PSA_Velocity, it is required to ensure that the particle doesn't 
        // 'spin' flat and come into view.
        return DirToCamera * CameraPayloadOffset * HALF_WORLD_MAX;
    }
}

FVector GetCameraOffsetFromPayload(
    int32 InCameraPayloadOffset,
    const FBaseParticle& InParticle,
    const FVector& InParticlePosition,
    const FVector& InCameraPosition
)
{
    checkSlow(InCameraPayloadOffset > 0);

    FVector DirToCamera = InCameraPosition - InParticlePosition;
    FCameraOffsetParticlePayload* CameraPayload = ((FCameraOffsetParticlePayload*)((uint8*)(&InParticle) + InCameraPayloadOffset));

    return GetCameraOffset(CameraPayload->Offset, DirToCamera);
}



pxr::UsdGeomXform FUSDExporter::ExportParticleSystem(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const class UParticleSystemComponent& PartSys, pxr::UsdTimeCode TimeCode, bool bPreferPointInstancer, const bool bExportTangentX)
{
    // particles are in world space (or are converted to world space below). we'll put these in a special scenegraph branch. 
    const pxr::SdfPath WorldSpaceEmittersPath("/WorldSpaceParticleEmitters");
    auto WorldSpaceEmitters = pxr::UsdGeomXform::Define(Stage, WorldSpaceEmittersPath);

    // Right hand to left hand conversion
    auto UpAxisValue = pxr::UsdGeomGetStageUpAxis(Stage);
    const FMatrix& ConversionMatrix = UpAxisValue == pxr::UsdGeomTokens->z ? SimReadyZUpConversion : SimReadyYUpConversion;

    ULocalPlayer* LocalPlayer = PartSys.GetWorld()->GetFirstLocalPlayerFromController();

    // hack - if there's no player, then we're not sequence exporting, and baking the particle system makes no sense. bail out. 
    if (LocalPlayer == nullptr)
        return pxr::UsdGeomXform();

    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        LocalPlayer->ViewportClient->Viewport,
        PartSys.GetWorld()->Scene,
        LocalPlayer->ViewportClient->EngineShowFlags)
        .SetRealtimeUpdate(true));

    FVector ViewLocation;
    FRotator ViewRotation;
    FSceneView* View = LocalPlayer->CalcSceneView(&ViewFamily, /*out*/ ViewLocation, /*out*/ ViewRotation, LocalPlayer->ViewportClient->Viewport);

    for (auto It = PartSys.EmitterInstances.CreateConstIterator(); It; ++It)
    {
        FParticleEmitterInstance* EmitterInstance = *It;

        // even if no active particles, process the emitter. otherwise, we'll never terminate a particle system that goes empty. 
        if (EmitterInstance)
        {
            const UParticleLODLevel* LODLevel = EmitterInstance->CurrentLODLevel;
            const UParticleModuleTypeDataBase* TypeDataModule = LODLevel->TypeDataModule;

            auto EmitterName = pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*EmitterInstance->SpriteTemplate->GetEmitterName().ToString()));
            auto EmitterPath = WorldSpaceEmittersPath.AppendElementString(EmitterName);

            const FMatrix& LocalToWorld = LODLevel->RequiredModule->bUseLocalSpace ? PartSys.GetRenderMatrix() : // RendererScene.cpp:985
                                          FMatrix::Identity;

            if (const UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule))
            {
                if (bPreferPointInstancer)
                {
                    auto USDPoints = pxr::UsdGeomPointInstancer::Define(Stage, EmitterPath);

                    // ideally mesh particle emitters would share the same prototype mesh, however not yet sure how to put them into a shared location without
                    // them being drawn. keeping them as children of the PointInstancer prevents them from being drawn as non-instances					
                    auto MeshPath = EmitterPath.AppendElementString("Prototype");
                    auto USDMesh = FUSDExporter::ExportStaticMesh(Stage, MeshPath, MeshTD->Mesh->RenderData->LODResources[0], nullptr, bExportTangentX);
                    if (USDMesh)
                    {
                        pxr::SdfPathVector prototypesPaths;
                        prototypesPaths.push_back(MeshPath);
                        USDPoints.CreatePrototypesRel().SetTargets(prototypesPaths);
                    }

                    pxr::VtArray<pxr::GfVec3f> Positions(EmitterInstance->ActiveParticles);
                    pxr::VtArray<pxr::GfVec3f> Scales(EmitterInstance->ActiveParticles);
                    pxr::VtArray<pxr::GfQuath> Orientations(EmitterInstance->ActiveParticles);
                    pxr::VtArray<int> ProtoIndices(EmitterInstance->ActiveParticles);
                    for (int32 p = 0; p < EmitterInstance->ActiveParticles; p++)
                    {
                        DECLARE_PARTICLE_CONST(Particle, EmitterInstance->ParticleData + EmitterInstance->ParticleStride * EmitterInstance->ParticleIndices[p]);

                        const FMeshRotationPayloadData* RotationPayload = (const FMeshRotationPayloadData*)((const uint8*)&Particle + EmitterInstance->GetMeshRotationOffset());
                        FVector RotationPayloadInitialOrientation = RotationPayload->InitialOrientation;
                        FVector RotationPayloadRotation = RotationPayload->Rotation;

                        FVector CameraPayloadCameraOffset = FVector::ZeroVector;
                        if (EmitterInstance->CameraPayloadOffset != 0)
                        {
                            // Put the camera origin in the appropriate coordinate space.
                            FVector CameraPosition = View->ViewMatrices.GetViewOrigin();
                            if (LODLevel->RequiredModule->bUseLocalSpace)
                            {
                                const FMatrix InvLocalToWorld = LocalToWorld.Inverse();
                                CameraPosition = InvLocalToWorld.TransformPosition(CameraPosition);
                            }

                            CameraPayloadCameraOffset = GetCameraOffsetFromPayload(EmitterInstance->CameraPayloadOffset, Particle, Particle.Location, CameraPosition);
                        }

                        FVector OrbitPayloadOrbitOffset = FVector::ZeroVector;
                        if (EmitterInstance->OrbitModuleOffset != 0)
                        {
                            const uint8* ParticleBase = (const uint8*)&Particle;

                            int32 CurrentOffset = EmitterInstance->OrbitModuleOffset;
                            PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
                            OrbitPayloadOrbitOffset = OrbitPayload.Offset;
                        }

                        FMatrix ParticleTransform;
                        CalculateParticleTransform(
                            LODLevel->RequiredModule,
                            MeshTD,
                            LocalToWorld,
                            Particle.Location,
                            Particle.Rotation,
                            Particle.Velocity,
                            Particle.Size,
                            RotationPayloadInitialOrientation, RotationPayloadRotation,
                            CameraPayloadCameraOffset, OrbitPayloadOrbitOffset,
                            View->ViewMatrices.GetViewOrigin(),
                            View->GetViewDirection(),
                            PartSys.GetComponentTransform().GetScale3D(),
                            ParticleTransform);

                        ParticleTransform = ConversionMatrix * ParticleTransform * ConversionMatrix;

                        // "consume" transform & scale so we get a clean, independent orientation. 
                        FVector Point = ParticleTransform.GetOrigin();
                        Positions[p] = { Point.X, Point.Y, Point.Z };
                        ParticleTransform.RemoveTranslation();

                        FVector Scale = ParticleTransform.GetScaleVector();
                        Scales[p] = { Scale.X, Scale.Y, Scale.Z };
                        ParticleTransform.RemoveScaling();

                        // gotta be an easier way to go from UE4 matrix to USD quat? 
                        FVector Axis;
                        float Angle;
                        ParticleTransform.Rotator().Quaternion().ToAxisAndAngle(Axis, Angle);
                        pxr::GfRotation PxrRotation(pxr::GfVec3d(Axis.X, Axis.Y, Axis.Z), FMath::RadiansToDegrees(Angle));
                        Orientations[p] = pxr::GfQuath(PxrRotation.GetQuat());

                        ProtoIndices[p] = 0; // always just one instance per emitter
                    }

                    USDPoints.CreatePositionsAttr().Set(Positions, TimeCode);
                    USDPoints.CreateScalesAttr().Set(Scales, TimeCode);
                    USDPoints.CreateOrientationsAttr().Set(Orientations, TimeCode);
                    USDPoints.CreateProtoIndicesAttr().Set(ProtoIndices, TimeCode);
                    
                    pxr::VtArray<pxr::GfVec3f> Extent;
                    USDPoints.ComputeExtentAtTime(&Extent, TimeCode, TimeCode);
                    USDPoints.CreateExtentAttr().Set(Extent, TimeCode);
                }
                else
                {

                    FUSDGeomMeshAttributes Attributes;
                    FUSDExporter::ExportStaticMesh(Stage, MeshTD->Mesh->RenderData->LODResources[0], nullptr, Attributes);

                    pxr::VtArray<pxr::GfVec3f> PointCacheVertices;
                    pxr::VtArray<int32> PointCacheFaceVertexCounts;
                    pxr::VtArray<int32> PointCacheFaceVertexIndices;
                    pxr::VtArray<pxr::GfVec3f> PointCacheColors;
                    pxr::VtArray<float> PointCacheOpacities;
                    pxr::VtArray<pxr::GfVec3f> PointCacheNormals;
                    pxr::VtArray<pxr::GfVec3f> PointCacheTangentX;
                    pxr::VtArray<pxr::VtArray<pxr::GfVec2f>> PointCacheTexCoordChannels(Attributes.UVs.size()); // channel count constant between source & accumulated mesh

                    // TODO: support mesh subsets (multi-materials) once we know we need them
                    //pxr::VtArray<pxr::VtArray<int>> PointCacheFaceIndices;

                    for (int32 p = 0; p < EmitterInstance->ActiveParticles; p++)
                    {
                        DECLARE_PARTICLE_CONST(Particle, EmitterInstance->ParticleData + EmitterInstance->ParticleStride * EmitterInstance->ParticleIndices[p]);

                        const FMeshRotationPayloadData* RotationPayload = (const FMeshRotationPayloadData*)((const uint8*)&Particle + EmitterInstance->GetMeshRotationOffset());
                        FVector RotationPayloadInitialOrientation = RotationPayload->InitialOrientation;
                        FVector RotationPayloadRotation = RotationPayload->Rotation;

                        FVector CameraPayloadCameraOffset = FVector::ZeroVector;
                        if (EmitterInstance->CameraPayloadOffset != 0)
                        {
                            // Put the camera origin in the appropriate coordinate space.
                            FVector CameraPosition = View->ViewMatrices.GetViewOrigin();
                            if (LODLevel->RequiredModule->bUseLocalSpace)
                            {
                                const FMatrix InvLocalToWorld = LocalToWorld.Inverse();
                                CameraPosition = InvLocalToWorld.TransformPosition(CameraPosition);
                            }

                            CameraPayloadCameraOffset = GetCameraOffsetFromPayload(EmitterInstance->CameraPayloadOffset, Particle, Particle.Location, CameraPosition);
                        }

                        FVector OrbitPayloadOrbitOffset = FVector::ZeroVector;
                        if (EmitterInstance->OrbitModuleOffset != 0)
                        {
                            const uint8* ParticleBase = (const uint8*)&Particle;

                            int32 CurrentOffset = EmitterInstance->OrbitModuleOffset;
                            PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
                            OrbitPayloadOrbitOffset = OrbitPayload.Offset;
                        }

                        FMatrix ParticleTransform;
                        CalculateParticleTransform(
                            LODLevel->RequiredModule,
                            MeshTD,
                            LocalToWorld, 					
                            Particle.Location,
                            Particle.Rotation,
                            Particle.Velocity,
                            Particle.Size,
                            RotationPayloadInitialOrientation, RotationPayloadRotation,
                            CameraPayloadCameraOffset, OrbitPayloadOrbitOffset,
                            View->ViewMatrices.GetViewOrigin(),
                            View->GetViewDirection(),
                            PartSys.GetComponentTransform().GetScale3D(),
                            ParticleTransform);

                        // indices for this mesh instance will begin at (Num Previous Particles)*(Num Verts). grab that before we start modifying
                        //  the vertex array
                        int BeginVertIndex = int(PointCacheVertices.size());
                        for (int v = 0; v < Attributes.Points.size(); ++v)
                        {
                            {	// position
                                FVector PointUE4 = { Attributes.Points[v].data()[0], Attributes.Points[v].data()[1], Attributes.Points[v].data()[2] };
                                FVector VertTransformed = ParticleTransform.TransformPosition(PointUE4);
                                VertTransformed = USDConvertPosition(Stage, VertTransformed);
                                pxr::GfVec3f Vert = { VertTransformed.X, VertTransformed.Y, VertTransformed.Z };
                                PointCacheVertices.push_back(Vert);
                            }
                        }

                        const int VERTS_PER_FACE = 3;
                        const int FACE_COUNT = Attributes.FaceVertexIndices.size() / VERTS_PER_FACE;
                        for (int f = 0; f < FACE_COUNT; ++f)
                        {
                            PointCacheFaceVertexCounts.push_back(VERTS_PER_FACE);
                        }

                        for (int i = 0; i < Attributes.FaceVertexIndices.size(); ++i)
                        {
                            int AdjustedIndex = BeginVertIndex + Attributes.FaceVertexIndices[i];
                            check(AdjustedIndex < PointCacheVertices.size());
                            PointCacheFaceVertexIndices.push_back(AdjustedIndex);
                        }

                        // face-varying components should exist for every vert index
                        if (Attributes.Normals.size())
                        {
                            for (int n = 0; n < Attributes.Normals.size(); ++n)
                            {
                                FVector NormalUE4 = { Attributes.Normals[n].data()[0], Attributes.Normals[n].data()[1], Attributes.Normals[n].data()[2] };
                                FVector NormalTransformed = ParticleTransform.TransformVector(NormalUE4);
                                NormalTransformed = USDConvertVector(Stage, NormalTransformed);
                                pxr::GfVec3f Normal = { NormalTransformed.X, NormalTransformed.Y, NormalTransformed.Z };
                                PointCacheNormals.push_back(Normal);
                            }
                            check(PointCacheNormals.size() == PointCacheFaceVertexIndices.size());
                        }

                        if (Attributes.TangentX.size())
                        {
                            for (int n = 0; n < Attributes.TangentX.size(); ++n)
                            {
                                FVector TangentXUE4 = { Attributes.TangentX[n].data()[0], Attributes.TangentX[n].data()[1], Attributes.TangentX[n].data()[2] };
                                FVector TangentXTransformed = ParticleTransform.TransformVector(TangentXUE4);
                                TangentXTransformed = USDConvertVector(Stage, TangentXTransformed);
                                pxr::GfVec3f TangentX = { TangentXTransformed.X, TangentXTransformed.Y, TangentXTransformed.Z };
                                PointCacheTangentX.push_back(TangentX);
                            }
                            check(PointCacheTangentX.size() == PointCacheFaceVertexIndices.size());
                        }

                        if (Attributes.Colors.size())
                        {
                            std::copy(Attributes.Colors.begin(), Attributes.Colors.end(), std::back_inserter(PointCacheColors));
                            check(PointCacheColors.size() == PointCacheFaceVertexIndices.size());
                        }

                        if (Attributes.Opacities.size())
                        {
                            std::copy(Attributes.Opacities.begin(), Attributes.Opacities.end(), std::back_inserter(PointCacheOpacities));
                            check(PointCacheOpacities.size() == PointCacheFaceVertexIndices.size());
                        }

                        if (Attributes.UVs.size())
                        {
                            // for each channel
                            for (int i = 0; i < PointCacheTexCoordChannels.size(); ++i)
                            {
                                std::copy(Attributes.UVs[i].begin(), Attributes.UVs[i].end(), std::back_inserter(PointCacheTexCoordChannels[i]));
                                check(PointCacheTexCoordChannels[i].size() == PointCacheFaceVertexIndices.size());
                            }
                        }
                    }

                    pxr::VtArray<pxr::GfVec3f> Extent;
                    pxr::UsdGeomPointBased::ComputeExtent(PointCacheVertices, &Extent);

                    for (int i = EmitterInstance->ActiveParticles; i < EmitterInstance->MaxActiveParticles; ++i)
                    {
                        for (int v = 0; v < Attributes.Points.size(); ++v)
                        {
                            pxr::GfVec3f vert = { 0, 0, 0 };
                            PointCacheVertices.push_back(vert);
                        }
                    }
                    check(PointCacheVertices.size() == EmitterInstance->MaxActiveParticles * Attributes.Points.size());

                    auto USDMesh = pxr::UsdGeomMesh::Define(Stage, EmitterPath);
                    if (USDMesh)
                    {
                        USDMesh.CreatePointsAttr().Set(pxr::VtValue(PointCacheVertices), TimeCode);
                        USDMesh.CreateFaceVertexCountsAttr().Set(pxr::VtValue(PointCacheFaceVertexCounts), TimeCode);
                        USDMesh.CreateFaceVertexIndicesAttr().Set(pxr::VtValue(PointCacheFaceVertexIndices), TimeCode);
                        USDMesh.CreateExtentAttr().Set(Extent, TimeCode);

                        USDMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));

                        // if the original mesh has normals, include them in the particle mesh. we don't check PointCacheNormals here,
                        //  because we want to pack the array, even if zero-length array 
                        if (Attributes.Normals.size())
                        {
                            USDMesh.CreateNormalsAttr().Set(pxr::VtValue(PointCacheNormals), TimeCode);
                            USDMesh.SetNormalsInterpolation(pxr::UsdGeomTokens->faceVarying);
                        }
                        // ditto
                        if (Attributes.Colors.size())
                        {
                            USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheColors, TimeCode);
                        }

                        if (Attributes.Opacities.size())
                        {
                            USDMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheOpacities, TimeCode);
                        }

                        // ditto again
                        if (Attributes.UVs.size())
                        {
                            for (int i = 0; i < PointCacheTexCoordChannels.size(); ++i)
                            {
                                auto Primvar = USDMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + (i == 0 ? "" : std::to_string(i))), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying);
                                Primvar.Set(PointCacheTexCoordChannels[i], TimeCode);
                            }
                        }
                        if (bExportTangentX && Attributes.TangentX.size())
                        {
                            auto Primvar = USDMesh.CreatePrimvar(USDTokens.tangentX, pxr::SdfValueTypeNames->Float3Array, pxr::UsdGeomTokens->faceVarying);
                            Primvar.Set(pxr::VtValue(PointCacheTangentX), TimeCode);
                        }
                    }
                }
            }
            else if (const UParticleModuleTypeDataRibbon* RibbonTD = Cast<UParticleModuleTypeDataRibbon>(LODLevel->TypeDataModule))
            {
                const FParticleRibbonEmitterInstance* RibbonEmitterInstance = static_cast<const FParticleRibbonEmitterInstance*>(EmitterInstance);

                // seems like a hack, but if the emitter is disabled then reenabled, the FParticleRibbonEmitterInstance init logic can end up in a state
                //  where trianglecount == 0 but activeparticles > 0
                if (RibbonEmitterInstance->TriangleCount > 0)
                {
                    int NumStripIndices = RibbonEmitterInstance->TriangleCount + 2;
                    TArray<uint16> StripIndexData;
                    StripIndexData.SetNum(NumStripIndices); // FParticleRibbonEmitterInstance::FillReplayData
                    FillIndexData(LODLevel->RequiredModule,
                        RibbonEmitterInstance,
                        StripIndexData);

                    TArray<FParticleBeamTrailVertex> VertexData;
                    VertexData.SetNum(RibbonEmitterInstance->VertexCount);
                    FillVertexData(LODLevel->RequiredModule,
                        RibbonEmitterInstance,
                        View->ViewMatrices.GetInvViewMatrix(),
                        PartSys.GetComponentTransform().GetScale3D(),
                        LocalToWorld,
                        VertexData);

                    bool SingleSided = RibbonEmitterInstance->CurrentMaterial ? !RibbonEmitterInstance->CurrentMaterial->IsTwoSided() :
                                                                                true;
                    int NumIndices = (NumStripIndices - 2) * 3;
                    if (SingleSided)
                        NumIndices *= 2;
                    TArray<uint16> IndexData;
                    IndexData.SetNum(NumIndices);

                    TriangleStrip2TriangleIndices(StripIndexData.GetData(), IndexData.GetData(), StripIndexData.Num(), SingleSided);

                    pxr::VtArray<pxr::GfVec3f> PointCacheVertices;
                    pxr::VtArray<int> PointCacheFaceVertexIndices;
                    pxr::VtArray<int> PointCacheFaceVertexCounts;
                    pxr::VtArray<pxr::GfVec3f>  PointCacheVelocities;
                    pxr::VtArray<pxr::GfVec3f> PointCacheColors;
                    pxr::VtArray<float> PointCacheOpacities;
                    pxr::VtArray<pxr::GfVec2f> PointCacheTexCoordChannels[2];

                    for (FParticleBeamTrailVertex Vertex : VertexData)
                    {
                        FVector PositionTransformed = USDConvertPosition(Stage, Vertex.Position);
                        PointCacheVertices.push_back(USDConvert(PositionTransformed));
                    }

                    for (uint16 Index : IndexData)
                    {
                        PointCacheFaceVertexIndices.push_back(Index);

                        // TODO: should we only do this if there's a color module? 
                        PointCacheColors.push_back(USDConvert(VertexData[Index].Color));
                        PointCacheOpacities.push_back(VertexData[Index].Color.A);

                        // TexCoords are face-varying, so de-index them. two channels per vertex
                        FVector2D UV1(VertexData[Index].Tex_U, VertexData[Index].Tex_V);
                        PointCacheTexCoordChannels[0].push_back(USDConvert(UV1));

                        FVector2D UV2(VertexData[Index].Tex_U2, VertexData[Index].Tex_V2);
                        PointCacheTexCoordChannels[1].push_back(USDConvert(UV2));
                    }

                    int NumTriangles = SingleSided ? RibbonEmitterInstance->TriangleCount * 2 :
                                                     RibbonEmitterInstance->TriangleCount;
                    for (int i = 0; i < NumTriangles; ++i)
                    {
                        PointCacheFaceVertexCounts.push_back(3);
                    }

                    pxr::VtArray<pxr::GfVec3f> Extent;
                    pxr::UsdGeomPointBased::ComputeExtent(PointCacheVertices, &Extent);

                    // here is where we would pad PointCacheVertices to account for worst-case size, based on max particle count.
                    //  however, for ribbons, which have points added to the curve based on distance, this i don't yet know how to
                    //  do this safely without it being enormous. TBD. 

                    pxr::UsdGeomMesh USDMesh = pxr::UsdGeomMesh::Define(Stage, EmitterPath);
                    USDMesh.CreatePointsAttr().Set(PointCacheVertices, TimeCode);
                    USDMesh.CreateFaceVertexCountsAttr().Set(PointCacheFaceVertexCounts, TimeCode);
                    USDMesh.CreateFaceVertexIndicesAttr().Set(PointCacheFaceVertexIndices, TimeCode);
                    USDMesh.CreateExtentAttr().Set(Extent, TimeCode);
                    USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheColors, TimeCode);
                    USDMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheOpacities, TimeCode);

                    USDMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));

                    auto PrimvarTexCoords0 = USDMesh.CreatePrimvar(pxr::UsdUtilsGetPrimaryUVSetName(), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying);
                    PrimvarTexCoords0.Set(PointCacheTexCoordChannels[0], TimeCode);
                    auto PrimvarTexCoords1 = USDMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + std::to_string(1)), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying);
                    PrimvarTexCoords1.Set(PointCacheTexCoordChannels[1], TimeCode);
                }
            }
            else if (LODLevel->TypeDataModule == nullptr) // sprite
            {
                // not entirely sure why we have to define verts in yz space. 
                FVector BillboardVerts[] = {
                    {  0, -0.5, -0.5},
                    {  0,  0.5, -0.5},
                    {  0,  0.5,  0.5},
                    {  0, -0.5,  0.5} };
                size_t BillboardVertsCount = sizeof(BillboardVerts) / sizeof(*BillboardVerts);
                int BillboardIndices[] = { 0, 2, 1, 0, 3, 2 };
                size_t BillboardIndicesCount = sizeof(BillboardIndices) / sizeof(*BillboardIndices);

                FVector2D TexCoords[] = {
                    {  0, 0 },
                    {  1, 0 },
                    {  1, 1 },
                    {  0, 1 } };
                size_t TexCoordsCount = sizeof(TexCoords) / sizeof(*TexCoords);

                if (bPreferPointInstancer)
                {
                    auto USDPoints = pxr::UsdGeomPointInstancer::Define(Stage, EmitterPath);

                    auto BillboardMeshPath = EmitterPath.AppendElementString("Prototype");
                    auto USDMesh = pxr::UsdGeomMesh::Get(Stage, BillboardMeshPath);
                    if (!USDMesh)
                    {
                        // like static mesh emitters, ideally mesh particle emitters would share the same prototype mesh, however not yet sure how to put them into
                        //  a shared location without them being drawn. keeping them as children of the PointInstancer prevents them from being drawn as non-instances					
                        pxr::VtArray<pxr::GfVec3f> MeshVertices;
                        pxr::VtArray<int> MeshFaceVertexIndices;
                        pxr::VtArray<int> MeshFaceVertexCounts;
                        pxr::VtArray<pxr::GfVec2f> MeshTexCoords;

                        for (int v = 0; v < BillboardVertsCount; ++v)
                        {
                            // weird that we're passing through conversion matrix here? mesh emitter path does it via UStaticMesh conversion, keep it similar. 
                            FVector VertTransformed = USDConvertPosition(Stage, BillboardVerts[v]);
                            pxr::GfVec3f vert = { VertTransformed.X, VertTransformed.Y, VertTransformed.Z };
                            MeshVertices.push_back(vert);
                        }

                        const int VERTS_PER_FACE = 3;
                        const int FACE_COUNT = BillboardIndicesCount / VERTS_PER_FACE;
                        for (int f = 0; f < FACE_COUNT; ++f)
                        {
                            MeshFaceVertexCounts.push_back(VERTS_PER_FACE);
                        }

                        for (int i = 0; i < BillboardIndicesCount; ++i)
                        {
                            MeshFaceVertexIndices.push_back(BillboardIndices[i]);

                            // TexCoords are face-varying, so de-index them
                            MeshTexCoords.push_back(USDConvert(TexCoords[BillboardIndices[i]]));
                        }

                        pxr::VtArray<pxr::GfVec3f> Extent;
                        pxr::UsdGeomPointBased::ComputeExtent(MeshVertices, &Extent);

                        USDMesh = pxr::UsdGeomMesh::Define(Stage, BillboardMeshPath);
                        USDMesh.CreatePointsAttr().Set(MeshVertices);
                        USDMesh.CreateFaceVertexCountsAttr().Set(MeshFaceVertexCounts);
                        USDMesh.CreateFaceVertexIndicesAttr().Set(MeshFaceVertexIndices);
                        USDMesh.CreateExtentAttr().Set(Extent);

                        USDMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));

                        USDMesh.CreatePrimvar(pxr::UsdUtilsGetPrimaryUVSetName(), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying)
                               .Set(MeshTexCoords, TimeCode);

                        pxr::SdfPathVector PrototypesPaths;
                        PrototypesPaths.push_back(BillboardMeshPath);
                        USDPoints.CreatePrototypesRel().SetTargets(PrototypesPaths);
                    }

                    pxr::VtArray<pxr::GfVec3f> Positions(EmitterInstance->ActiveParticles);
                    pxr::VtArray<pxr::GfVec3f> Scales(EmitterInstance->ActiveParticles);
                    pxr::VtArray<pxr::GfQuath> Orientations(EmitterInstance->ActiveParticles);
                    pxr::VtArray<int> ProtoIndices(EmitterInstance->ActiveParticles);

                    for (int i = 0; i < EmitterInstance->ActiveParticles; ++i)
                    {
                        DECLARE_PARTICLE_CONST(Particle, EmitterInstance->ParticleData + EmitterInstance->ParticleStride * EmitterInstance->ParticleIndices[i]);

                        // from GetParticleSize
                        FVector2D InputScale = { FMath::Abs(Particle.Size.X * LocalToWorld.GetScaleVector().X), FMath::Abs(Particle.Size.Y * LocalToWorld.GetScaleVector().Y) };
                        if (LODLevel->RequiredModule->ScreenAlignment == PSA_Square || LODLevel->RequiredModule->ScreenAlignment == PSA_FacingCameraPosition || LODLevel->RequiredModule->ScreenAlignment == PSA_FacingCameraDistanceBlend)
                        {
                            InputScale.Y = InputScale.X;
                        }

                        FVector ParticleSize(1.0, InputScale.X, InputScale.Y); // wtf
                        FMatrix ParticleTransform;
                        CalculateParticleTransform(LODLevel->RequiredModule,
                            PartSys.GetComponentTransform().GetScale3D(),
                            LocalToWorld,
                            Particle.Location, Particle.Rotation, Particle.Velocity, ParticleSize,
                            View->ViewMatrices.GetViewOrigin(),
                            View->GetViewDirection(),
                            ParticleTransform);

                        ParticleTransform = ConversionMatrix * ParticleTransform * ConversionMatrix;

                        // "consume" transform & scale so we get a clean, independent orientation. 
                        FVector Point = ParticleTransform.GetOrigin();
                        Positions[i] = { Point.X, Point.Y, Point.Z };
                        ParticleTransform.RemoveTranslation();

                        FVector Scale = ParticleTransform.GetScaleVector();
                        Scales[i] = { Scale.X, Scale.Y, Scale.Z };
                        ParticleTransform.RemoveScaling();

                        // gotta be an easier way to go from UE4 matrix to USD quat? 
                        FVector Axis;
                        float Angle;
                        ParticleTransform.Rotator().Quaternion().ToAxisAndAngle(Axis, Angle);
                        pxr::GfRotation PxrRotation(pxr::GfVec3d(Axis.X, Axis.Y, Axis.Z), FMath::RadiansToDegrees(Angle));
                        Orientations[i] = pxr::GfQuath(PxrRotation.GetQuat());

                        ProtoIndices[i] = 0; // always just one instance per emitter
                    }

                    USDPoints.CreatePositionsAttr().Set(Positions, TimeCode);
                    USDPoints.CreateScalesAttr().Set(Scales, TimeCode);
                    USDPoints.CreateOrientationsAttr().Set(Orientations, TimeCode);
                    USDPoints.CreateProtoIndicesAttr().Set(ProtoIndices, TimeCode);

                    pxr::VtArray<pxr::GfVec3f> Extent;
                    USDPoints.ComputeExtentAtTime(&Extent, TimeCode, TimeCode);
                    USDPoints.CreateExtentAttr().Set(Extent, TimeCode);
                }
                else
                {
                    pxr::VtArray<pxr::GfVec3f> PointCacheVertices;
                    pxr::VtArray<int> PointCacheFaceVertexIndices;
                    pxr::VtArray<int> PointCacheFaceVertexCounts;
                    pxr::VtArray<pxr::GfVec3f>  PointCacheVelocities;
                    pxr::VtArray<pxr::GfVec3f> PointCacheColors;
                    pxr::VtArray<float> PointCacheOpacities;
                    pxr::VtArray<pxr::GfVec2f> PointCacheTexCoords;

                    for (int p = 0; p < EmitterInstance->ActiveParticles; ++p)
                    {
                        int BeginVertIndex = int(PointCacheVertices.size());

                        DECLARE_PARTICLE_CONST(Particle, EmitterInstance->ParticleData + EmitterInstance->ParticleStride * EmitterInstance->ParticleIndices[p]);

                        // from GetParticleSize
                        FVector2D Scale = { FMath::Abs(Particle.Size.X * LocalToWorld.GetScaleVector().X), FMath::Abs(Particle.Size.Y * LocalToWorld.GetScaleVector().Y) };
                        if (LODLevel->RequiredModule->ScreenAlignment == PSA_Square || LODLevel->RequiredModule->ScreenAlignment == PSA_FacingCameraPosition || LODLevel->RequiredModule->ScreenAlignment == PSA_FacingCameraDistanceBlend)
                        {
                            Scale.Y = Scale.X;
                        }

                        FVector ParticleSize(1.0, Scale.X, Scale.Y); // wtf
                        FMatrix ParticleTransform;
                        CalculateParticleTransform(LODLevel->RequiredModule,
                            PartSys.GetComponentTransform().GetScale3D(),
                            LocalToWorld,
                            Particle.Location, Particle.Rotation, Particle.Velocity, ParticleSize,
                            View->ViewMatrices.GetViewOrigin(),
                            View->GetViewDirection(),
                            ParticleTransform);

                        for (int v = 0; v < BillboardVertsCount; ++v)
                        {
                            FVector VertTransformed = ParticleTransform.TransformPosition(BillboardVerts[v]);
                            VertTransformed = USDConvertPosition(Stage, VertTransformed);
                            pxr::GfVec3f vert = { VertTransformed.X, VertTransformed.Y, VertTransformed.Z };
                            PointCacheVertices.push_back(vert);

                            // we're applying the particle's velocity to each vertex. does not account for angular velocity
                            //  or growing/shrinking 
                            pxr::GfVec3f vel = { Particle.Velocity.X, Particle.Velocity.Y, Particle.Velocity.Z };
                            PointCacheVelocities.push_back(vel);
                        }

                        const int VERTS_PER_FACE = 3;
                        const int FACE_COUNT = BillboardIndicesCount / VERTS_PER_FACE;
                        for (int f = 0; f < FACE_COUNT; ++f)
                        {
                            PointCacheFaceVertexCounts.push_back(VERTS_PER_FACE);
                        }

                        for (int i = 0; i < BillboardIndicesCount; ++i)
                        {
                            int adjustedIndex = BeginVertIndex + BillboardIndices[i];
                            PointCacheFaceVertexIndices.push_back(adjustedIndex);

                            // TODO: should we only do this if there's a color module? 
                            PointCacheColors.push_back(USDConvert(Particle.Color));
                            PointCacheOpacities.push_back(Particle.Color.A);

                            // TexCoords are face-varying, so de-index them
                            PointCacheTexCoords.push_back(USDConvert(TexCoords[BillboardIndices[i]]));
                        }
                    }

                    // compute extent...
                    pxr::VtArray<pxr::GfVec3f> extent;
                    pxr::UsdGeomPointBased::ComputeExtent(PointCacheVertices, &extent);

                    // ...before padding vertex buffer to constant worst-case size, so that bogus vertices don't skew extents
                    for (int i = EmitterInstance->ActiveParticles; i < EmitterInstance->MaxActiveParticles; ++i)
                    {
                        for (int v = 0; v < BillboardVertsCount; ++v)
                        {
                            pxr::GfVec3f vert = { 0, 0, 0 };
                            PointCacheVertices.push_back(vert);
                        }
                    }
                    check(PointCacheVertices.size() == EmitterInstance->MaxActiveParticles * BillboardVertsCount);


                    std::string reason;
                    check(pxr::UsdGeomMesh::ValidateTopology(PointCacheFaceVertexIndices, PointCacheFaceVertexCounts, PointCacheVertices.size(), &reason));

                    // optimization - prebuilt index buffer that's prebuilt for worst-case size.
                    // problematic, USDView complains. 
                    /*pxr::VtArray<int> faceVertexIndices_timeOptimized;
                    for (int p = 0; p < EmitterInstance->MaxActiveParticles; ++p)
                    {
                        int BeginVertIndex = p * BillboardVertsCount;
                        for (int i = 0; i < BillboardIndicesCount; ++i)
                        {
                            int adjustedIndex = BeginVertIndex + BillboardIndices[i];
                            faceVertexIndices_timeOptimized.push_back(adjustedIndex);
                        }
                    }
                    emitter.CreateFaceVertexIndicesAttr().Set(faceVertexIndices_timeOptimized);*/

                    pxr::UsdGeomMesh USDMesh = pxr::UsdGeomMesh::Define(Stage, EmitterPath);
                    USDMesh.CreatePointsAttr().Set(PointCacheVertices, TimeCode);
                    USDMesh.CreateFaceVertexCountsAttr().Set(PointCacheFaceVertexCounts, TimeCode);
                    USDMesh.CreateFaceVertexIndicesAttr().Set(PointCacheFaceVertexIndices, TimeCode);
                    USDMesh.CreateVelocitiesAttr().Set(PointCacheVelocities, TimeCode);
                    USDMesh.CreateExtentAttr().Set(extent, TimeCode);
                    USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheColors, TimeCode);
                    USDMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->faceVarying).Set(PointCacheOpacities, TimeCode);

                    auto PrimvarTexCoords = USDMesh.CreatePrimvar(pxr::UsdUtilsGetPrimaryUVSetName(), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying);
                    PrimvarTexCoords.Set(PointCacheTexCoords, TimeCode);

                    USDMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));
                }
            }
        }
    }

    // return an empty transform, which could be used for indicating emitter location/etc. but no particles live here
    return pxr::UsdGeomXform::Define(Stage, Path);
}


bool USDImportCamera(const pxr::UsdGeomCamera& Prim, class UCameraComponent& CameraComponent, FNamedParameterTimeSamples& NamedParameterTimeSamples)
{
    auto static ConvertVector = [](const pxr::GfVec4f & Vector) -> FVector4
    {
        return FVector4(Vector[0], Vector[1], Vector[2], Vector[3]);
    };

    auto Stage = Prim.GetPrim().GetStage();
    std::vector<double> Times;
    Prim.GetTimeSamples(&Times);
    auto PrimCamera = Prim.GetCamera(Times.size() > 0 ? Times[0] : pxr::UsdTimeCode::Default());
    if (auto CineCamera = Cast<UCineCameraComponent>(&CameraComponent))
    {
        auto PostGetFunc = [&](float& InOutValue)
        {
        };

        auto PostGetWorldUnitFunc = [&](float& InOutValue)
        {
            InOutValue = USDConvertLength(Stage, InOutValue, true);
        };

        LOAD_NAMED_PARAMETER(FFloatTimeSamples, NamedParameterTimeSamples.ScalarTimeSamples, Prim, float, FStop, UCineCameraComponent, CineCamera, CurrentAperture, PostGetFunc);
        LOAD_NAMED_PARAMETER(FFloatTimeSamples, NamedParameterTimeSamples.ScalarTimeSamples, Prim, float, FocusDistance, UCineCameraComponent, CineCamera, FocusSettings.ManualFocusDistance, PostGetWorldUnitFunc); // cm or world unit
        LOAD_NAMED_PARAMETER(FFloatTimeSamples, NamedParameterTimeSamples.ScalarTimeSamples, Prim, float, FocalLength, UCineCameraComponent, CineCamera, CurrentFocalLength, PostGetWorldUnitFunc); // mm or tenths of world unit
        LOAD_NAMED_PARAMETER(FFloatTimeSamples, NamedParameterTimeSamples.ScalarTimeSamples, Prim, float, HorizontalAperture, UCineCameraComponent, CineCamera, Filmback.SensorWidth, PostGetWorldUnitFunc); //mm or tenths of world unit
        LOAD_NAMED_PARAMETER(FFloatTimeSamples, NamedParameterTimeSamples.ScalarTimeSamples, Prim, float, VerticalAperture, UCineCameraComponent, CineCamera, Filmback.SensorHeight, PostGetWorldUnitFunc); //mm or tenths of world unit

        auto CameraPrim = Prim.GetPrim();

#define GET_PROPERTY(PropertyName, PropertyValue, PropertyType) \
        {\
            auto USDVtValue = CameraPrim.GetCustomDataByKey(pxr::TfToken(PropertyName));\
            if (!USDVtValue.IsEmpty())\
            {\
                PropertyValue = USDVtValue.Get<PropertyType>();\
            }\
        }

#define GET_ENUM_PROPERTY(PropertyName, PropertyValue, PropertyType) \
        {\
            auto USDVtValue = CameraPrim.GetCustomDataByKey(pxr::TfToken(PropertyName));\
            if (!USDVtValue.IsEmpty())\
            {\
                int IntValue = USDVtValue.Get<int>();\
                PropertyValue = (PropertyType) IntValue;\
            }\
        }

#define GET_UCHAR_PROPERTY(PropertyName, PropertyValue) \
        GET_PROPERTY(PropertyName, PropertyValue, uint8)

#define Get_VEC4_PROPERTY(PropertyName, PropertyValue) \
        {\
            auto USDVtValue = CameraPrim.GetCustomDataByKey(pxr::TfToken(PropertyName));\
            if (!USDVtValue.IsEmpty())\
            {\
                pxr::GfVec4f VecValue = USDVtValue.Get<pxr::GfVec4f>();\
                PropertyValue = ConvertVector(VecValue);\
            }\
        }\

        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_MIN_FSTOP, CineCamera->LensSettings.MinFStop, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_MAX_FSTOP, CineCamera->LensSettings.MaxFStop, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY, CineCamera->PostProcessSettings.SceneFringeIntensity, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET, CineCamera->PostProcessSettings.ChromaticAberrationStartOffset, float);
        GET_ENUM_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD, CineCamera->PostProcessSettings.BloomMethod, EBloomMethod);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY, CineCamera->PostProcessSettings.BloomIntensity, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD, CineCamera->PostProcessSettings.BloomThreshold, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED, CineCamera->PostProcessSettings.CameraShutterSpeed, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO, CineCamera->PostProcessSettings.CameraISO, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE, CineCamera->PostProcessSettings.DepthOfFieldFstop, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE, CineCamera->PostProcessSettings.DepthOfFieldMinFstop, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES, CineCamera->PostProcessSettings.DepthOfFieldBladeCount, int);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN, CineCamera->PostProcessSettings.AutoExposureBias, float);
        GET_ENUM_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE, CineCamera->PostProcessSettings.AutoExposureMethod, EAutoExposureMethod);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS, CineCamera->PostProcessSettings.AutoExposureMaxBrightness, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS, CineCamera->PostProcessSettings.AutoExposureMinBrightness, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP, CineCamera->PostProcessSettings.AutoExposureSpeedUp, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN, CineCamera->PostProcessSettings.AutoExposureSpeedDown, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY, CineCamera->PostProcessSettings.VignetteIntensity, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER, CineCamera->PostProcessSettings.GrainJitter, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY, CineCamera->PostProcessSettings.GrainIntensity, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM, CineCamera->PostProcessSettings.DepthOfFieldDepthBlurAmount, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS, CineCamera->PostProcessSettings.DepthOfFieldDepthBlurRadius, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE, CineCamera->PostProcessSettings.DepthOfFieldFarBlurSize, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION, CineCamera->PostProcessSettings.DepthOfFieldFarTransitionRegion, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION, CineCamera->PostProcessSettings.DepthOfFieldFocalRegion, float);
        //4.22 deprecated GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE, CineCamera->PostProcessSettings.DepthOfFieldMaxBokehSize, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE, CineCamera->PostProcessSettings.DepthOfFieldNearBlurSize, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION, CineCamera->PostProcessSettings.DepthOfFieldNearTransitionRegion, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE, CineCamera->PostProcessSettings.DepthOfFieldScale, float);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE, CineCamera->PostProcessSettings.DepthOfFieldFocalDistance, float);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF, CineCamera->PostProcessSettings.bMobileHQGaussian);
        //4.22 deprecated GET_ENUM_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD, CineCamera->PostProcessSettings.DepthOfFieldMethod, EDepthOfFieldMethod);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST, CineCamera->PostProcessSettings.ColorContrast);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION, CineCamera->PostProcessSettings.ColorSaturation);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN, CineCamera->PostProcessSettings.ColorGain);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA, CineCamera->PostProcessSettings.ColorGamma);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET, CineCamera->PostProcessSettings.ColorOffset);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST, CineCamera->PostProcessSettings.ColorContrastShadows);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION, CineCamera->PostProcessSettings.ColorSaturationShadows);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN, CineCamera->PostProcessSettings.ColorGainShadows);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA, CineCamera->PostProcessSettings.ColorGammaShadows);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET, CineCamera->PostProcessSettings.ColorOffsetShadows);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX, CineCamera->PostProcessSettings.ColorCorrectionShadowsMax, float);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST, CineCamera->PostProcessSettings.ColorContrastMidtones);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION, CineCamera->PostProcessSettings.ColorSaturationMidtones);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN, CineCamera->PostProcessSettings.ColorGainMidtones);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA, CineCamera->PostProcessSettings.ColorGammaMidtones);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET, CineCamera->PostProcessSettings.ColorOffsetMidtones);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST, CineCamera->PostProcessSettings.ColorContrastHighlights);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION, CineCamera->PostProcessSettings.ColorSaturationHighlights);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN, CineCamera->PostProcessSettings.ColorGainHighlights);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA, CineCamera->PostProcessSettings.ColorGammaHighlights);
        Get_VEC4_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET, CineCamera->PostProcessSettings.ColorOffsetHighlights);
        GET_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN, CineCamera->PostProcessSettings.ColorCorrectionHighlightsMin, float);

        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_SceneFringeIntensity);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_CHROMATIC_ABERRATION_START_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ChromaticAberrationStartOffset);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_METHOD_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomMethod);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomIntensity);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_BLOOM_THRESHOLD_TOGGLE, CineCamera->PostProcessSettings.bOverride_BloomThreshold);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_SHUTTER_SPEED_TOGGLE, CineCamera->PostProcessSettings.bOverride_CameraShutterSpeed);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_ISO_TOGGLE, CineCamera->PostProcessSettings.bOverride_CameraISO);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_APERTURE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFstop);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_MAX_APERTURE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMinFstop);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_NUM_DIA_BLADES_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldBladeCount);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_LENS_CAMERA_EXPO_COMPEN_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureBias);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_METERING_MODE_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMethod);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MAX_BRIGHTNESS_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMaxBrightness);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_MIN_BRIGHTNESS_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureMinBrightness);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_UP_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureSpeedUp);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_EXPOSURE_SPEED_DOWN_TOGGLE, CineCamera->PostProcessSettings.bOverride_AutoExposureSpeedDown);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_VEGNETTE_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_VignetteIntensity);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_JITTER_TOGGLE, CineCamera->PostProcessSettings.bOverride_GrainJitter);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_IMAGE_EFFECTS_GRAIN_INTENSITY_TOGGLE, CineCamera->PostProcessSettings.bOverride_GrainIntensity);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_KM_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldDepthBlurAmount);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_BLUR_RADIUS_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldDepthBlurRadius);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_BLUR_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFarBlurSize);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FAR_TRANSITION_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFarTransitionRegion);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_FOCAL_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFocalRegion);
        //4.22 deprecated GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_MAX_BOKEH_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMaxBokehSize);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_BLUR_SIZE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldNearBlurSize);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_NEAR_TRANSITION_REGION_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldNearTransitionRegion);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_DEPTH_SCALE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldScale);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_FOCAL_DISTANCE_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldFocalDistance);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_HIGH_QUALITY_GAUSSIAN_DOF_TOGGLE, CineCamera->PostProcessSettings.bOverride_MobileHQGaussian);
        //4.22 deprecated GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_DOF_METHOD_TOGGLE, CineCamera->PostProcessSettings.bOverride_DepthOfFieldMethod);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrast);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturation);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGain);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGamma);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_GLOBAL_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffset);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastShadows);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationShadows);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainShadows);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaShadows);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetShadows);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_SHADOWS_MAX_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorCorrectionShadowsMax);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastMidtones);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationMidtones);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainMidtones);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaMidtones);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_MIDTONES_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetMidtones);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_CONTRAST_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorContrastHighlights);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_SATURATION_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorSaturationHighlights);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGainHighlights);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_GAMMA_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorGammaHighlights);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_OFFSET_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorOffsetHighlights);
        GET_UCHAR_PROPERTY(CUSTOM_KEY_UE4_CINE_CAMERA_COLOR_GRADING_HIGHLIGHTS_MIN_TOGGLE, CineCamera->PostProcessSettings.bOverride_ColorCorrectionHighlightsMin);

        // zero focus distance using the setting from post-process
        if ( FMath::IsNearlyZero(CineCamera->FocusSettings.ManualFocusDistance) )
        {
            CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::DoNotOverride;
        }
        if (CineCamera->CurrentFocalLength < CineCamera->LensSettings.MinFocalLength)
        {
            CineCamera->LensSettings.MinFocalLength = CineCamera->CurrentFocalLength;
        }
        if (CineCamera->CurrentFocalLength > CineCamera->LensSettings.MaxFocalLength)
        {
            CineCamera->LensSettings.MaxFocalLength = CineCamera->CurrentFocalLength;
        }
        if (CineCamera->CurrentAperture < CineCamera->LensSettings.MinFStop)
        {
            CineCamera->LensSettings.MinFStop = CineCamera->CurrentAperture;
        }
        if (CineCamera->CurrentAperture > CineCamera->LensSettings.MaxFStop)
        {
            CineCamera->LensSettings.MaxFStop = CineCamera->CurrentAperture;
        }
        if (CineCamera->FocusSettings.ManualFocusDistance < CineCamera->LensSettings.MinimumFocusDistance * 0.1f)
        {
            CineCamera->LensSettings.MinimumFocusDistance = CineCamera->FocusSettings.ManualFocusDistance / 0.1f;
        }
    }
    else
    {
        switch (PrimCamera.GetProjection())
        {
        case pxr::GfCamera::Perspective:
            CameraComponent.SetProjectionMode(ECameraProjectionMode::Perspective);
            {
                std::vector<double> HATimeSamples;
                std::vector<double> FLTimeSamples;
                Prim.GetHorizontalApertureAttr().GetTimeSamples(&HATimeSamples);
                Prim.GetFocalLengthAttr().GetTimeSamples(&FLTimeSamples);
                if (HATimeSamples.size() > 0 || FLTimeSamples.size() > 0)
                {
                    FFloatTimeSamples FloatTimeSamples;
                    for (auto Time : Times)
                    {
                        auto Camera = Prim.GetCamera(pxr::UsdTimeCode(Time));
                        FloatTimeSamples.TimeSamples.Add(Time, Camera.GetFieldOfView(pxr::GfCamera::FOVDirection::FOVHorizontal));
                    }

                    auto Name = GET_MEMBER_NAME_CHECKED(UCameraComponent, FieldOfView);
                    NamedParameterTimeSamples.ScalarTimeSamples.Add(Name, FloatTimeSamples);
                }
            }
            CameraComponent.SetFieldOfView(PrimCamera.GetFieldOfView(pxr::GfCamera::FOVDirection::FOVHorizontal));
            break;

        case pxr::GfCamera::Orthographic:
            CameraComponent.SetProjectionMode(ECameraProjectionMode::Orthographic);
            CameraComponent.SetOrthoWidth(PrimCamera.GetHorizontalAperture() * PrimCamera.APERTURE_UNIT);
            CameraComponent.SetOrthoNearClipPlane(PrimCamera.GetClippingRange().GetMin());
            CameraComponent.SetOrthoFarClipPlane(PrimCamera.GetClippingRange().GetMax());
            break;
        }


        std::vector<double> HATimeSamples;
        std::vector<double> VATimeSamples;
        Prim.GetHorizontalApertureAttr().GetTimeSamples(&HATimeSamples);
        Prim.GetVerticalApertureAttr().GetTimeSamples(&VATimeSamples);
        if (HATimeSamples.size() > 0 || VATimeSamples.size() > 0)
        {
            FFloatTimeSamples FloatTimeSamples;
            for (auto Time : Times)
            {
                auto Camera = Prim.GetCamera(pxr::UsdTimeCode(Time));
                FloatTimeSamples.TimeSamples.Add(Time, Camera.GetAspectRatio());	
            }

            auto Name = GET_MEMBER_NAME_CHECKED(UCameraComponent, AspectRatio);
            NamedParameterTimeSamples.ScalarTimeSamples.Add(Name, FloatTimeSamples);
        }
        CameraComponent.SetAspectRatio(PrimCamera.GetAspectRatio());
    }

    return true;
}

FString MakeAssetPathRelative(FString Path, pxr::UsdStage& USDStage)
{
    auto LayerPath = USDStage.GetEditTarget().GetLayer()->GetRealPath();

    return FSimReadyPathHelper::ComputeRelativePath(Path, UTF8_TO_TCHAR(LayerPath.c_str()));
}

enum class ESimReadyLightType : uint8_t
{
    Point,
    Spot,
    Rect,
    Directional,
    Sky,
    Unknown
};

// convert intensity from usd to UE4 (rect/spot/point light), default unit of UE4 is candelas(cm^2)
static float ConvertIntensityFromNitToCandelas(float Intensity, float Area)
{
    Area = FMath::Max(UNIT_PER_SQUARE_METRE, Area);
    // nit = cd / m^2
    // for usd light -> point/spot/rect light
    // USD light unit is m^2 from cm^2 of UE4
    return Intensity * Area * UNIT_PER_SQUARE_METRE * PI;
}

// convert intensity from usd to UE4 (directional light), default unit of UE4 is Lux(cm^2)
static float ConvertIntensityFromNitToLux(float Intensity)
{
    // nit = cd / m^2 = lm / sr / m^2
    // lux = lm / m^2 
    // lux = nit * sr, sr = pi
    // for usd light -> directional light
    // UE4 light unit is cm^2 from m^2 of USD
    return Intensity * 16.0f * PI * UNIT_PER_SQUARE_METRE;
}

// convert intensity from UE4 (directional light) to usd
static float ConvertIntensityFromLuxToNit(float Intensity)
{
    // nit = lux / sr, sr = pi
    // for directional light -> usd light
    // USD light unit is m^2 from cm^2 of UE4
    return Intensity / (16.0f * PI) / UNIT_PER_SQUARE_METRE;
}

// for compatible with old usd
static float ConvertIntensityFromLumens(ESimReadyLightType LightType, ELightUnits IntensityUnits, float Intensity, float ConeAngle)
{
    switch (IntensityUnits)
    {
    case ELightUnits::Candelas:
        if (LightType == ESimReadyLightType::Point)
        {
            Intensity /= 12.6f;
        }
        else if (LightType == ESimReadyLightType::Spot)
        {
            Intensity /= 1.76f;
        }
        else if (LightType == ESimReadyLightType::Rect)
        {
            Intensity /= 3.14f;
        }

        break;

    case ELightUnits::Unitless:
        if (LightType == ESimReadyLightType::Point)
        {
            Intensity *= 49.7f;
        }
        else if (LightType == ESimReadyLightType::Spot)
        {
            Intensity *= 99.0f / (1 - FMath::Cos(ConeAngle * PI / 180.f));
        }
        else if (LightType == ESimReadyLightType::Rect)
        {
            Intensity *= 199.0f;
        }

        break;
    }

    return Intensity;
}

// convert intensity from UE4 (rect/point/spot light) to usd
static float ConvertIntensityToNit(ESimReadyLightType LightType, ELightUnits IntensityUnits, float Intensity, float ConeAngle, float Area)
{
    // convert intensity from cm^2 to m^2
    Intensity /= UNIT_PER_SQUARE_METRE;
    
    Area = FMath::Max(UNIT_PER_SQUARE_METRE, Area);
    // for directional light -> usd light
    switch (IntensityUnits)
    {
    case ELightUnits::Candelas:
        // nit = cd / m^2
        Intensity /= Area;
        break;

    case ELightUnits::Lumens:
        // nit = lumen / sr / m^2
        if (LightType == ESimReadyLightType::Point)
        {
            Intensity = Intensity / (4.0f * PI) / Area;
        }
        else if (LightType == ESimReadyLightType::Spot)
        {
            Intensity = Intensity / (2.0f * PI * (1.0f - FMath::Cos(ConeAngle * PI / 180.f))) / Area;
        }
        else if (LightType == ESimReadyLightType::Rect)
        {
            Intensity = Intensity / PI / Area;
        }

        break;

    case ELightUnits::Unitless:
        // nit = unit / 625 / m^2
        Intensity = Intensity / 625.0f / Area;
        break;
    }

    return Intensity / PI;
}

// input USD intensity
// output UE4 intensity
float USDImportLightIntensity(const pxr::UsdPrim& Light, ULightComponentBase& LightComponent, float Intensity)
{
    ESimReadyLightType LightType = ESimReadyLightType::Unknown;
    ELightUnits LightUnits = ELightUnits::Candelas; // this's for rect/point/spot light
    // Compatible with old usd with ue4 light unit
    auto IntensityUnitsValue = Light.GetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_INTENSITY_UNITS));

    if (auto LocalLight = Cast<ULocalLightComponent>(&LightComponent))
    {
         LightUnits = LocalLight->IntensityUnits;
    }

    float ConeAngle = 44.0f;
    if (auto USDDistanceLight = pxr::UsdLuxDistantLight(Light))
    {
        LightType = ESimReadyLightType::Directional;

        // Compatible with old usd with ue4 light unit
        if (IntensityUnitsValue.IsEmpty())
        {
            Intensity = ConvertIntensityFromNitToLux(Intensity);
        }
    }
    else if (auto USDLight = pxr::UsdLuxSphereLight(Light))
    {
        float Area = UNIT_PER_SQUARE_METRE; // 1 m^2

        // Create spot light component and actor
        USpotLightComponent* SpotLight = Cast<USpotLightComponent>(&LightComponent);
        if (SpotLight)
        {
            LightType = ESimReadyLightType::Spot;
            ConeAngle = SpotLight->OuterConeAngle;
            Area *= 4.0f * PI * SpotLight->SourceRadius * SpotLight->SourceRadius; // 1m^2
        }
        else
        {
            LightType = ESimReadyLightType::Point;
            UPointLightComponent* PointLight = Cast<UPointLightComponent>(&LightComponent);
            if (PointLight)
            {
                Area *= 4.0f * PI * PointLight->SourceRadius * PointLight->SourceRadius;
            }
        }

        if (IntensityUnitsValue.IsEmpty())
        {
            Intensity = ConvertIntensityFromNitToCandelas(Intensity, Area);
        }
        // Compatible with old usd with ue4 light unit
        else
        {
            Intensity = ConvertIntensityFromLumens(LightType, LightUnits, Intensity, ConeAngle);
        }
    }
    else if (auto USDRectLight = pxr::UsdLuxRectLight(Light))
    {
        LightType = ESimReadyLightType::Rect;
        float Area = UNIT_PER_SQUARE_METRE; // m^2
        URectLightComponent* RectLightComponent = Cast<URectLightComponent>(&LightComponent);
        if (RectLightComponent)
        {
            Area *= RectLightComponent->SourceWidth * RectLightComponent->SourceHeight;
        }

        if (IntensityUnitsValue.IsEmpty())
        {
            Intensity = ConvertIntensityFromNitToCandelas(Intensity, Area);
        }
        // Compatible with old usd with ue4 light unit
        else
        {
            Intensity = ConvertIntensityFromLumens(LightType, LightUnits, Intensity, ConeAngle);
        }
    }

    return Intensity;
}

bool USDImportLight(const pxr::UsdPrim& Light, ULightComponent& LightComponent, FNamedParameterTimeSamples& NamedParameterTimeSamples)
{
    if (!Light)
    {
        return false;
    }

    // Get the old/new UsdLuxLight schema preference
    bool bPreferNewSchema = GetDefault<USimReadySettings>()->bPreferNewUsdLuxLightSchemaOnImport;
    pxr::UsdAttribute LightAttr;

    auto Stage = Light.GetStage();
    auto CastShadowsValue = Light.GetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_CAST_SHADOWS));
    if (!CastShadowsValue.IsEmpty())
    {
        auto bCastShadows = CastShadowsValue.Get<uint32>();
        LightComponent.CastShadows = bCastShadows;
    }

    ELightUnits LightUnits = ELightUnits::Candelas; // this's for rect/point/spot light

    // Compatible with old usd with ue4 light unit
    auto IntensityUnitsValue = Light.GetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_INTENSITY_UNITS));
    if (!IntensityUnitsValue.IsEmpty())
    {
        pxr::TfToken IntensityUnitsToken = IntensityUnitsValue.Get<pxr::TfToken>();
        if (IntensityUnitsToken == pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_INTENSITY_CANDES))
        {
            LightUnits = ELightUnits::Candelas;
        }
        else if (IntensityUnitsToken == pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_INTENSITY_UNITLESS))
        {
            LightUnits = ELightUnits::Unitless;
        }
        else //if (IntensityUnitsToken == pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_INTENSITY_LUMENS))
        {
            LightUnits = ELightUnits::Lumens;
        }
    }

    if (auto LocalLight = Cast<ULocalLightComponent>(&LightComponent))
    {
        auto AttenuateDistanceValue = Light.GetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_ATTENUATE_DISTANCE));
        if (!AttenuateDistanceValue.IsEmpty())
        {
            LocalLight->AttenuationRadius = AttenuateDistanceValue.Get<float>();
        }
        LocalLight->AttenuationRadius = USDConvertLength(Stage, LocalLight->AttenuationRadius, true);
        LocalLight->IntensityUnits = LightUnits;
    }

    if (auto USDDistanceLight = pxr::UsdLuxDistantLight(Light))
    {
        auto DirectionalLightComponent = Cast<UDirectionalLightComponent>(&LightComponent);
        if (DirectionalLightComponent)
        {
            LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDDistanceLight.GetPrim(), USDDistanceLight.GetAngleAttr(), bPreferNewSchema);
            DirectionalLightComponent->LightSourceAngle = GetUSDValue<float>(LightAttr);
        }
    }
    else if (auto USDLight = pxr::UsdLuxSphereLight(Light))
    {
        UPointLightComponent* PointLight = Cast<UPointLightComponent>(&LightComponent);
        if (PointLight)
        {
            LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDLight.GetPrim(), USDLight.GetRadiusAttr(), bPreferNewSchema);
            PointLight->SetSourceRadius(USDConvertLength(Stage, GetUSDValue<float>(LightAttr), true));
        }

        // Create spot light component and actor
        USpotLightComponent* SpotLight = Cast<USpotLightComponent>(&LightComponent);
        if (SpotLight)
        {
            auto Cone = pxr::UsdLuxShapingAPI(Light);
            if (Cone)
            {
                LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDLight.GetPrim(), Cone.GetShapingConeAngleAttr(), bPreferNewSchema);
                auto ConeAngle = GetUSDValue<float>(LightAttr);
                LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDLight.GetPrim(), Cone.GetShapingConeSoftnessAttr(), bPreferNewSchema);
                auto Softness = GetUSDValue<float>(LightAttr);
                SpotLight->OuterConeAngle = ConeAngle;
                const float ClampedOuterConeAngle = SpotLight->OuterConeAngle * (float)PI / 180.0f;
                float CosInnerCone = FMath::Cos(ClampedOuterConeAngle) + Softness;
                SpotLight->InnerConeAngle = FMath::Acos(CosInnerCone) * 180.0f / (float)PI;
            }
        }
    }
    else if (auto USDRectLight = pxr::UsdLuxRectLight(Light))
    {
        URectLightComponent* RectLightComponent = Cast<URectLightComponent>(&LightComponent);
        if (RectLightComponent)
        {
            LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDRectLight.GetPrim(), USDRectLight.GetWidthAttr(), bPreferNewSchema);
            RectLightComponent->SourceWidth = USDConvertLength(Stage, GetUSDValue<float>(LightAttr), true);
            LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDRectLight.GetPrim(), USDRectLight.GetHeightAttr(), bPreferNewSchema);
            RectLightComponent->SourceHeight = USDConvertLength(Stage, GetUSDValue<float>(LightAttr), true);
        }
    }
    else
    {
        return false;
    }

#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111

    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetEnableColorTemperatureAttr(), bPreferNewSchema);
    LightComponent.bUseTemperature = GetUSDValue<bool>(LightAttr);
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetColorTemperatureAttr(), bPreferNewSchema);
    LightComponent.Temperature = GetUSDValue<float>(LightAttr);

    auto PostGetIntensity = [&](float& InOutIntensity)
    {
        InOutIntensity = USDImportLightIntensity(Light, LightComponent, InOutIntensity);
    };

    FFloatTimeSamples IntensityTimeSamples;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetIntensityAttr(), bPreferNewSchema);
    if (GetUSDTimeSamples<float, FFloatTimeSamples>(LightAttr, IntensityTimeSamples, PostGetIntensity))
    {
        auto Name = GET_MEMBER_NAME_CHECKED(ULightComponent, Intensity);
        
        FFloatTimeSamples ExposureTimeSamples;
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetExposureAttr(), bPreferNewSchema);
        if (GetUSDTimeSamples<float, FFloatTimeSamples>(LightAttr, ExposureTimeSamples))
        {
            FFloatTimeSamples BlendTimeSamples;
            // Blend Exposure and Intensity to UE4 Intensity
            TArray<int32> IntensityKeys;
            IntensityTimeSamples.TimeSamples.GetKeys(IntensityKeys);
            IntensityKeys.Sort();
            TArray<int32> ExposureKeys;
            ExposureTimeSamples.TimeSamples.GetKeys(ExposureKeys);
            ExposureKeys.Sort();
            const int32 Start = IntensityKeys[0] < ExposureKeys[0] ? IntensityKeys[0] : ExposureKeys[0];
            const int32 End = IntensityKeys[IntensityKeys.Num() - 1] < ExposureKeys[ExposureKeys.Num() - 1] ? ExposureKeys[ExposureKeys.Num() - 1] : IntensityKeys[IntensityKeys.Num() - 1];
            float LastIntensity = IntensityTimeSamples.TimeSamples[IntensityKeys[0]];
            float LastExposure = ExposureTimeSamples.TimeSamples[ExposureKeys[0]];
            int32 NextIntensityKey = 0;
            int32 NextExposureKey = 0;
            for (int32 TimeCode = Start; TimeCode <= End; ++TimeCode)
            {
                int32 LastTimeCode = (TimeCode == Start) ? Start : (TimeCode - 1);
                float* IntensityValue = IntensityTimeSamples.TimeSamples.Find(TimeCode);
                float* ExposureValue = ExposureTimeSamples.TimeSamples.Find(TimeCode);
                float FinalIntensity;
                float FinalExposure;

                // Calculate intensity
                if (IntensityValue == nullptr)
                {
                    if (NextIntensityKey >= IntensityKeys.Num())
                    {
                        FinalIntensity = LastIntensity;
                    }
                    else
                    {
                        FinalIntensity = FMath::Lerp(LastIntensity, IntensityTimeSamples.TimeSamples[IntensityKeys[NextIntensityKey]],
                            (float)(TimeCode - LastTimeCode) / (float)(IntensityKeys[NextIntensityKey] - LastTimeCode));
                    }
                }
                else
                {
                    ++NextIntensityKey;
                    FinalIntensity = *IntensityValue;
                }

                // Calculate exposure
                if (ExposureValue == nullptr)
                {
                    if (NextExposureKey >= ExposureKeys.Num())
                    {
                        FinalExposure = LastExposure;
                    }
                    else
                    {
                        FinalExposure = FMath::Lerp(LastExposure, ExposureTimeSamples.TimeSamples[ExposureKeys[NextExposureKey]],
                            (float)(TimeCode - LastTimeCode) / (float)(ExposureKeys[NextExposureKey] - LastTimeCode));
                    }
                }
                else
                {
                    ++NextExposureKey;
                    FinalExposure = *ExposureValue;
                }

                LastIntensity = FinalIntensity;
                LastExposure = FinalExposure;

                BlendTimeSamples.TimeSamples.Add(TimeCode, FinalIntensity * FMath::Pow(2.0f, FinalExposure));
            }
            NamedParameterTimeSamples.ScalarTimeSamples.Add(Name, BlendTimeSamples);
        }
        else
        {
            NamedParameterTimeSamples.ScalarTimeSamples.Add(Name, IntensityTimeSamples);
        }
    }
    else
    {
        FFloatTimeSamples ExposureTimeSamples;
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetExposureAttr(), bPreferNewSchema);
        if (GetUSDTimeSamples<float, FFloatTimeSamples>(LightAttr, ExposureTimeSamples))
        {
            auto Name = GET_MEMBER_NAME_CHECKED(ULightComponent, Intensity);
            for (auto& Pair : ExposureTimeSamples.TimeSamples)
            {
                Pair.Value = LightComponent.Intensity * FMath::Pow(2.0f, Pair.Value);
            }
            NamedParameterTimeSamples.ScalarTimeSamples.Add(Name, ExposureTimeSamples);
        }
    }
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetIntensityAttr(), bPreferNewSchema);
    LightComponent.Intensity = USDImportLightIntensity(Light, LightComponent, GetUSDValue<float>(LightAttr)); // usd intensity is nits;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetExposureAttr(), bPreferNewSchema);
    LightComponent.Intensity *= FMath::Pow(2.0f, GetUSDValue<float>(LightAttr));

    FColorTimeSamples ColorTimeSamples;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetColorAttr(), bPreferNewSchema);
    if (GetUSDTimeSamples<pxr::GfVec3f, FColorTimeSamples>(LightAttr, ColorTimeSamples))
    {
        auto Name = GET_MEMBER_NAME_CHECKED(ULightComponent, LightColor);
        NamedParameterTimeSamples.ColorTimeSamples.Add(Name, ColorTimeSamples);
    }

    pxr::GfVec3f Color = GetUSDValue<pxr::GfVec3f>(LightAttr);
    LightComponent.LightColor = USDConvertToLinearColor(Color).ToFColor(true);

    return true;
}

const float GetLightIntensityScale()
{
    return 4000 / ConvertIntensityFromLuxToNit(6000);	// CARLA light intensity is too large. To correctly scale it down, we assume the sun light has the same brightness in AV Sim and CARLA when it's at the very top of sky, where the sun light is vertically down. In this condition, AV Sim default dome light brightness is approximately equal to a dist light of intensity 4000. CARLA sun light has intensity of 6000 Lux. We scale the CARLA sun light to USD dist light intensity 4000, and use the scale value to scale down all lights.
}

TOptional<float> GetRadiusForNoShadowPointLight(const UPointLightComponent& PointLight)
{
    if (!PointLight.CastShadows && !PointLight.IESTexture)
    {
        return 25.f;
    }
    else
    {
        return {};
    }
}

float ConvertUSDLightIntensity(const ULightComponentBase& LightComponent, float Intensity)
{
    ESimReadyLightType LightType = ESimReadyLightType::Unknown;
    ELightUnits LightUnits = ELightUnits::Candelas; // Get 
    float ConeAngle = 44.0f;
    if (auto LocalLight = Cast<ULocalLightComponent>(&LightComponent))
    {
        LightUnits = LocalLight->IntensityUnits;
    }

    if (auto DirectionalLightComponent = Cast<UDirectionalLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Directional;
        Intensity = ConvertIntensityFromLuxToNit(Intensity);
    }
    else if (auto PointLight = Cast<UPointLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Point;
        float SourceRadius = FMath::Max(PointLight->SourceRadius, USD_LIGHT_MIN_RADIUS);
        auto NewRadius = GetRadiusForNoShadowPointLight(*PointLight);
        if (NewRadius.IsSet())
        {
            SourceRadius = *NewRadius;
        }
        float Area = 4.0f * PI * SourceRadius * SourceRadius * UNIT_PER_SQUARE_METRE; // 1m^2

        if (auto SpotLight = Cast<USpotLightComponent>(&LightComponent))
        {
            LightType = ESimReadyLightType::Spot;
            ConeAngle = SpotLight->OuterConeAngle;
        }
        Intensity = ConvertIntensityToNit(LightType, LightUnits, Intensity, ConeAngle, Area);
    }
    else if (auto RectLightComponent = Cast<URectLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Rect;
        float Width = FMath::Max(USD_LIGHT_MIN_RADIUS, RectLightComponent->SourceWidth);
        float Height = FMath::Max(USD_LIGHT_MIN_RADIUS, RectLightComponent->SourceHeight);
        float Area = Width * Height * UNIT_PER_SQUARE_METRE; // m^2
        Intensity = ConvertIntensityToNit(LightType, LightUnits, Intensity, ConeAngle, Area);
    }
    else if (auto SkyLightComponent = Cast<USkyLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Sky;
        Intensity = ConvertIntensityToNit(LightType, LightUnits, Intensity, ConeAngle, 1.0f);
    }

    Intensity *= GetLightIntensityScale();

    return Intensity;
}

bool ConvertUSDLight(const ULightComponentBase& LightComponent, const pxr::UsdPrim& Light)
{
    bool bPreferNewSchema = GetDefault<USimReadySettings>()->bPreferNewUsdLuxLightSchemaOnImport;

    const FNamedParameterTimeSamples* ParameterTimeSamples = nullptr;
    ParameterTimeSamples = FSequenceExtractor::Get().GetParameterTimeSamples(&LightComponent);
    auto Stage = Light.GetStage();
    ESimReadyLightType LightType = ESimReadyLightType::Unknown;
    if (auto LocalLight = Cast<ULocalLightComponent>(&LightComponent))
    {
        float AttenuationRadius = USDConvertLength(Stage, LocalLight->AttenuationRadius);
        Light.SetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_ATTENUATE_DISTANCE),
            pxr::VtValue(AttenuationRadius));
    }

    Light.SetCustomDataByKey(pxr::TfToken(CUSTOM_KEY_UE4_LIGHT_CAST_SHADOWS), pxr::VtValue(LightComponent.CastShadows));

    if (auto DirectionalLightComponent = Cast<UDirectionalLightComponent>(&LightComponent))
    {
        auto USDDistanceLight = pxr::UsdLuxDistantLight(Light);
        LightType = ESimReadyLightType::Directional;
        if (USDDistanceLight)
        {
            SimReadyUsdLuxLightCompat::CreateAngleAttr(Light, pxr::VtValue(DirectionalLightComponent->LightSourceAngle));
        }
    }
    else if (auto PointLight = Cast<UPointLightComponent>(&LightComponent))
    {
        auto NewRadius = GetRadiusForNoShadowPointLight(*PointLight);
        if (NewRadius.IsSet())
        {
            UE_LOG(LogSimReadyUsd, Log, TEXT("Use larger radius for light %s"), *PointLight->GetFullName());
        }

        LightType = ESimReadyLightType::Point;
        float SourceRadius = USDConvertLength(Stage, FMath::Max(PointLight->SourceRadius, NewRadius.IsSet() ? *NewRadius : USD_LIGHT_MIN_RADIUS));
        auto USDSphereLight = pxr::UsdLuxSphereLight(Light);
        if (USDSphereLight)
        {
            SimReadyUsdLuxLightCompat::CreateRadiusAttr(Light, pxr::VtValue(SourceRadius));
        }

        if (auto SpotLight = Cast<USpotLightComponent>(&LightComponent))
        {
            LightType = ESimReadyLightType::Spot;
            pxr::UsdLuxShapingAPI Cone = pxr::UsdLuxShapingAPI::Apply(USDSphereLight.GetPrim());
            if (Cone)
            {
                SimReadyUsdLuxLightCompat::CreateShapingConeAngleAttr(Light, pxr::VtValue(SpotLight->OuterConeAngle));
                const float ClampedInnerConeAngle = FMath::Clamp(SpotLight->InnerConeAngle, 0.0f, 89.0f) * (float)PI / 180.0f;
                const float ClampedOuterConeAngle = FMath::Clamp(SpotLight->OuterConeAngle * (float)PI / 180.0f, ClampedInnerConeAngle + 0.001f, 89.0f * (float)PI / 180.0f + 0.001f);
                float CosOuterCone = FMath::Cos(ClampedOuterConeAngle);
                float CosInnerCone = FMath::Cos(ClampedInnerConeAngle);
                float CosConeDifference = CosInnerCone - CosOuterCone;
                SimReadyUsdLuxLightCompat::CreateShapingConeSoftnessAttr(Light, pxr::VtValue(CosConeDifference));
            }
        }

        // No UsdLuxLight compatibility conversion required
        USDSphereLight.CreateTreatAsPointAttr().Set(pxr::VtValue(NewRadius.IsSet() ? false : PointLight->SourceRadius <= 0));
    }
    else if (auto RectLightComponent = Cast<URectLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Rect;
        auto USDRectLight = pxr::UsdLuxRectLight(Light);
        float Width = USDConvertLength(Stage, FMath::Max(USD_LIGHT_MIN_RADIUS, RectLightComponent->SourceWidth));
        float Height = USDConvertLength(Stage, FMath::Max(USD_LIGHT_MIN_RADIUS, RectLightComponent->SourceHeight));

        if (USDRectLight)
        {
            SimReadyUsdLuxLightCompat::CreateWidthAttr(Light, pxr::VtValue(Width));
            SimReadyUsdLuxLightCompat::CreateHeightAttr(Light, pxr::VtValue(Height));
        }
    }
    else if (auto SkyLightComponent = Cast<USkyLightComponent>(&LightComponent))
    {
        LightType = ESimReadyLightType::Sky;
        auto USDDomeLight = pxr::UsdLuxDomeLight(Light);
        if (USDDomeLight)
        {
            if (SkyLightComponent->SourceType == SLS_SpecifiedCubemap && SkyLightComponent->Cubemap)
            {
                FString TexFileRelativePath = SkyLightComponent->Cubemap->GetName() + TEXT(".dds");
                FString TexFileFullPath = FPaths::GetPath(UExporter::CurrentFilename) / TexFileRelativePath;
                if (!FSimReadyAssetExportHelper::IsTextureExported(SkyLightComponent->Cubemap))
                {
                    FSimReadyAssetExportHelper::ExportTextureToPath(SkyLightComponent->Cubemap, TexFileFullPath);
                    FSimReadyAssetExportHelper::RegisterExportedTexture(SkyLightComponent->Cubemap);
                }
                SimReadyUsdLuxLightCompat::CreateTextureFileAttr(Light, pxr::VtValue(pxr::SdfAssetPath(TCHAR_TO_UTF8(*TexFileRelativePath))));
            }
        }
    }

    if (LightType == ESimReadyLightType::Unknown)
    {
        return false;
    }

    if (LightComponent.IsA<ULightComponent>())
    {
        SimReadyUsdLuxLightCompat::CreateEnableColorTemperatureAttr(Light, pxr::VtValue(Cast<ULightComponent>(&LightComponent)->bUseTemperature == 1));
        SimReadyUsdLuxLightCompat::CreateColorTemperatureAttr(Light, pxr::VtValue(Cast<ULightComponent>(&LightComponent)->Temperature));
    }

#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111

    //NOTE: There's no exposure for unreal light, we can just keep the existed exposure and change intensity
    pxr::UsdAttribute LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(Light, LightAPI.GetExposureAttr(), bPreferNewSchema);
    float Exposure = GetUSDValue<float>(LightAttr);
    auto Name = GET_MEMBER_NAME_CHECKED(ULightComponentBase, Intensity);
    if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Scalar))
    {
        auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
        {
            pxr::VtValue USDLightIntensity(ConvertUSDLightIntensity(LightComponent, TimeSample / FMath::Pow(2.0f, Exposure)));
            SimReadyUsdLuxLightCompat::CreateIntensityAttr(Light, USDLightIntensity, pxr::UsdTimeCode(TimeCode));
        };
        ParameterTimeSamples->TraverseScalarTimeSamples(Name, TraverseCallback);
    }
    else
    {
        pxr::VtValue USDLightIntensity(ConvertUSDLightIntensity(LightComponent, LightComponent.Intensity / FMath::Pow(2.0f, Exposure)));
        SimReadyUsdLuxLightCompat::CreateIntensityAttr(Light, USDLightIntensity);
    }

    Name = GET_MEMBER_NAME_CHECKED(ULightComponentBase, LightColor);
    if (ParameterTimeSamples && ParameterTimeSamples->HasNamedParameter(Name, ENamedParameterType::Color))
    {
        auto TraverseCallback = [&](int32 TimeCode, const FLinearColor& TimeSample)
        {
            pxr::VtValue LightColor(USDConvert(TimeSample));
            SimReadyUsdLuxLightCompat::CreateColorAttr(Light, LightColor, TimeCode);
        };
        ParameterTimeSamples->TraverseColorTimeSamples(Name, TraverseCallback);
    }
    else
    {
        pxr::VtValue LightColor(USDConvert(LightComponent.GetLightColor()));
        SimReadyUsdLuxLightCompat::CreateColorAttr(Light, LightColor);
    }

    SimReadyUsdLuxLightCompat::CreateExtentAttr(Light);

    return true;
}

pxr::SdfLayerHandle GetLayerHandle(const pxr::UsdPrim& Prim)
{
    for (auto PrimSpec : Prim.GetPrimStack())
    {
        if (PrimSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            return PrimSpec->GetLayer();
        }
    }

    return pxr::SdfLayerHandle();
}

pxr::SdfLayerHandle GetLayerHandle(const pxr::UsdAttribute& Attribute)
{
    for (auto PropertySpec : Attribute.GetPropertyStack(pxr::UsdTimeCode::EarliestTime()))
    {
        if (PropertySpec->HasDefaultValue() 
        || PropertySpec->GetLayer()->GetNumTimeSamplesForPath(PropertySpec->GetPath()) > 0)
        {
            return PropertySpec->GetLayer();
        }
    }

    return pxr::SdfLayerHandle();
}

pxr::GfVec3f USDConvert(const FLinearColor& Color)
{
    return pxr::GfVec3f(Color.R, Color.G, Color.B);
}

pxr::GfVec3h USDConvertH(const FLinearColor& Color)
{
    return pxr::GfVec3h(USDConvert(Color));
}

pxr::GfVec3d USDConvertD(const FLinearColor& Color)
{
    return pxr::GfVec3d(Color.R, Color.G, Color.B);
}

pxr::GfQuatf USDConvert(const FQuat& Quat)
{
    return pxr::GfQuatf(Quat.W, Quat.X, Quat.Y, Quat.Z);
}

FQuat USDConvert(const pxr::GfQuath& Quat)
{
    // MUST sperate to X Y Z to set Quat, using Vector will be treated as axis
    FVector Imaginary = USDConvert(Quat.GetImaginary());
    return FQuat(Imaginary.X, Imaginary.Y, Imaginary.Z, Quat.GetReal());
}

FQuat USDConvert(const pxr::GfQuatf& Quat)
{
    // MUST sperate to X Y Z to set Quat, using Vector will be treated as axis
    FVector Imaginary = USDConvert(Quat.GetImaginary());
    return FQuat(Imaginary.X, Imaginary.Y, Imaginary.Z, Quat.GetReal());
}

FQuat USDConvert(const pxr::GfQuatd& Quat)
{
    // MUST sperate to X Y Z to set Quat, using Vector will be treated as axis
    FVector Imaginary = USDConvert(Quat.GetImaginary());
    return FQuat(Imaginary.X, Imaginary.Y, Imaginary.Z, Quat.GetReal());
}

float USDConvert(const pxr::GfHalf& Value)
{
    return float(Value);
}

float USDConvert(const float Value)
{
    return Value;
}

float USDConvert(const double Value)
{
    return (float)Value;
}

void ExportStaticMeshLOD(const FStaticMeshLODResources& StaticMeshLOD, FMeshDescription& OutRawMesh, const TArray<FStaticMaterial>& Materials)
{
    const int32 NumWedges = StaticMeshLOD.IndexBuffer.GetNumIndices();
    const int32 NumVertexPositions = StaticMeshLOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
    const int32 NumFaces = NumWedges / 3;

    OutRawMesh.Empty();

    if (NumVertexPositions <= 0 || StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() <= 0)
    {
        return;
    }

    TVertexAttributesRef<FVector> VertexPositions = OutRawMesh.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);
    TEdgeAttributesRef<bool> EdgeHardnesses = OutRawMesh.EdgeAttributes().GetAttributesRef<bool>(MeshAttribute::Edge::IsHard);
    TEdgeAttributesRef<float> EdgeCreaseSharpnesses = OutRawMesh.EdgeAttributes().GetAttributesRef<float>(MeshAttribute::Edge::CreaseSharpness);
    TPolygonGroupAttributesRef<FName> PolygonGroupImportedMaterialSlotNames = OutRawMesh.PolygonGroupAttributes().GetAttributesRef<FName>(MeshAttribute::PolygonGroup::ImportedMaterialSlotName);
    TVertexInstanceAttributesRef<FVector> VertexInstanceNormals = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Normal);
    TVertexInstanceAttributesRef<FVector> VertexInstanceTangents = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Tangent);
    TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);
    TVertexInstanceAttributesRef<FVector4> VertexInstanceColors = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector4>(MeshAttribute::VertexInstance::Color);
    TVertexInstanceAttributesRef<FVector2D> VertexInstanceUVs = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);

    OutRawMesh.ReserveNewVertices(NumVertexPositions);
    OutRawMesh.ReserveNewVertexInstances(NumWedges);
    OutRawMesh.ReserveNewPolygons(NumFaces);
    OutRawMesh.ReserveNewEdges(NumWedges);

    const int32 NumTexCoords = StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    VertexInstanceUVs.SetNumIndices(NumTexCoords);


    for (int32 SectionIndex = 0; SectionIndex < StaticMeshLOD.Sections.Num(); ++SectionIndex)
    {
        const FStaticMeshSection& Section = StaticMeshLOD.Sections[SectionIndex];
        FPolygonGroupID CurrentPolygonGroupID = OutRawMesh.CreatePolygonGroup();
        check(CurrentPolygonGroupID.GetValue() == SectionIndex);
        if (Materials.IsValidIndex(Section.MaterialIndex))
        {
            PolygonGroupImportedMaterialSlotNames[CurrentPolygonGroupID] = Materials[Section.MaterialIndex].ImportedMaterialSlotName;
        }
        else
        {
            PolygonGroupImportedMaterialSlotNames[CurrentPolygonGroupID] = FName(*(TEXT("MeshMergeMaterial_") + FString::FromInt(SectionIndex)));
        }
    }

    //Create the vertex
    for (int32 VertexIndex = 0; VertexIndex < NumVertexPositions; ++VertexIndex)
    {
        FVertexID VertexID = OutRawMesh.CreateVertex();
        VertexPositions[VertexID] = StaticMeshLOD.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
    }

    //Create the vertex instances
    for (int32 TriangleIndex = 0; TriangleIndex < NumFaces; ++TriangleIndex)
    {
        FPolygonGroupID CurrentPolygonGroupID = FPolygonGroupID::Invalid;
        for (int32 SectionIndex = 0; SectionIndex < StaticMeshLOD.Sections.Num(); ++SectionIndex)
        {
            const FStaticMeshSection& Section = StaticMeshLOD.Sections[SectionIndex];
            uint32 FirstTriangle = Section.FirstIndex / 3;
            uint32 LastTriangle = FirstTriangle + Section.NumTriangles - 1;
            if ((uint32)TriangleIndex >= FirstTriangle && (uint32)TriangleIndex <= LastTriangle)
            {
                CurrentPolygonGroupID = FPolygonGroupID(SectionIndex);
                break;
            }
        }
        check(CurrentPolygonGroupID != FPolygonGroupID::Invalid);

        FVertexID VertexIDs[3];
        TArray<FVertexInstanceID> VertexInstanceIDs;
        VertexInstanceIDs.SetNum(3);

        for (int32 Corner = 0; Corner < 3; ++Corner)
        {
            int32 WedgeIndex = StaticMeshLOD.IndexBuffer.GetIndex(TriangleIndex * 3 + Corner);
            FVertexID VertexID(WedgeIndex);
            FVertexInstanceID VertexInstanceID = OutRawMesh.CreateVertexInstance(VertexID);
            VertexIDs[Corner] = VertexID;
            VertexInstanceIDs[Corner] = VertexInstanceID;

            //NTBs
            FVector TangentX = StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(WedgeIndex);
            FVector TangentY = StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentY(WedgeIndex);
            FVector TangentZ = StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(WedgeIndex);
            VertexInstanceTangents[VertexInstanceID] = TangentX;
            VertexInstanceBinormalSigns[VertexInstanceID] = GetBasisDeterminantSign(TangentX, TangentY, TangentZ);
            VertexInstanceNormals[VertexInstanceID] = TangentZ;

            // Vertex colors
            if (StaticMeshLOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() > 0)
            {
                FColor VertexColor = StaticMeshLOD.VertexBuffers.ColorVertexBuffer.VertexColor(WedgeIndex);
                VertexInstanceColors[VertexInstanceID] = FLinearColor(VertexColor);
            }
            else
            {
                VertexInstanceColors[VertexInstanceID] = FLinearColor::White;
            }

            //Tex coord
            for (int32 TexCoodIdx = 0; TexCoodIdx < NumTexCoords; ++TexCoodIdx)
            {
                VertexInstanceUVs.Set(VertexInstanceID, TexCoodIdx, StaticMeshLOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(WedgeIndex, TexCoodIdx));
            }
        }
        //Create a polygon from this triangle
        const FPolygonID NewPolygonID = OutRawMesh.CreatePolygon(CurrentPolygonGroupID, VertexInstanceIDs);
    }
}

FLinearColor USDConvertToLinearColor(const pxr::GfVec2f & Color)
{
    return MoveTempIfPossible(FLinearColor(Color[0], Color[1], 1.0f));
}

FLinearColor USDConvertToLinearColor(const pxr::GfVec3f & Color)
{
    return MoveTempIfPossible(FLinearColor(Color[0], Color[1], Color[2]));
}

FLinearColor USDConvertToLinearColor(const pxr::GfVec4f & Color)
{
    return MoveTempIfPossible(FLinearColor(Color[0], Color[1], Color[2], Color[3]));
}

pxr::GfMatrix4d USDConvert(const FMatrix & Matrix)
{
    return MoveTempIfPossible(pxr::GfMatrix4d(
        Matrix.M[0][0], Matrix.M[0][1], Matrix.M[0][2], Matrix.M[0][3],
        Matrix.M[1][0], Matrix.M[1][1], Matrix.M[1][2], Matrix.M[1][3],
        Matrix.M[2][0], Matrix.M[2][1], Matrix.M[2][2], Matrix.M[2][3],
        Matrix.M[3][0], Matrix.M[3][1], Matrix.M[3][2], Matrix.M[3][3]
        ));
}


FMatrix USDConvert(const pxr::GfMatrix4d & InMatrix)
{
    FMatrix Matrix;
    std::copy_n(pxr::GfMatrix4f(InMatrix).GetArray(), 16, reinterpret_cast<float*>(Matrix.M));
    return MoveTemp(Matrix);
}

FVector USDConvert(const pxr::GfVec3h & InValue)
{
    return FVector(InValue[0], InValue[1], InValue[2]);
}

FVector USDConvert(const pxr::GfVec3f & InValue)
{
    return FVector(InValue[0], InValue[1], InValue[2]);
}

FVector USDConvert(const pxr::GfVec3d & InValue)
{
    return FVector(InValue[0], InValue[1], InValue[2]);
}

FVector2D USDConvert(const pxr::GfVec2f & InValue)
{
    return FVector2D(InValue[0], InValue[1]);
}

pxr::GfVec2f USDConvert(const FVector2D & InValue)
{
    return pxr::GfVec2f(InValue.X, InValue.Y);
}

FTransform USDConvert(const pxr::UsdStageRefPtr & Stage, const pxr::GfMatrix4d & InMatrix)
{
    pxr::GfVec3f USDTranslate;
    pxr::GfQuatf USDRotate;
    pxr::GfVec3h USDScale;

    FTransform Transform;
    if (pxr::UsdSkelDecomposeTransform(InMatrix, &USDTranslate, &USDRotate, &USDScale))
    {
        Transform = FTransform(USDConvert(USDRotate), USDConvert(USDTranslate), USDConvert(USDScale));
    }
    else
    {
        FMatrix UEMatrix = USDConvert(InMatrix);
        Transform = FTransform(UEMatrix.Rotator(), UEMatrix.GetOrigin(), UEMatrix.GetScaleVector());
    }

    Transform = RHSTransformConvert(Stage, Transform, true);
    return MoveTemp(Transform);
}

pxr::GfMatrix4d USDConvert(const pxr::UsdStageRefPtr & Stage, FTransform Transform)
{
    Transform = RHSTransformConvert(Stage, Transform, false);
    return USDConvert(Transform.ToMatrixWithScale());
}

pxr::VtMatrix4dArray USDConvert(const pxr::UsdStageRefPtr& Stage, const TArray<FTransform>& InValue)
{
    pxr::VtMatrix4dArray MatrixArray;
    for (int i = 0; i < InValue.Num(); ++i)
    {
        MatrixArray.push_back(USDConvert(Stage, InValue[i]));
    }

    return MatrixArray;
}

FTransform RHSTransformConvert(const pxr::UsdStageRefPtr & Stage, FTransform Transform, bool UsdToUE)
{
    bool bZUp = GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z;
    // Translate
    auto Translate = Transform.GetTranslation();

    if(bZUp)
    {
        Translate.Y = -Translate.Y;
    }
    else
    {
        Swap(Translate.Y, Translate.Z);
    }

    Transform.SetTranslation(Translate * (UsdToUE ? UnitScaleFromUSDToUE(Stage) : UnitScaleFromUEToUSD(Stage)));

    // Rotation
    auto Rotation = Transform.GetRotation();

    if(bZUp)
    {
        Rotation.X = -Rotation.X;
        Rotation.Z = -Rotation.Z;
    }
    else
    {
        Rotation = Rotation.Inverse();
        Swap(Rotation.Y, Rotation.Z);
    }

    Rotation.Normalize();
    Transform.SetRotation(Rotation);

    // Scale
    if(!bZUp)
    {
        auto Scale = Transform.GetScale3D();
        Swap(Scale.Y, Scale.Z);
        Transform.SetScale3D(Scale);
    }

    return MoveTemp(Transform);
}

FVector RHSTranslationConvert(const pxr::UsdStageRefPtr & Stage, FVector Translation, bool UsdToUE)
{
    bool bZUp = GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z;
    // Translate
    if (bZUp)
    {
        Translation.Y = -Translation.Y;
    }
    else
    {
        Swap(Translation.Y, Translation.Z);
    }

    Translation *= (UsdToUE ? UnitScaleFromUSDToUE(Stage) : UnitScaleFromUEToUSD(Stage));

    return MoveTemp(Translation);
}

FQuat RHSRotationConvert(const pxr::UsdStageRefPtr & Stage, FQuat Rotation)
{
    bool bZUp = GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z;

    // Rotation
    if (bZUp)
    {
        Rotation.X = -Rotation.X;
        Rotation.Z = -Rotation.Z;
    }
    else
    {
        Rotation = Rotation.Inverse();
        Swap(Rotation.Y, Rotation.Z);
    }

    return MoveTemp(Rotation);
}

FVector RHSScaleConvert(const pxr::UsdStageRefPtr & Stage, FVector Scale)
{
    bool bZUp = GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z;

    // Scale
    if (!bZUp)
    {
        Swap(Scale.Y, Scale.Z);
    }

    return MoveTemp(Scale);
}

void GetPrimTranslationAndPivot(const pxr::UsdPrim& Prim, const pxr::UsdTimeCode& TimeCode, FVector& Translation, FVector& Pivot)
{
    auto XformAPI = pxr::UsdGeomXformCommonAPI(Prim);
    pxr::GfVec3d UsdTranslation;
    pxr::GfVec3f UsdRotation;
    pxr::GfVec3f UsdScale;
    pxr::GfVec3f UsdPivot;
    pxr::UsdGeomXformCommonAPI::RotationOrder UsdRotOrder;
    if (XformAPI.GetXformVectors(&UsdTranslation, &UsdRotation, &UsdScale, &UsdPivot, &UsdRotOrder, TimeCode))
    {
        Translation = USDConvertPosition(Prim.GetStage(), USDConvert(UsdTranslation), true);
        Pivot = USDConvertPosition(Prim.GetStage(), USDConvert(UsdPivot), true);
    }
    else
    {
        Translation = FVector::ZeroVector;
        Pivot = FVector::ZeroVector;
    }
}

bool IsSphericalOrCylindricalProjectionUsed(UMeshComponent* MeshComponent)
{
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
    {
        UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(MaterialIndex);
        if (MaterialInterface == nullptr)
        {
            continue;
        }

        auto Material = MaterialInterface->GetMaterial();
        if (Material == nullptr)
        {
            continue;
        }

        for (int32 ExpressionIndex = 0; ExpressionIndex < Material->Expressions.Num(); ++ExpressionIndex)
        {
            auto Expression = Material->Expressions[ExpressionIndex];
            if (Expression->IsA<UMaterialExpressionMaterialFunctionCall>())
            {
                auto FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression);
                if (FunctionCall->MaterialFunction && 
                    FunctionCall->MaterialFunction->GetName() == TEXT("mdl_base_coordinate_projection"))
                {
                    check(FunctionCall->FunctionInputs.Num() >= 3)
                    FExpressionInput& Input = FunctionCall->FunctionInputs[2].Input;

                    // check projection type. Only spherical and cylindrical need z-up axis correction.
                    if (Input.InputName == TEXT("projection_type"))
                    {
                        UMaterialExpressionConstant* ProjectionType = Cast<UMaterialExpressionConstant>(Input.Expression);
                        check(ProjectionType);
                        /*
                        projection_cubic = 1
                            [[ anno::description("Projected space has a cube-shaped appearance") ]],
                        projection_spherical = 2
                            [[ anno::description("Projected space forms a sphere around the projector") ]],
                        projection_cylindrical = 3
                            [[ anno::description("Projected space forms a capped cylinder") ]],
                        projection_infinite_cylindrical = 4
                            [[ anno::description("Projected space forms an infinite cylinder") ]],
                        projection_planar = 5
                            [[ anno::description("Planar projection along the z axis of the projectors space") ]],
                        projection_spherical_normalized = 6
                            [[ anno::description("Like projection_spherical, but u is normalized between -1 and 1 and v between -0.5 and 0.5") ]],
                        projection_cylindrical_normalized = 7
                            [[ anno::description("Like projection_cylindrical, but u is normalized between -1 and 1 (if not on the cap)") ]],
                        projection_infinite_cylindrical_normalized = 8
                            [[ anno::description("Like projection_cylindrical_infinite, but u is normalized between -1 and 1") ]],
                        projection_tri_planar = 9
                            [[ anno::description("Like projection_cubic, but blends the texture smoothly on the edges") ]]
                        */
                        if (ProjectionType->R == 2.0f || ProjectionType->R == 3.0f || ProjectionType->R == 4.0f
                        || ProjectionType->R == 6.0f || ProjectionType->R == 7.0f || ProjectionType->R == 8.0f)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

pxr::UsdShadeMaterial FindShadeMaterial(const pxr::UsdPrim& InPrim)
{
    auto Prim = InPrim.GetParent();
    while (!Prim.IsPseudoRoot())
    {
        auto USDMaterial = pxr::UsdShadeMaterial(Prim);
        if (USDMaterial)
        {
            return USDMaterial;
        }

        Prim = Prim.GetParent();
    }

    return pxr::UsdShadeMaterial();
}

void FUSDExporter::ExportSkeletalMesh(const pxr::UsdStageRefPtr& USDStage, const FSkeletalMeshLODRenderData& LODModel, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX)
{
    Attributes.AddZeroed(LODModel.RenderSections.Num());

    const int32 NumTexCoord = LODModel.GetNumTexCoords();
    bool bHasColor = LODModel.StaticVertexBuffers.ColorVertexBuffer.GetNumVertices() > 0;
    bool bAllColorsAreEqual = AllColorsAreEqual(&LODModel.StaticVertexBuffers.ColorVertexBuffer);

    // Handle subsets
    for (int32 SectionIndex = 0; SectionIndex < LODModel.RenderSections.Num(); ++SectionIndex)
    {
        const FSkelMeshRenderSection& Section = LODModel.RenderSections[SectionIndex];

        const int32 NumFaces = Section.NumTriangles;
        if (NumFaces == 0)
        {
            continue;
        }
        const int32 NumWedges = NumFaces * 3;
        Attributes[SectionIndex].FaceVertexCounts.reserve(NumFaces);
        for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
        {
            Attributes[SectionIndex].FaceVertexCounts.push_back(3);
        }

        Attributes[SectionIndex].FaceVertexIndices.reserve(NumWedges);
        for (int32 Index = 0; Index < NumWedges; ++Index)
        {
            uint32 VertexIndex = LODModel.MultiSizeIndexContainer.GetIndexBuffer()->Get(Section.BaseIndex + Index);
            Attributes[SectionIndex].FaceVertexIndices.push_back(VertexIndex - Section.BaseVertexIndex);
        }

        int32 NumVertices = Section.NumVertices;
        Attributes[SectionIndex].Points.resize(NumVertices);
        Attributes[SectionIndex].Normals.resize(NumVertices);
        Attributes[SectionIndex].UVs.resize(NumTexCoord);
        if (bExportTangentX)
        {
            Attributes[SectionIndex].TangentX.resize(NumVertices);
        }

        for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
        {
            Attributes[SectionIndex].UVs[TexCoordIndex].resize(NumVertices);
        }

        if (bHasColor)
        {
            Attributes[SectionIndex].Colors.resize(bAllColorsAreEqual ? 1 : NumVertices);
            Attributes[SectionIndex].Opacities.resize(bAllColorsAreEqual ? 1 : NumVertices);
        }

        for (uint32 VertexIndex = Section.BaseVertexIndex; VertexIndex < (Section.BaseVertexIndex + Section.NumVertices); ++VertexIndex)
        {
            FVector Point = LODModel.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
            FVector Tangent = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
            FVector Normal = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);

            int32 SectionVertexIndex = VertexIndex - Section.BaseVertexIndex;
            Attributes[SectionIndex].Points[SectionVertexIndex] = USDConvert(USDConvertPosition(USDStage, Point));
            Attributes[SectionIndex].Normals[SectionVertexIndex] = USDConvert(USDConvertVector(USDStage, Normal));
            if (bExportTangentX)
            {
                Attributes[SectionIndex].TangentX[SectionVertexIndex] = USDConvert(USDConvertVector(USDStage, Tangent));
            }

            if (bHasColor)
            {
                FLinearColor LinearColor = LODModel.StaticVertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex).ReinterpretAsLinear();
                Attributes[SectionIndex].Colors[bAllColorsAreEqual ? 0 : SectionVertexIndex] = USDConvert(LinearColor);
                Attributes[SectionIndex].Opacities[bAllColorsAreEqual ? 0 : SectionVertexIndex] = LinearColor.A;
            }

            for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
            {
                FVector2D TexCoord = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, TexCoordIndex);
                TexCoord[1] = 1.f - TexCoord[1];
                Attributes[SectionIndex].UVs[TexCoordIndex][SectionVertexIndex] = USDConvert(TexCoord);
            }
        }
    }
}

void FUSDExporter::ExportSkeletalMesh(const pxr::UsdStageRefPtr& USDStage, const FSkeletalMeshLODRenderData& LODModel, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX, const bool bExportGeomSubset)
{
    // Create Mesh primitive
    // Right hand to left hand conversion
    // Add points
    const int32 NumVertices = LODModel.GetNumVertices();
    const int32 NumFaces = LODModel.GetTotalFaces();
    const int32 NumWedges = NumFaces * 3;
    const int32 NumTexCoord = LODModel.GetNumTexCoords();

    Attributes.Points.resize(NumVertices);
    Attributes.Normals.resize(NumVertices);
    Attributes.UVs.resize(NumTexCoord);
    if (bExportTangentX)
    {
        Attributes.TangentX.resize(NumVertices);
    }

    for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
    {
        Attributes.UVs[TexCoordIndex].resize(NumVertices);
    }

    bool bHasColor = LODModel.StaticVertexBuffers.ColorVertexBuffer.GetNumVertices() > 0;
    bool bAllColorsAreEqual = AllColorsAreEqual(&LODModel.StaticVertexBuffers.ColorVertexBuffer);
    if (bHasColor)
    {
        Attributes.Colors.resize(bAllColorsAreEqual ? 1 : NumVertices);
        Attributes.Opacities.resize(bAllColorsAreEqual ? 1 : NumVertices);
    }

    for (int32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
    {
        FVector Point = LODModel.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
        FVector Tangent = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
        FVector Normal = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);

        Attributes.Points[VertexIndex] = USDConvert(USDConvertPosition(USDStage, Point));
        Attributes.Normals[VertexIndex] = USDConvert(USDConvertVector(USDStage, Normal));
        if (bExportTangentX)
        {
            Attributes.TangentX[VertexIndex] = USDConvert(USDConvertVector(USDStage, Tangent));
        }

        if (bHasColor)
        {
            FLinearColor LinearColor = LODModel.StaticVertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex).ReinterpretAsLinear();
            Attributes.Colors[bAllColorsAreEqual ? 0 : VertexIndex] = USDConvert(LinearColor);
            Attributes.Opacities[bAllColorsAreEqual ? 0 : VertexIndex] = LinearColor.A;
        }

        for (int32 TexCoordIndex = 0; TexCoordIndex < NumTexCoord; ++TexCoordIndex)
        {
            FVector2D TexCoord = LODModel.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, TexCoordIndex);
            TexCoord[1] = 1.f - TexCoord[1];
            Attributes.UVs[TexCoordIndex][VertexIndex] = USDConvert(TexCoord);
        }
    }

    // Add faces
    Attributes.FaceVertexCounts.reserve(NumFaces);
    for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
    {
        Attributes.FaceVertexCounts.push_back(3);
    }
    Attributes.FaceVertexIndices.reserve(NumWedges);

    for (int32 Index = 0; Index < NumWedges; ++Index)
    {
        uint32 VertexIndex = LODModel.MultiSizeIndexContainer.GetIndexBuffer()->Get(Index);
        Attributes.FaceVertexIndices.push_back(VertexIndex);
    }

    if (bExportGeomSubset)
    {
        // Handle subsets
        for (int32 SectionIndex = 0; SectionIndex < LODModel.RenderSections.Num(); ++SectionIndex)
        {
            const FSkelMeshRenderSection& Section = LODModel.RenderSections[SectionIndex];
            pxr::VtArray<int> Subset;
            Subset.reserve(Section.NumTriangles);
            for (uint32 Index = 0; Index < Section.NumTriangles; ++Index)
            {
                Subset.push_back(Section.BaseIndex / 3 + Index);
            }
            Attributes.FaceIndices.push_back(Subset);
        }
    }
}
pxr::UsdGeomXformable FUSDExporter::ExportSkeletalMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FSkeletalMeshLODRenderData& LODModel, const bool bExportTangentX, const bool bExportGeomSubset)
{
    if (bExportGeomSubset|| LODModel.RenderSections.Num() == 1)
    {
        // Create Mesh primitive
        auto USDMesh = pxr::UsdGeomMesh::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }
        FUSDGeomMeshAttributes Attributes;
        ExportSkeletalMesh(Stage, LODModel, Attributes, bExportTangentX, bExportGeomSubset);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->vertex, Attributes);
        SetJointInfluencesAndWeights(LODModel, USDMesh);
        return USDMesh;
    }
    else
    {
        auto USDMesh = pxr::UsdGeomXform::Define(Stage, Path);
        if (!USDMesh)
        {
            return USDMesh;
        }

        TArray<FUSDGeomMeshAttributes> Attributes;
        ExportSkeletalMesh(Stage, LODModel, Attributes, bExportTangentX);
        FillUSDMesh(Stage, USDMesh, pxr::UsdGeomTokens->vertex, Attributes);
        SetJointInfluencesAndWeights(LODModel, USDMesh);
        return USDMesh;
    }
}

TArray<FTransform> LocalToWorldSpace(const FReferenceSkeleton& RefSkeleton)
{
    TArray<FTransform> LocalTransforms = RefSkeleton.GetRefBonePose(); // relative to parent bone
    TArray<FTransform> GlobalTransforms = LocalTransforms;
    for (int i = 0; i < LocalTransforms.Num(); ++i)
    {
        int Parent = RefSkeleton.GetParentIndex(i);
        if (Parent >= 0)
        {
            // parent should have already been processed
            GlobalTransforms[i] *= GlobalTransforms[Parent];
        }
    }

    return GlobalTransforms;
}

TArray<int> GetChildBones(const FReferenceSkeleton& Skeleton, int Parent)
{
    TArray<int> ChildBones;
    FName ParentBone = Skeleton.GetBoneName(Parent);
    for (int i = 0; i < Skeleton.GetNum(); ++i)
    {
        FName ThisBoneName = Skeleton.GetBoneName(i);
        int32 ThisBoneIndex = Skeleton.FindBoneIndex(ThisBoneName);
        FName ThisParentBone = NAME_None;
        if ((ThisBoneIndex != INDEX_NONE) && (ThisBoneIndex > 0))
        {
            ThisParentBone = Skeleton.GetBoneName(Skeleton.GetParentIndex(ThisBoneIndex));
        }

        if (ThisParentBone == ParentBone)
        {
            ChildBones.Add(i);
        }
    }

    return ChildBones;
}

pxr::SdfPathVector BuildJointTree(const pxr::SdfPath& RootPath, const pxr::SdfPath& Path, pxr::SdfPathVector& PathVector, const FReferenceSkeleton& Skeleton, int BoneIndex, int USDIndex)
{
    pxr::SdfPath NewPath = Path.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*Skeleton.GetBoneName(BoneIndex).ToString()))).MakeRelativePath(RootPath);
    PathVector.push_back(NewPath);
    USDIndex++;

    TArray<int> ChildBones = GetChildBones(Skeleton, BoneIndex);
    for (int j = 0; j < ChildBones.Num(); ++j)
    {
        BuildJointTree(RootPath, NewPath, PathVector, Skeleton, ChildBones[j], USDIndex);
    }

    return PathVector;
}

//TArray<FTransform> ConvertToUSDJointOrder(const TArray<FTransform> UE4Joints, const TMap<int, int> UEToUSDBoneIndexMap)
//{
//	TArray<FTransform> USDJoints = UE4Joints;
//	for (int i = 0; i < UE4Joints.Num(); ++i)
//	{
//		int USDJointIndex = UEToUSDBoneIndexMap[i];
//		USDJoints[USDJointIndex] = UE4Joints[i];
//	}
//	return USDJoints;
//}

pxr::UsdGeomXformable FUSDExporter::ExportSkeletalMeshComponent(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMeshComponent& SkeletalMeshComponent, const bool bExportTangentX, pxr::UsdTimeCode TimeCode)
{
    if (!(&SkeletalMeshComponent) || !(SkeletalMeshComponent.SkeletalMesh))
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Could not find SkeletalMeshComponent or SkeletalMesh"));
        return pxr::UsdGeomMesh();
    }

    pxr::UsdGeomXformable UsdGeomBoundable;

    // if this skeletal mesh has active cloth, export it as a point cache (for now). 
    bool ExportAsPointCache = SkeletalMeshComponent.SkeletalMesh->HasActiveClothingAssets();
    if (ExportAsPointCache)
    {
        UsdGeomBoundable = ExportSkeletalMeshComponentToPointCache(USDStage, NewPrimPath, SkeletalMeshComponent, bExportTangentX, TimeCode);
    }
    else
    {
        UsdGeomBoundable = ExportSkeletalMesh(USDStage, NewPrimPath, *SkeletalMeshComponent.SkeletalMesh, bExportTangentX);

        // animated pose too
        pxr::SdfPath AnimationPath = NewPrimPath.AppendElementString("Anim");
        UAnimSingleNodeInstance* SingleNodeInstance = SkeletalMeshComponent.GetSingleNodeInstance();
        if (SingleNodeInstance)
        {
            TArray<FTransformTimeSamples> SocketTransformTimeSamples;
            ExportSkeletalMeshAnimation(USDStage, AnimationPath, SingleNodeInstance->GetAnimationAsset(), SkeletalMeshComponent.SkeletalMesh, SocketTransformTimeSamples);
            TArray<FName> SocketNames = SkeletalMeshComponent.GetAllSocketNames();
            for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
            {
                FSequenceExtractor::Get().SetSocketTimeSamples(&SkeletalMeshComponent, SocketNames[SocketIndex], SocketTransformTimeSamples[SocketIndex]);
            }
        }
    }

    return UsdGeomBoundable;
}

void SetupBasePose(UDebugSkelMeshComponent* DebugSkelMeshComponent, UAnimSequence* Additive, UAnimSequence* AdditiveBase, float CurrentTime)
{
    DebugSkelMeshComponent->EnablePreview(true, AdditiveBase);

    switch (Additive->RefPoseType)
    {
        // use whole animation as a base pose. Need BasePoseSeq.
        case ABPT_AnimScaled:
        {
            // normalize time to fit base seq
            const float Fraction = (Additive->SequenceLength > 0.f) ? FMath::Clamp<float>(CurrentTime / Additive->SequenceLength, 0.f, 1.f) : 0.f;
            const float BasePoseTime = AdditiveBase->SequenceLength * Fraction;
            DebugSkelMeshComponent->SetPosition(BasePoseTime, false);
            break;
        }
        // use animation as a base pose. Need BasePoseSeq and RefFrameIndex (will clamp if outside).
        case ABPT_AnimFrame:
        {
            const float Fraction = (AdditiveBase->GetRawNumberOfFrames() > 0) ? FMath::Clamp<float>((float)Additive->RefFrameIndex / (float)AdditiveBase->GetRawNumberOfFrames(), 0.f, 1.f) : 0.f;
            const float BasePoseTime = AdditiveBase->SequenceLength * Fraction;
            DebugSkelMeshComponent->SetPosition(BasePoseTime, false);
            break;
        }
    }

    DebugSkelMeshComponent->RefreshBoneTransforms();
}

bool FUSDExporter::ExportSkeletalMeshAnimation(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, UAnimationAsset* AnimationAsset, USkeletalMesh* InSkeletalMesh, TArray<FTransformTimeSamples>& SocketTransformTimeSamples, const bool bRootIdentity)
{
    UAnimSequenceBase* AnimSequenceBase = Cast<UAnimSequenceBase>(AnimationAsset);

    if (AnimSequenceBase)
    {
        USkeletalMesh* SkeletalMesh = InSkeletalMesh ? InSkeletalMesh : (AnimSequenceBase->GetSkeleton() ? AnimSequenceBase->GetSkeleton()->GetPreviewMesh() : nullptr);
        
        // Can't find preview mesh from skeleton, trying to find the compatible mesh
        if (!SkeletalMesh && AnimSequenceBase->GetSkeleton())
        {
            SkeletalMesh = AnimSequenceBase->GetSkeleton()->FindCompatibleMesh();
        }
        
        if (!SkeletalMesh)
        {
            return false;
        }

        if (!SkeletalMesh->Skeleton)
        {
            return false;
        }

        pxr::UsdSkelAnimation Animation = pxr::UsdSkelAnimation::Define(USDStage, NewPrimPath);
        if (!Animation)
        {
            UE_LOG(LogSimReadyUsd, Warning, TEXT("Could not find UsdSkelAnimation"));
            return false;
        }

        auto RootLayerHandle = USDStage->GetRootLayer();
        const double StartTimeCode = 0.0;
        const double EndTimeCode = AnimSequenceBase->GetNumberOfFrames() - 1;
        const double TimeCodesPerSecond = FMath::TruncToFloat(((EndTimeCode) / AnimSequenceBase->SequenceLength) + 0.5f);
        RootLayerHandle->SetStartTimeCode(StartTimeCode);
        RootLayerHandle->SetEndTimeCode(EndTimeCode);
        RootLayerHandle->SetTimeCodesPerSecond(TimeCodesPerSecond);

        // Set joint info
        auto RefSkeleton = SkeletalMesh->RefSkeleton;
        int NumBones = RefSkeleton.GetNum();
        pxr::SdfPathVector JointArray;
        if (NumBones > 0)
        {
            JointArray = BuildJointTree(NewPrimPath, NewPrimPath, JointArray, RefSkeleton, 0, 0);

            pxr::VtTokenArray JointTokenArray(NumBones);
            for (int i = 0; i < JointArray.size(); ++i)
            {
                JointTokenArray[i] = pxr::TfToken(JointArray[i].GetString());
            }
            Animation.GetJointsAttr().Set(JointTokenArray);
        }

        // get local bones transform of UE4 skeleton
        UDebugSkelMeshComponent* DebugSkelMeshComponent = NewObject<UDebugSkelMeshComponent>();
        DebugSkelMeshComponent->RegisterComponentWithWorld(GWorld);
        DebugSkelMeshComponent->EmptyOverrideMaterials();
        DebugSkelMeshComponent->SetSkeletalMesh(SkeletalMesh);
        TArray<FName> SocketNames = DebugSkelMeshComponent->GetAllSocketNames();

        UAnimSequence* AdditiveBase = AnimSequenceBase->GetAdditiveBasePose();
        TArray<FTransform> DefaultBaseBoneTransforms;
        if (AdditiveBase)
        {
            DefaultBaseBoneTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();
        }

        TArray<FTransform> DefaultSocketTransforms;
        DefaultSocketTransforms.AddZeroed(SocketNames.Num());

        for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
        {
            FTransform SocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);
            DefaultSocketTransforms[SocketIndex] = SocketTransform;
        }

        DebugSkelMeshComponent->EnablePreview(true, AnimSequenceBase);
        SocketTransformTimeSamples.AddZeroed(SocketNames.Num());

        UAnimMontage* Montage = Cast<UAnimMontage>(AnimSequenceBase);
        const ScalarRegister VBlendWeight(1.0f);
        for (double TimeCode = StartTimeCode; TimeCode <= EndTimeCode; TimeCode += 1.0)
        {
            float AnimPosition = TimeCode / TimeCodesPerSecond;
            DebugSkelMeshComponent->SetPosition(AnimPosition, false);
            UAnimSingleNodeInstance* SingleNodeInstance = DebugSkelMeshComponent->GetSingleNodeInstance();
            if (SingleNodeInstance && Montage)
            {
                SingleNodeInstance->UpdateMontageWeightForTimeSkip(Montage->BlendIn.GetBlendTime());
            }
            DebugSkelMeshComponent->RefreshBoneTransforms();
    
            for(int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
            {
                FTransform SocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);
                SocketTransformTimeSamples[SocketIndex].Transform.Add(TimeCode, SocketTransform);
            }

            TArray<FTransform> LocalBoneTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();

            UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimationAsset);
            if (AdditiveBase && AnimSequence)
            {
                SetupBasePose(DebugSkelMeshComponent, AnimSequence, AdditiveBase, AnimPosition);

                for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
                {
                    FTransform BaseSocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);
                    FTransform Additive = *SocketTransformTimeSamples[SocketIndex].Transform.Find(TimeCode);
                    FAnimationRuntime::ConvertTransformToAdditive(Additive, BaseSocketTransform);
                    FTransform Base = DefaultSocketTransforms[SocketIndex];
                    FTransform::BlendFromIdentityAndAccumulate(Base, Additive, VBlendWeight);
                    SocketTransformTimeSamples[SocketIndex].Transform[TimeCode] = Base;
                }

                TArray<FTransform> AdditiveBaseTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();
                    
                for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
                {
                    FAnimationRuntime::ConvertTransformToAdditive(LocalBoneTransforms[BoneIndex], AdditiveBaseTransforms[BoneIndex]);	
                    FTransform Base = DefaultBaseBoneTransforms[BoneIndex];
                    FTransform::BlendFromIdentityAndAccumulate(Base, LocalBoneTransforms[BoneIndex], VBlendWeight);
                    LocalBoneTransforms[BoneIndex] = Base;
                }
                    
                DebugSkelMeshComponent->EnablePreview(true, AnimSequence);
            }

            if (bRootIdentity && LocalBoneTransforms.Num() > 0)
            {
                LocalBoneTransforms[0] *= RefSkeleton.GetRefBonePose()[0].Inverse();
            }

            pxr::VtVec3fArray Translations;
            pxr::VtQuatfArray Rotations;
            pxr::VtVec3hArray Scalings; // MUST vec3h

            for (auto LocalBoneTransform : LocalBoneTransforms)
            {
                auto Transform = RHSTransformConvert(USDStage, LocalBoneTransform, false);
                Translations.push_back(USDConvert(Transform.GetTranslation()));
                Rotations.push_back(USDConvert(Transform.GetRotation()));
                Scalings.push_back(USDConvertH(Transform.GetScale3D()));
            }
                
            Animation.CreateTranslationsAttr().Set(pxr::VtValue(Translations), pxr::UsdTimeCode(TimeCode));
            Animation.CreateRotationsAttr().Set(pxr::VtValue(Rotations), pxr::UsdTimeCode(TimeCode));
            Animation.CreateScalesAttr().Set(pxr::VtValue(Scalings), pxr::UsdTimeCode(TimeCode));
        }

        DebugSkelMeshComponent->UnregisterComponent();
        DebugSkelMeshComponent = nullptr;

        return true;
    }

    return false;
}

bool FUSDExporter::ExportSkeletalMeshAnimationTimeSamples(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, USkeletalMeshComponent& SkeletalMeshComponent, const TArray<FSkeletalAnimationTimeSamples>& AnimationTimeSamples, const double StartTimeCode, const double EndTimeCode, const double TimeCodesPerSecond, const bool bRootIdentity)
{
    if (!SkeletalMeshComponent.SkeletalMesh->Skeleton)
    {
        return false;
    }

    pxr::UsdSkelAnimation Animation = pxr::UsdSkelAnimation::Define(USDStage, NewPrimPath);
    if (!Animation)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Could not find UsdSkelAnimation"));
        return false;
    }

    // Set joint info
    auto RefSkeleton = SkeletalMeshComponent.SkeletalMesh->RefSkeleton;
    const int NumBones = RefSkeleton.GetNum();
    pxr::SdfPathVector JointArray;
    if (NumBones > 0)
    {
        JointArray = BuildJointTree(NewPrimPath, NewPrimPath, JointArray, RefSkeleton, 0, 0);

        pxr::VtTokenArray JointTokenArray(NumBones);
        for (int i = 0; i < JointArray.size(); ++i)
        {
            JointTokenArray[i] = pxr::TfToken(JointArray[i].GetString());
        }
        Animation.GetJointsAttr().Set(JointTokenArray);
    }

    auto RootLayerHandle = USDStage->GetRootLayer();
    RootLayerHandle->SetStartTimeCode(StartTimeCode);
    RootLayerHandle->SetEndTimeCode(EndTimeCode);
    RootLayerHandle->SetTimeCodesPerSecond(TimeCodesPerSecond);

    int32 NumTimeCodes = EndTimeCode - StartTimeCode + 1;

    // get local bones transform of UE4 skeleton
    UDebugSkelMeshComponent* DebugSkelMeshComponent = NewObject<UDebugSkelMeshComponent>();
    DebugSkelMeshComponent->RegisterComponentWithWorld(SkeletalMeshComponent.GetWorld());
    DebugSkelMeshComponent->EmptyOverrideMaterials();
    DebugSkelMeshComponent->SetSkeletalMesh(SkeletalMeshComponent.SkeletalMesh);
    TArray<FName> SocketNames = DebugSkelMeshComponent->GetAllSocketNames();
    DebugSkelMeshComponent->RefreshBoneTransforms();

    TArray<FTransform> DefaultSocketTransforms;
    DefaultSocketTransforms.AddZeroed(SocketNames.Num());

    for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
    {
        FTransform SocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);
        DefaultSocketTransforms[SocketIndex] = SocketTransform;
    }

    TArray<FTransform> DefaultBaseTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();

    // Socket
    TArray<FTransformTimeSamples> SocketTransformTimeSamples;
    SocketTransformTimeSamples.AddZeroed(SocketNames.Num());

    for (int32 TimeCode = StartTimeCode; TimeCode <= EndTimeCode; ++TimeCode)
    {
        int32 TimeCodeIndex = TimeCode - StartTimeCode;

        float TotalAdditiveWeight = 0.0f;
        float TotalFullBlendWeight = 0.0f;

        // get total weight
        for (auto SkeletalAnimation : AnimationTimeSamples)
        {
            float* Weight = SkeletalAnimation.Weights.TimeSamples.Find(TimeCode);
            if (Weight)
            {
                if (SkeletalAnimation.Animation->IsValidAdditive())
                {
                    TotalAdditiveWeight += *Weight;
                }
                else
                {
                    TotalFullBlendWeight += *Weight;
                }
            }
        }
        
        TArray<FTransform> BaseTransforms;
        TArray<FTransform> AdditiveTransforms;
        BaseTransforms.AddUninitialized(NumBones);
        AdditiveTransforms.AddUninitialized(NumBones);

        TArray<FTransform> BaseSocketTransforms;
        TArray<FTransform> AdditiveSocketTransforms;
        BaseSocketTransforms.AddUninitialized(SocketNames.Num());
        AdditiveSocketTransforms.AddUninitialized(SocketNames.Num());

        // if there's no valid full blend, using Default local bone transform
        int32 AdditivePoseIndex = 0;
        int32 FullBlendPoseIndex = 0;
        for (auto SkeletalAnimation : AnimationTimeSamples)
        {
            bool bAdditive = SkeletalAnimation.Animation->IsValidAdditive();
            // check total weight at first
            if (bAdditive)
            {			
                if (TotalAdditiveWeight <= ZERO_ANIMWEIGHT_THRESH)
                {
                    continue;
                }
            }
            else
            {
                if (TotalFullBlendWeight <= ZERO_ANIMWEIGHT_THRESH)
                {
                    continue;
                }
            }

            float* Weight = SkeletalAnimation.Weights.TimeSamples.Find(TimeCode);
            if (Weight)
            {
                if (*Weight <= ZERO_ANIMWEIGHT_THRESH)
                {
                    continue;
                }

                float AnimPosition = float(TimeCodeIndex + StartTimeCode - SkeletalAnimation.Range.StartTimeCode) / TimeCodesPerSecond;
                if (SkeletalAnimation.PlayRate > 0.0f)
                {
                    AnimPosition *= SkeletalAnimation.PlayRate;
                }

                float SequenceLength = SkeletalAnimation.Animation->SequenceLength - (SkeletalAnimation.StartFrameOffset + SkeletalAnimation.EndFrameOffset) / TimeCodesPerSecond;
                float FirstLoopSequenceLength = SequenceLength - SkeletalAnimation.FirstLoopStartFrameOffset / TimeCodesPerSecond;

                bool bFirstLoop = AnimPosition < FirstLoopSequenceLength;
                if (!bFirstLoop)
                {
                    AnimPosition -= FirstLoopSequenceLength;
                    while (AnimPosition >= SequenceLength)
                    {
                        AnimPosition -= SequenceLength;
                    }
                }
    
                if (SkeletalAnimation.bReverse)
                {
                    AnimPosition = (bFirstLoop ? FirstLoopSequenceLength : SequenceLength) - AnimPosition;
                    AnimPosition += SkeletalAnimation.EndFrameOffset / TimeCodesPerSecond;
                }
                else
                {
                    AnimPosition += (bFirstLoop ? SkeletalAnimation.FirstLoopStartFrameOffset : 0 + SkeletalAnimation.StartFrameOffset) / TimeCodesPerSecond;
                }

                TArray<FTransform> AdditiveBaseTransforms;
                TArray<FTransform> AdditiveBaseSocketTransforms;
                UAnimSequence* AnimSequence = Cast<UAnimSequence>(SkeletalAnimation.Animation);
                UAnimSequence* AdditiveBase = SkeletalAnimation.Animation->GetAdditiveBasePose();
                if (AdditiveBase && AnimSequence)
                {
                    SetupBasePose(DebugSkelMeshComponent, AnimSequence, AdditiveBase, AnimPosition);
                    AdditiveBaseTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();
                    AdditiveBaseSocketTransforms.AddUninitialized(SocketNames.Num());
                    for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
                    {
                        AdditiveBaseSocketTransforms[SocketIndex] = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);
                    }
                }

                DebugSkelMeshComponent->EnablePreview(true, SkeletalAnimation.Animation);
                DebugSkelMeshComponent->SetPosition(AnimPosition, false);
                UAnimSingleNodeInstance* SingleNodeInstance = DebugSkelMeshComponent->GetSingleNodeInstance();
                UAnimMontage* Montage = Cast<UAnimMontage>(SkeletalAnimation.Animation);
                if (SingleNodeInstance && Montage)
                {
                    SingleNodeInstance->UpdateMontageWeightForTimeSkip(Montage->BlendIn.GetBlendTime());
                }
                DebugSkelMeshComponent->RefreshBoneTransforms();

                TArray<FTransform> LocalBoneTransforms = DebugSkelMeshComponent->GetBoneSpaceTransforms();
                if (bAdditive)
                {
                    TotalAdditiveWeight = FMath::Clamp(TotalAdditiveWeight, 0.0f, 1.0f);
                    float BlendWeight = FMath::Clamp(*Weight, 0.0f, 1.0f) * TotalAdditiveWeight;

                    for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
                    {
                        if (AdditiveBase)
                        {
                            FAnimationRuntime::ConvertTransformToAdditive(LocalBoneTransforms[BoneIndex], AdditiveBaseTransforms[BoneIndex]);
                        }
                        else
                        {
                            FAnimationRuntime::ConvertTransformToAdditive(LocalBoneTransforms[BoneIndex], DefaultBaseTransforms[BoneIndex]);
                        }

                        if (AdditivePoseIndex == 0)
                        {
                            BlendTransform<ETransformBlendMode::Overwrite>(LocalBoneTransforms[BoneIndex], AdditiveTransforms[BoneIndex], BlendWeight);
                        }
                        else
                        {
                            BlendTransform<ETransformBlendMode::Accumulate>(LocalBoneTransforms[BoneIndex], AdditiveTransforms[BoneIndex], BlendWeight);
                        }
                    }

                    for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
                    {
                        FTransform SocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);

                        if (AdditiveBase)
                        {
                            FAnimationRuntime::ConvertTransformToAdditive(SocketTransform, AdditiveBaseSocketTransforms[SocketIndex]);
                        }
                        else
                        {
                            FAnimationRuntime::ConvertTransformToAdditive(SocketTransform, BaseSocketTransforms[SocketIndex]);
                        }

                        if (AdditivePoseIndex == 0)
                        {
                            BlendTransform<ETransformBlendMode::Overwrite>(SocketTransform, AdditiveSocketTransforms[SocketIndex], BlendWeight);
                        }
                        else
                        {
                            BlendTransform<ETransformBlendMode::Accumulate>(SocketTransform, AdditiveSocketTransforms[SocketIndex], BlendWeight);
                        }
                    }

                    ++AdditivePoseIndex;
                }
                else
                {
                    // normalize
                    float BlendWeight = FMath::Clamp(*Weight / TotalFullBlendWeight, 0.0f, 1.0f);
                    for (int32 BoneIndex = 0 ; BoneIndex < NumBones; ++BoneIndex)
                    {
                        if (FullBlendPoseIndex == 0)
                        {
                            BlendTransform<ETransformBlendMode::Overwrite>(LocalBoneTransforms[BoneIndex], BaseTransforms[BoneIndex], BlendWeight);
                        }
                        else
                        {
                            BlendTransform<ETransformBlendMode::Accumulate>(LocalBoneTransforms[BoneIndex], BaseTransforms[BoneIndex], BlendWeight);
                        }
                    }

                    for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
                    {
                        FTransform SocketTransform = DebugSkelMeshComponent->GetSocketTransform(SocketNames[SocketIndex]);

                        if (FullBlendPoseIndex == 0)
                        {
                            BlendTransform<ETransformBlendMode::Overwrite>(SocketTransform, BaseSocketTransforms[SocketIndex], BlendWeight);
                        }
                        else
                        {
                            BlendTransform<ETransformBlendMode::Accumulate>(SocketTransform, BaseSocketTransforms[SocketIndex], BlendWeight);
                        }
                    }

                    ++FullBlendPoseIndex;
                }
            }
        }

        pxr::VtVec3fArray Translations;
        pxr::VtQuatfArray Rotations;
        pxr::VtVec3hArray Scalings; // MUST vec3h

        for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
        {
            FTransform FinalBlendTransform = FullBlendPoseIndex > 0 ? BaseTransforms[BoneIndex] : DefaultBaseTransforms[BoneIndex];
            if (bRootIdentity && BoneIndex == 0)
            {
                FinalBlendTransform *= RefSkeleton.GetRefBonePose()[BoneIndex].Inverse();
            }
            
            if (FullBlendPoseIndex > 1)
            {
                FinalBlendTransform.NormalizeRotation();
            }

            if (AdditivePoseIndex > 0)
            {
                if (AdditivePoseIndex > 1)
                {
                    AdditiveTransforms[BoneIndex].NormalizeRotation();
                }

                const ScalarRegister VBlendWeight(TotalAdditiveWeight);
                FTransform::BlendFromIdentityAndAccumulate(FinalBlendTransform, AdditiveTransforms[BoneIndex], VBlendWeight);
                FinalBlendTransform.NormalizeRotation();
            }
            
            auto Transform = RHSTransformConvert(USDStage, FinalBlendTransform, false);
            Translations.push_back(USDConvert(Transform.GetTranslation()));
            Rotations.push_back(USDConvert(Transform.GetRotation()));
            Scalings.push_back(USDConvertH(Transform.GetScale3D()));
        }

        Animation.CreateTranslationsAttr().Set(pxr::VtValue(Translations), pxr::UsdTimeCode(TimeCode));
        Animation.CreateRotationsAttr().Set(pxr::VtValue(Rotations), pxr::UsdTimeCode(TimeCode));
        Animation.CreateScalesAttr().Set(pxr::VtValue(Scalings), pxr::UsdTimeCode(TimeCode));

        for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
        {
            FTransform FinalSocketTransform = FullBlendPoseIndex > 0 ? BaseSocketTransforms[SocketIndex] : DefaultSocketTransforms[SocketIndex];
            if (FullBlendPoseIndex > 1)
            {
                FinalSocketTransform.NormalizeRotation();
            }

            if (AdditivePoseIndex > 0)
            {
                if (AdditivePoseIndex > 1)
                {
                    AdditiveSocketTransforms[SocketIndex].NormalizeRotation();
                }

                const ScalarRegister VBlendWeight(TotalAdditiveWeight);
                FTransform::BlendFromIdentityAndAccumulate(FinalSocketTransform, AdditiveSocketTransforms[SocketIndex], VBlendWeight);
                FinalSocketTransform.NormalizeRotation();
            }

            SocketTransformTimeSamples[SocketIndex].Transform.Add(TimeCode, FinalSocketTransform);
        }
    }

    for (int32 SocketIndex = 0; SocketIndex < SocketNames.Num(); ++SocketIndex)
    {
        FSequenceExtractor::Get().SetSocketTimeSamples(&SkeletalMeshComponent, SocketNames[SocketIndex], SocketTransformTimeSamples[SocketIndex]);
    }

    DebugSkelMeshComponent->UnregisterComponent();
    DebugSkelMeshComponent = nullptr;

    return true;

}

pxr::UsdGeomMesh FUSDExporter::ExportConvexMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& Path, const FKConvexElem& Convex)
{
    using namespace pxr;

    if(!Convex.GetConvexMesh())
    {
        return UsdGeomMesh();
    }

    auto& ConvexMesh = *Convex.GetConvexMesh();

    auto Mesh = UsdGeomMesh::Define(USDStage, Path);

    VtVec3fArray Points;
    Points.resize(ConvexMesh.getNbVertices());
    for(uint32 I = 0; I < ConvexMesh.getNbVertices(); ++I)
    {
        auto& Point = Points[I];
        const auto& Vertex = ConvexMesh.getVertices()[I];

        FVector Pos = USDConvertPosition(USDStage, (FVector&)Vertex);
        Point = (GfVec3f&)Pos;
    }

    Mesh.CreatePointsAttr().Set(Points);

    VtIntArray FaceIndices, FaceCounts;

    for(uint32 I = 0; I < ConvexMesh.getNbPolygons(); ++I)
    {
        PxHullPolygon Polygon;
        ConvexMesh.getPolygonData(I, Polygon);

        FaceCounts.push_back(Polygon.mNbVerts);
        for(uint32 Vert = 0; Vert < Polygon.mNbVerts; ++Vert)
        {
            FaceIndices.push_back(ConvexMesh.getIndexBuffer()[Polygon.mIndexBase + (Polygon.mNbVerts - Vert - 1)]);
        }
    }

    Mesh.CreateFaceVertexIndicesAttr().Set(FaceIndices);
    Mesh.CreateFaceVertexCountsAttr().Set(FaceCounts);

    VtVec3fArray Extent;
    Extent.push_back((GfVec3f&)Convex.ElemBox.Min);
    Extent.push_back((GfVec3f&)Convex.ElemBox.Max);
    Mesh.CreateExtentAttr().Set(Extent);

    setLocalTransformMatrix(Mesh, RHSTransformConvert(USDStage, Convex.GetTransform(), false));

    return Mesh;
}

pxr::UsdGeomMesh FUSDExporter::ExportTriangleMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& Path, const struct FTriMeshCollisionData& TriMesh)
{
    using namespace pxr;

    if (TriMesh.Vertices.Num() == 0 || TriMesh.Indices.Num() == 0)
    {
        return UsdGeomMesh();
    }

    auto Mesh = UsdGeomMesh::Define(USDStage, Path);

    VtVec3fArray Points{};
    for (auto const& Vertex : TriMesh.Vertices)
    {
        auto TransformedVertex = USDConvertPosition(USDStage, Vertex);
        Points.push_back(USDConvert(TransformedVertex));
    }

    VtIntArray FaceCounts{};
    VtIntArray FaceIndices{};
    for (auto const& Index : TriMesh.Indices)
    {
        FaceIndices.push_back(Index.v0);
        FaceIndices.push_back(Index.v1);
        FaceIndices.push_back(Index.v2);
        FaceCounts.push_back(3);
    }

    Mesh.CreatePointsAttr().Set(Points);
    Mesh.CreateFaceVertexIndicesAttr().Set(FaceIndices);
    Mesh.CreateFaceVertexCountsAttr().Set(FaceCounts);

    FBox Bounds{TriMesh.Vertices};
    VtVec3fArray Extent;
    Extent.push_back((GfVec3f&)Bounds.Min);
    Extent.push_back((GfVec3f&)Bounds.Max);
    Mesh.CreateExtentAttr().Set(Extent);

    return Mesh;
}

pxr::VtArray<pxr::UsdGeomGprim> FUSDExporter::ExportPhysics(const pxr::UsdGeomXformable& Xform, UStaticMesh& StaticMesh)
{
    using namespace pxr;

    VtArray<pxr::UsdGeomGprim> NewCollisions;
    return NewCollisions;
}

pxr::UsdGeomMesh FUSDExporter::ExportBSP(const pxr::UsdStageRefPtr& USDStage, UModel& BSPModel, const FString& CustomPrimName)
{
    using namespace pxr;
    UsdGeomMesh USDMesh;

    uint32 VertCount(BSPModel.VertexBuffer.Vertices.Num());
    // Skip if no vertex exist
    if (VertCount == 0)
    {
        return USDMesh;
    }

    auto DefaultPrim = USDStage->GetDefaultPrim();
    if (!DefaultPrim)
    {
        DefaultPrim = USDStage->GetPseudoRoot();
    }

    if (CustomPrimName.IsEmpty())
    {
        // Create mesh prim
        for(int Postfix = 0;; ++Postfix)
        {
            auto PrimName = BSPModel.GetName() + (Postfix == 0 ? TEXT("") : FString::FromInt(Postfix));
            auto PrimPath = DefaultPrim.GetPrimPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PrimName)));

            if(USDStage->GetPrimAtPath(PrimPath))
            {
                continue;
            }

            USDMesh = UsdGeomMesh::Define(USDStage, PrimPath);

            break;
        }
    }
    else
    {
        auto PrimPath = DefaultPrim.GetPrimPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*CustomPrimName)));
        USDMesh = UsdGeomMesh::Define(USDStage, PrimPath);
    }

    if(!USDMesh)
    {
        return USDMesh;
    }

    USDMesh.GetPrim().SetDocumentation("Exported from UE4 Binary Space Partitioning.");

    FUSDGeomMeshAttributes Attributes;
    pxr::VtArray<pxr::VtArray<int>> FaceIndices;

    Attributes.Points.resize(VertCount);
    Attributes.Normals.resize(VertCount);
    int32 TexCoordCount = 2; // include light uv
    Attributes.UVs.resize(TexCoordCount);
    for (int32 TexCoordIndex = 0; TexCoordIndex < TexCoordCount; ++TexCoordIndex)
    {
        Attributes.UVs[TexCoordIndex].resize(VertCount);
    }

    // Filling Vertex
    for (uint32 VertexIndex = 0; VertexIndex < VertCount; ++VertexIndex)
    {
        const FModelVertex& Vertex = BSPModel.VertexBuffer.Vertices[VertexIndex];
        FVector Normal = Vertex.TangentZ;
        FVector FinalVertexPos = Vertex.Position;

        Attributes.Points[VertexIndex] = USDConvert(USDConvertPosition(USDStage, FinalVertexPos));
        Attributes.Normals[VertexIndex] = USDConvert(USDConvertVector(USDStage, Normal));
        
        FVector2D TexCoord = Vertex.TexCoord;
        TexCoord[1] = 1.f - TexCoord[1];
        Attributes.UVs[0][VertexIndex] = USDConvert(TexCoord);

        // Light UV
        TexCoord = Vertex.ShadowTexCoord;
        TexCoord[1] = 1.f - TexCoord[1];
        Attributes.UVs[1][VertexIndex] = USDConvert(TexCoord);
    }

    // Filling GeomSubset by Material Index Buffers
    for (auto MaterialIterator = BSPModel.MaterialIndexBuffers.CreateIterator(); MaterialIterator; ++MaterialIterator)
    {
        UMaterialInterface* MaterialInterface = MaterialIterator.Key();
        FRawIndexBuffer16or32& IndexBuffer = *MaterialIterator.Value();
        int32 IndexCount = IndexBuffer.Indices.Num();
        if (IndexCount < 3) continue;
        
        // Retrieve and fill in the index buffer.
        const int32 TriangleCount = IndexCount / 3;

        for( int32 TriangleIdx = 0; TriangleIdx < TriangleCount; ++TriangleIdx )
        {
            Attributes.FaceVertexCounts.push_back(3);

            for( int32 IndexIdx = 0; IndexIdx < 3; ++IndexIdx )
            {
                Attributes.FaceVertexIndices.push_back(IndexBuffer.Indices[ TriangleIdx * 3 + IndexIdx ]);
            }
        }
    }

    FillUSDMesh(USDStage, USDMesh, pxr::UsdGeomTokens->vertex, Attributes);

    return USDMesh;
}

bool FUSDExporter::ShouldSaveMember(const TArray<FString>& Members, bool bIncludeSub)const
{
    if(Changes.Num() <= 0)
    {
        return true;
    }

    for(auto& Change : Changes)
    {
        if(Change.IsEmpty())
        {
            return true;
        }

        static const FString MemberName = "Member";

        if(Members.Num() <= 0)
        {
            if(Change == MemberName || Change.StartsWith(MemberName + "/"))
            {
                return true;
            }
        }
        else
        {
            for(auto& Member : Members)
            {
                auto FullMember = MemberName / Member;
                if(Change == FullMember || (bIncludeSub && Change.StartsWith(FullMember + "/", ESearchCase::CaseSensitive)))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool FUSDExporter::ShouldSaveRenderState()const
{
    if(Changes.Num() <= 0)
    {
        return true;
    }

    for(auto& Change : Changes)
    {
        if(Change == "RenderState")
        {
            return true;
        }
    }

    return false;
}

bool FUSDExporter::setLocalTransformMatrix(const pxr::UsdGeomXformable& xform, const FTransform& finalTransform/*const pxr::GfMatrix4d& final*/, const pxr::UsdTimeCode& timeCode,
    bool bTranslateTimeSample, bool bRotateTimeSample, bool bScaleTimeSample, bool bIgnoreTranslate, bool bIgnoreRotate, bool bIgnoreScale)
{
    bool resetXFormStack;
    auto xformOps = xform.GetOrderedXformOps(&resetXFormStack);
    bool foundTransformOp = false;
    bool success = true;

    for(auto xformOp : xformOps)
    {
        // Found transform op, trying to set its value
        if(xformOp.GetOpType() == pxr::UsdGeomXformOp::TypeTransform)
        {
            foundTransformOp = true;
            success &= xformOp.Set(USDConvert(finalTransform.ToMatrixWithScale()));
            break;
        }
    }

    // If transformOp is not found, make individual xformOp or reuse old ones.
    if(!foundTransformOp)
    {
        pxr::GfVec3d translation = pxr::GfVec3d(finalTransform.GetLocation().X, finalTransform.GetLocation().Y, finalTransform.GetLocation().Z);
        pxr::GfVec3d scale = pxr::GfVec3d(finalTransform.GetScale3D().X, finalTransform.GetScale3D().Y, finalTransform.GetScale3D().Z);
        pxr::GfQuatd rotationQuat = pxr::GfQuatd(finalTransform.GetRotation().W, finalTransform.GetRotation().X, finalTransform.GetRotation().Y, finalTransform.GetRotation().Z);
        pxr::GfRotation rotation = pxr::GfRotation(rotationQuat);

        // Don't use UsdGeomXformCommonAPI. It can only manipulate a very limited subset of xformOpOrder combinations
        // Do it manually as non-destructively as possible
        pxr::UsdGeomXformOp xformOp;
        std::vector<pxr::UsdGeomXformOp> newXformOps;
        pxr::UsdGeomXformOp::Precision precision = pxr::UsdGeomXformOp::PrecisionDouble;

        auto findOrAdd = [&xformOps, &xform](pxr::UsdGeomXformOp::Type xformOpType, pxr::UsdGeomXformOp& outXformOp,
            bool createIfNotExist, pxr::UsdGeomXformOp::Precision& precision,
            pxr::TfToken const& opSuffix = pxr::TfToken()) {
                for(auto xformOp : xformOps)
                {
                    if(xformOp.GetOpType() == xformOpType)
                    {
                        // To differentiate translate and translate:pivot
                        std::string expectedOpName = pxr::UsdGeomXformOp::GetOpName(xformOpType, opSuffix);
                        std::string opName = xformOp.GetOpName().GetString();
                        if(opName == expectedOpName)
                        {
                            precision = xformOp.GetPrecision();
                            outXformOp = xformOp;
                            return true;
                        }
                    }
                }

                if(createIfNotExist)
                {
                    outXformOp = xform.AddXformOp(xformOpType, precision, opSuffix);
                    if (outXformOp)
                    {
                        return true;
                    }
                }
                return false;
        };

        auto getFirstRotateOpType = [&xformOps](pxr::UsdGeomXformOp::Precision& precision) {
            for(auto xformOp : xformOps)
            {
                if(xformOp.GetOpType() >= pxr::UsdGeomXformOp::Type::TypeRotateX &&
                    xformOp.GetOpType() <= pxr::UsdGeomXformOp::Type::TypeOrient)
                {
                    precision = xformOp.GetPrecision();
                    return xformOp.GetOpType();
                }
            }
            return pxr::UsdGeomXformOp::Type::TypeInvalid;
        };

        auto decomposeAndSetValue = [&rotation, &findOrAdd, &newXformOps, &success, &bIgnoreRotate](
            pxr::UsdGeomXformOp::Type rotationType, const pxr::GfVec3d& axis0,
            const pxr::GfVec3d& axis1, const pxr::GfVec3d& axis2, size_t xIndex,
            size_t yIndex, size_t zIndex, pxr::UsdGeomXformOp::Precision precision, const pxr::UsdTimeCode& timeCode) {
            pxr::GfVec3d angles = rotation.Decompose(axis0, axis1, axis2);
                pxr::GfVec3d rotate = {angles[xIndex], angles[yIndex], angles[zIndex]};
                pxr::UsdGeomXformOp xformOp;
                if(findOrAdd(rotationType, xformOp, true, precision))
                {
                    if (!bIgnoreRotate)
                    {
                        bool bRet = setValuedWithPrecision<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, pxr::GfVec3d>(xformOp, rotate, timeCode);
                        if (bRet)
                        {
                            newXformOps.push_back(xformOp);
                        }
                        success &= bRet;
                    }
                }
        };

        // Set translation
        if(findOrAdd(pxr::UsdGeomXformOp::TypeTranslate, xformOp, true, precision))
        {
            if (!bIgnoreTranslate)
            {
                bool bRet = setValuedWithPrecision<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, pxr::GfVec3d>(xformOp, translation, bTranslateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                if (bRet)
                {
                    newXformOps.push_back(xformOp);
                }
                success &= bRet;
            }
        }

        // Set pivot
        bool hasPivot = false;
        pxr::UsdGeomXformOp pivotOp;
        pxr::GfVec3d pivotValue(0., 0., 0.);
        if((hasPivot = findOrAdd(pxr::UsdGeomXformOp::TypeTranslate, pivotOp, false, precision, pxr::TfToken("pivot"))))
        {
            // NOTE: UE4 always reset pivot
            bool bRet = setValuedWithPrecision<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, pxr::GfVec3d>(pivotOp, pivotValue, pxr::UsdTimeCode::Default());
            if (bRet)
            {
                newXformOps.push_back(pivotOp);
            }
        }

        // Set rotation
        auto firstRotateOpType = getFirstRotateOpType(precision);
        switch(firstRotateOpType)
        {
        case pxr::UsdGeomXformOp::TypeRotateX:
        case pxr::UsdGeomXformOp::TypeRotateY:
        case pxr::UsdGeomXformOp::TypeRotateZ:
        {
            pxr::GfVec3d angles = rotation.Decompose(pxr::GfVec3d::ZAxis(), pxr::GfVec3d::YAxis(), pxr::GfVec3d::XAxis());
            pxr::GfVec3d rotateZYX = {angles[2], angles[1], angles[0]};
            if(findOrAdd(pxr::UsdGeomXformOp::TypeRotateZ, xformOp, true, precision))
            {
                if (!bIgnoreRotate)
                {
                    bool bRet = setValuedWithPrecision<pxr::GfHalf, float, double, double>(xformOp, rotateZYX[2], bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                    if (bRet)
                    {
                        newXformOps.push_back(xformOp);
                    }
                    success &= bRet;
                }
            }
            if(findOrAdd(pxr::UsdGeomXformOp::TypeRotateY, xformOp, true, precision))
            {
                if (!bIgnoreRotate)
                {
                    bool bRet = setValuedWithPrecision<pxr::GfHalf, float, double, double>(xformOp, rotateZYX[1], bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                    if (bRet)
                    {
                        newXformOps.push_back(xformOp);
                    }
                    success &= bRet;
                }
            }
            if(findOrAdd(pxr::UsdGeomXformOp::TypeRotateX, xformOp, true, precision))
            {
                if (!bIgnoreRotate)
                {
                    bool bRet = setValuedWithPrecision<pxr::GfHalf, float, double, double>(xformOp, rotateZYX[0], bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                    if (bRet)
                    {
                        newXformOps.push_back(xformOp);
                    }
                    success &= bRet;
                }
            }
            break;
        }
        case pxr::UsdGeomXformOp::TypeRotateXYZ:
            decomposeAndSetValue(firstRotateOpType, pxr::GfVec3d::ZAxis(), pxr::GfVec3d::YAxis(),
                pxr::GfVec3d::XAxis(), 2, 1, 0, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        case pxr::UsdGeomXformOp::TypeRotateXZY:
            decomposeAndSetValue(firstRotateOpType, pxr::GfVec3d::YAxis(), pxr::GfVec3d::ZAxis(),
                pxr::GfVec3d::XAxis(), 2, 0, 1, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        case pxr::UsdGeomXformOp::TypeRotateYXZ:
            decomposeAndSetValue(firstRotateOpType, pxr::GfVec3d::ZAxis(), pxr::GfVec3d::XAxis(),
                pxr::GfVec3d::YAxis(), 1, 2, 0, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        case pxr::UsdGeomXformOp::TypeRotateYZX:
            decomposeAndSetValue(firstRotateOpType, pxr::GfVec3d::XAxis(), pxr::GfVec3d::ZAxis(),
                pxr::GfVec3d::YAxis(), 0, 2, 1, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        case pxr::UsdGeomXformOp::TypeRotateZXY:
            decomposeAndSetValue(firstRotateOpType, pxr::GfVec3d::YAxis(), pxr::GfVec3d::XAxis(),
                pxr::GfVec3d::ZAxis(), 1, 0, 2, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        case pxr::UsdGeomXformOp::TypeOrient:
        case pxr::UsdGeomXformOp::TypeInvalid:
            if(findOrAdd(pxr::UsdGeomXformOp::TypeOrient, xformOp, true, precision))
            {
                if (!bIgnoreRotate)
                {
                    bool bRet = setValuedWithPrecision<pxr::GfQuath, pxr::GfQuatf, pxr::GfQuatd, pxr::GfQuatd>(
                        xformOp, rotationQuat, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                    if (bRet)
                    {
                        newXformOps.push_back(xformOp);
                    }
                    success &= bRet;
                }
            }
            break;
        case pxr::UsdGeomXformOp::TypeRotateZYX:
        default:
            decomposeAndSetValue(pxr::UsdGeomXformOp::TypeRotateZYX, pxr::GfVec3d::XAxis(),
                pxr::GfVec3d::YAxis(), pxr::GfVec3d::ZAxis(), 0, 1, 2, precision, bRotateTimeSample ? timeCode : pxr::UsdTimeCode::Default());
            break;
        }

        // Set scale
        if(findOrAdd(pxr::UsdGeomXformOp::TypeScale, xformOp, true, precision))
        {
            if (!bIgnoreScale)
            {
                bool bRet = setValuedWithPrecision<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, pxr::GfVec3d>(xformOp, scale, bScaleTimeSample ? timeCode : pxr::UsdTimeCode::Default());
                if (bRet)
                {
                    newXformOps.push_back(xformOp);
                }
                success &= bRet;
            }
        }

        // Set inverse pivot
        if(hasPivot)
        {
            // Assume the last xformOps is the pivot
            newXformOps.push_back(xformOps.back());
        }

        success &= xform.SetXformOpOrder(newXformOps);
    }
    return success;
}

float USDConvertLength(const pxr::UsdStageRefPtr & Stage, float InLength, bool UsdToUE)
{
    return InLength * (UsdToUE ? UnitScaleFromUSDToUE(Stage) : UnitScaleFromUEToUSD(Stage));
}

FVector USDConvertPosition(const pxr::UsdStageRefPtr & Stage, const FVector& InPosition, bool UsdToUE)
{
    auto ConversionMatrix = pxr::UsdGeomGetStageUpAxis(Stage) == pxr::UsdGeomTokens->z ? SimReadyZUpConversion : SimReadyYUpConversion;
    return ConversionMatrix.TransformPosition(InPosition) * (UsdToUE ? UnitScaleFromUSDToUE(Stage) : UnitScaleFromUEToUSD(Stage));
}

FVector USDConvertVector(const pxr::UsdStageRefPtr & Stage, const FVector& InVector)
{
    auto ConversionMatrix = pxr::UsdGeomGetStageUpAxis(Stage) == pxr::UsdGeomTokens->z ? SimReadyZUpConversion : SimReadyYUpConversion;
    return ConversionMatrix.TransformVector(InVector);
}

bool HasTimeSampleRelatedToActor(AActor* InActor)
{
    auto CurrentActor = InActor;
    while(CurrentActor)
    {
        const FTranslateRotateScaleTimeSamples* ActorTransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(CurrentActor);
        if (ActorTransformTimeSamples && ActorTransformTimeSamples->HasAnyTimeSamples())
        {
            return true;
        }

        CurrentActor = CurrentActor->GetAttachParentActor();
    }

    return false;
}

FTransform GetActorWorldTransformInSequence(AActor* InActor, int32 TimeCode)
{
    FTransform Result = FTransform::Identity;
    auto CurrentActor = InActor;
    while (CurrentActor)
    {
        FTransform RelativeTransform = CurrentActor->GetRootComponent()->GetRelativeTransform();
        const FTranslateRotateScaleTimeSamples* TransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(CurrentActor);
        if (TransformTimeSamples && TransformTimeSamples->HasAnyTimeSamples())
        {
            auto Translate = TransformTimeSamples->Translate.Find(TimeCode);
            if (Translate)
            {
                RelativeTransform.SetLocation(*Translate);
            }
            auto Rotate = TransformTimeSamples->Rotate.Find(TimeCode);
            if (Rotate)
            {
                RelativeTransform.SetRotation(FRotator(Rotate->Y, Rotate->Z, Rotate->X).Quaternion());
            }
            auto Scale = TransformTimeSamples->Scale.Find(TimeCode);
            if (Scale)
            {
                RelativeTransform.SetScale3D(*Scale);
            }
        }
        Result *= RelativeTransform;

        CurrentActor = CurrentActor->GetAttachParentActor();
    }

    return Result;
}

void FixCineCameraLookAtTracking(ACineCameraActor* CineCameraActor)
{
    if (CineCameraActor->LookatTrackingSettings.bEnableLookAtTracking)
    {
        FTranslateRotateScaleTimeSamples NewCameraTrack;

        auto FixCineCameraLookAt = [&](const FVector& LookatLoc)
        {
            FVector const ToLookat = LookatLoc - CineCameraActor->GetActorLocation();
            FRotator FinalRot = ToLookat.Rotation();

            if (CineCameraActor->LookatTrackingSettings.bAllowRoll)
            {
                FinalRot.Roll = CineCameraActor->GetActorRotation().Roll;
            }

            CineCameraActor->SetActorRotation(FinalRot);
        };
        
        auto FixCineCameraLookAtTimeSamples = [&](const FVector& LookatLoc)
        {
            const FTranslateRotateScaleTimeSamples* CameraTransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(CineCameraActor);
            if (CameraTransformTimeSamples == nullptr)
            {
                FixCineCameraLookAt(LookatLoc);
            }
            else
            {
                if (CameraTransformTimeSamples->GetNumTranslateTimeSamples() == 0 && CameraTransformTimeSamples->GetNumRotateTimeSamples() == 0)
                {
                    FixCineCameraLookAt(LookatLoc);
                }
                else
                {
                    FTranslateRotateScaleTimeSamples FixedCameraTrack;

                    auto TraverseCallback = [&](int32 TimeCode, const FTransform& InTransform)
                    {
                        CineCameraActor->SetActorTransform(InTransform);

                        FixCineCameraLookAt(LookatLoc);

                        FixedCameraTrack.Translate.FindOrAdd(TimeCode, CineCameraActor->GetActorLocation());
                        FRotator Rotator = CineCameraActor->GetActorRotation();
                        FixedCameraTrack.Rotate.FindOrAdd(TimeCode, FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw));
                        FixedCameraTrack.Scale.FindOrAdd(TimeCode, CineCameraActor->GetActorScale());
                    };
                    CameraTransformTimeSamples->Traverse(TraverseCallback);

                    FSequenceExtractor::Get().SetTransformTimeSamples(CineCameraActor, FixedCameraTrack);
                }
            }
        };

        auto FixOrCreateCineCameraLookAtTrack = [&](int32 TimeCode, const FVector& LookatLoc)
        {
            const FTranslateRotateScaleTimeSamples* CameraTransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(CineCameraActor);
            if ((CameraTransformTimeSamples == nullptr) || 
                (CameraTransformTimeSamples->GetNumTranslateTimeSamples() == 0 && CameraTransformTimeSamples->GetNumRotateTimeSamples() == 0))
            {
                FixCineCameraLookAt(LookatLoc);		
            }
            else
            {
                FTransform CameraTransform = CineCameraActor->GetActorTransform();
                auto T = CameraTransformTimeSamples->Translate.Find(TimeCode);
                if (T)
                {
                    CineCameraActor->SetActorLocation(*T);
                }
                auto R = CameraTransformTimeSamples->Rotate.Find(TimeCode);
                if (R)
                {
                    FRotator Rotator(R->Y, R->Z, R->X);
                    CineCameraActor->SetActorRotation(Rotator);
                }
                auto S = CameraTransformTimeSamples->Scale.Find(TimeCode);
                if (S)
                {
                    CineCameraActor->SetActorScale3D(*S);
                }
                FixCineCameraLookAt(LookatLoc);
            }

            NewCameraTrack.Translate.FindOrAdd(TimeCode, CineCameraActor->GetActorLocation());
            FRotator Rotator = CineCameraActor->GetActorRotation();
            NewCameraTrack.Rotate.FindOrAdd(TimeCode, FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw));
            NewCameraTrack.Scale.FindOrAdd(TimeCode, CineCameraActor->GetActorScale());
        };

        if (AActor* ActorToTrack = CineCameraActor->LookatTrackingSettings.ActorToTrack.Get())
        {
            if (!HasTimeSampleRelatedToActor(ActorToTrack))
            {
                FTransform const BaseTransform = ActorToTrack->GetActorTransform();
                FVector LookatLoc = BaseTransform.TransformPosition(CineCameraActor->LookatTrackingSettings.RelativeOffset);

                FixCineCameraLookAtTimeSamples(LookatLoc);
            }
            else
            {
                for (int32 TimeCode = FSequenceExtractor::Get().GetMasterStartTimeCode(); TimeCode <= FSequenceExtractor::Get().GetMasterEndTimeCode(); ++TimeCode)
                {
                    FTransform const BaseTransform = GetActorWorldTransformInSequence(ActorToTrack, TimeCode);
                    FVector LookatLoc = BaseTransform.TransformPosition(CineCameraActor->LookatTrackingSettings.RelativeOffset);

                    FixOrCreateCineCameraLookAtTrack(TimeCode, LookatLoc);
                }

                FSequenceExtractor::Get().SetTransformTimeSamples(CineCameraActor, NewCameraTrack);
            }
        }
        else
        {
            FVector LookatLoc = CineCameraActor->LookatTrackingSettings.RelativeOffset;

            FixCineCameraLookAtTimeSamples(LookatLoc);
        }
    }
}

FTransform GetRelativeTransformFromAttachActorInSequence(AActor* Actor, int32 TimeCode)
{
    const TArray<FObjectBindingTimeSamples>* ObjectBindingTimeSamples = FSequenceExtractor::Get().GetObjectBindingTimeSamples(Actor);
    if (ObjectBindingTimeSamples == nullptr)
    {
        return FTransform::Identity;
    }

    FTransform ReletiveTransform = FTransform::Identity;
    for(auto ObjectBinding : *ObjectBindingTimeSamples)
    {
        auto AttachParentActor = ObjectBinding.BindingObject;
        if (AttachParentActor == nullptr)
        {
            continue;
        }
        auto CurrentParentActor = Actor->GetAttachParentActor();
        if (CurrentParentActor == AttachParentActor)
        {
            continue;
        }

        // check time
        if (TimeCode < ObjectBinding.Range.StartTimeCode || TimeCode > ObjectBinding.Range.EndTimeCode)
        {
            continue;
        }

        FTransform SrcTransform = GetActorWorldTransformInSequence(CurrentParentActor, TimeCode);
        FTransform DestTransform = GetActorWorldTransformInSequence(AttachParentActor, TimeCode);

        ReletiveTransform *= DestTransform.GetRelativeTransform(SrcTransform);
    }

    return ReletiveTransform;
}	

bool FUSDExporter::ExportSceneComponent(const pxr::UsdGeomImageable& Imageable, USceneComponent& SceneComp, bool bSkipChildren, bool bInheritParent, const TArray<FTimeSampleRange>* Ranges)
{
    if(!Imageable)
    {
        return false;
    }

    FixPrimKind(Imageable.GetPrim());

    bool bValidRootComponent = SceneComp.GetOwner() && (SceneComp.GetOwner()->GetRootComponent() == &SceneComp);

    // Visibility
    if(!bInheritParent && (ShouldSaveMember({
        "bVisible",
        GET_MEMBER_NAME_STRING_CHECKED(USceneComponent, bHiddenInGame),
        "bHidden"
        }) 
        // NOTE: HiddenInEditor only send RenderState event
        || ShouldSaveRenderState()))
    {
        const FBooleanTimeSamples* VisibilityTimeSamples = nullptr;
        VisibilityTimeSamples = FSequenceExtractor::Get().GetVisibilityTimeSamples(&SceneComp);
        if (VisibilityTimeSamples == nullptr && bValidRootComponent)
        {
            VisibilityTimeSamples = FSequenceExtractor::Get().GetVisibilityTimeSamples(SceneComp.GetOwner());
        }

        const FBooleanTimeSamples* OwnerVisibilityTimeSamples = nullptr;
        // Is this component the root component of the owner?
        if (bValidRootComponent)
        {
            OwnerVisibilityTimeSamples = FSequenceExtractor::Get().GetVisibilityTimeSamples(SceneComp.GetOwner());
        }

        if (VisibilityTimeSamples == nullptr)
        {
            if (SceneComp.ShouldRender())
            {
                Imageable.GetVisibilityAttr().Set(pxr::UsdGeomTokens->inherited);
            }
            else
            {
                Imageable.GetVisibilityAttr().Set(pxr::UsdGeomTokens->invisible);
            }
        }
        else
        {
            if (OwnerVisibilityTimeSamples && OwnerVisibilityTimeSamples != VisibilityTimeSamples)
            {
                auto FillVis = [](const TMap<int32, bool>& VisMap, TArray<bool>& VisArray)
                {
                    int32 Start = FSequenceExtractor::Get().GetMasterStartTimeCode();
                    VisArray.AddUninitialized(FSequenceExtractor::Get().GetMasterEndTimeCode() - FSequenceExtractor::Get().GetMasterStartTimeCode() + 1);
                    TOptional<bool> bLastVisible;
                    int32 Index = 0;
                    for (const TPair<int32, bool>& Element : VisMap)
                    {
                        for (int32 Time = Start; Time < Element.Key; ++Time)
                        {
                            VisArray[Index++] = bLastVisible.IsSet() ? bLastVisible.GetValue() : Element.Value;
                        }
                        bLastVisible = Element.Value;
                        Start = Element.Key;
                    }

                    for (int32 Time = Start; Time <= FSequenceExtractor::Get().GetMasterEndTimeCode(); ++Time)
                    {
                        VisArray[Index++] = bLastVisible.GetValue();
                    }
                };

                TArray<bool> OwnerVis;
                TArray<bool> CompVis;
                FillVis(VisibilityTimeSamples->TimeSamples, CompVis);
                FillVis(OwnerVisibilityTimeSamples->TimeSamples, OwnerVis);

                // Test if the SceneComp is a CapsuleComponent.  These are almost always invisible, Character BPs use them as the root comp.
                // Only look at the owner visibility in this case.
                bool bCapsuleComponent = false;
                if (Cast<UCapsuleComponent>(&SceneComp))
                {
                    bCapsuleComponent = true;
                }

                TOptional<bool> bLastVisible;
                for (int32 Time = FSequenceExtractor::Get().GetMasterStartTimeCode(); Time <= FSequenceExtractor::Get().GetMasterEndTimeCode(); ++Time)
                {
                    int32 Index = Time - FSequenceExtractor::Get().GetMasterStartTimeCode();
                    bool bVisible = OwnerVis[Index] && (CompVis[Index] || bCapsuleComponent);

                    if (!bLastVisible.IsSet() || (bLastVisible.IsSet() && (bLastVisible.GetValue() != bVisible)))
                    {
                        if (IsInRange(Time, Ranges))
                        {
                            Imageable.GetVisibilityAttr().Set(bVisible ? pxr::TfToken("inherited") : pxr::TfToken("invisible"), pxr::UsdTimeCode(Time));
                            bLastVisible = bVisible;
                        }
                    }
                }
            }
            else
            {
                for (const TPair<int32, bool>& Element : VisibilityTimeSamples->TimeSamples)
                {
                    if (IsInRange(Element.Key, Ranges))
                    {
                        Imageable.GetVisibilityAttr().Set(Element.Value ? pxr::TfToken("inherited") : pxr::TfToken("invisible"), pxr::UsdTimeCode(Element.Key));
                    }
                }
            }
        }

        // Reload visibility for all children
        auto PropPath = Imageable.GetPath().AppendProperty(pxr::UsdGeomTokens->visibility);
        ReloadPaths.Add(PropPath.GetText());
    }

    pxr::UsdGeomXformable Xformable = pxr::UsdGeomXformable(Imageable);
    // Transform
    if(Xformable && ShouldSaveMember({
        "RelativeLocation",
        "RelativeRotation",
        "RelativeScale3D"
        }))
    {
        if (bValidRootComponent && SceneComp.GetOwner()->IsA<ACineCameraActor>() && Ranges == nullptr)
        {
            auto CineCameraActor = Cast<ACineCameraActor>(SceneComp.GetOwner());
            FixCineCameraLookAtTracking(CineCameraActor);
        }

        FTransform RelTransform = ConvertRelativeTransformFromUE4ToUSD(SceneComp);
        FTransformTimeSamples ParentTransformTimeSamples;
        FTransform DefaultParentTransform;
        if (bInheritParent)
        {
            std::vector<double> TimeSamples;
            Xformable.GetTimeSamples(&TimeSamples);

            if (TimeSamples.size() > 0)
            {
                for (double Time = TimeSamples[0]; Time <= TimeSamples[TimeSamples.size() - 1]; Time += 1.0)
                {
                    if (IsInRange(Time, Ranges))
                    {
                        pxr::GfMatrix4d LocalMatrix;
                        bool bReset = false;
                        Xformable.GetLocalTransformation(&LocalMatrix, &bReset, pxr::UsdTimeCode(Time));
                        ParentTransformTimeSamples.Transform.Add(Time, FTransform(USDConvert(LocalMatrix)));
                    }
                }
            }
            else
            {
                pxr::GfMatrix4d LocalMatrix;
                bool bReset = false;
                Xformable.GetLocalTransformation(&LocalMatrix, &bReset);
                DefaultParentTransform = FTransform(USDConvert(LocalMatrix));
            }	
        }

        auto SetPrimTransform = [&](FTransform USDTransform, const pxr::UsdTimeCode& timeCode, bool bTranslateTimeSamples, bool bRotateTimeSamples, bool bScaleTimeSamples, bool bIgnoreTranslate, bool bIgnoreRotate, bool bIgnoreScale)
        {
            bool bIsLight = SimReadyUsdLuxLightCompat::PrimIsALight(Xformable.GetPrim());
            if ((SceneComp.IsA<ULightComponent>() && bIsLight) ||
                (SceneComp.IsA<UCameraComponent>() && Xformable.GetPrim().IsA<pxr::UsdGeomCamera>()))
            {
                // convert from UE4 light (+x) to usd light (-z)
                // convert from UE4 camera (+x and z-up) to usd camera (-z and y-up)
                // convert from Width of UE4 in the local Y axis to Width of USD, in the local X axis.	
                if (GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z)
                {
                    USDTransform.SetRotation(USDTransform.GetRotation() * FQuat(FVector(0, 1, 0), -HALF_PI) * FQuat(FVector(0, 0, 1), HALF_PI));
                }
                else
                {
                    USDTransform.SetRotation(USDTransform.GetRotation() * FQuat(FVector(0, 0, 1), HALF_PI));
                }

                // Scale for component shouldn't affect the shape of light, always set identity scale to light
                if (SceneComp.IsA<ULightComponent>() && bIsLight)
                {
                    USDTransform.SetScale3D(FVector::OneVector);
                }
            }
            else if (SceneComp.IsA<USpringArmComponent>())
            {
                USpringArmComponent* SpringArmComp = Cast<USpringArmComponent>(&SceneComp);
                USDTransform = USDTransform * SpringArmComp->GetSocketTransform(NAME_None, RTS_Component);
                USDTransform.NormalizeRotation();
            }
            else if (SceneComp.IsA<UMeshComponent>())
            {
                if (GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->y)
                {
                    auto MeshComponent = Cast<UMeshComponent>(&SceneComp);
                    if (IsSphericalOrCylindricalProjectionUsed(MeshComponent))
                    {
                        USDTransform.SetRotation(USDTransform.GetRotation() * FQuat(FVector(1, 0, 0), HALF_PI));
                    }
                }
            }

            if (Xformable.GetPrim().IsA<pxr::UsdLuxDomeLight>())
            {
                if (GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->y)
                {
                    USDTransform.SetRotation(USDTransform.GetRotation() * FQuat(FVector(1, 0, 0), HALF_PI));
                }
            }

            auto SetRotationAndScaleWithAxis = [Xformable](const pxr::TfToken& MeshAxis, FTransform& Transform)
            {
                auto Scale = Transform.GetScale3D();
                auto UpAxis = pxr::UsdGeomGetStageUpAxis(Xformable.GetPrim().GetStage());
                if (MeshAxis == pxr::UsdGeomTokens->x)
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(0, 1, 0), -HALF_PI));
                    // swap x and z
                    Swap(Scale.X, Scale.Z);
                }
                else if (MeshAxis == pxr::UsdGeomTokens->y && UpAxis == pxr::UsdGeomTokens->z)
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(1, 0, 0), -HALF_PI));
                    // swap y and z
                    Swap(Scale.Y, Scale.Z);
                }
                else if (MeshAxis == pxr::UsdGeomTokens->z && UpAxis == pxr::UsdGeomTokens->y)
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(1, 0, 0), HALF_PI));
                    // swap y and z
                    Swap(Scale.Y, Scale.Z);
                }

                Transform.SetScale3D(Scale);
            };

            // Reassign the scale for Basic USD shape
            float UnitScale = UnitScaleFromUSDToUE(Xformable.GetPrim().GetStage()) * 0.01f;
            if (auto USDSphere = pxr::UsdGeomSphere(Xformable))
            {
                float Radius = GetUSDValue<double>(USDSphere.GetRadiusAttr());
                USDTransform.SetScale3D(USDTransform.GetScale3D() / FVector(Radius * 2.0f * UnitScale));
            }
            else if (auto USDCube = pxr::UsdGeomCube(Xformable))
            {
                float Size = GetUSDValue<double>(USDCube.GetSizeAttr());
                USDTransform.SetScale3D(USDTransform.GetScale3D() / FVector(Size * UnitScale));
            }
            else if (auto USDCone = pxr::UsdGeomCone(Xformable))
            {
                float Radius = GetUSDValue<double>(USDCone.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCone.GetHeightAttr());
                USDTransform.SetScale3D(USDTransform.GetScale3D() / (FVector(Radius * 2, Radius * 2, Height) * UnitScale));
                SetRotationAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCone.GetAxisAttr()), USDTransform);
            }
            else if (auto USDCylinder = pxr::UsdGeomCylinder(Xformable))
            {
                float Radius = GetUSDValue<double>(USDCylinder.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCylinder.GetHeightAttr());
                USDTransform.SetScale3D(USDTransform.GetScale3D() / (FVector(Radius * 2, Radius * 2, Height) * UnitScale));
                SetRotationAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCylinder.GetAxisAttr()), USDTransform);
            }
            else if (auto USDCapsule = pxr::UsdGeomCapsule(Xformable))
            {
                float Radius = GetUSDValue<double>(USDCapsule.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCapsule.GetHeightAttr());
                USDTransform.SetScale3D(USDTransform.GetScale3D() / (FVector(Radius * 4, Radius * 4, Height * 2) * UnitScale));
                SetRotationAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCapsule.GetAxisAttr()), USDTransform);
            }

            if (!USDTransform.Equals(FTransform::Identity) || Xformable.GetXformOpOrderAttr())
            {
                FTransform AttachRelTransform = FTransform::Identity;
                if (bValidRootComponent)
                {
                    AttachRelTransform = GetRelativeTransformFromAttachActorInSequence(SceneComp.GetOwner(), timeCode.GetValue());
                }

                FTransform TransformRH = RHSTransformConvert(Stage, USDTransform * AttachRelTransform, false);
                if (bInheritParent)
                {
                    // Has the relative transform time samples from the parent
                    if (ParentTransformTimeSamples.HasAnyTimeSamples())
                    {
                        // No time samples for child, extend samples from the parent
                        if (timeCode == pxr::UsdTimeCode::Default())
                        {
                            auto TraverseTransform = [&](int32 Time, const FTransform& LocalTransform)
                            {
                                setLocalTransformMatrix(Xformable, TransformRH * LocalTransform, pxr::UsdTimeCode(Time),
                                    true, true, true);
                            };

                            ParentTransformTimeSamples.Traverse(TraverseTransform);
                        }
                        else
                        {
                            // get the same time sample from the parent to apply with the child
                            auto ParentTransform = ParentTransformTimeSamples.Transform.Find(timeCode.GetValue());
                            check(ParentTransform)
                            setLocalTransformMatrix(Xformable, TransformRH * (*ParentTransform), timeCode,
                                bTranslateTimeSamples, bRotateTimeSamples, bScaleTimeSamples,
                                bIgnoreTranslate, bIgnoreRotate, bIgnoreScale);
                        }
                    }
                    else
                    {
                        // neither parent nor child has the time sample
                        setLocalTransformMatrix(Xformable, TransformRH * DefaultParentTransform, timeCode,
                            bTranslateTimeSamples, bRotateTimeSamples, bScaleTimeSamples,
                            bIgnoreTranslate, bIgnoreRotate, bIgnoreScale);
                    }
                }
                else
                {
                    setLocalTransformMatrix(Xformable, TransformRH, timeCode,
                        bTranslateTimeSamples, bRotateTimeSamples, bScaleTimeSamples,
                        bIgnoreTranslate, bIgnoreRotate, bIgnoreScale);
                }
            }
        };

        const FTranslateRotateScaleTimeSamples* TransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(&SceneComp);
        if (bValidRootComponent
        && (TransformTimeSamples == nullptr || !TransformTimeSamples->HasAnyTimeSamples()))
        {
            TransformTimeSamples = FSequenceExtractor::Get().GetTransformTimeSamples(SceneComp.GetOwner());
        }

        const FTransformTimeSamples* SocketTransformTimeSamples = nullptr;
        if (SceneComp.GetAttachParent() && !SceneComp.GetAttachSocketName().IsNone())
        {
            SocketTransformTimeSamples = FSequenceExtractor::Get().GetSocketTimeSamples(SceneComp.GetAttachParent(), SceneComp.GetAttachSocketName());
        }

        if (TransformTimeSamples == nullptr || !TransformTimeSamples->HasAnyTimeSamples())
        {
            if (SocketTransformTimeSamples)
            {
                for (const TPair<int32, FTransform>& Element : SocketTransformTimeSamples->Transform)
                {
                    if (IsInRange(Element.Key, Ranges))
                    {
                        SetPrimTransform(RelTransform * Element.Value, pxr::UsdTimeCode(Element.Key), true, true, true, false, false, false);
                    }
                }
            }
            else
            {
                if (!bInheritParent && Ranges)
                {
                    auto RangeTimeCodes = GetTimeCodesFromRange(Ranges);
                    for(auto RangeTimeCode : RangeTimeCodes)
                    {
                        SetPrimTransform(RelTransform, pxr::UsdTimeCode(RangeTimeCode), true, true, true, false, false, false);
                    }
                }
                else
                {
                    SetPrimTransform(RelTransform, pxr::UsdTimeCode::Default(), false, false, false, false, false, false);
                }
            }
        }
        else
        {
            for (int32 TimeCodeIndex = FSequenceExtractor::Get().GetMasterStartTimeCode(); TimeCodeIndex <= FSequenceExtractor::Get().GetMasterEndTimeCode(); ++TimeCodeIndex)
            {
                if (IsInRange(TimeCodeIndex, Ranges))
                {
                    FTransform NewRelTransform = RelTransform;
                    auto Translate = TransformTimeSamples->Translate.Find(TimeCodeIndex);
                    auto Rotate = TransformTimeSamples->Rotate.Find(TimeCodeIndex);
                    auto Scale = TransformTimeSamples->Scale.Find(TimeCodeIndex);

                    if (Translate)
                    {
                        NewRelTransform.SetLocation(*Translate);
                    }
                    if (Rotate)
                    {
                        NewRelTransform.SetRotation(FRotator(Rotate->Y, Rotate->Z, Rotate->X).Quaternion());
                    }
                    if (Scale)
                    {
                        NewRelTransform.SetScale3D(*Scale);
                    }

                    if (SocketTransformTimeSamples && SocketTransformTimeSamples->Transform.Find(TimeCodeIndex))
                    {
                        SetPrimTransform(NewRelTransform * SocketTransformTimeSamples->Transform[TimeCodeIndex], pxr::UsdTimeCode(TimeCodeIndex), true, true, true, false, false, false);
                    }
                    else
                    {
                        SetPrimTransform(NewRelTransform, pxr::UsdTimeCode(TimeCodeIndex), true, true, true, false, false, false);
                    }
                }
            }
        }
    }

    // Save children
    if (!bSkipChildren && (Changes.Find("") >= 0 || Changes.Num() <= 0))
    {
        TArray<USceneComponent*> Components;
        SceneComp.GetChildrenComponents(false, Components);
        for(auto ChildComp : Components)
        {
            if(ChildComp)
            {
                ExportSceneObject(*ChildComp);
            }
        }
    }

    return true;
}

void SetQcode(const pxr::UsdPrim& Prim, const std::string& QcodeData)
{
    // Apply API schema
    pxr::SdfTokenListOp TokenListOp;
    Prim.GetMetadata(pxr::UsdTokens->apiSchemas, &TokenListOp);

    if (TokenListOp.IsExplicit())
    {
        pxr::TfTokenVector ExplicitItems = TokenListOp.GetExplicitItems();
        ExplicitItems.push_back(SimReadyTokens::SemanticsApiWikidataQcode);
        ExplicitItems.push_back(SimReadyTokens::SemanticsLabelsApiWikidataQcode);
        TokenListOp.SetExplicitItems(ExplicitItems);
    }
    else
    {
        pxr::TfTokenVector PrependedItems = TokenListOp.GetPrependedItems();
        PrependedItems.push_back(SimReadyTokens::SemanticsApiWikidataQcode);
        PrependedItems.push_back(SimReadyTokens::SemanticsLabelsApiWikidataQcode);
        TokenListOp.SetPrependedItems(PrependedItems);
    }

    Prim.SetMetadata(pxr::UsdTokens->apiSchemas, TokenListOp);

    // Add Q code value
    static const std::string WikidataQcode = "wikidata_qcode";
    auto QcodeTypeAttr = Prim.CreateAttribute(SimReadyTokens::WikiDataQCodeSemanticType, pxr::SdfValueTypeNames->String);
    QcodeTypeAttr.Set(WikidataQcode);

    auto QcodeDataAttr = Prim.CreateAttribute(SimReadyTokens::WikiDataQCodeSemanticData, pxr::SdfValueTypeNames->String);
    QcodeDataAttr.Set(QcodeData);

    auto QcodeAttr = Prim.CreateAttribute(SimReadyTokens::WikiDataQCode, pxr::SdfValueTypeNames->TokenArray, true, pxr::SdfVariabilityVarying);
    QcodeAttr.Set(pxr::VtTokenArray{ pxr::TfToken(QcodeData) });
}

void SetNonVisualTags(const pxr::UsdPrim& Prim, const pxr::TfToken& Base, const pxr::VtTokenArray& Attributes, const pxr::TfToken& Coating)
{
    auto EditTarget = Prim.GetStage()->GetEditTarget();

    auto NonVisualAttr = Prim.CreateAttribute(SimReadyTokens::NonVisualAttributes, pxr::SdfValueTypeNames->TokenArray, true);
    NonVisualAttr.Set(Attributes);

    auto NonVisualBaseAttr = Prim.CreateAttribute(SimReadyTokens::NonVisualBase, pxr::SdfValueTypeNames->Token, true);
    if (Base.IsEmpty())
    {
        const static pxr::VtTokenArray AllowedTokens = {
            pxr::TfToken("aluminum"),
            pxr::TfToken("steel"),
            pxr::TfToken("oxidized_steel"),
            pxr::TfToken("iron"),
            pxr::TfToken("oxidized_iron"),
            pxr::TfToken("silver"),
            pxr::TfToken("brass"),
            pxr::TfToken("bronze"),
            pxr::TfToken("oxidized_Bronze_Patina"),
            pxr::TfToken("tin"),
            pxr::TfToken("plastic"),
            pxr::TfToken("fiberglass"),
            pxr::TfToken("carbon_fiber"),
            pxr::TfToken("vinyl"),
            pxr::TfToken("plexiglass"),
            pxr::TfToken("pvc"),
            pxr::TfToken("nylon"),
            pxr::TfToken("polyester"),
            pxr::TfToken("clear_glass"),
            pxr::TfToken("frosted_glass"),
            pxr::TfToken("one_way_mirror"),
            pxr::TfToken("mirror"),
            pxr::TfToken("ceramic_glass"),
            pxr::TfToken("asphalt"),
            pxr::TfToken("concrete"),
            pxr::TfToken("leaf_grass"),
            pxr::TfToken("dead_leaf_grass"),
            pxr::TfToken("rubber"),
            pxr::TfToken("wood"),
            pxr::TfToken("bark"),
            pxr::TfToken("cardboard"),
            pxr::TfToken("paper"),
            pxr::TfToken("fabric"),
            pxr::TfToken("skin"),
            pxr::TfToken("fur_hair"),
            pxr::TfToken("leather"),
            pxr::TfToken("marble"),
            pxr::TfToken("brick"),
            pxr::TfToken("stone"),
            pxr::TfToken("gravel"),
            pxr::TfToken("dirt"),
            pxr::TfToken("mud"),
            pxr::TfToken("water"),
            pxr::TfToken("salt_water"),
            pxr::TfToken("snow"),
            pxr::TfToken("ice"),
            pxr::TfToken("calibration_lambertian") };

        
        auto PropSpec = EditTarget.GetPropertySpecForScenePath(NonVisualBaseAttr.GetPath());
        auto AttrSpec = pxr::SdfSpecDynamic_cast<pxr::SdfAttributeSpecHandle>(PropSpec);
        if (AttrSpec)
        {
            AttrSpec->SetAllowedTokens(AllowedTokens);
        }

        UE_LOG(LogSimReadyUsd, Warning, TEXT("Couldn't determine non-visual material attributes for material %s in stage %s. You need to set it in USD after exporting."), *FString(Prim.GetPath().GetText()), *FString(Prim.GetStage()->GetRootLayer()->GetIdentifier().c_str()));
    }
    else
    {
        NonVisualBaseAttr.Set(Base);
    }

    auto NonVisualCoatingAttr = Prim.CreateAttribute(SimReadyTokens::NonVisualCoating, pxr::SdfValueTypeNames->Token, true);
    NonVisualCoatingAttr.Set(Coating);
    if (Coating == SimReadyTokens::None)
    {
        auto PropSpec = EditTarget.GetPropertySpecForScenePath(NonVisualCoatingAttr.GetPath());
        auto AttrSpec = pxr::SdfSpecDynamic_cast<pxr::SdfAttributeSpecHandle>(PropSpec);
        if (AttrSpec)
        {
            const static pxr::VtTokenArray AllowedTokens = {
            pxr::TfToken("none"),
            pxr::TfToken("paint"),
            pxr::TfToken("clearcoat"),
            pxr::TfToken("paint_clearcoat")
            };

            AttrSpec->SetAllowedTokens(AllowedTokens);
        }
    }
}

void SetNonVisualTags(const pxr::UsdPrim& Prim, const pxr::TfToken& Base)
{
    SetNonVisualTags(Prim, Base, pxr::VtTokenArray{ SimReadyTokens::None }, SimReadyTokens::None);
}

void ExportCarlaTag(const pxr::UsdPrim& Prim, const UObject* Asset, const UActorComponent* Component)
{
    // Get asset folder name
    check(Prim);
    if (auto StaticMeshComp = Cast<UStaticMeshComponent>(Component))
    {
        Asset = StaticMeshComp->GetStaticMesh();
    }
    else if (auto SkelMeshComp = Cast<USkeletalMeshComponent>(Component))
    {
        Asset = SkelMeshComp->GetPhysicsAsset();
    }

    if (!Asset)
    {
        return;
    }

    FString PathName;
    PathName = Asset->GetPathName();

    TArray<FString> StringArray;
    PathName.ParseIntoArray(StringArray, TEXT("/"), false);
    if (StringArray.Num() <= 4)
        return;

    auto FolderName = StringArray[4];

    // Map asset folder name to Wiki Q code.
    static const std::map<std::string, std::string> FolderNameToQCode = {
        {"Road", "Q34442"},
        {"SideWalk", "Q177749"},
        {"Building", "Q41176"},
        {"Wall", "Q42948"},
        {"Fence", "Q148571"},
        {"Pole", "Q2180428"},
        {"TrafficLight", "Q8004"},
        {"TrafficSign", "Q170285"},
        {"Vegetation", "Q187997"},
        {"Terrain", "Q186131"},
        {"Sky", "Q527"},
        {"Pedestrian", "Q221488"},
        {"Rider", "Q11998775"},
        {"Car", "Q1420"},
        {"Truck", "Q43193"},
        {"Bus", "Q5638"},
        {"Train", "Q870"},
        {"Motorcycle", "Q34493"},
        {"Bicycle", "Q11442"},
        {"Static", "Q2302426"},
        {"Dynamic", "Q2302426"},
        {"Other", "Q55107540"},
        {"Water", "Q283"},
        {"RoadLine", "Q1392287"},
        {"Ground", "Q36133"},
        {"Bridge", "Q12280"},
        {"RailTrack", "Q57498564"},
        {"GuardRail", "Q1188866"},
    };

    if (Component)
    {
        auto CarlaWheeledVehicleClass = FindObject<UClass>(ANY_PACKAGE, TEXT("CarlaWheeledVehicle"));
        check(CarlaWheeledVehicleClass);
        if (FolderName == "Pedestrian" && Component->GetOwner()->IsA(CarlaWheeledVehicleClass))
        {
            FolderName = "Rider";
        }
    }

    auto Itr = FolderNameToQCode.find(TCHAR_TO_ANSI(*FolderName));
    if (Itr == FolderNameToQCode.end())
    {
        UE_LOG(LogSimReadyUsd, Error, TEXT("Can't map asset folder name %s to Q code for object %s"), *FolderName, *Asset->GetFullName());
        return;
    }

    SetQcode(Prim, Itr->second);
}

bool GetAssetFolderName(const UPrimitiveComponent& Component, FString& FolderName)
{
    // Get asset folder name
    FString PathName;
    if (auto StaticMeshComp = Cast<UStaticMeshComponent>(&Component))
    {
        PathName = StaticMeshComp->GetStaticMesh()->GetPathName();
    }
    else if (auto SkelMeshComp = Cast<USkeletalMeshComponent>(&Component))
    {
        PathName = SkelMeshComp->GetPhysicsAsset()->GetPathName();
    }
    else
    {
        return false;
    }

    TArray<FString> StringArray;
    PathName.ParseIntoArray(StringArray, TEXT("/"), false);
    if (StringArray.Num() <= 4)
    {
        return false;
    }

    FolderName = StringArray[4];
    return true;
}

bool FindKeyWordInMaterial(const UMaterialInterface& IMaterial, const FString& Word)
{
    // Collect path words of the material
    TSet<FString> PathWords;

    TArray<FString> PathArray;
    IMaterial.GetPathName().ParseIntoArray(PathArray, TEXT("/"));
    PathWords.Append(PathArray);
    PathWords.Add(IMaterial.GetName());

    IMaterial.GetMaterial()->GetPathName().ParseIntoArray(PathArray, TEXT("/"));
    PathWords.Append(PathArray);
    PathWords.Add(IMaterial.GetMaterial()->GetName());

    // Check if the path contains a specific key word
    for (auto PathWord : PathWords)
    {
        auto Index = PathWord.Find(Word);
        if (Index == INDEX_NONE)
        {
            continue;
        }

        // Check previouse and post letters
        if (Index > 0 && !FChar::IsUpper(PathWord[Index]))
        {
            auto Pre = PathWord[Index - 1];
            if (FChar::IsLower(Pre))
            {
                continue;
            }
        }

        if (Index + Word.Len() < PathWord.Len())
        {
            auto Post = PathWord[Index + Word.Len()];
            if (FChar::IsLower(Post))
            {
                continue;
            }
        }

        return true;
    }

    return false;
}

void ExportSimReadyNonVisMaterialDataForPedestrian(const pxr::UsdPrim& MeshPrim, const UMaterialInterface& IMaterial)
{
    // Check if it's a pedestrian
    bool IsPedestrian = false;
    for (auto Prim = MeshPrim; Prim; Prim = Prim.GetParent())
    {
        auto Attr = Prim.GetAttribute(SimReadyTokens::WikiDataQCode);
        if (!Attr)
        {
            continue;
        }

        pxr::VtTokenArray TokenArray;
        Attr.Get(&TokenArray);
        for (auto Token : TokenArray)
        {
            if (Token == "Q11998775" || Token == "Q221488")
            {
                IsPedestrian = true;
                break;
            }
        }

        if (IsPedestrian)
        {
            break;
        }
    }

    if (!IsPedestrian)
    {
        return;
    }

    // The map contains categories and corresponding key words
    TMap<const pxr::TfToken*, TArray<FString>> CategoryMap = {
        {&SimReadyTokens::Skin, {"skin", "lacrimal", "eye", "eyes"}},
        {&SimReadyTokens::Fabric, {"fabric", "tshirt", "trousers", "shoes", "cloth", "pants"}},
        {&SimReadyTokens::FurHair, {"hair", "eyebrow"}}
    };

    // Determine material categoray by key word in material's path
    const pxr::TfToken* Category = nullptr;
    for (auto& Item : CategoryMap)
    {
        for (auto Word : Item.Get<1>())
        {
            if (FindKeyWordInMaterial(IMaterial, Word))
            {
                Category = Item.Get<0>();
                break;
            }
        }

        if (Category)
        {
            break;
        }
    }

    // Write categoray information to USD
    auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI(MeshPrim);
    auto Rel = USDMaterialBinding.GetDirectBindingRel();
    pxr::SdfPathVector Paths;
    Rel.GetTargets(&Paths);
    if (Paths.empty())
    {
        return;
    }

    auto MaterialPrim = MeshPrim.GetStage()->GetPrimAtPath(Paths[0]);
    if (!MaterialPrim)
    {
        return;
    }

    SetNonVisualTags(MaterialPrim, Category ? *Category : pxr::TfToken());
}

void ExportSimReadyEnvironmentNonVisualTag(const pxr::UsdPrim& Prim, const UPrimitiveComponent& Component, const UMaterialInterface& IMaterial, const pxr::UsdPrim& MaterailPrim)
{
    // Map asset folder name to Base Materials.
    static const std::map<std::string, pxr::TfToken> FolderNameToBase = {
        {"Road", SimReadyTokens::Asphalt},
        {"Building", SimReadyTokens::Concrete},
        {"Wall", SimReadyTokens::Brick},
        {"Fence", SimReadyTokens::Wood},
        {"Pole", SimReadyTokens::Aluminum},
        {"TrafficLight", SimReadyTokens::Plexiglass},
        {"TrafficSign", SimReadyTokens::Aluminum},
        {"Vegetation", SimReadyTokens::Bark},
        //{"Terrain", SimReadyTokens::None},
        {"Sky", SimReadyTokens::None},
        {"Pedestrian", SimReadyTokens::Fabric},
        {"Rider", SimReadyTokens::Fabric},
        {"Car", SimReadyTokens::Aluminum},
        {"Truck", SimReadyTokens::Aluminum},
        {"Bus", SimReadyTokens::Aluminum},
        {"Train", SimReadyTokens::Aluminum},
        {"Motorcycle", SimReadyTokens::Aluminum},
        {"Bicycle", SimReadyTokens::Aluminum},
        {"Static", SimReadyTokens::Plastic},
        {"Dynamic", SimReadyTokens::Plastic},
        //{"Other", SimReadyTokens::None},
        {"Water", SimReadyTokens::Water},
        {"RoadLine", SimReadyTokens::Asphalt},
        {"Ground", SimReadyTokens::Gravel},
        {"Bridge", SimReadyTokens::Steel},
        {"RailTrack", SimReadyTokens::Steel},
        {"GuardRail", SimReadyTokens::Wood},
    };

    pxr::TfToken MaterialBase;
    FString FolderName;
    if (GetAssetFolderName(Component, FolderName))
    {
        auto Itr = FolderNameToBase.find(TCHAR_TO_ANSI(*FolderName));
        if (Itr != FolderNameToBase.end())
        {
            MaterialBase = Itr->second;
        }
    }

    pxr::VtTokenArray Attributes = { SimReadyTokens::None };
    pxr::TfToken Coating = SimReadyTokens::None;

    if (FolderName == "RoadLine")
    {
        Attributes = { SimReadyTokens::Retroreflective };
        Coating = SimReadyTokens::Paint;
    }
    else if (Component.IsA<USplineMeshComponent>())
    {
        auto Blueprint = Cast<UBlueprint>(Component.GetOwner()->GetClass()->ClassGeneratedBy);
        if (Blueprint)
        {
            if (Blueprint->GetPathName().StartsWith(TEXT("/Game/Carla/Static/Pole/PoweLine/")))
            {
                MaterialBase = SimReadyTokens::Aluminum;
            }
        }
    }
    else if (MaterialBase.IsEmpty())
    {
        static const TMap<FString, pxr::TfToken> MaterialBaseMap = {
            {"Grass", pxr::TfToken("leaf_grass")},
            {"Sidewalk", SimReadyTokens::Concrete},
            {"Papers", pxr::TfToken("paper")},
            {"Asphalt", SimReadyTokens::Asphalt},
            {"Gutter", SimReadyTokens::Concrete},
            {"Curb", SimReadyTokens::Concrete},
            {"Banner", pxr::TfToken("polyester")},
            {"SkatePark", SimReadyTokens::Concrete},
            {"SkatePool", SimReadyTokens::Concrete},
            {"BasketCourt", SimReadyTokens::Concrete},
            {"Access", SimReadyTokens::Concrete},
            {"tilesOLD", SimReadyTokens::Brick},
            {"Red", SimReadyTokens::Fabric},
            {"Dirt", SimReadyTokens::Dirt},
            {"Mountain", SimReadyTokens::Stone},
            {"Interurban", SimReadyTokens::Gravel},
            {"Rural", pxr::TfToken("dead_leaf_grass")},
            {"LaneMarking", SimReadyTokens::Asphalt},
            {"Comunity", SimReadyTokens::Asphalt},
            // Engine
            {"WorldGridMaterial", SimReadyTokens::Plastic},
        };

        for (auto& Item : MaterialBaseMap)
        {
            if (FindKeyWordInMaterial(IMaterial, Item.Get<0>()))
            {
                MaterialBase = Item.Get<1>();
                break;
            }
        }

        if (MaterialBase.IsEmpty())
        {
            if (Component.GetOwner()->GetName().Contains("_crosswalk_"))
            {
                MaterialBase = SimReadyTokens::Asphalt;
            }
        }
    }

    if (MaterialBase.IsEmpty())
    {
        FSimReadyAssetExportHelper::LogMessage(&IMaterial, FSimReadyExportMessageSeverity::Error, TEXT("omni:simready:nonvisual:base is empty, need to edit manually."));
    }

    SetNonVisualTags(MaterailPrim, MaterialBase, Attributes, Coating);
}

void ExportSimReadyNonVisMaterialDataForGeneralObject(const pxr::UsdPrim& MeshPrim, const UPrimitiveComponent& Component, const UMaterialInterface& IMaterial)
{
    auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI(MeshPrim);
    pxr::UsdShadeMaterial Material = USDMaterialBinding.ComputeBoundMaterial();

    if (!Material)
    {
        return;
    }

    auto MaterialPrim = Material.GetPrim();
    if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(&Component))
    {
        // Transparent material, mark as glass
        if (IsTranslucentBlendMode(IMaterial.GetBlendMode()))
        {
            FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualGlass(MaterialPrim);
        }
        else if (FUSDCARLAVehicleTools::IsCarlaVehicleSirenLightMaterial(&IMaterial))
        {
            FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualGlass(MaterialPrim);
        }
        else
        {
            FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualMaterial(MaterialPrim, IMaterial);
        }
    }
    else
    {
        ExportSimReadyEnvironmentNonVisualTag(MeshPrim, Component, IMaterial, MaterialPrim);
    }
}

pxr::UsdGeomImageable FUSDExporter::ExportSceneObject(USceneComponent& SceneComp)
{
    // Save different types of object
    pxr::UsdGeomImageable Imageable;

    auto ExportSimReadyData = [&]() {
        if (!Imageable)
        {
            return;
        }

        if (SceneComp.GetOwner()->GetName().Contains("_crosswalk_"))
        {
            if (SceneComp.GetOwner()->GetRootComponent() == &SceneComp)
            {
                SetQcode(Imageable.GetPrim(), "Q1392287");	// Road mark
            }

        }

        ExportSimReadyTimeOfDayInfo(Imageable.GetPrim(), SceneComp);

        if (auto MeshComp = Cast<UStaticMeshComponent>(&SceneComp))
        {
            ExportSimReadyTrafficLightColor(Imageable.GetPrim(), *MeshComp);
        }
        };

    if(CustomExport)
    {
        bool bContinue = true;
        Imageable = CustomExport(SceneComp, bContinue);
        if(!bContinue)
        {
            ExportSimReadyData();
            return Imageable;
        }
    }

    if (SceneComp.GetOwner() && SceneComp.GetOwner()->GetRootComponent() == &SceneComp && SceneComp.GetOwner()->IsA<ACameraActor>())
    {
        Imageable = ExportCameraActor(StaticCast<ACameraActor&>(*SceneComp.GetOwner()));
    }
    else if (IsPointInstancer(SceneComp))
    {
        Imageable = ExportPointInstancer(SceneComp);
    }
    else if(auto InstancedMeshComponent = Cast<UInstancedStaticMeshComponent>(&SceneComp))
    {
        // Must call before StaticMeshComponent
        Imageable = ExportInstancedStaticMeshComponent(*InstancedMeshComponent);
    }
    else if(auto CameraComponent = Cast<UCameraComponent>(&SceneComp))
    {
        Imageable = ExportCameraComponent(*CameraComponent);
    }
    else if(auto LightComponent = Cast<ULightComponentBase>(&SceneComp))
    {
        Imageable = ExportLightComponent(*LightComponent);
    }
    else if(auto MeshComponent = Cast<UStaticMeshComponent>(&SceneComp))
    {
        Imageable = ExportStaticMeshComponent(*MeshComponent);
    }
    else if(auto SkeletalMeshComp = Cast<USkeletalMeshComponent>(&SceneComp))
    {
        Imageable = ExportSkeletalMeshComponent(*SkeletalMeshComp);
    }
    else if(auto ParticleComp = Cast<UParticleSystemComponent>(&SceneComp))
    {
        Imageable = ExportParticleSystemComponent(*ParticleComp);
    }
    else
    {
        Imageable = ExportSceneComponent(SceneComp);
    }

    ExportSimReadyData();

    return Imageable;
}

bool FUSDExporter::HasValidChildren(const USceneComponent& SceneComp)
{
    if (SceneComp.GetNumChildrenComponents() == 0)
    {
        return false;
    }

    for (auto ChildComp : SceneComp.GetAttachChildren())
    {
        if ((ChildComp && ChildComp->IsVisible()) || (FSequenceExtractor::Get().GetVisibilityTimeSamples(ChildComp) != nullptr))
        {
            return true;
        }
    }

    return false;
}

pxr::UsdGeomImageable FUSDExporter::ExportSceneComponent(USceneComponent& SceneComp)
{
    pxr::UsdGeomImageable Imageable;
    auto Path = PathToObject.FindKey((UObject*)&SceneComp);
    if (Path)
    {
        Imageable = pxr::UsdGeomXformable(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
    }
    else
    {
        // Create prim
        // Lock location actor will be exported as Usd scope
        if (SceneComp.GetOwner()->bLockLocation)
        {
            Imageable = DefineNewPrim<pxr::UsdGeomScope>(SceneComp);
        }
        else
        {
            Imageable = DefineNewPrim<pxr::UsdGeomXform>(SceneComp);
        }

        if (Imageable)
        {
            pxr::UsdModelAPI(Imageable).SetKind(pxr::KindTokens->group);
        }
    }
    
    if (Imageable)
    {
        ExportSceneComponent(Imageable, SceneComp);
    }

    return Imageable;
}

pxr::UsdGeomXformable FUSDExporter::ExportLightComponent(ULightComponentBase& LightComp)
{
    bool bHasChildren = HasValidChildren(LightComp);

    pxr::UsdGeomXformable USDLight;
    pxr::UsdPrim UELight;

    if (bHasChildren)
    {
        auto Path = PathToObject.FindKey((UObject*)&LightComp);
        if (Path)
        {
            USDLight = pxr::UsdGeomXformable(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
        }
        else
        {
            USDLight = DefineNewPrim<pxr::UsdGeomXform>(LightComp);
        }

        if(!USDLight)
        {
            return USDLight;
        }

        if (SimReadyUsdLuxLightCompat::PrimIsALight(USDLight.GetPrim()))
        {
            UELight = USDLight.GetPrim();
        }
        else
        {
            if(auto DirLight = Cast<UDirectionalLightComponent>(&LightComp))
            {
                UELight = pxr::UsdLuxDistantLight::Define(Stage, USDLight.GetPath().AppendElementString("DistantLight")).GetPrim();;
            }
            else if(auto PointLight = Cast<UPointLightComponent>(&LightComp))
            {
                UELight = pxr::UsdLuxSphereLight::Define(Stage, USDLight.GetPath().AppendElementString("SphereLight")).GetPrim();;
            }
            else if(auto RectLight = Cast<URectLightComponent>(&LightComp))
            {
                UELight = pxr::UsdLuxRectLight::Define(Stage, USDLight.GetPath().AppendElementString("RectLight")).GetPrim();;
            }
            else if (auto SkyLight = Cast<USkyLightComponent>(&LightComp))
            {
                UELight = pxr::UsdLuxDomeLight::Define(Stage, USDLight.GetPath().AppendElementString("DomeLight")).GetPrim();;
            }
            else
            {
                // Unhandled light type
                FString Name = LightComp.GetFullName();
                UE_LOG(LogSimReadyUsd, Log, TEXT("unhandled light %s (%s)"), *Name);
            }
        }

        if(!UELight)
        {
            return USDLight;
        }
    }
    else
    {
        auto Path = PathToObject.FindKey((UObject*)&LightComp);
        if (Path)
        {
            USDLight = pxr::UsdGeomXformable(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
        }
        else
        {
            if(auto DirLight = Cast<UDirectionalLightComponent>(&LightComp))
            {
                USDLight = DefineNewPrim<pxr::UsdLuxDistantLight>(LightComp);
            }
            else if(auto PointLight = Cast<UPointLightComponent>(&LightComp))
            {
                USDLight = DefineNewPrim<pxr::UsdLuxSphereLight>(LightComp);
            }
            else if(auto RectLight = Cast<URectLightComponent>(&LightComp))
            {
                USDLight = DefineNewPrim<pxr::UsdLuxRectLight>(LightComp);
            }
            else if (auto SkyLight = Cast<USkyLightComponent>(&LightComp))
            {
                USDLight = DefineNewPrim<pxr::UsdLuxDomeLight>(LightComp);
            }
            else
            {
                // Unhandled light type
                FString Name = LightComp.GetFullName();
                UE_LOG(LogSimReadyUsd, Log, TEXT("unhandled light %s (%s)"), *Name);
            }
        }

        if(!USDLight)
        {
            return USDLight;
        }
    }

    static const TArray<FString> LightingMembers = 
    {
        GET_MEMBER_NAME_STRING_CHECKED(ULightComponentBase, Intensity),
        GET_MEMBER_NAME_STRING_CHECKED(ULightComponentBase, LightColor),
        GET_MEMBER_NAME_STRING_CHECKED(ULightComponentBase, CastShadows),
        GET_MEMBER_NAME_STRING_CHECKED(ULightComponent, bUseTemperature),
        GET_MEMBER_NAME_STRING_CHECKED(ULightComponent, Temperature),
        GET_MEMBER_NAME_STRING_CHECKED(UDirectionalLightComponent, LightSourceAngle),
        GET_MEMBER_NAME_STRING_CHECKED(ULocalLightComponent, AttenuationRadius),
        GET_MEMBER_NAME_STRING_CHECKED(ULocalLightComponent, IntensityUnits),
        GET_MEMBER_NAME_STRING_CHECKED(UPointLightComponent, SourceLength),
        GET_MEMBER_NAME_STRING_CHECKED(UPointLightComponent, SourceRadius),
        GET_MEMBER_NAME_STRING_CHECKED(USpotLightComponent, InnerConeAngle),
        GET_MEMBER_NAME_STRING_CHECKED(USpotLightComponent, OuterConeAngle),
        GET_MEMBER_NAME_STRING_CHECKED(URectLightComponent, SourceWidth),
        GET_MEMBER_NAME_STRING_CHECKED(URectLightComponent, SourceHeight),
    };

    if(ShouldSaveMember(LightingMembers))
    {
        if (bHasChildren)
        {
            ConvertUSDLight(LightComp, UELight);
        }
        else
        {
            ConvertUSDLight(LightComp, USDLight.GetPrim());
        }
    }

    if (bHasChildren)
    {
        FTransform LightTransform = FTransform::Identity;
        if (GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z)
        {
            LightTransform.SetRotation(LightTransform.GetRotation() * FQuat(FVector(0, 1, 0), -HALF_PI) * FQuat(FVector(0, 0, 1), HALF_PI));
        }
        else
        {
            LightTransform.SetRotation(LightTransform.GetRotation() * FQuat(FVector(0, 0, 1), HALF_PI));
        }

        setLocalTransformMatrix(pxr::UsdGeomXformable(UELight), RHSTransformConvert(Stage, LightTransform, false));
    }

    ExportSceneComponent(USDLight, LightComp);

    if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(&LightComp))
    {
        FUSDCARLAVehicleTools::ExportSimReadyVehicleLight(USDLight.GetPrim(), LightComp);
    }

    return USDLight;
}

pxr::UsdGeomXformable FUSDExporter::ExportCameraComponent(UCameraComponent& CameraComp)
{
    bool bHasChildren = HasValidChildren(CameraComp);

    pxr::UsdGeomXformable USDCamera;
    pxr::UsdGeomCamera UECamera;

    if (bHasChildren)
    {
        auto Path = PathToObject.FindKey((UObject*)&CameraComp);
        if (Path)
        {
            USDCamera = pxr::UsdGeomXformable(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
        }
        else
        {
            // NOTE: Adding a prim for USD ligiht along -Z axis
            USDCamera = DefineNewPrim<pxr::UsdGeomXform>(CameraComp);
        }

        if(!USDCamera)
        {
            return USDCamera;
        }

        UECamera = pxr::UsdGeomCamera::Define(Stage, USDCamera.GetPath().AppendElementString("Camera"));
        if(!UECamera)
        {
            return USDCamera;
        }
    }
    else
    {
        auto Path = PathToObject.FindKey((UObject*)&CameraComp);
        if (Path)
        {
            USDCamera = pxr::UsdGeomXformable(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
        }
        else
        {
            USDCamera = DefineNewPrim<pxr::UsdGeomCamera>(CameraComp);
        }

        if(!USDCamera)
        {
            return USDCamera;
        }
    }

    ConvertUSDCamera(Stage, bHasChildren ? UECamera.GetPath() : USDCamera.GetPath(), CameraComp, nullptr);
        
    if (bHasChildren)
    {
        FTransform CameraTransform = FTransform::Identity;
        if (GetUSDStageAxis(Stage) == pxr::UsdGeomTokens->z)
        {
            CameraTransform.SetRotation(CameraTransform.GetRotation() * FQuat(FVector(0, 1, 0), -HALF_PI) * FQuat(FVector(0, 0, 1), HALF_PI));
        }
        else
        {
            CameraTransform.SetRotation(CameraTransform.GetRotation() * FQuat(FVector(0, 0, 1), HALF_PI));
        }

        setLocalTransformMatrix(UECamera, RHSTransformConvert(Stage, CameraTransform, false));
    }
        
    ExportSceneComponent(USDCamera, CameraComp);
    ExportSimReadyCameraShot(CameraComp);

    return USDCamera;
}

void FUSDExporter::ExportViewportCameraShot(AActor* CameraActor, const TArray<FTimeSampleRange>& TimeSamples)
{
    if (CameraActor)
    {
        // Copy the camera actor to the Omniverse shot
        pxr::SdfPath CameraShotPath;
        if (RootPrim)
        {
            CameraShotPath = RootPrim.GetPath();
        }
        else
        {
            CameraShotPath = pxr::SdfPath::AbsoluteRootPath();
        }
        CameraShotPath = CameraShotPath.AppendElementString(TCHAR_TO_ANSI(*SimReadyCameraShot));

        auto CameraShot = pxr::UsdGeomCamera::Get(Stage, CameraShotPath);
        if (!CameraShot)
        {
            CameraShot = pxr::UsdGeomCamera::Define(Stage, CameraShotPath);
        }
        if (CameraShot)
        {
            UCameraComponent* Camera = NewObject<UCameraComponent>();
            ConvertUSDCamera(Stage, CameraShot.GetPath(), *Camera, &TimeSamples);
            ExportSceneComponent(CameraShot, *CameraActor->GetRootComponent(), true, false, &TimeSamples);
            ExportSceneComponent(CameraShot, *Camera, true, true, &TimeSamples);
            Camera->DestroyComponent();
        }
    }
}

void FUSDExporter::ExportSimReadyCameraShot(UCameraComponent& CameraComponent) 
{
    const TArray<FTimeSampleRange>* CameraShotsTimeSamples = nullptr;
    AActor* ParentActor = CameraComponent.GetOwner();
    CameraShotsTimeSamples = FSequenceExtractor::Get().GetCameraShotsTimeSamples(ParentActor);
    if (CameraShotsTimeSamples)
    {
        // duplicate range into Omniverse Camera Shot
        pxr::SdfPath CameraShotPath;
        if (RootPrim)
        {
            CameraShotPath = RootPrim.GetPath();
        }
        else
        {
            CameraShotPath = pxr::SdfPath::AbsoluteRootPath();
        }
        CameraShotPath = CameraShotPath.AppendElementString(TCHAR_TO_ANSI(*SimReadyCameraShot));
        
        auto CameraShot = pxr::UsdGeomCamera::Get(Stage, CameraShotPath);
        if (!CameraShot)
        {
            CameraShot = pxr::UsdGeomCamera::Define(Stage, CameraShotPath);
        }
        if (CameraShot)
        {
            TArray<USceneComponent*> CameraToRoot;
            USceneComponent* Comp = &CameraComponent;
            CameraToRoot.Add(Comp);
            while (Comp->GetAttachParent())
            {
                Comp = Comp->GetAttachParent();
                CameraToRoot.Add(Comp);
            }

            ConvertUSDCamera(Stage, CameraShot.GetPath(), CameraComponent, CameraShotsTimeSamples);
            const int32 RootIndex = CameraToRoot.Num() - 1;
            for (int32 Index = RootIndex; Index >= 0; --Index)
            {
                ExportSceneComponent(CameraShot, *CameraToRoot[Index], true, (Index == RootIndex) ? false : true, CameraShotsTimeSamples);				
            }
        }
    }
}

pxr::UsdGeomXformable FUSDExporter::ExportCameraActor(ACameraActor& CameraActor)
{
    if (HasValidChildren(*CameraActor.GetCameraComponent()))
    {
        // NOTE: if the camera actor has the children, the Unreal Camera Hierarchy will be kept and adding a prim for USD ligiht along -Z axis
        auto CameraRootPrim = DefineNewPrim<pxr::UsdGeomXform>(*CameraActor.GetRootComponent());
        if (!CameraRootPrim)
        {
            return CameraRootPrim;
        }

        ExportSceneComponent(CameraRootPrim, *CameraActor.GetRootComponent());

        return CameraRootPrim;
    }
    else
    {
        // Combine root component and camera component of camera actor
        auto USDCamera = DefineNewPrim<pxr::UsdGeomCamera>(*CameraActor.GetRootComponent());
        if (!USDCamera)
        {
            return USDCamera;
        }

        ConvertUSDCamera(Stage, USDCamera.GetPath(), *CameraActor.GetCameraComponent(), nullptr);
        ExportSceneComponent(USDCamera, *CameraActor.GetRootComponent(), true);
        ExportSceneComponent(USDCamera, *CameraActor.GetCameraComponent(), true, true);

        ExportSimReadyCameraShot(*CameraActor.GetCameraComponent());

        return USDCamera;
    }
}

pxr::UsdSkelRoot FUSDExporter::ExportSkeletalMeshComponent(USkeletalMeshComponent& SkelMeshComp)
{
    pxr::UsdSkelRoot SkelRoot;

    // get-or-create fully-populated SkelRoot
    if(auto PathKey = PathToObject.FindKey(&SkelMeshComp))
    {
        auto Path = FSimReadyPathHelper::KeyToPrimPath(*PathKey);
        auto Prim = Stage->GetPrimAtPath(ToUSDPath(Path));
        while (Prim && !Prim.IsPseudoRoot())
        {
            if (Prim.IsA<pxr::UsdSkelRoot>())
            {
                SkelRoot = pxr::UsdSkelRoot(Prim);
                break;
            }
            Prim = Prim.GetParent();
        }

        if (SkelRoot)
        {
            if (ShouldSaveMember({ GET_MEMBER_NAME_STRING_CHECKED(UMeshComponent, OverrideMaterials) }))
            {
                pxr::UsdSkelCache USDSkelCache;
                USDSkelCache.Populate(SkelRoot);

                std::vector< pxr::UsdSkelBinding > USDSkeletonBindings;
                USDSkelCache.ComputeSkelBindings(SkelRoot, &USDSkeletonBindings);

                for (auto SkeletonBinding : USDSkeletonBindings)
                {
                    FString SkeletalPath = SkeletonBinding.GetSkeleton().GetPrim().GetParent().GetPath().GetText();
                    if (*Path == SkeletalPath)
                    {
                        int32 MaterialIndex = 0;
                        for (int32 SkinningIndex = 0; SkinningIndex < SkeletonBinding.GetSkinningTargets().size(); ++SkinningIndex)
                        {
                            const pxr::UsdSkelSkinningQuery& SkinningQuery = SkeletonBinding.GetSkinningTargets()[SkinningIndex];
                            pxr::UsdGeomMesh SkinningMesh = pxr::UsdGeomMesh(SkinningQuery.GetPrim());

                            auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(SkinningMesh);
                            if (!USDGeomSubsets.empty())
                            {
                                for (auto USDGeomSubset : USDGeomSubsets)
                                {
                                    ExportMaterial(USDGeomSubset.GetPrim(), SkelMeshComp, MaterialIndex);
                                    ++MaterialIndex;
                                }
                            }
                            else
                            {
                                ExportMaterial(SkinningMesh.GetPrim(), SkelMeshComp, MaterialIndex);
                                ++MaterialIndex;
                            }
                        }
                    }
                }
            }

            ExportSceneComponent(SkelRoot, SkelMeshComp);
        }
    }
    else
    {
        //TODO: skeletal mesh live creating
    }

    return SkelRoot;
}

void FUSDExporter::ExportMeshComponent(const pxr::UsdGeomXformable& USDGprim, UMeshComponent& MeshComp)
{
    if(ShouldSaveMember({GET_MEMBER_NAME_STRING_CHECKED(UMeshComponent, OverrideMaterials)}))
    {
        for(auto SlotIndex = 0; SlotIndex < MeshComp.GetNumMaterials(); ++SlotIndex)
        {
            ExportMaterial(USDGprim.GetPrim(), MeshComp, SlotIndex);
        }
    }

    if(ShouldSaveMember({GET_MEMBER_NAME_STRING_CHECKED(UMeshComponent, CastShadow)}))
    {
        bool bHasAllUnLitMaterials = true;
        const int32 NumMaterials = MeshComp.GetNumMaterials();
        for (int32 MaterialIndex = 0; (MaterialIndex < NumMaterials); ++MaterialIndex)
        {
            UMaterialInterface* Material = MeshComp.GetMaterial(MaterialIndex);

            if (Material)
            {
                if (Material->GetShadingModels().IsLit())
                {
                    bHasAllUnLitMaterials = false;
                    break;
                }
            }
            else
            {
                // Default material is lit
                bHasAllUnLitMaterials = false;
                break;
            }
        }

        if (!MeshComp.CastShadow || (bHasAllUnLitMaterials && (NumMaterials > 0)))
        {
            USDGprim.CreatePrimvar(USDTokens.doNotCastShadows, pxr::SdfValueTypeNames->Bool).Set(true);
        }
    }

    ExportSceneComponent(USDGprim, MeshComp);
}

pxr::UsdGeomXform FUSDExporter::ExportParticleSystemComponent(class UParticleSystemComponent& ParticleComp)
{
    auto USDMesh = DefineNewPrim<pxr::UsdGeomXform>(ParticleComp);
    if(!USDMesh)
    {
        return USDMesh;
    }

    if(Changes.Find("") < 0 && Changes.Num() > 0)
    {
        return USDMesh;
    }

    ExportParticleSystem(Stage, USDMesh.GetPath(), ParticleComp, TimeCode, true, true);

    ExportSceneComponent(USDMesh, ParticleComp);

    return USDMesh;
}

pxr::UsdGeomGprim FUSDExporter::ExportStaticMeshComponent(UStaticMeshComponent& StaticMeshComp)
{
    pxr::UsdGeomGprim USDGprim;
    
    auto Path = PathToObject.FindKey(&StaticMeshComp);
    if(Path)
    {
        USDGprim = pxr::UsdGeomGprim(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*Path))));
    }
    else
    {
        pxr::UsdGeomGprim USDMesh;
        UStaticMesh* StaticMesh = StaticMeshComp.GetStaticMesh();

        if(StaticMeshComp.IsRegistered() && StaticMesh)
        {
            USDMesh = DefineNewPrim<pxr::UsdGeomMesh>(StaticMeshComp);
        }

        if(!USDMesh)
        {
            return USDMesh;
        }

        if((Changes.Find("") >= 0 || Changes.Num() <= 0) && StaticMesh->RenderData->LODResources.Num() > 0)
        {
            const int32 SMCurrentMinLOD = StaticMesh->MinLOD.GetValue();
            int32 LODToExport = FMath::Clamp(StaticMeshComp.bOverrideMinLOD ? StaticMeshComp.MinLOD : SMCurrentMinLOD, 0, StaticMesh->RenderData->LODResources.Num() - 1);

            FColorVertexBuffer* ColorVertexBuffer = (LODToExport < StaticMeshComp.LODData.Num()) ? StaticMeshComp.LODData[LODToExport].OverrideVertexColors : nullptr;
            ExportStaticMesh(Stage, USDMesh.GetPath(), StaticMesh->RenderData->LODResources[LODToExport], ColorVertexBuffer);
        }

        ExportMeshComponent(USDMesh, StaticMeshComp);

        ExportCarlaTag(USDMesh.GetPrim(), nullptr, &StaticMeshComp);

        return USDMesh;
    }

    if(USDGprim)
    {
//#if LIGHTMAPS_EXPORT_PRIMVARS || LIGHTMAPS_EXPORT_SCHEMA
//		if(Changes.Find("") >= 0)
//		{
//			SaveLightmappingInfo(GetUSDStage(), USDMesh.GetPrim(), MeshComp, StageTimeCode, LightMaps);
//		}
//#endif
        ExportMeshComponent(USDGprim, StaticMeshComp);
    }
    else
    {
        // UE4 dome light is a sky sphere mesh
        pxr::UsdLuxDomeLight USDDomelight = pxr::UsdLuxDomeLight(Stage->GetPrimAtPath(ToUSDPath(*Path)));
        if (USDDomelight)
        {
            // Clean the translate and scale, dome light don't need that
            StaticMeshComp.SetRelativeLocation(FVector::ZeroVector);
            StaticMeshComp.SetRelativeScale3D(FVector::OneVector);
            ExportSceneComponent(USDDomelight, StaticMeshComp);
        }
    }

    return USDGprim;
}

void FUSDExporter::ExportMaterial(pxr::UsdPrim Model, class UMeshComponent& MeshComp, int32 SlotIndex)
{
    // Do not live sync material when material loading is disabled
    if (GetDefault<USimReadySettings>()->bDisableMaterialLoading)
    {
        return;
    }	

    FMaterialBinding OldMaterialBinding;
    pxr::UsdGeomSubset Subset;

    if (Model.IsA<pxr::UsdGeomSubset>())
    {
        Subset = pxr::UsdGeomSubset(Model);
    }
    else
    {
        auto GeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(pxr::UsdGeomImageable(Model));
        if (GeomSubsets.size() > 0 && SlotIndex >= 0 && SlotIndex < GeomSubsets.size())
        {
            OldMaterialBinding = ParsePrimMaterial(GeomSubsets[SlotIndex].GetPrim());
            if (OldMaterialBinding.Material)
            {
                Model = GeomSubsets[SlotIndex].GetPrim();
                Subset = GeomSubsets[SlotIndex];
            }
        }
    }
    
    if (!OldMaterialBinding.Material)
    {
        OldMaterialBinding = ParsePrimMaterial(Model);
    }

    // Remove material prim
    auto ClearMaterialBinding = [&]()
    {
        Model.RemoveProperty(pxr::UsdShadeTokens->materialBinding);
        if (Subset)
        {
            Subset.GetFamilyNameAttr().Clear();
        }
        Stage->RemovePrim(OldMaterialBinding.Material.GetPath());
        Stage->RemovePrim(OldMaterialBinding.Shader.GetPath());
        Stage->RemovePrim(OldMaterialBinding.MdlSurfaceShader.GetPath());
        PathToObject.Remove(FSimReadyPathHelper::PrimPathToKey(OldMaterialBinding.Shader.GetPath().GetText()));
        PathToObject.Remove(FSimReadyPathHelper::PrimPathToKey(OldMaterialBinding.MdlSurfaceShader.GetPath().GetText()));
    };

    UMaterialInterface* CurrentMaterial = MeshComp.GetMaterial(SlotIndex);
    if (CurrentMaterial == nullptr)
    {
        ClearMaterialBinding();
        return;
    }

    if (Subset)
    {
        Subset.GetFamilyNameAttr().Set(pxr::UsdShadeTokens->materialBind);
    }

    // Check if it's from Omniverse
    USimReadyMDL* SimReadyMDL = USimReadyAsset::GetSimReadyAsset<USimReadyMDL>(*CurrentMaterial);

    bool bMdlSchema = SimReadyMDL != nullptr;
    bool bPreviewSurface = false;

    // Not from Omniverse. It should be local UE4 material.
    UMaterialInterface* Material = CurrentMaterial;
    UMaterialInstance* MaterialInst = nullptr;
    FString MaterialName = CurrentMaterial->GetName();
    FString UsdMaterialName = CurrentMaterial->GetName();
    auto LocalBaseMDL = USimReadyMDL::GetLocalBaseMaterial(CurrentMaterial);
    bool IsBaseMDL = (LocalBaseMDL != nullptr);
    FString MdlMaterialName = LocalBaseMDL ? LocalBaseMDL->GetName() : TEXT("");
    if (!IsBaseMDL)
    {
        IsBaseMDL = FSimReadyMaterialReparentUtility::FindOmniMaterial(CurrentMaterial, MdlMaterialName, ESimReadyReparentUsage::Export) != INDEX_NONE;
    }
    if(!SimReadyMDL)
    {
        // Get parent material if it's a material instance
        MaterialInst = Cast<UMaterialInstance>(Material);
        if(MaterialInst)
        {
            Material = MaterialInst->Parent;
            if(!Material)
            {
                return;
            }

            SimReadyMDL = USimReadyAsset::GetSimReadyAsset<USimReadyMDL>(*Material);
            
            if (SimReadyMDL)
            {
                MaterialName = Material->GetName();
            }
            else if (IsBaseMDL)
            {
                MaterialName = MdlMaterialName;
            }

            UMaterial* PreviewSurfaceMaterial = LoadObject<UMaterial>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/PreviewSurfaceMaterial")));

            bMdlSchema = MaterialInst->IsA<UMaterialInstanceConstant>() && (SimReadyMDL || IsBaseMDL);
            bPreviewSurface = MaterialInst->IsA<UMaterialInstanceConstant>() && Material == PreviewSurfaceMaterial;
        }
        else if (IsBaseMDL)
        {
            MaterialName = MdlMaterialName;
            bMdlSchema = true;
        }
    }

    // If it uses plugin default material, we just export display color
    if(Material->GetPathName().StartsWith(SimReadyDefaultMaterial))
    {
        ClearMaterialBinding();
        return;
    }

    if (bMdlSchema)
    {
        auto NeedToCreateMaterial = [&](const pxr::UsdShadeShader& Shader, UMaterialInterface* CurrentMaterial) -> bool
        {
            auto SavedMaterial = PathToObject.Find(FSimReadyPathHelper::PrimPathToKey(Shader.GetPath().GetText()));
            return !(SavedMaterial && (*SavedMaterial == CurrentMaterial));
        };

        bool bCreateMaterial = false;
        pxr::UsdShadeShader PrevMdlSurfaceShader = OldMaterialBinding.MdlSurfaceShader;
        pxr::UsdShadeShader PrevSurfaceShader = OldMaterialBinding.Shader;
        // using existed MaterialBinding, checking if the parameters need to be updated.
        pxr::UsdShadeShader MdlSurfaceShader;

        if (!PrevMdlSurfaceShader)
        {
            if (PrevSurfaceShader)
            {
                if (NeedToCreateMaterial(PrevSurfaceShader, CurrentMaterial))
                {
                    bCreateMaterial = true;
                }
                else
                {
                    MdlSurfaceShader = PrevSurfaceShader;
                }
            }
            else
            {
                bCreateMaterial = true;
            }
        }
        else
        {
            if (NeedToCreateMaterial(PrevMdlSurfaceShader, CurrentMaterial))
            {
                bCreateMaterial = true;
            }
            else
            {
                MdlSurfaceShader = PrevMdlSurfaceShader;
            }
        }

        // Add new binding
        if (bCreateMaterial)
        {
            // Check if there's existed material prim for current UE4 material
            auto USDPathKey = PathToObject.FindKey(CurrentMaterial);
            if (USDPathKey)
            {
                auto USDPrim = Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*USDPathKey)));
                if (USDPrim)
                {
                    pxr::UsdShadeMaterial USDMaterial = pxr::UsdShadeMaterial(USDPrim);				
                    if (!USDMaterial)
                    {
                        pxr::UsdShadeShader USDShader(USDPrim);
                        if (USDShader)
                        {
                            USDMaterial = FindShadeMaterial(USDPrim);
                        }
                    }

                    if (!USDMaterial)
                    {
                        return;
                    }

                    auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI::Apply(Model);
                    USDMaterialBinding.Bind(USDMaterial);

                    auto NewMaterialBinding = FUSDConversion::ParsePrimMaterial(Model);
                    MdlSurfaceShader = NewMaterialBinding.MdlSurfaceShader ? NewMaterialBinding.MdlSurfaceShader : NewMaterialBinding.Shader;
                }
            }
            else
            {
                auto DefaultPrim = Stage->GetDefaultPrim();
                if (!DefaultPrim)
                {
                    DefaultPrim = Stage->GetPseudoRoot();
                }
                // create shader
                auto ScopePrimPath = DefaultPrim.GetPath().AppendElementString(pxr::UsdUtilsGetMaterialsScopeName().GetString());
                auto ScopePrim = pxr::UsdGeomScope::Define(Stage, ScopePrimPath);
                auto MaterialPrimPath = ScopePrimPath.AppendElementString(TCHAR_TO_ANSI(*UsdMaterialName));
                static int32 MaterialSuffixIndex = 0;
                while (pxr::UsdShadeMaterial::Get(Stage, MaterialPrimPath))
                {
                    MaterialPrimPath = ScopePrimPath.AppendElementString(TCHAR_TO_ANSI(*(UsdMaterialName + TEXT("_") + FString::FromInt(MaterialSuffixIndex++))));
                }
                auto USDMaterial = pxr::UsdShadeMaterial::Define(Stage, MaterialPrimPath);
                if (!USDMaterial)
                {
                    return;
                }

                // Bind material
                auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI::Apply(Model);
                USDMaterialBinding.Bind(USDMaterial);

                auto ShaderPrimPath = USDMaterial.GetPath().AppendElementString(TCHAR_TO_ANSI(*MaterialName));
                if (pxr::UsdShadeShader::Get(Stage, ShaderPrimPath))
                {
                    ShaderPrimPath = USDMaterial.GetPath().AppendElementString(TCHAR_TO_ANSI(*(MaterialName + TEXT("_mdl"))));
                }
                // nothing found, create new shader
                MdlSurfaceShader = pxr::UsdShadeShader::Define(Stage, ShaderPrimPath);
                if (!MdlSurfaceShader)
                {
                    return;
                }

                // material connect to shader
                USDMaterial.CreateSurfaceOutput(USDTokens.mdl).ConnectToSource(MdlSurfaceShader.CreateOutput(USDTokens.out, pxr::SdfValueTypeNames->Token));
        
                FString MaterialRelativePath;
                if (IsBaseMDL)
                {
                    FString ModuleName;
                    USimReadyMDL::GetMDLModuleByMaterialName(MaterialName, ModuleName);
                    MaterialRelativePath = ModuleName + TEXT(".mdl");
                }
                MdlSurfaceShader.SetSourceAsset(pxr::SdfAssetPath(TCHAR_TO_ANSI(*MaterialRelativePath)), USDTokens.mdl);
                MdlSurfaceShader.SetSourceAssetSubIdentifier(pxr::TfToken(TCHAR_TO_ANSI(*MaterialName)), USDTokens.mdl);
            }
        }

        if (MaterialInst)
        {
            FUSDExporter::ExportMdlSchema(Stage, *MaterialInst, MdlSurfaceShader);
        }
        else
        {
            // create instance and set to component
            auto MaterialInstConst = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Public | EObjectFlags::RF_Transactional);
            MaterialInstConst->SetParentEditorOnly(Material);
            MeshComp.SetMaterial(SlotIndex, MaterialInstConst);
            MaterialInst = MaterialInstConst;
        }

        PathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(MdlSurfaceShader.GetPath().GetText()), MaterialInst);
    }
    else if (bPreviewSurface)
    {
        // do nothing
    }
}

bool FUSDExporter::ExportMaterialInstance(UMaterialInstance& MaterialInstance)
{
    auto USDPathKey = PathToObject.FindKey(&MaterialInstance);
    if(!USDPathKey)
    {
        return false;
    }

    pxr::UsdShadeShader USDShader(Stage->GetPrimAtPath(ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*USDPathKey))));
    if(!USDShader)
    {
        return false;
    }

    static const TArray<FString> ParameterArray = {
        GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, ScalarParameterValues),
        GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, VectorParameterValues),
        GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, TextureParameterValues),
    };

    // for mdl schema and preview surface [UMaterialEditorInstanceConstant]
    if (ShouldSaveMember({ TEXT("ParameterGroups") }))
    {
        const auto ImplementationSource = USDShader.GetImplementationSource();
        if (ImplementationSource == USDTokens.sourceAsset)
        {
            FUSDExporter::ExportMdlSchema(Stage, MaterialInstance, USDShader);
        }
        else if (ImplementationSource == USDTokens.id)
        {
            const auto ShaderID = GetUSDValue<pxr::TfToken>(USDShader.GetIdAttr());
            if (ShaderID == USDTokens.previewSurface)
            {
                FUSDExporter::ExportPreviewSurface(Stage, MaterialInstance, USDShader);
            }
        }
    }
    else if(ShouldSaveMember(ParameterArray, false))
    {
        FUSDConversion::SyncMaterialParameters(MaterialInstance, USDShader, FUSDConversion::EDirection::Export);
    }
    else if(ShouldSaveMember(ParameterArray))
    {
        for(auto Change : Changes)
        {
            TArray<FString> Tokens;
            Change.ParseIntoArray(Tokens, TEXT("/"));

            if(Tokens.Num() < 3 || ParameterArray.Find(Tokens[1]) < 0)
            {
                continue;
            }

            FUSDConversion::SyncMaterialParameter(FName(*Tokens[2]), Tokens[1], MaterialInstance, USDShader, FUSDConversion::EDirection::Export);
        }
    }

    return true;
}

FUSDConversion::FMaterialBinding FUSDConversion::ParsePrimMaterial(pxr::UsdPrim Prim)
{
    // Find the material used by the mesh
    FMaterialBinding MaterialBinding;
    auto LoadMaterialBinding = [&](const pxr::UsdPrim& InPrim)
    {
        pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(InPrim);
        MaterialBinding.Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());

        // Find the output graph
        if(MaterialBinding.Material)
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            auto SurfaceOutput = MaterialBinding.Material.GetSurfaceOutput();
            if (SurfaceOutput)
            {
                if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                {
                    MaterialBinding.Shader = pxr::UsdShadeShader(Source);
                }
            }

            auto MdlSurfaceOutput = MaterialBinding.Material.GetSurfaceOutput(USDTokens.mdl);
            if (MdlSurfaceOutput)
            {
                if (MdlSurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                {
                    MaterialBinding.MdlSurfaceShader = pxr::UsdShadeShader(Source);
                }			
            }
            return true;
        }

        return false;
    };


    for(;;)
    {
        if(!Prim)
        {
            break;
        }

        if (LoadMaterialBinding(Prim))
        {
            break;
        }
        else if (Prim.IsA<pxr::UsdGeomSubset>())
        {
            // only search for the material bound from the mesh parent
            Prim = Prim.GetParent();
            LoadMaterialBinding(Prim);
            break;
        }
        else
        {
            // search the parent
            Prim = Prim.GetParent();
        }
    }

    return MaterialBinding;
}

pxr::UsdSkelRoot FUSDExporter::ExportSkeletalMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMesh& SkeletalMesh, const bool bExportTangentX, const bool bRootIdentity)
{
    const FSkeletalMeshRenderData * SkelMeshRenderData = SkeletalMesh.GetResourceForRendering();
    if (SkelMeshRenderData == nullptr || SkelMeshRenderData->LODRenderData.Num() <= 0)
    {
        return pxr::UsdSkelRoot();
    }

    pxr::UsdSkelRoot SkelRoot = pxr::UsdSkelRoot::Define(USDStage, NewPrimPath);
    if (!SkelRoot)
    {
        return pxr::UsdSkelRoot();
    }

    // export mesh
    const FSkeletalMeshLODRenderData& LODData = SkelMeshRenderData->LODRenderData[0];
    pxr::SdfPath MeshPath = NewPrimPath.AppendElementString("Mesh");
    ExportSkeletalMesh(USDStage, MeshPath, LODData, bExportTangentX);

    if (!SkeletalMesh.Skeleton)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("No bones found in skeleton"));
        return SkelRoot;
    }

    // create UsdSkelSkeleton
    pxr::UsdSkelSkeleton Skeleton;
    pxr::SdfPath SkeletonPath = NewPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SkeletalMesh.Skeleton->GetName())));
    Skeleton = pxr::UsdSkelSkeleton::Define(USDStage, SkeletonPath);
    if (!Skeleton)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Failed to create UsdSkelSkeleton"));
        return SkelRoot;
    }

    // build joint array
    pxr::SdfPathVector JointArray;
    auto RefSkeleton = SkeletalMesh.RefSkeleton;
    int NumBones = RefSkeleton.GetNum();
    if (NumBones > 0)
    {
        JointArray = BuildJointTree(SkeletonPath, SkeletonPath, JointArray, RefSkeleton, 0, 0);
    }
    else
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("No bones found in skeleton"));
        return SkelRoot;
    }

    if (JointArray.size() != NumBones)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("JointArray size mismatch. Expected %d, got %d"), NumBones, JointArray.size());
        return SkelRoot;
    }

    // construct topology
    pxr::UsdSkelTopology Topology(JointArray);
    std::string Reason;
    if (!Topology.Validate(&Reason))
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Failed to create UsdSkelTopology"));
        return SkelRoot;
    }

    // convert joint array to token array
    pxr::VtTokenArray JointTokenArray(NumBones);
    for (int i = 0; i < JointArray.size(); ++i)
    {
        JointTokenArray[i] = pxr::TfToken(JointArray[i].GetString());
    }
    Skeleton.GetJointsAttr().Set(JointTokenArray);

    // populate bind transform - requires world space transforms
    TArray<FTransform> BindTransform = LocalToWorldSpace(RefSkeleton);
    if (BindTransform.Num() != JointTokenArray.size())
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Joint array size mismatch - rest pose. Expected %d, got %d"), JointTokenArray.size(), BindTransform.Num());
        return SkelRoot;
    }
    Skeleton.CreateBindTransformsAttr().Set(USDConvert(USDStage, BindTransform));

    // populate rest transform - local space transforms
    TArray<FTransform> RestTransform = RefSkeleton.GetRefBonePose();
    FTransform RootTransform = FTransform::Identity;
    if (bRootIdentity)
    {
        RootTransform = RestTransform[0];
        RestTransform[0] *= RootTransform.Inverse();
    }
    Skeleton.GetRestTransformsAttr().Set(USDConvert(USDStage, RestTransform));
    Skeleton.CreatePurposeAttr().Set(pxr::UsdGeomTokens->guide);
    {
        pxr::VtArray<pxr::GfVec3f> Extent;
        pxr::UsdGeomBoundable Boundable(Skeleton.GetPrim());
        pxr::UsdGeomBoundable::ComputeExtentFromPlugins(Boundable, pxr::UsdTimeCode::Default(), &Extent);
        Skeleton.CreateExtentAttr().Set(Extent);
    }

    // AVSim needs skeleton bound to meshes
#if 0
    pxr::UsdSkelBindingAPI SkeletonBinding = pxr::UsdSkelBindingAPI::Apply(SkelRoot.GetPrim());
    SkeletonBinding.CreateSkeletonRel().SetTargets(pxr::SdfPathVector({ Skeleton.GetPath() }));
#else
    TArray<pxr::UsdPrim> MeshPrims;
    GetMeshFromSkelRoot(SkelRoot, MeshPrims);

    for (auto& Mesh : MeshPrims)
    {
        pxr::UsdSkelBindingAPI SkeletonBinding = pxr::UsdSkelBindingAPI::Apply(Mesh);
        SkeletonBinding.CreateSkeletonRel().SetTargets(pxr::SdfPathVector({ Skeleton.GetPath() }));
    }
#endif

    if (bRootIdentity)
    {
        FTransform RootTransformRH = RHSTransformConvert(USDStage, RootTransform, false);
        setLocalTransformMatrix(SkelRoot, RootTransformRH);
    }

    {
        pxr::VtArray<pxr::GfVec3f> Extent;
        pxr::UsdGeomBoundable Boundable(SkelRoot.GetPrim());
        pxr::UsdGeomBoundable::ComputeExtentFromPlugins(Boundable, pxr::UsdTimeCode::Default(), &Extent);
        SkelRoot.CreateExtentAttr().Set(Extent);
    }

    return SkelRoot;
}

void FUSDExporter::SetJointInfluencesAndWeights(const FSkeletalMeshLODRenderData& LODModel, const pxr::UsdGeomXformable& USDMesh, bool bIsGeomSubset)
{
    // we only want to keep the top 4 influence bones
    const int KEEP_INFLUENCES = 4;

    // set joint weights and influences
    const int32 NumVertices = LODModel.GetNumVertices();

    if (NumVertices == 0)
    {
        return;
    }

    const uint32 NumInfluences = LODModel.SkinWeightVertexBuffer.GetMaxBoneInfluences();
    auto CompressInfluence = [&](const pxr::VtIntArray& SrcJointInfluences, const pxr::VtFloatArray& SrcJointWeights, pxr::VtIntArray& TargetJointInfluences, pxr::VtFloatArray& TargetJointWeights)
    {
        for (int i = 0; i < SrcJointInfluences.size(); i += NumInfluences)
        {
            for (int j = 0; j < KEEP_INFLUENCES; ++j)
            {
                TargetJointWeights.push_back(SrcJointWeights[i + j]);
                TargetJointInfluences.push_back(SrcJointInfluences[i + j]);
            }
        }

        pxr::UsdSkelNormalizeWeights(&TargetJointWeights, KEEP_INFLUENCES);
    };

    pxr::VtFloatArray TotalJointWeights;
    TotalJointWeights.reserve(NumInfluences * NumVertices);

    pxr::VtIntArray TotalJointInfluences;
    TotalJointInfluences.reserve(NumInfluences * NumVertices);

    const int32 SectionCount = LODModel.RenderSections.Num();
    for (int32 SectionIndex = 0; SectionIndex < SectionCount; ++SectionIndex)
    {
        const FSkelMeshRenderSection& Section = LODModel.RenderSections[SectionIndex];

        if (Section.NumVertices == 0)
        {
            continue;
        }

        pxr::VtFloatArray SectionJointWeights;
        SectionJointWeights.reserve(NumInfluences * Section.NumVertices);

        pxr::VtIntArray SectionJointInfluences;
        SectionJointInfluences.reserve(NumInfluences * Section.NumVertices);

        for (uint32 VertexIndex = 0; VertexIndex < Section.NumVertices; ++VertexIndex)
        {
            FSkinWeightInfo SrcWeight = LODModel.SkinWeightVertexBuffer.GetVertexSkinWeights(Section.BaseVertexIndex + VertexIndex);

            for (uint32 InfluenceIndex = 0; InfluenceIndex < NumInfluences; ++InfluenceIndex)
            {
                int32 InfluenceBone = Section.BoneMap[SrcWeight.InfluenceBones[InfluenceIndex]];
                float InfluenceWeight = SrcWeight.InfluenceWeights[InfluenceIndex] / 255.f;

                if (bIsGeomSubset)
                {
                    TotalJointWeights.push_back(InfluenceWeight);
                    TotalJointInfluences.push_back(InfluenceBone);
                }
                else
                {
                    SectionJointWeights.push_back(InfluenceWeight);
                    SectionJointInfluences.push_back(InfluenceBone);
                }
            }
        }

        if (!bIsGeomSubset)
        {
            pxr::UsdSkelSortInfluences(&SectionJointInfluences, &SectionJointWeights, NumInfluences);

            pxr::VtFloatArray CompressedSectionJointWeights;
            CompressedSectionJointWeights.reserve(KEEP_INFLUENCES * Section.NumVertices);
            pxr::VtIntArray CompressedSectionJointInfluences;
            CompressedSectionJointInfluences.reserve(KEEP_INFLUENCES * Section.NumVertices);
            CompressInfluence(SectionJointInfluences, SectionJointWeights, CompressedSectionJointInfluences, CompressedSectionJointWeights);

            if (SectionCount == 1)
            {
                // bind weights and influences
                pxr::UsdSkelBindingAPI MeshBinding = pxr::UsdSkelBindingAPI::Apply(USDMesh.GetPrim());
                MeshBinding.CreateJointIndicesPrimvar(false, KEEP_INFLUENCES).Set(CompressedSectionJointInfluences);
                MeshBinding.CreateJointWeightsPrimvar(false, KEEP_INFLUENCES).Set(CompressedSectionJointWeights);
            }
            else
            {
                FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);
                auto SectionMesh = USDMesh.GetPrim().GetChild(pxr::TfToken(TCHAR_TO_ANSI(*SectionName)));
                if (SectionMesh)
                {
                    pxr::UsdSkelBindingAPI MeshBinding = pxr::UsdSkelBindingAPI::Apply(SectionMesh);
                    MeshBinding.CreateJointIndicesPrimvar(false, KEEP_INFLUENCES).Set(CompressedSectionJointInfluences);
                    MeshBinding.CreateJointWeightsPrimvar(false, KEEP_INFLUENCES).Set(CompressedSectionJointWeights);
                }
            }
        }
    }

    if (bIsGeomSubset)
    {
        pxr::UsdSkelSortInfluences(&TotalJointInfluences, &TotalJointWeights, NumInfluences);

        pxr::VtFloatArray CompressedTotalJointWeights;
        CompressedTotalJointWeights.reserve(KEEP_INFLUENCES * NumVertices);
        pxr::VtIntArray CompressedTotalJointInfluences;
        CompressedTotalJointInfluences.reserve(KEEP_INFLUENCES * NumVertices);
        CompressInfluence(TotalJointInfluences, TotalJointWeights, CompressedTotalJointInfluences, CompressedTotalJointWeights);

        // bind weights and influences
        pxr::UsdSkelBindingAPI MeshBinding = pxr::UsdSkelBindingAPI::Apply(USDMesh.GetPrim());
        MeshBinding.CreateJointIndicesPrimvar(false, KEEP_INFLUENCES).Set(CompressedTotalJointInfluences);
        MeshBinding.CreateJointWeightsPrimvar(false, KEEP_INFLUENCES).Set(CompressedTotalJointWeights);
    }
}

pxr::UsdGeomXformable FUSDExporter::ExportSkeletalMeshComponentToPointCache(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMeshComponent& SkeletalMeshComponent, const bool bExportTangentX, pxr::UsdTimeCode TimeCode)
{
    pxr::SdfPath MeshPath = NewPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SkeletalMeshComponent.SkeletalMesh->GetName())));

    // export mesh
    const FSkeletalMeshRenderData * SkelMeshRenderData = SkeletalMeshComponent.SkeletalMesh->GetResourceForRendering();
    if (SkelMeshRenderData->LODRenderData.Num() <= 0)
    {
        return pxr::UsdGeomMesh();
    }

    const FSkeletalMeshLODRenderData& LODModel = SkelMeshRenderData->LODRenderData[0];

    if (LODModel.RenderSections.Num() == 1)
    {
        FUSDGeomMeshAttributes Attributes;
        ExportSkeletalMesh(USDStage, LODModel, Attributes, bExportTangentX);

        // Create Mesh primitive
        auto USDMesh = pxr::UsdGeomMesh::Get(USDStage, NewPrimPath);
        if (!USDMesh)
        {
            USDMesh = pxr::UsdGeomMesh::Define(USDStage, NewPrimPath);
            if (!USDMesh)
            {
                return USDMesh;
            }

            USDMesh.CreateFaceVertexCountsAttr(pxr::VtValue(Attributes.FaceVertexCounts));
            USDMesh.CreateFaceVertexIndicesAttr(pxr::VtValue(Attributes.FaceVertexIndices));

            if (!Attributes.Colors.empty())
            {
                auto Primvar = USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->vertex);
                Primvar.Set(Attributes.Colors);
            }

            if (!Attributes.Colors.empty())
            {
                auto Primvar = USDMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->vertex);
                Primvar.Set(Attributes.Colors);
            }

            // Vertex color opacity
            if (!Attributes.Opacities.empty())
            {
                auto Primvar = USDMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->vertex);
                Primvar.Set(Attributes.Opacities);
            }

            // Texture UV
            for (int i = 0; i < Attributes.UVs.size(); ++i)
            {
                auto Primvar = USDMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + (i == 0 ? "" : std::to_string(i))), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->vertex);
                Primvar.Set(Attributes.UVs[i]);
            }

            for (int i = 0; i < Attributes.FaceIndices.size(); ++i)
            {
                pxr::UsdGeomSubset::CreateUniqueGeomSubset(USDMesh, USDTokens.subset, pxr::UsdGeomTokens->face, Attributes.FaceIndices[i]);
            }
        }

        USDMesh.CreatePointsAttr().Set(pxr::VtValue(Attributes.Points), TimeCode);
        if (!Attributes.Normals.empty())
        {
            USDMesh.CreateNormalsAttr().Set(pxr::VtValue(Attributes.Normals), TimeCode);
            USDMesh.SetNormalsInterpolation(pxr::UsdGeomTokens->vertex);
        }

        if (!Attributes.TangentX.empty())
        {
            pxr::UsdGeomPrimvar TangentXPrimvar = USDMesh.CreatePrimvar(USDTokens.tangentX, pxr::SdfValueTypeNames->Float3Array, pxr::UsdGeomTokens->vertex);
            TangentXPrimvar.Set(Attributes.TangentX, TimeCode);
        }

        return USDMesh;
    }
    else
    {
        TArray<FUSDGeomMeshAttributes> Attributes;
        ExportSkeletalMesh(USDStage, LODModel, Attributes, bExportTangentX);

        bool bNewMesh = false;
        auto USDMesh = pxr::UsdGeomXform::Get(USDStage, NewPrimPath);
        if (!USDMesh)
        {
            USDMesh = pxr::UsdGeomXform::Get(USDStage, NewPrimPath);
            if (!USDMesh)
            {
                return USDMesh;
            }

            bNewMesh = true;
        }

        for (int32 SectionIndex = 0; SectionIndex < Attributes.Num(); ++SectionIndex)
        {
            FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);
            auto SectionPath = USDMesh.GetPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SectionName)));
            auto SectionMesh = pxr::UsdGeomMesh::Define(USDStage, SectionPath);	

            if (SectionMesh)
            {
                auto SectionAttributes = Attributes[SectionIndex];
                if (bNewMesh)
                {
                    SectionMesh.CreateFaceVertexCountsAttr(pxr::VtValue(SectionAttributes.FaceVertexCounts));
                    SectionMesh.CreateFaceVertexIndicesAttr(pxr::VtValue(SectionAttributes.FaceVertexIndices));

                    if (!SectionAttributes.Colors.empty())
                    {
                        auto Primvar = SectionMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->vertex);
                        Primvar.Set(SectionAttributes.Colors);
                    }

                    // Vertex color opacity
                    if (!SectionAttributes.Opacities.empty())
                    {
                        auto Primvar = SectionMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->vertex);
                        Primvar.Set(SectionAttributes.Opacities);
                    }

                    // Texture UV
                    for (int i = 0; i < SectionAttributes.UVs.size(); ++i)
                    {
                        auto Primvar = USDMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + (i == 0 ? "" : std::to_string(i))), pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->vertex);
                        Primvar.Set(SectionAttributes.UVs[i]);
                    }
                }

                SectionMesh.CreatePointsAttr().Set(pxr::VtValue(SectionAttributes.Points), TimeCode);
                if (!SectionAttributes.Normals.empty())
                {
                    SectionMesh.CreateNormalsAttr().Set(pxr::VtValue(SectionAttributes.Normals), TimeCode);
                    SectionMesh.SetNormalsInterpolation(pxr::UsdGeomTokens->vertex);
                }

                if (!SectionAttributes.TangentX.empty())
                {
                    pxr::UsdGeomPrimvar TangentXPrimvar = SectionMesh.CreatePrimvar(USDTokens.tangentX, pxr::SdfValueTypeNames->Float3Array, pxr::UsdGeomTokens->vertex);
                    TangentXPrimvar.Set(SectionAttributes.TangentX, TimeCode);
                }
            }
        }

        return USDMesh;
    }
}

bool FUSDConversion::SyncMaterialParameters(UMaterialInstance& MaterialInst, pxr::UsdShadeShader ShaderPrim, EDirection ConversionType)
{
    // Clear overridden parameters
    bool bUpdated = false;

    if(ConversionType == EDirection::Import)
    {
        if(auto MaterialInstDynamic = Cast<UMaterialInstanceDynamic>(&MaterialInst))
        {
            MaterialInstDynamic->ClearParameterValues();
        }
        else if(auto MaterialInstConstant = Cast<UMaterialInstanceConstant>(&MaterialInst))
        {
#if WITH_EDITOR
            MaterialInstConstant->ClearParameterValuesEditorOnly();
#endif
        }
    }

    // Deal with scalar parameters
    TArray<FMaterialParameterInfo> ParameterInfos;
    TArray<FGuid> ParameterIDs;
    MaterialInst.GetMaterial()->GetAllScalarParameterInfo(ParameterInfos, ParameterIDs);
    for(auto& ParameterInfo : ParameterInfos)
    {
        bUpdated |= SyncMaterialParameter(ParameterInfo.Name, GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, ScalarParameterValues), MaterialInst, ShaderPrim, ConversionType);
    }

    // Deal with vector parameters
    MaterialInst.GetMaterial()->GetAllVectorParameterInfo(ParameterInfos, ParameterIDs);
    for(auto& ParameterInfo : ParameterInfos)
    {
        bUpdated |= SyncMaterialParameter(ParameterInfo.Name, GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, VectorParameterValues), MaterialInst, ShaderPrim, ConversionType);
    }

    // Deal with texture parameters
    MaterialInst.GetMaterial()->GetAllTextureParameterInfo(ParameterInfos, ParameterIDs);
    for(auto& ParameterInfo : ParameterInfos)
    {
        bUpdated |= SyncMaterialParameter(ParameterInfo.Name, GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, TextureParameterValues), MaterialInst, ShaderPrim, ConversionType);
    }

    return bUpdated;
}

bool FUSDConversion::SyncMaterialParameter(const FName& ParameterName, const FString& ParameterType, UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShaderPrim, EDirection ConversionType)
{
    bool bUpdated = false;

    // Convert to attribute name
    pxr::TfToken ShaderInputName;

    auto BaseMaterial = MaterialInst.GetBaseMaterial();
    if(BaseMaterial)
    {
        {
            auto TrimedName = ParameterName.ToString().Replace(TEXT(" "), TEXT("_"));
            ShaderInputName = pxr::TfToken(TCHAR_TO_ANSI(*TrimedName));
        }
    }

    if(ShaderInputName.IsEmpty())
    {
        ShaderInputName = pxr::TfToken(TCHAR_TO_ANSI(*ParameterName.ToString()));
    }

    pxr::TfToken PropertyName("inputs:" + ShaderInputName.GetString());	// Used for removal

    // Set attribute
    auto MaterialInstDynamic = Cast<UMaterialInstanceDynamic>(&MaterialInst);
    auto MaterialInstConst = Cast<UMaterialInstanceConstant>(&MaterialInst);

    if(ParameterType == GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, ScalarParameterValues))
    {
        // Set parameter
        if(ConversionType == EDirection::Import)
        {
            auto USDScalar = ShaderPrim.GetInput(ShaderInputName);
            if(USDScalar)
            {
                if(MaterialInstDynamic)
                {
                    MaterialInstDynamic->SetScalarParameterValue(ParameterName, GetUSDValue<float>(USDScalar));
                }
                else if(MaterialInstConst)
                {
#if WITH_EDITOR
                    MaterialInstConst->SetScalarParameterValueEditorOnly(ParameterName, GetUSDValue<float>(USDScalar));
#endif
                }

                bUpdated = true;
            }
        }
        else
        {
            FScalarParameterValue* OverriddenValue = MaterialInst.ScalarParameterValues.FindByPredicate([&](const FScalarParameterValue& Value) {return Value.ParameterInfo.Name == ParameterName; });

            if(OverriddenValue != nullptr)
            {
                bUpdated |= ShaderPrim.CreateInput(ShaderInputName, pxr::SdfValueTypeNames->Float).Set(OverriddenValue->ParameterValue);
            }
            else	// Remove attribute
            {
                bUpdated |= ShaderPrim.GetPrim().RemoveProperty(PropertyName);
            }
        }
    }
    else if(ParameterType == GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, VectorParameterValues))
    {
        if(ConversionType == EDirection::Import)
        {
            auto USDColor = ShaderPrim.GetInput(ShaderInputName);
            if(USDColor)
            {
                if(MaterialInstDynamic)
                {
                    MaterialInstDynamic->SetVectorParameterValue(ParameterName, USDConvertToLinearColor(GetUSDValue<pxr::GfVec3f>(USDColor)));
                }
                else if(MaterialInstConst)
                {
#if WITH_EDITOR
                    MaterialInstConst->SetVectorParameterValueEditorOnly(ParameterName, USDConvertToLinearColor(GetUSDValue<pxr::GfVec3f>(USDColor)));
#endif // WITH_EDITOR
                }

                bUpdated = true;
            }
        }
        else
        {
            FVectorParameterValue* OverriddenValue = MaterialInst.VectorParameterValues.FindByPredicate([&](const FVectorParameterValue& Value) {return Value.ParameterInfo.Name == ParameterName; });

            if(OverriddenValue != nullptr)
            {
                bUpdated |= ShaderPrim.CreateInput(ShaderInputName, pxr::SdfValueTypeNames->Color3f).Set(USDConvert(OverriddenValue->ParameterValue));
            }
            else	// Remove attribute
            {
                bUpdated |= ShaderPrim.GetPrim().RemoveProperty(PropertyName);
            }
        }
    }
    else if(ParameterType == GET_MEMBER_NAME_STRING_CHECKED(UMaterialInstance, TextureParameterValues))
    {
        if(ConversionType == EDirection::Import)
        {
            auto AssetPath = ShaderPrim.GetInput(ShaderInputName);
            if(AssetPath)
            {
                // Load texture
                FString OmniPath = GetUSDValue<pxr::SdfAssetPath>(AssetPath).GetAssetPath().c_str();

                auto Texture = LoadObject<UTexture>(nullptr, *OmniPath);

                // Set to material
                if(MaterialInstDynamic)
                {
                    MaterialInstDynamic->SetTextureParameterValue(ParameterName, Texture);
                }
                else if(MaterialInstConst)
                {
#if WITH_EDITOR
                    MaterialInstConst->SetTextureParameterValueEditorOnly(ParameterName, Texture);
#endif // WITH_EDITOR
                }

                bUpdated = true;
            }
        }
        else
        {
            FTextureParameterValue* OverriddenValue = MaterialInst.TextureParameterValues.FindByPredicate([&](const FTextureParameterValue& Value) {return Value.ParameterInfo.Name == ParameterName; });

            if(OverriddenValue != nullptr)
            {
                FString OmniPath;
                auto* OmniAsset = USimReadyAsset::GetSimReadyAsset(*OverriddenValue->ParameterValue);
                if (OmniAsset == nullptr)
                {
                    OmniPath = OverriddenValue->ParameterValue->GetPathName();
                }

                bUpdated |= ShaderPrim.CreateInput(ShaderInputName, pxr::SdfValueTypeNames->Asset).Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*OmniPath)));
            }
            else	// Remove attribute
            {
                bUpdated |= ShaderPrim.GetPrim().RemoveProperty(PropertyName);
            }
        }
    }

    return bUpdated;
}

FTransform ConvertRelativeTransformFromUE4ToUSD(USceneComponent& SceneComp)
{
    USceneComponent* AttachedParent = SceneComp.GetAttachParent();
    // NOTE: USD used matrix while UE4 used FTransform to do the local to world transform. They're different. 
    // So we need to convert to matrix to get the USD relative transform to export.
    if (AttachedParent && SceneComp.GetAttachSocketName().IsNone())
    {
        const FTransform ParentToWorld = AttachedParent->GetComponentToWorld();
        const FTransform CompToWorld = SceneComp.GetComponentToWorld();
        FMatrix ParentToWorldMatrix = ParentToWorld.ToMatrixWithScale();
        FMatrix CompToWorldMatrix = CompToWorld.ToMatrixWithScale();
        FMatrix RelativeMatrix = CompToWorldMatrix * ParentToWorldMatrix.Inverse();
        return FTransform(RelativeMatrix);
    }
    else
    {
        FTransform RelativeTransform = SceneComp.GetRelativeTransform();
        if (AttachedParent && !SceneComp.GetAttachSocketName().IsNone())
        {
            RelativeTransform = RelativeTransform * AttachedParent->GetSocketTransform(SceneComp.GetAttachSocketName(), RTS_Component);
            RelativeTransform.NormalizeRotation();
        }
        return RelativeTransform;
    }
}

FTransform ConvertRelativeTransformFromUSDToUE4(const pxr::UsdPrim& Prim, const pxr::UsdTimeCode& time)
{
    pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache(time);
    pxr::GfMatrix4d WorldMatrix = Cache.GetLocalToWorldTransform(Prim);
    pxr::GfMatrix4d ParentWorldMatrix = Cache.GetParentToWorldTransform(Prim);

    FTransform ParentWorldTransform = USDConvert(Prim.GetStage(), ParentWorldMatrix);
    FTransform WorldTransform = USDConvert(Prim.GetStage(), WorldMatrix);

    return WorldTransform.GetRelativeTransform(ParentWorldTransform);
}

template <class HalfType, class FloatType, class DoubleType, class OutValueType>
void GetValueFromXformOp(const pxr::UsdGeomXformOp XformOp, const pxr::UsdTimeCode& TimeCode, OutValueType& OutValue)
{
    switch (XformOp.GetPrecision())
    {
        case pxr::UsdGeomXformOp::PrecisionHalf:
        {
            HalfType Value;
            XformOp.Get(&Value, TimeCode);
            OutValue = USDConvert(Value);
            break;
        }
        case pxr::UsdGeomXformOp::PrecisionFloat:
        {
            FloatType Value;
            XformOp.Get(&Value, TimeCode);
            OutValue = USDConvert(Value);
            break;
        }
        case pxr::UsdGeomXformOp::PrecisionDouble:
        {
            DoubleType Value;
            XformOp.Get(&Value, TimeCode);
            OutValue = USDConvert(Value);
            break;
        }
    }
}


bool GetRelativeTransformTimeSamples(const pxr::UsdGeomXformable& Xform, FTranslateRotateScaleTimeSamples& TransformTimeSamples)
{
    {
        bool resetXFormStack;
        auto xformOps = Xform.GetOrderedXformOps(&resetXFormStack);
        auto USDStage = Xform.GetPrim().GetStage();

        for (auto XformOp : xformOps)
        {
            std::vector<double> TimeSamples;
            XformOp.GetTimeSamples(&TimeSamples);

            for (int32 TimeSampleIndex = 0; TimeSampleIndex < TimeSamples.size(); ++TimeSampleIndex)
            {
                pxr::UsdTimeCode TimeCode = TimeSamples[TimeSampleIndex];
                switch (XformOp.GetOpType())
                {
                case pxr::UsdGeomXformOp::TypeTranslate:
                {
                    FVector Translate;
                    GetValueFromXformOp<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, FVector>(XformOp, TimeCode, Translate);
                    TransformTimeSamples.Translate.Add(TimeCode.GetValue(), RHSTranslationConvert(USDStage, Translate, true));
                    break;
                }
                case pxr::UsdGeomXformOp::TypeScale:
                {
                    FVector Scale;
                    GetValueFromXformOp<pxr::GfVec3h, pxr::GfVec3f, pxr::GfVec3d, FVector>(XformOp, TimeCode, Scale);
                    TransformTimeSamples.Scale.Add(TimeCode.GetValue(), RHSScaleConvert(USDStage, Scale));
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateX:
                {
                    float RotateX;
                    GetValueFromXformOp<pxr::GfHalf, float, double, float>(XformOp, TimeCode, RotateX);
                    TransformTimeSamples.Rotate.Add(TimeCode.GetValue(), FVector(RotateX, 0, 0));
                    break;
                }	
                case pxr::UsdGeomXformOp::TypeRotateY:
                {
                    float RotateY;
                    GetValueFromXformOp<pxr::GfHalf, float, double, float>(XformOp, TimeCode, RotateY);
                    TransformTimeSamples.Rotate.Add(TimeCode.GetValue(), FVector(0, RotateY, 0));
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateZ:
                {
                    float RotateZ;
                    GetValueFromXformOp<pxr::GfHalf, float, double, float>(XformOp, TimeCode, RotateZ);
                    TransformTimeSamples.Rotate.Add(TimeCode.GetValue(), FVector(0, 0, RotateZ));
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateXYZ:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateXZY:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateYXZ:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateYZX:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateZXY:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeRotateZYX:
                {
                    break;
                }
                case pxr::UsdGeomXformOp::TypeOrient:
                {
                    FQuat Rotate;
                    GetValueFromXformOp<pxr::GfQuath, pxr::GfQuatf, pxr::GfQuatd, FQuat>(XformOp, TimeCode, Rotate);
                    FRotator Rotator = RHSRotationConvert(USDStage, Rotate).Rotator();
                    TransformTimeSamples.Rotate.Add(TimeCode.GetValue(), FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw));
                    break;
                }
                }
            }
        }
        return true;
    }

    return false;
}

bool IsSameOrChildPrimPath(const FString& ParentPath, const FString& ChildPath)
{
    FString DecodedParent = FSimReadyPathHelper::KeyToPrimPath(ParentPath);
    FString DecodedChild = FSimReadyPathHelper::KeyToPrimPath(ChildPath);

    // NOTE: must case sensitive
    return (DecodedChild.Equals(DecodedParent) 
        || DecodedChild.StartsWith(DecodedParent.Equals(TEXT("/")) ? DecodedParent : (DecodedParent + TEXT("/")), ESearchCase::CaseSensitive));
}

pxr::GfVec2f Vec4ToVec2(pxr::GfVec4f Vec)
{
    return pxr::GfVec2f(Vec[0], Vec[1]);
}

pxr::GfVec3f Vec4ToVec3(pxr::GfVec4f Vec)
{
    return pxr::GfVec3f(Vec[0], Vec[1], Vec[2]);
}

pxr::GfVec4f LinearColorToVec(const FLinearColor& Color)
{
    return pxr::GfVec4f(Color.R, Color.G, Color.B, Color.A);
}

void FUSDExporter::SetColorSpace(const FString& ParameterName, pxr::UsdShadeShader& Shader, bool bSRGB)
{
    auto Input = Shader.GetInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)));
    if (Input)
    {
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            Input.GetConnectedSource(&Source, &SourceName, &SourceType);

            Input = Source.GetInput(SourceName);
        }

        if (Input)
        {
            Input.GetAttr().SetColorSpace(bSRGB ? USDTokens.sRGB : USDTokens.raw);
        }
    }
}

void FUSDExporter::ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, float ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input)
{
    if (ValueType == pxr::SdfValueTypeNames->Int)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set((int32)ParameterValue);
        }
        else
        {
            Input.Set((int32)ParameterValue);
        }
    }
    else if (ValueType == pxr::SdfValueTypeNames->Float)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(ParameterValue);
        }
        else
        {
            Input.Set(ParameterValue);
        }
    }
    else if (ValueType == pxr::SdfValueTypeNames->Bool)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(ParameterValue != 0.0f ? true : false);
        }
        else
        {
            Input.Set(ParameterValue != 0.0f ? true : false);
        }
    }
}

void FUSDExporter::ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, pxr::GfVec4f ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input)
{
    if (ValueType == pxr::SdfValueTypeNames->Color3f)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(Vec4ToVec3(ParameterValue));
        }
        else
        {
            Input.Set(Vec4ToVec3(ParameterValue));
        }
    }
    else if (ValueType == pxr::SdfValueTypeNames->Normal3f)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(Vec4ToVec3(ParameterValue));
        }
        else
        {
            Input.Set(Vec4ToVec3(ParameterValue));
        }	
    }
    else if (ValueType == pxr::SdfValueTypeNames->Float2)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(Vec4ToVec2(ParameterValue));
        }
        else
        {
            Input.Set(Vec4ToVec2(ParameterValue));
        }
    }
    else if (ValueType == pxr::SdfValueTypeNames->Float3)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(Vec4ToVec3(ParameterValue));
        }
        else
        {
            Input.Set(Vec4ToVec3(ParameterValue));
        }
    }
    else if (ValueType == pxr::SdfValueTypeNames->Float4)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(ParameterValue);
        }
        else
        {
            Input.Set(ParameterValue);
        }
    }
}

void FUSDExporter::ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, const FString& ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input)
{
    if (ValueType == pxr::SdfValueTypeNames->Asset)
    {
        if (bNewInput)
        {
            Shader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)), ValueType).Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*ParameterValue)));
        }
        else
        {
            Input.Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*ParameterValue)));
        }
    }
}