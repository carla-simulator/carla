// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "SimReadyPxr.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Containers/StringConv.h"
#include "PhysicsEngine/ConvexElem.h"
#include "Model.h"
#include "MeshDescription.h"
#include "SimReadyPathHelper.h"
#include "SimReadyExportSettings.h"


// OM-9653 All inputs in Kit is now only defined on Shader prim
#define USD_DIRECT_SHADER_INPUT	(1) // use material input or shader input in usd schema

#define LOAD_NAMED_PARAMETER(TimeSamplesParameterType, TimeSamplesMap, UsdPrim, ParameterType, ParameterName, ComponentType, Component, ComponentParameterName, OnPostGetFunc) \
        {\
            TimeSamplesParameterType TimeSamples;\
            if (GetUSDTimeSamples<ParameterType, TimeSamplesParameterType>(UsdPrim.Get##ParameterName##Attr(), TimeSamples, OnPostGetFunc))\
            {\
                auto Name = GET_MEMBER_NAME_CHECKED(ComponentType, ComponentParameterName);\
                TimeSamplesMap.Add(Name, TimeSamples);\
            }\
            Component->ComponentParameterName = GetUSDValue<ParameterType>(UsdPrim.Get##ParameterName##Attr());\
            OnPostGetFunc(Component->ComponentParameterName);\
        }

#ifdef WITH_CARLA
const FString SimReadyDefaultMaterial("/Game/Carla/Omniverse/Omniverse/DefaultMaterial");
#else
const FString SimReadyDefaultMaterial("/SimReady/DefaultMaterial");
#endif
const FString SimReadyCameraShot("SimReadyCameraShot");

const FString SimReadyRootPrim("Root");

FString MakeAssetPathRelative(FString Path, pxr::UsdStage& USDStage);

pxr::SdfLayerHandle GetLayerHandle(const pxr::UsdAttribute& Attribute);
pxr::SdfLayerHandle GetLayerHandle(const pxr::UsdPrim& Prim);
bool USDImportCamera(const pxr::UsdGeomCamera& Prim, class UCameraComponent& CameraComponent, struct FNamedParameterTimeSamples& NamedParameterTimeSamples);
bool USDImportLight(const pxr::UsdPrim& Light, class ULightComponent& LightComponent, struct FNamedParameterTimeSamples& NamedParameterTimeSamples);

pxr::GfVec3f USDConvert(const FLinearColor& Color);
pxr::GfVec3h USDConvertH(const FLinearColor& Color);
pxr::GfVec3d USDConvertD(const FLinearColor& Color);
pxr::GfQuatf USDConvert(const FQuat& Quat);
FLinearColor USDConvertToLinearColor(const pxr::GfVec2f& Color);
FLinearColor USDConvertToLinearColor(const pxr::GfVec3f& Color);
FLinearColor USDConvertToLinearColor(const pxr::GfVec4f& Color);
pxr::GfMatrix4d USDConvert(const FMatrix& Matrix);
FMatrix USDConvert(const pxr::GfMatrix4d& InMatrix);
FVector USDConvert(const pxr::GfVec3h& InValue);
FVector USDConvert(const pxr::GfVec3f& InValue);
FVector USDConvert(const pxr::GfVec3d& InValue);
FQuat USDConvert(const pxr::GfQuath& Quat);
FQuat USDConvert(const pxr::GfQuatf& Quat);
FQuat USDConvert(const pxr::GfQuatd& Quat);
float USDConvert(const pxr::GfHalf& Value);
float USDConvert(const float Value);
float USDConvert(const double Value);
FVector2D USDConvert(const pxr::GfVec2f& InValue);
pxr::GfVec2f USDConvert(const FVector2D & InValue);
FTransform USDConvert(const pxr::UsdStageRefPtr& Stage, const pxr::GfMatrix4d& InMatrix);
pxr::GfMatrix4d USDConvert(const pxr::UsdStageRefPtr& Stage, FTransform Transform);

FTransform RHSTransformConvert(const pxr::UsdStageRefPtr & Stage, FTransform Transform, bool UsdToUE);
FVector RHSTranslationConvert(const pxr::UsdStageRefPtr & Stage, FVector Translation, bool UsdToUE);
FQuat RHSRotationConvert(const pxr::UsdStageRefPtr & Stage, FQuat Rotation);
FVector RHSScaleConvert(const pxr::UsdStageRefPtr & Stage, FVector Scale);
float USDConvertLength(const pxr::UsdStageRefPtr & Stage, float InLength, bool UsdToUE = false);
FVector USDConvertPosition(const pxr::UsdStageRefPtr & Stage, const FVector& InPosition, bool UsdToUE = false);
FVector USDConvertVector(const pxr::UsdStageRefPtr & Stage, const FVector& InVector);

pxr::VtMatrix4dArray USDConvert(const pxr::UsdStageRefPtr& Stage, const TArray<FTransform>& InValue);
pxr::TfToken GetUSDStageAxis(const pxr::UsdStageRefPtr& Stage);
float UnitScaleFromUSDToUE(const pxr::UsdStageRefPtr& Stage);
float UnitScaleFromUEToUSD(const pxr::UsdStageRefPtr& Stage);
bool IsSphericalOrCylindricalProjectionUsed(class UMeshComponent* MeshComponent);
void ExportStaticMeshLOD(const FStaticMeshLODResources& StaticMeshLOD, FMeshDescription& OutRawMesh, const TArray<FStaticMaterial>& Materials);
bool IsSameOrChildPrimPath(const FString& ParentPath, const FString& ChildPath);
pxr::GfVec4f LinearColorToVec(const FLinearColor& Color);
void GetPrimTranslationAndPivot(const pxr::UsdPrim& Prim, const pxr::UsdTimeCode& TimeCode, FVector& Translation, FVector& Pivot);
pxr::UsdShadeMaterial FindShadeMaterial(const pxr::UsdPrim& Prim);
bool AllColorsAreEqual(const class FColorVertexBuffer* ColorVertexBuffer);
const class FColorVertexBuffer* GetFinalColorVertexBuffer(const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors);
void FixPrimKind(const pxr::UsdPrim& Prim);
void AddReferenceOrPayload(bool bPayload, const pxr::UsdPrim& Prim, const FString& RelativePath);
void OverridePostProcessSettings(const struct FPostProcessSettings& Src, float Weight, struct FPostProcessSettings& FinalPostProcessSettings);
void GetMeshFromSkelRoot(const pxr::UsdSkelRoot& USDSkelRoot, TArray<pxr::UsdPrim>& MeshPrims);
void SetQcode(const pxr::UsdPrim& Prim, const std::string& QcodeData);
void ExportCarlaTag(const pxr::UsdPrim&, const class UObject* Asset, const UActorComponent* Component);
void SetNonVisualTags(const pxr::UsdPrim& Prim, const pxr::TfToken& Base, const pxr::VtTokenArray& Attributes, const pxr::TfToken& Coating);
void SetNonVisualTags(const pxr::UsdPrim& Prim, const pxr::TfToken& Base);

template<typename T>
pxr::UsdPrim IsPrimFromUsdSchema(const pxr::UsdPrim& Prim)
{
    if (auto Parent = Prim.GetParent())
    {
        if (Parent.IsA<T>())
        {
            return Parent;
        }
        else
        {
            return IsPrimFromUsdSchema<T>(Parent);
        }
    }

    return pxr::UsdPrim();
}

FTransform ConvertRelativeTransformFromUE4ToUSD(USceneComponent& SceneComp);
FTransform ConvertRelativeTransformFromUSDToUE4(const pxr::UsdPrim& prim, const pxr::UsdTimeCode& time);
bool GetRelativeTransformTimeSamples(const pxr::UsdGeomXformable& Xform, struct FTranslateRotateScaleTimeSamples& TransformTimeSamples);

struct FUSDConversion
{
    enum class EDirection
    {
        Export,
        Import,
    };

    struct FMaterialBinding
    {
        pxr::UsdShadeMaterial Material;
        pxr::UsdShadeShader Shader;
        pxr::UsdShadeShader MdlSurfaceShader;
        pxr::UsdShadeShader MdlVolumeShader;
        pxr::UsdShadeShader MdlDisplacementShader;
    };

    static bool SyncMaterialParameters(class UMaterialInstance& MaterialInst, pxr::UsdShadeShader ShaderPrim, EDirection ConversionType);
    static bool SyncMaterialParameter(const FName& ParameterName, const FString& ParameterType, UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShaderPrim, EDirection ConversionType);
    static FMaterialBinding ParsePrimMaterial(pxr::UsdPrim Prim);
};

struct FUSDGeomMeshAttributes
{
    pxr::VtArray<pxr::GfVec3f> Points;
    pxr::VtArray<int32> FaceVertexCounts;
    pxr::VtArray<int32> FaceVertexIndices; 
    pxr::VtArray<pxr::GfVec3f> Colors;
    pxr::VtArray<float> Opacities;
    pxr::VtArray<pxr::GfVec3f> Normals;
    pxr::VtArray<pxr::VtArray<pxr::GfVec2f>> UVs;
    pxr::VtArray<pxr::GfVec3f> TangentX;
    pxr::VtArray<pxr::VtArray<int>> FaceIndices;
};

struct FUSDExporter:public FUSDConversion
{
    pxr::UsdStagePtr Stage;
    TMap<FString, TWeakObjectPtr<UObject>> PathToObject;
    pxr::UsdTimeCode TimeCode = pxr::UsdTimeCode::Default();
    TArray<FString> Changes;
    pxr::UsdPrim RootPrim;
    TFunction<pxr::UsdGeomImageable(class USceneComponent&, bool&)> CustomExport;
    TSet<FString> ReloadPaths;

    pxr::UsdGeomImageable ExportSceneObject(class USceneComponent& SceneComp);
    pxr::UsdGeomImageable ExportSceneComponent(class USceneComponent& SceneComp);
    bool ExportSceneComponent(const pxr::UsdGeomImageable& Imageable, class USceneComponent& SceneComp, bool bSkipChildren = false, bool bInheritParent = false, const TArray<struct FTimeSampleRange>* Ranges = nullptr);
    pxr::UsdGeomXformable ExportCameraActor(class ACameraActor& CameraActor);
    pxr::UsdGeomXformable ExportLightComponent(class ULightComponentBase& LightComp);
    pxr::UsdGeomXformable ExportCameraComponent(class UCameraComponent& CameraComp);
    pxr::UsdSkelRoot ExportSkeletalMeshComponent(class USkeletalMeshComponent& SkelMeshComp);
    pxr::UsdGeomXform ExportParticleSystemComponent(class UParticleSystemComponent& ParticleComp);
    pxr::UsdGeomGprim ExportStaticMeshComponent(class UStaticMeshComponent& StaticMeshComp);
    void ExportMeshComponent(const pxr::UsdGeomXformable& USDGprim, class UMeshComponent& MeshComp);
    void ExportMaterial(pxr::UsdPrim Model, class UMeshComponent& MeshComp, int32 SlotIndex);
    bool ExportMaterialInstance(class UMaterialInstance& MaterialInstance);
    bool IsPointInstancer(class USceneComponent& SceneComp);
    pxr::UsdGeomPointInstancer ExportPointInstancer(class USceneComponent& SceneComp);
    pxr::UsdGeomMesh ExportInstancedStaticMeshComponent(class UInstancedStaticMeshComponent& InstancedStaticMeshComp);
    void ExportSimReadyCameraShot(class UCameraComponent& CameraComponent);
    void ExportViewportCameraShot(class AActor* CameraActor, const TArray<struct FTimeSampleRange>& TimeSamples);
    bool HasValidChildren(const class USceneComponent& SceneComp);
    static void ExportMdlSchema(const pxr::UsdStageRefPtr& Stage, class UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader);
    static void ExportMdlSchemaParameters(const pxr::UsdStageRefPtr& Stage, class UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader, const FSimReadyExportTextureSettings& TextureSettings = FSimReadyExportTextureSettings(), const FString& CustomPath = TEXT(""), bool bUniqueName = false);
    static void ExportPreviewSurface(const pxr::UsdStageRefPtr& Stage, class UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader);
    static pxr::UsdGeomXformable ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const struct FMeshDescription& MeshDesc, const bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const struct FMeshDescription& MeshDesc, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportMeshDescription(const pxr::UsdStageRefPtr& Stage, const struct FMeshDescription& MeshDesc, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX = false);
    static pxr::UsdGeomXformable ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors = nullptr, const bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportStaticMesh(const pxr::UsdStageRefPtr& Stage, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX = false);
    static pxr::UsdGeomXformable ExportStaticMeshBackSide(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const FStaticMeshLODResources& LODModel, int32 SectionIndex, const FColorVertexBuffer* OverrideVertexColors = nullptr);
    static pxr::UsdGeomXform ExportParticleSystem(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const class UParticleSystemComponent& PartSys, pxr::UsdTimeCode TimeCode, bool bPreferPointInstancer, const bool bExportTangentX);
    static pxr::UsdGeomXformable ExportSkeletalMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const class FSkeletalMeshLODRenderData& LODModel, bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportSkeletalMesh(const pxr::UsdStageRefPtr& Stage, const class FSkeletalMeshLODRenderData& LODModel, FUSDGeomMeshAttributes& Attributes, const bool bExportTangentX = false, const bool bExportGeomSubset = false);
    static void ExportSkeletalMesh(const pxr::UsdStageRefPtr& Stage, const class FSkeletalMeshLODRenderData& LODModel, TArray<FUSDGeomMeshAttributes>& Attributes, const bool bExportTangentX = false);
    static pxr::UsdSkelRoot ExportSkeletalMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMesh& SkeletalMesh, const bool bExportTangentX = false, const bool bRootIdentity = false);
    static pxr::UsdGeomXformable ExportSkeletalMeshComponentToPointCache(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMeshComponent& SkeletalMesh, const bool bExportTangentX, pxr::UsdTimeCode TimeCode);
    static pxr::UsdGeomXformable ExportSkeletalMeshComponent(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMeshComponent& SkeletalMesh, const bool bExportTangentX, pxr::UsdTimeCode TimeCode);
    static bool ExportSkeletalMeshAnimation(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class UAnimationAsset* AnimationAsset, class USkeletalMesh* InSkeletalMesh, TArray<struct FTransformTimeSamples>& SocketTransformTimeSamples, const bool bRootIdentity = false);
    static bool ExportSkeletalMeshAnimationTimeSamples(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& NewPrimPath, class USkeletalMeshComponent& SkeletalMeshComponent, const TArray<struct FSkeletalAnimationTimeSamples>& AnimationTimeSamples, const double StartTimeCode, const double EndTimeCode, const double TimeCodesPerSecond, const bool bRootIdentity = false);
    static pxr::UsdGeomMesh ExportConvexMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const struct FKConvexElem& Convex);
    static pxr::UsdGeomMesh ExportTriangleMesh(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const struct FTriMeshCollisionData& TriMesh);
    static pxr::VtArray<pxr::UsdGeomGprim> ExportPhysics(const pxr::UsdGeomXformable& Xform, class UStaticMesh& StaticMesh);
    static pxr::UsdGeomMesh ExportBSP(const pxr::UsdStageRefPtr& Stage, UModel& BSPModel, const FString& CustomPrimName = TEXT(""));
    static void FillUSDMesh(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomMesh& USDMesh, const pxr::TfToken& Interpolation, const FUSDGeomMeshAttributes& Attributes);
    static void FillUSDMesh(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomXformable& USDMesh, const pxr::TfToken& Interpolation, const TArray<FUSDGeomMeshAttributes>& Attributes);
    static void ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& Path, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors, const bool bExportGeomSubset = false);
    static void ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors, FUSDGeomMeshAttributes& Attributes);
    static void ExportOverrideVertexColor(const pxr::UsdStageRefPtr& Stage, const struct FStaticMeshLODResources& LODModel, const class FColorVertexBuffer* OverrideVertexColors, TArray<FUSDGeomMeshAttributes>& Attributes);
    static void FillVertexColor(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomMesh& USDMesh, const pxr::TfToken& Interpolation, const FUSDGeomMeshAttributes& Attributes);
    static void FillVertexColor(const pxr::UsdStageRefPtr& Stage, pxr::UsdGeomXformable& USDMesh, const pxr::TfToken& Interpolation, const TArray<FUSDGeomMeshAttributes>& Attributes);
    static bool setLocalTransformMatrix(const pxr::UsdGeomXformable& xform, const FTransform& finalTransform/*const pxr::GfMatrix4d& final*/, const pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default(),
        bool bTranslateTimeSample = false, bool bRotateTimeSample = false, bool bScaleTimeSample = false,
        bool bIgnoreTranslate = false, bool bIgnoreRotate = false, bool bIgnoreScale = false);	// Copied from Graphene
    static void SetJointInfluencesAndWeights(const FSkeletalMeshLODRenderData& LODModel, const pxr::UsdGeomXformable& USDMesh, bool bIsGeomSubset = false);

    static void SetColorSpace(const FString& ParameterName, pxr::UsdShadeShader& Shader, bool bSRGB);
    static void ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, float ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input);
    static void ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, pxr::GfVec4f ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input);
    static void ExportInput(const FString& ParameterName, const pxr::SdfValueTypeName& ValueType, const FString& ParameterValue, bool bNewInput, pxr::UsdShadeShader& Shader, pxr::UsdShadeInput Input);

    template<typename T>
    static void ExportParameter(const pxr::SdfValueTypeName& ValueType, const FString& ParameterName, pxr::UsdShadeShader& ShadeShader, T ParameterValue)
    {
        if (!ParameterName.IsEmpty())
        {
            bool bInputFound = false;
            auto Input = ShadeShader.GetInput(pxr::TfToken(TCHAR_TO_ANSI(*ParameterName)));
            if (Input)
            {
                if (Input.HasConnectedSource())
                {
                    pxr::UsdShadeConnectableAPI Source;
                    pxr::TfToken SourceName;
                    pxr::UsdShadeAttributeType SourceType;
                    Input.GetConnectedSource(&Source, &SourceName, &SourceType);
                                    
                    auto ConnectInput = Source.GetInput(SourceName);
                    if (ConnectInput)
                    {
                        Input = ConnectInput;
                    }
                }

                bInputFound = true;
            }

            ExportInput(ParameterName, ValueType, ParameterValue, !bInputFound, ShadeShader, Input);
        }
    }

protected:
    pxr::SdfPath ToUSDPath(const FString& Path)
    {
        return pxr::SdfPath(TCHAR_TO_ANSI(*Path));
    }

    bool ShouldSaveMember(const TArray<FString>& Members = {}, bool bIncludeSub = true)const;
    bool ShouldSaveRenderState()const;

    template<typename PrimType>
    PrimType DefineNewPrim(USceneComponent& Component);

    template <class HalfType, class FloatType, class DoubleType, class ValueType>
    static bool setValuedWithPrecision(pxr::UsdGeomXformOp& xformOp, const ValueType& value, const pxr::UsdTimeCode& timeCode = pxr::UsdTimeCode::Default())
    {
        switch(xformOp.GetPrecision())
        {
        case pxr::UsdGeomXformOp::PrecisionHalf:
            return xformOp.Set(HalfType(FloatType(value)), timeCode);
        case pxr::UsdGeomXformOp::PrecisionFloat:
            return xformOp.Set(FloatType(value), timeCode);
        case pxr::UsdGeomXformOp::PrecisionDouble:
            return xformOp.Set(DoubleType(value), timeCode);
        }
        return false;
    };
};

template<typename PrimType>
PrimType FUSDExporter::DefineNewPrim(class USceneComponent& Component)
{
    // Return existing prim
    auto PathKey = PathToObject.FindKey((UObject*)&Component);
    if(PathKey)
    {
        auto Prim = PrimType::Get(Stage, ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*PathKey)));
        if(Prim)
        {
            return Prim;
        }
    }

    // Parent must exist
    pxr::SdfPath ParentPath;
    auto Parent = Component.GetAttachParent();
    if(Parent)
    {
        auto AddedParentPathKey = PathToObject.FindKey((UObject*)Parent);
        if(!AddedParentPathKey)
        {
            return PrimType();
        }

        ParentPath = ToUSDPath(FSimReadyPathHelper::KeyToPrimPath(*AddedParentPathKey));
    }
    else
    {
        if(RootPrim)
        {
            ParentPath = RootPrim.GetPath();
        }
        else
        {
            ParentPath = pxr::SdfPath::AbsoluteRootPath();
        }
    }

    // Create new prim
    const bool bRootComponent = Component.GetOwner() && Component.GetOwner()->GetRootComponent() == &Component;
    const bool bHasBlueprintComponents = Component.GetOwner() && Component.GetOwner()->BlueprintCreatedComponents.Num() > 0;

    auto PrimName = (bRootComponent && !bHasBlueprintComponents ? Component.GetOwner()->GetActorLabel() : Component.GetName());
    auto PrimPath = ParentPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PrimName)));

    while (PrimType::Get(Stage, PrimPath))
    {
        static int32 SuffixIndex = 0;
        PrimName = (bRootComponent && !bHasBlueprintComponents ? Component.GetOwner()->GetActorLabel() : Component.GetName()) + TEXT("_") + FString::FromInt(SuffixIndex++);
        PrimPath = ParentPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PrimName)));
    }

    auto Prim = PrimType::Define(Stage, PrimPath);
    if(Prim)
    {
        PathToObject.Add(FSimReadyPathHelper::PrimPathToKey(PrimPath.GetText())) = &Component;
    }

    return Prim;
}


// Helper to get value
template<typename TValue>
TValue GetUSDValue(const pxr::UsdAttribute& Attribute)
{
    TValue Value = TValue();
    if(Attribute)
    {
        std::vector<double> times;
        Attribute.GetTimeSamples(&times);

        Attribute.Get(&Value, times.size() > 0 ? times[0] : pxr::UsdTimeCode::Default());
    }

    return Value;
}

template<typename TUSDValue, typename TValue>
bool GetUSDTimeSamples(const pxr::UsdAttribute& Attribute, TValue& OutValues, TFunction<void(TUSDValue&)> OnPostGetAttribute = nullptr)
{
    if (Attribute)
    {
        std::vector<double> TimeSamples;
        Attribute.GetTimeSamples(&TimeSamples);

        for(auto TimeCode : TimeSamples)
        {
            TUSDValue Value;
            Attribute.Get(&Value, pxr::UsdTimeCode(TimeCode));
            if (OnPostGetAttribute)
            {
                OnPostGetAttribute(Value);
            }
            OutValues.TimeSamples.Add(TimeCode, USDConvert(Value));
        }

        return TimeSamples.size() > 0;
    }

    return false;
}

template <typename ScalarType>
void ComputeFlattened(const pxr::UsdGeomPrimvar& Primvar, pxr::VtArray<ScalarType>* Value)
{
    if(Primvar)
    {
        std::vector<double> times;
        Primvar.GetTimeSamples(&times);
        Primvar.ComputeFlattened(Value, times.size() > 0 ? times[0] : pxr::UsdTimeCode::Default());
    }
}

template <typename T>
void FindAllPrims(const pxr::UsdPrim& Prim, TArray<pxr::UsdPrim>& OutPrims)
{
    if (Prim.IsA<T>())
    {
        OutPrims.Add(Prim); 
    }

    auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
    for(auto ChildPrim : ChildPrims)
    {
        FindAllPrims<T>(ChildPrim, OutPrims);
    }
}