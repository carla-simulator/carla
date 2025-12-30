// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDHashGenerator.h"
#include "USDConverter.h"
#include "SimReadyUSDTokens.h"

void UpdateHashFromStage(const pxr::UsdStageRefPtr& USDStage, FSHA1& HashState)
{
    uint8 UpAxis = GetUSDStageAxis(USDStage) == pxr::UsdGeomTokens->z;
    HashState.Update((uint8*)&UpAxis, sizeof(uint8));
    const double USDMetersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(USDStage);
    HashState.Update((uint8*)&USDMetersPerUnit, sizeof(double));
}

void UpdateHashFromGeomMesh(const pxr::UsdGeomMesh& USDMesh, FSHA1& HashState)
{
    auto FaceVertexCounts = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexCountsAttr());
    HashState.Update((uint8*)FaceVertexCounts.data(), FaceVertexCounts.size() * sizeof(int));

    auto Points = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDMesh.GetPointsAttr());
    HashState.Update((uint8*)Points.data(), Points.size() * sizeof(pxr::GfVec3f));

    auto FaceVertexIndices = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexIndicesAttr());
    HashState.Update((uint8*)FaceVertexIndices.data(), FaceVertexIndices.size() * sizeof(int));

    if (auto NormalAttri = USDMesh.GetNormalsAttr())
    {
        auto NormalArray = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(NormalAttri);
        HashState.Update((uint8*)NormalArray.data(), NormalArray.size() * sizeof(pxr::GfVec3f));
    }

    auto ColorPrimvar = USDMesh.GetDisplayColorPrimvar();
    if (ColorPrimvar)
    {
        pxr::VtArray<pxr::GfVec3f> USDColors;
        ComputeFlattened<pxr::GfVec3f>(ColorPrimvar, &USDColors);
        HashState.Update((uint8*)USDColors.data(), USDColors.size() * sizeof(pxr::GfVec3f));
    }

    auto OpacityPrimvar = USDMesh.GetDisplayOpacityPrimvar();
    if (OpacityPrimvar)
    {
        pxr::VtArray<float> USDOpacities;
        ComputeFlattened<float>(OpacityPrimvar, &USDOpacities);
        HashState.Update((uint8*)USDOpacities.data(), USDOpacities.size() * sizeof(float));
    }

    auto Primvars = USDMesh.GetPrimvars();
    for (auto Iter = Primvars.begin(); Iter != Primvars.end(); ++Iter)
    {
        auto PrimVar = *Iter;

        // Texture UV
        if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Float2Array.GetCPPTypeName()) == 0)
        {
            // Copy data
            pxr::VtArray<pxr::GfVec2f> UVArray;
            ComputeFlattened<pxr::GfVec2f>(PrimVar, &UVArray);
            HashState.Update((uint8*)UVArray.data(), UVArray.size() * sizeof(pxr::GfVec2f));
        }
        // Vertex Color
        else if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Color4fArray.GetCPPTypeName()) == 0)
        {
            // Copy data
            pxr::VtArray<pxr::GfVec4f> ColorArray;
            ComputeFlattened<pxr::GfVec4f>(PrimVar, &ColorArray);
            HashState.Update((uint8*)ColorArray.data(), ColorArray.size() * sizeof(pxr::GfVec4f));
        }
    }

    auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
    if (!USDGeomSubsets.empty())
    {
        for (auto USDGeomSubset : USDGeomSubsets)
        {
            auto FaceIndices = GetUSDValue<pxr::VtArray<int>>(USDGeomSubset.GetIndicesAttr());
            HashState.Update((uint8*)FaceIndices.data(), FaceIndices.size() * sizeof(int));
        }
    }
}

FSHAHash FUSDHashGenerator::ComputeSHAHash(const pxr::UsdGeomMesh& USDMesh)
{
    FSHA1 HashState;
    FSHAHash OutHash;
    auto UsdStage = USDMesh.GetPrim().GetStage();
    UpdateHashFromStage(UsdStage, HashState);
    UpdateHashFromGeomMesh(USDMesh, HashState);
    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    return OutHash;
}

FSHAHash FUSDHashGenerator::ComputeSHAHash(const pxr::UsdSkelCache& SkelCache, const pxr::UsdSkelBinding& SkelBinding)
{
    FSHA1 HashState;
    FSHAHash OutHash;

    // Skinning and Skeleton are bound together
    // Skeleton
    const pxr::UsdSkelSkeleton& Skeleton = SkelBinding.GetSkeleton();
    pxr::UsdSkelSkeletonQuery SkelQuery = SkelCache.GetSkelQuery(Skeleton);

    auto UsdStage = Skeleton.GetPrim().GetStage();
    UpdateHashFromStage(UsdStage, HashState);

    pxr::VtArray<pxr::TfToken> JointOrder = SkelQuery.GetJointOrder();
    const pxr::UsdSkelTopology& Topology = SkelQuery.GetTopology();
    for (uint32 Index = 0; Index < Topology.GetNumJoints(); ++Index)
    {
        FString BonePath = pxr::SdfPath(JointOrder[Index]).GetText();
        HashState.UpdateWithString(*BonePath, BonePath.Len());
        int ParentIndex = Topology.GetParent(Index);
        HashState.Update((uint8*)&ParentIndex, sizeof(int));
    }

    pxr::VtArray<pxr::GfMatrix4d> WorldBindTransforms;
    if (SkelQuery.GetJointWorldBindTransforms(&WorldBindTransforms))
    {
        HashState.Update((uint8*)WorldBindTransforms.data(), WorldBindTransforms.size() * sizeof(pxr::GfMatrix4d));
    }

    // Skinning
    for (const pxr::UsdSkelSkinningQuery& SkinningQuery : SkelBinding.GetSkinningTargets())
    {
        pxr::UsdGeomMesh SkinningMesh = pxr::UsdGeomMesh(SkinningQuery.GetPrim());
        if (SkinningMesh)
        {
            const pxr::UsdPrim& SkinningPrim = SkinningQuery.GetPrim();
            pxr::UsdSkelBindingAPI SkinningBinding(SkinningPrim);
            pxr::UsdAttribute GeomBindingAttribute = SkinningBinding.GetGeomBindTransformAttr();
            if (GeomBindingAttribute)
            {
                pxr::GfMatrix4d GeomBindingTransform(1);
                GeomBindingAttribute.Get(&GeomBindingTransform);
                HashState.Update((uint8*)&GeomBindingTransform, sizeof(pxr::GfMatrix4d));
            }

            pxr::UsdGeomMesh USDMesh = pxr::UsdGeomMesh(SkinningPrim);
            UpdateHashFromGeomMesh(USDMesh, HashState);
            
            // Joint in skinning
            pxr::VtArray<pxr::TfToken> SkinningJointOrder;
            if (SkinningQuery.GetJointOrder(&SkinningJointOrder))
            {
                for (uint32 Index = 0; Index < SkinningJointOrder.size(); ++Index)
                {
                    FString BonePath = pxr::SdfPath(SkinningJointOrder[Index]).GetText();
                    HashState.UpdateWithString(*BonePath, BonePath.Len());
                }
            }

            pxr::VtArray<int> JointIndices;
            pxr::VtArray<float> JointWeights;
            auto Points = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDMesh.GetPointsAttr());
            SkinningQuery.ComputeVaryingJointInfluences(Points.size(), &JointIndices, &JointWeights);
            HashState.Update((uint8*)JointIndices.data(), JointIndices.size() * sizeof(int));
            HashState.Update((uint8*)JointIndices.data(), JointIndices.size() * sizeof(float));
        }
    }

    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    return OutHash;
}

FSHAHash FUSDHashGenerator::ComputeSHAHash(const pxr::UsdSkelAnimQuery& AnimQuery, double StartTimeCode, double EndTimeCode, double TimeCodesPerSecond)
{
    FSHA1 HashState;
    FSHAHash OutHash;

    auto UsdStage = AnimQuery.GetPrim().GetStage();
    UpdateHashFromStage(UsdStage, HashState);

    HashState.Update((uint8*)&StartTimeCode, sizeof(double));
    HashState.Update((uint8*)&EndTimeCode, sizeof(double));
    HashState.Update((uint8*)&TimeCodesPerSecond, sizeof(double));

    std::vector<double> TimeSamples;
    if (AnimQuery.GetJointTransformTimeSamples(&TimeSamples))
    {
        for (double TimeSample : TimeSamples)
        {
            HashState.Update((uint8*)&TimeSample, sizeof(double));
            pxr::VtArray<pxr::GfMatrix4d> USDJointLocalTransforms;
            AnimQuery.ComputeJointLocalTransforms(&USDJointLocalTransforms, TimeSample);
            HashState.Update((uint8*)USDJointLocalTransforms.data(), USDJointLocalTransforms.size() * sizeof(pxr::GfMatrix4d));
        }
    }

    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    return OutHash;
}

FSHAHash FUSDHashGenerator::ComputeSHAHash(const pxr::UsdSkelSkeleton& SkelSkeleton, bool bWithRestTransforms)
{
    FSHA1 HashState;
    FSHAHash OutHash;

    auto UsdStage = SkelSkeleton.GetPrim().GetStage();
    UpdateHashFromStage(UsdStage, HashState);

    auto BindTransforms = GetUSDValue<pxr::VtArray<pxr::GfMatrix4d>>(SkelSkeleton.GetBindTransformsAttr());
    HashState.Update((uint8*)BindTransforms.data(), BindTransforms.size() * sizeof(pxr::GfMatrix4d));
    if (bWithRestTransforms)
    {
        auto RestTransforms = GetUSDValue<pxr::VtArray<pxr::GfMatrix4d>>(SkelSkeleton.GetRestTransformsAttr());
        HashState.Update((uint8*)RestTransforms.data(), RestTransforms.size() * sizeof(pxr::GfMatrix4d));
    }

    auto Joints = GetUSDValue<pxr::VtArray<pxr::TfToken>>(SkelSkeleton.GetJointsAttr());
    for (auto Joint : Joints)
    {
        FString JointPath = pxr::SdfPath(Joint).GetText();
        HashState.UpdateWithString(*JointPath, JointPath.Len());
    }

    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    return OutHash;
}

void UpdateHashFromInput(const pxr::UsdShadeInput& Input, FSHA1& HashState)
{
    pxr::VtValue VtValue;
    if (Input && Input.Get(&VtValue))
    {
        FString BaseName = Input.GetBaseName().GetText();
        HashState.UpdateWithString(*BaseName, BaseName.Len());
        FString DisplayName = Input.GetAttr().GetDisplayName().c_str();
        if (!DisplayName.IsEmpty())
        {
            HashState.UpdateWithString(*DisplayName, DisplayName.Len());
        }

        auto Type = VtValue.GetType();
        if (Type == pxr::SdfValueTypeNames->Float.GetType())
        {	
            float Value = VtValue.Get<float>();
            HashState.Update((uint8*)&Value, sizeof(float));
        }
        else if (Type == pxr::SdfValueTypeNames->Int.GetType())
        {
            int32 Value = VtValue.Get<int>();
            HashState.Update((uint8*)&Value, sizeof(int));
        }
        else if (Type == pxr::SdfValueTypeNames->Bool.GetType())
        {
            bool Value = VtValue.Get<bool>();
            HashState.Update((uint8*)&Value, sizeof(bool));
        }
        else if (Type == pxr::SdfValueTypeNames->Float2.GetType())
        {
            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec2f>());
            HashState.Update((uint8*)&Value, sizeof(FLinearColor));
        }
        else if (Type == pxr::SdfValueTypeNames->Float3.GetType())
        {
            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec3f>());
            HashState.Update((uint8*)&Value, sizeof(FLinearColor));
        }
        else if (Type == pxr::SdfValueTypeNames->Float4.GetType())
        {
            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec4f>());
            HashState.Update((uint8*)&Value, sizeof(FLinearColor));
        }
        else if (Type == pxr::SdfValueTypeNames->Asset.GetType())
        {
            pxr::SdfAssetPath AssetPath = VtValue.Get<pxr::SdfAssetPath>();
            FString Path = AssetPath.GetAssetPath().c_str();
            HashState.UpdateWithString(*Path, Path.Len());
        }
        else if (Type == pxr::SdfValueTypeNames->Token.GetType())
        {
            FString Token = VtValue.Get<pxr::TfToken>().GetText();
            HashState.UpdateWithString(*Token, Token.Len());
        }
    }
}

extern void UpdateHashFromShader(const pxr::UsdShadeShader& Shader, FSHA1& HashState);
extern void UpdateHashFromNodeGraph(const pxr::UsdShadeNodeGraph& NodeGraph, FSHA1& HashState);

void UpdateHashFromShader(const pxr::UsdShadeShader& Shader, FSHA1& HashState)
{
    pxr::TfToken IdToken;
    if (Shader.GetShaderId(&IdToken))
    {
        FString Id = IdToken.GetText();
        HashState.UpdateWithString(*Id, Id.Len());
    }

    const auto ImplementationSourceToken = Shader.GetImplementationSource();
    FString ImplementationSource = ImplementationSourceToken.GetText();
    if (!ImplementationSource.IsEmpty())
    {
        HashState.UpdateWithString(*ImplementationSource, ImplementationSource.Len());
    }

    pxr::SdfAssetPath AssetPath;
    if (Shader.GetSourceAsset(&AssetPath, USDTokens.mdl))
    {
        FString Path = AssetPath.GetAssetPath().c_str();
        HashState.UpdateWithString(*Path, Path.Len());
    }

    pxr::TfToken MaterialNameToken; 
    if (Shader.GetSourceAssetSubIdentifier(&MaterialNameToken, USDTokens.mdl))
    {
        FString MaterialName = MaterialNameToken.GetText();
        HashState.UpdateWithString(*MaterialName, MaterialName.Len());
    }

    std::vector<pxr::UsdShadeInput> Inputs = Shader.GetInputs();
    for (auto Input : Inputs)
    {
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI InputSource;
            pxr::TfToken InputSourceName;
            pxr::UsdShadeAttributeType InputSourceType;
            if (Input.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType))
            {
                if (InputSourceType == pxr::UsdShadeAttributeType::Input)
                {
                    auto ConnectInput = InputSource.GetInput(InputSourceName);
                    UpdateHashFromInput(ConnectInput, HashState);
                }
                else if (InputSourceType == pxr::UsdShadeAttributeType::Output)
                {
                    if (InputSource.GetPrim().IsA<pxr::UsdShadeShader>())
                    {
                        UpdateHashFromShader(pxr::UsdShadeShader(InputSource.GetPrim()), HashState);
                    }
                    else if (InputSource.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
                    {
                        UpdateHashFromNodeGraph(pxr::UsdShadeNodeGraph(InputSource.GetPrim()), HashState);
                    }
                }
            }
        }
        else
        {
            UpdateHashFromInput(Input, HashState);
        }
    }

    std::vector<pxr::UsdShadeOutput> Outputs = Shader.GetOutputs();
    for (auto Output : Outputs)
    {
        if (Output && Output.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI OutputSource;
            pxr::TfToken OutputSourceName;
            pxr::UsdShadeAttributeType OutputSourceType;
            Output.GetConnectedSource(&OutputSource, &OutputSourceName, &OutputSourceType);

            UpdateHashFromShader(pxr::UsdShadeShader(OutputSource.GetPrim()), HashState);
        }
    }
}

void UpdateHashFromNodeGraph(const pxr::UsdShadeNodeGraph& NodeGraph, FSHA1& HashState)
{
    std::vector<pxr::UsdShadeInput> Inputs = NodeGraph.GetInputs();
    for (auto Input : Inputs)
    {
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI InputSource;
            pxr::TfToken InputSourceName;
            pxr::UsdShadeAttributeType InputSourceType;
            Input.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);
            if (InputSource.GetPrim().IsA<pxr::UsdShadeShader>())
            {
                UpdateHashFromShader(pxr::UsdShadeShader(InputSource.GetPrim()), HashState);
            }
            else if (InputSource.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
            {
                UpdateHashFromNodeGraph(pxr::UsdShadeNodeGraph(InputSource.GetPrim()), HashState);
            }
        }
        else
        {
            UpdateHashFromInput(Input, HashState);
        }
    }

    std::vector<pxr::UsdShadeOutput> Outputs = NodeGraph.GetOutputs();
    for (auto Output : Outputs)
    {
        if (Output && Output.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI OutputSource;
            pxr::TfToken OutputSourceName;
            pxr::UsdShadeAttributeType OutputSourceType;
            Output.GetConnectedSource(&OutputSource, &OutputSourceName, &OutputSourceType);

            UpdateHashFromShader(pxr::UsdShadeShader(OutputSource.GetPrim()), HashState);
        }
    }
}

FSHAHash FUSDHashGenerator::ComputeSHAHash(const pxr::UsdShadeMaterial& USDMaterial)
{
    FSHA1 HashState;
    FSHAHash OutHash;

    if(USDMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        auto SurfaceOutput = USDMaterial.GetSurfaceOutput();
        if (SurfaceOutput)
        {
            if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
            {
                UpdateHashFromShader(pxr::UsdShadeShader(Source), HashState);
            }
        }

        auto MdlSurfaceOutput = USDMaterial.GetSurfaceOutput(USDTokens.mdl);
        if (MdlSurfaceOutput)
        {
            if (MdlSurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
            {
                UpdateHashFromShader(pxr::UsdShadeShader(Source), HashState);
            }			
        }
    }

    HashState.Final();
    HashState.GetHash(&OutHash.Hash[0]);

    return OutHash;
}