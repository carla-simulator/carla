// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDCARLAVehicleTools.h"
#include "Containers/ArrayBuilder.h"
#include "USDConverter.h"
#include "SimReadyUSDTokens.h"
#include "Components/LightComponentBase.h"
#include "SimReadySettings.h"
#include "SimReadyUsdLuxLightCompat.h"

#define CARLA_VEHICLE_BP				TEXT("BaseVehiclePawn_C")
#define CARLA_VEHICLE_BP_NW				TEXT("BaseVehiclePawnNW_C")
#define CARLA_VEHICLELIGHT_MAT			TEXT("M_VehicleLightsMaster")
#define CARLA_VEHICLELIGHT_MAT2			TEXT("M_VehicleLights_Master")
#define CARLA_VEHICLEPLATE_MAT			TEXT("M_LicensePlate_Master")
#define CARLA_VEHICLELIGHTSIRENS_MAT	TEXT("M_VehicleLights_Sirens_Master")
#define CARLA_VEHICLEPLATE_MAT2			TEXT("M_LicensePlate")
#define CARLA_VEHICLEPLATE_MAT3			TEXT("M_LicensePlate_1")
#define CARLA_VEHICLELIGHTGLASS_MAT		TEXT("M_CarLightsGlass_Master")

enum ELightType
{
    FrontPosition,
    BackPosition,
    Brake,
    LeftBlinker,
    RightBlinker,
    FrontFog,
    BackFog,
    Reverse,
    HighBeam,
    LowBeam,
    LightCount,
};

static const TArray<FName> VehicleJointNames = TArrayBuilder<FName>()
    .Add(TEXT("Vehicle_Base"))
    .Add(TEXT("Wheel_Front_Left"))
    .Add(TEXT("Wheel_Front_Right"))
    .Add(TEXT("Wheel_Middle_Left"))
    .Add(TEXT("Wheel_Middle_Right"))
    .Add(TEXT("Wheel_Rear_Left"))
    .Add(TEXT("Wheel_Rear_Right"));

namespace SimReadyVehicleTokens
{
    extern const pxr::TfToken VehicleAttrName("omni:simready:vehicle");

    // Vehicle
    extern const pxr::TfToken Vehicle("vehicle");
    extern const pxr::TfToken Body("body");
    extern const pxr::TfToken Plate("plate");
    extern const pxr::TfToken Wheel("wheel");
    extern const pxr::TfToken Door("door");
    extern const pxr::TfToken Window("window");
    extern const pxr::TfToken Trunk("trunk");

    extern const pxr::TfToken LongitudinalAxisName("omni:simready:vehicle:longitudinalAxis");
    extern const pxr::TfToken PosX("posX");
    extern const pxr::TfToken PosY("posY");
    extern const pxr::TfToken PosZ("posZ");
    extern const pxr::TfToken NegX("negX");
    extern const pxr::TfToken NegY("negY");
    extern const pxr::TfToken NegZ("negZ");


    extern const pxr::TfToken DrivetrainName("omni:simready:vehicle:drivetrain");
    extern const pxr::TfToken RWD("RWD");
    extern const pxr::TfToken FourWD("4WD");
    extern const pxr::TfToken AWD("AWD");
    extern const pxr::TfToken FWD("FWD");

    extern const pxr::TfToken SteeringName("omni:simready:vehicle:steering");
    extern const pxr::TfToken Front("front");
    extern const pxr::TfToken Back("back");
    extern const pxr::TfToken All("all");
    extern const pxr::TfToken None("none");
    extern const pxr::TfToken Tank("tank");

    extern const pxr::TfToken LightAttrName("omni:simready:light");

    // Light
    extern const pxr::TfToken BrakeLight("brakeLights");
    extern const pxr::TfToken EmergencyLight("emergencyLights");
    extern const pxr::TfToken FogLight("fogLights");
    extern const pxr::TfToken HeadLight("headLights");
    extern const pxr::TfToken HighbeamLight("highbeamLights");
    extern const pxr::TfToken MarkerLight("markerLights");
    extern const pxr::TfToken Night("nightLights");
    extern const pxr::TfToken ParkingLight("parkingLights");
    extern const pxr::TfToken PlateLight("plateLights");
    extern const pxr::TfToken ReverseLight("reverseLights");
    extern const pxr::TfToken RunningLight("runningLights");
    extern const pxr::TfToken SignalLight("signalLights");
    extern const pxr::TfToken SignalLLight("signalLightsL");
    extern const pxr::TfToken SignalRLight("signalLightsR");
    extern const pxr::TfToken TailLight("tailLights");
    extern const pxr::TfToken InteriorLight("interiorLights");

    // Non visual
    extern const pxr::TfToken ClearGlass("clear_glass");
    extern const pxr::TfToken Steel("steel");
    extern const pxr::TfToken PaintClearcoat("paint_clearcoat");
    extern const pxr::TfToken Rubber("rubber");
}

namespace USDCARLAVehicleToolsInternal
{
    struct FJointInfo
    {
        FName JointName;
        int32 Index = INDEX_NONE;
        FVector Offset;
    };

    void GetJointIndices(const pxr::UsdSkelRoot& SkelRoot, TArray<FJointInfo>& OutJointIndices)
    {
        OutJointIndices.AddZeroed(VehicleJointNames.Num());
        pxr::UsdSkelCache USDSkelCache;
        USDSkelCache.Populate(SkelRoot);

        std::vector< pxr::UsdSkelBinding > USDSkeletonBindings;
        USDSkelCache.ComputeSkelBindings(SkelRoot, &USDSkeletonBindings);

        for (auto SkeletonBinding : USDSkeletonBindings)
        {
            auto Skeleton = SkeletonBinding.GetSkeleton();
            auto BindTransforms = GetUSDValue<pxr::VtArray<pxr::GfMatrix4d>>(Skeleton.GetBindTransformsAttr());
            auto Joints = GetUSDValue<pxr::VtArray<pxr::TfToken>>(Skeleton.GetJointsAttr());
            int32 JointIndex = 0;
            for (auto Joint : Joints)
            {
                for (int32 TargetIndex = 0; TargetIndex < VehicleJointNames.Num(); ++TargetIndex)
                {
                    if (FString(Joint.GetText()).EndsWith(VehicleJointNames[TargetIndex].ToString()))
                    {
                        OutJointIndices[TargetIndex] = { VehicleJointNames[TargetIndex], JointIndex, USDConvert(BindTransforms[JointIndex].ExtractTranslation()) };
                        break;
                    }
                }

                ++JointIndex;
            }
        }
    }

    void GetAffectedVertices(const pxr::UsdPrim& MeshPrim, int32 JointIndex, TArray<int32>& OutIndices)
    {
        pxr::UsdSkelBindingAPI SkelBindingAPI{ MeshPrim };

        auto JointWeights = GetUSDValue<pxr::VtArray<float>>(SkelBindingAPI.GetJointWeightsAttr());
        auto JointIndices = GetUSDValue<pxr::VtArray<int>>(SkelBindingAPI.GetJointIndicesAttr());

        check(JointWeights.size() == JointIndices.size());
        int32 ElementSize = SkelBindingAPI.GetJointIndicesPrimvar().GetElementSize();
        auto NumVertices = JointWeights.size() / ElementSize;

        for (auto VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
        {
            for (auto Index = VertexIndex * ElementSize; Index < (VertexIndex + 1) * ElementSize; ++Index)
            {
                if (JointIndices[Index] == JointIndex && JointWeights[Index] > 0.0f)
                {
                    OutIndices.Add(VertexIndex);
                }
            }
        }
    }

    struct FIslandInfo
    {
        pxr::SdfPath Path;		// Outout path
        TArray<int32> Indices; // Inidices in island
        FVector Offset;		// Offset to the world origin
        int32 SubMeshIndex = INDEX_NONE; // Create submesh if SubMeshIndex isn't INDEX_NONE
    };

    void SplitMeshByIslandsWithAttributes(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& MeshPrim, const TArray<FIslandInfo>& Islands, const pxr::TfToken& Interpolation)
    {
        auto USDMesh = pxr::UsdGeomMesh(MeshPrim);
        if (USDMesh)
        {
            auto FaceVertexCounts = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexCountsAttr());
            auto Points = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDMesh.GetPointsAttr());
            auto FaceVertexIndices = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexIndicesAttr());

            pxr::VtArray<pxr::GfVec3f> NormalArray;
            if (USDMesh.GetNormalsAttr())
            {
                NormalArray = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDMesh.GetNormalsAttr());
            }

            pxr::VtArray<pxr::GfVec3f> USDColors;
            pxr::TfToken ColorInterpType;
            if (USDMesh.GetDisplayColorPrimvar())
            {
                ComputeFlattened<pxr::GfVec3f>(USDMesh.GetDisplayColorPrimvar(), &USDColors);
                ColorInterpType = USDMesh.GetDisplayOpacityPrimvar().GetInterpolation();
            }

            pxr::VtArray<float> USDOpacities;
            if (USDMesh.GetDisplayOpacityPrimvar())
            {
                ComputeFlattened<float>(USDMesh.GetDisplayOpacityPrimvar(), &USDOpacities);
            }

            pxr::VtArray<pxr::VtArray<pxr::GfVec2f>> UVArray;
            auto Primvars = USDMesh.GetPrimvars();
            for (auto Iter = Primvars.begin(); Iter != Primvars.end(); ++Iter)
            {
                auto PrimVar = *Iter;

                // Texture UV
                if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Float2Array.GetCPPTypeName()) == 0)
                {
                    FString Name = PrimVar.GetName().GetText();
                    if (Name.StartsWith("primvars:vc"))
                    {
                        continue;
                    }

                    pxr::VtArray<pxr::GfVec2f> UV;
                    ComputeFlattened<pxr::GfVec2f>(PrimVar, &UV);
                    UVArray.push_back(UV);
                }
            }

            for (int32 IslandIndex = 0; IslandIndex < Islands.Num(); ++IslandIndex)
            {
                if (Islands[IslandIndex].Indices.Num() == 0)
                {
                    continue;
                }

                pxr::UsdGeomMesh SubMesh;
                pxr::UsdGeomXformable Xformable;
                auto Path = Islands[IslandIndex].Path;
                if (Islands[IslandIndex].SubMeshIndex != INDEX_NONE)
                {
                    auto Xform = pxr::UsdGeomXform::Define(USDStage, Path);
                    FString UniqueName = FString::Printf(TEXT("%s_%d"), *FString(Xform.GetPrim().GetName().GetText()), Islands[IslandIndex].SubMeshIndex);
                    Path = Path.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*UniqueName)));
                    SubMesh = pxr::UsdGeomMesh::Define(USDStage, Path);
                    Xformable = Xform;
                }
                else 
                {
                    SubMesh = pxr::UsdGeomMesh::Define(USDStage, Path);
                    Xformable = SubMesh;
                }
                pxr::VtArray<pxr::GfVec3f> SubMeshPoints;
                pxr::VtArray<int> SubMeshFaceVertexIndices;
                pxr::VtArray<int> SubMeshFaceVertexCounts;

                // might be vertex or face varying
                pxr::VtArray<pxr::GfVec3f> SubMeshNormals;
                pxr::VtArray<pxr::GfVec3f> SubMeshColors;
                pxr::VtArray<float> SubMeshOpacities;
                pxr::VtArray<pxr::VtArray<pxr::GfVec2f>> SubMeshUVs;
                SubMeshUVs.resize(UVArray.size());

                auto Island = Islands[IslandIndex];
                auto IslandOffset = USDConvert(Island.Offset);
                TMap<int32, int32> HashTable;
                int32 WeldIndex = 0;
                for (int32 OldIndex : FaceVertexIndices)
                {
                    bool bIsInIsland = false;
                    for (int32 IslandId : Island.Indices)
                    {
                        if (Interpolation == pxr::UsdGeomTokens->vertex)
                        {
                            if (IslandId == OldIndex)
                            {
                                bIsInIsland = true;
                                break;
                            }
                        }
                        else if (Interpolation == pxr::UsdGeomTokens->faceVarying)
                        {
                            if (IslandId == WeldIndex)
                            {
                                bIsInIsland = true;
                                break;
                            }
                        }
                    }

                    if (!bIsInIsland)
                    {
                        ++WeldIndex;
                        continue;
                    }

                    int32* FoundIndex = HashTable.Find(OldIndex);
                    int32 NewIndex = 0;
                    if (FoundIndex == nullptr)
                    {
                        NewIndex = SubMeshPoints.size();
                        HashTable.Add(OldIndex, NewIndex);
                        SubMeshPoints.push_back(Points[OldIndex] - IslandOffset);

                        if (Interpolation == pxr::UsdGeomTokens->vertex)
                        {
                            SubMeshNormals.push_back(NormalArray[OldIndex]);
                            if (ColorInterpType != pxr::UsdGeomTokens->constant)
                            {
                                SubMeshColors.push_back(USDColors[OldIndex]);
                                SubMeshOpacities.push_back(USDOpacities[OldIndex]);
                            }
                            for (int UVIndex = 0; UVIndex < UVArray.size(); ++UVIndex)
                            {
                                SubMeshUVs[UVIndex].push_back(UVArray[UVIndex][OldIndex]);
                            }
                        }
                    }
                    else
                    {
                        NewIndex = *FoundIndex;
                    }

                    SubMeshFaceVertexIndices.push_back(NewIndex);

                    if (Interpolation == pxr::UsdGeomTokens->faceVarying)
                    {
                        SubMeshNormals.push_back(NormalArray[WeldIndex]);
                        if (ColorInterpType != pxr::UsdGeomTokens->constant)
                        {
                            SubMeshColors.push_back(USDColors[WeldIndex]);
                            SubMeshOpacities.push_back(USDOpacities[WeldIndex]);
                        }
                        for (int UVIndex = 0; UVIndex < UVArray.size(); ++UVIndex)
                        {
                            SubMeshUVs[UVIndex].push_back(UVArray[UVIndex][WeldIndex]);
                        }
                    }

                    ++WeldIndex;
                }

                for (int Index = 0; Index < SubMeshFaceVertexIndices.size() / 3; ++Index)
                {
                    SubMeshFaceVertexCounts.push_back(3);
                }

                SubMesh.CreatePointsAttr(pxr::VtValue(SubMeshPoints));
                SubMesh.CreateFaceVertexCountsAttr(pxr::VtValue(SubMeshFaceVertexCounts));
                SubMesh.CreateFaceVertexIndicesAttr(pxr::VtValue(SubMeshFaceVertexIndices));

                pxr::VtArray<pxr::GfVec3f> Extent;
                pxr::UsdGeomPointBased::ComputeExtent(SubMeshPoints, &Extent);
                SubMesh.CreateExtentAttr().Set(Extent);

                SubMesh.CreateNormalsAttr(pxr::VtValue(SubMeshNormals));
                SubMesh.SetNormalsInterpolation(Interpolation);
                SubMesh.CreateSubdivisionSchemeAttr(pxr::VtValue(USDTokens.none));

                if (ColorInterpType == pxr::UsdGeomTokens->constant)
                {
                    auto ColorPrimvar = SubMesh.CreateDisplayColorPrimvar(pxr::UsdGeomTokens->constant);
                    ColorPrimvar.Set(USDColors);
                    auto OpacityPrimvar = SubMesh.CreateDisplayOpacityPrimvar(pxr::UsdGeomTokens->constant);
                    OpacityPrimvar.Set(USDOpacities);
                }
                else
                {
                    auto ColorPrimvar = SubMesh.CreateDisplayColorPrimvar(Interpolation);
                    ColorPrimvar.Set(SubMeshColors);
                    auto OpacityPrimvar = SubMesh.CreateDisplayOpacityPrimvar(Interpolation);
                    OpacityPrimvar.Set(SubMeshOpacities);
                }

                for (int i = 0; i < SubMeshUVs.size(); ++i)
                {
                    auto Primvar = SubMesh.CreatePrimvar(pxr::TfToken(pxr::UsdUtilsGetPrimaryUVSetName().GetString() + (i == 0 ? "" : std::to_string(i))), pxr::SdfValueTypeNames->TexCoord2fArray, Interpolation);
                    Primvar.Set(SubMeshUVs[i]);
                }

                Xformable.ClearXformOpOrder();
                auto TranslateOp = Xformable.AddTranslateOp(pxr::UsdGeomXformOp::PrecisionDouble);
                TranslateOp.Set(pxr::GfVec3d(IslandOffset));
                auto RotateOp = Xformable.AddRotateXYZOp(pxr::UsdGeomXformOp::PrecisionDouble);
                RotateOp.Set(pxr::GfVec3d(0.0));
                auto ScaleOp = Xformable.AddScaleOp(pxr::UsdGeomXformOp::PrecisionDouble);
                ScaleOp.Set(pxr::GfVec3d(1.0));
                std::vector<pxr::UsdGeomXformOp> OrderedOps = { TranslateOp, RotateOp, ScaleOp };
                Xformable.SetXformOpOrder(OrderedOps);

                auto USDMaterial = FUSDExporter::ParsePrimMaterial(MeshPrim).Material;
                auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI::Apply(SubMesh.GetPrim());
                USDMaterialBinding.Bind(USDMaterial);
            }

            // Remove old mesh prim
            USDStage->RemovePrim(MeshPrim.GetPath());
        }
    }

    void CreateSimReadyVehicleAttr(const pxr::UsdPrim& Prim, const pxr::TfToken& Token)
    {
        if (!Prim.HasAttribute(SimReadyVehicleTokens::VehicleAttrName))
        {
            auto Attr = Prim.CreateAttribute(SimReadyVehicleTokens::VehicleAttrName, pxr::SdfValueTypeNames->Token, true);
            Attr.Set(Token);
        }
    }

    void CreateSimReadyVehicleProperties(const pxr::UsdPrim& Prim)
    {
        pxr::VtTokenArray LongitudinalAxisAllowedTokens = {
            SimReadyVehicleTokens::NegX,
            SimReadyVehicleTokens::NegY,
            SimReadyVehicleTokens::NegZ,
            SimReadyVehicleTokens::PosX,
            SimReadyVehicleTokens::PosY,
            SimReadyVehicleTokens::PosZ
        };

        pxr::VtTokenArray DrivetrainAllowedTokens = {
            SimReadyVehicleTokens::FourWD,
            SimReadyVehicleTokens::AWD,
            SimReadyVehicleTokens::FWD,
            SimReadyVehicleTokens::RWD
        };

        pxr::VtTokenArray SteeringAllowedTokens = {
            SimReadyVehicleTokens::Front,
            SimReadyVehicleTokens::Back,
            SimReadyVehicleTokens::All,
            SimReadyVehicleTokens::None,
            SimReadyVehicleTokens::Tank
        };

        auto LongitudinalAxisAttr = Prim.CreateAttribute(SimReadyVehicleTokens::LongitudinalAxisName, pxr::SdfValueTypeNames->Token, true);
        LongitudinalAxisAttr.Set(SimReadyVehicleTokens::PosX);
        for (auto PropertySpec : LongitudinalAxisAttr.GetPropertyStack())
        {
            if (auto AttributeSpecHandle = PropertySpec->GetLayer()->GetAttributeAtPath(PropertySpec->GetPath()))
            {
                AttributeSpecHandle->SetAllowedTokens(LongitudinalAxisAllowedTokens);
            }
        }
        auto DrivetrainAttr = Prim.CreateAttribute(SimReadyVehicleTokens::DrivetrainName, pxr::SdfValueTypeNames->Token, true);
        DrivetrainAttr.Set(SimReadyVehicleTokens::RWD);
        for (auto PropertySpec : DrivetrainAttr.GetPropertyStack())
        {
            if (auto AttributeSpecHandle = PropertySpec->GetLayer()->GetAttributeAtPath(PropertySpec->GetPath()))
            {
                AttributeSpecHandle->SetAllowedTokens(DrivetrainAllowedTokens);
            }
        }
        auto SteeringAttr = Prim.CreateAttribute(SimReadyVehicleTokens::SteeringName, pxr::SdfValueTypeNames->Token, true);
        SteeringAttr.Set(SimReadyVehicleTokens::Front);
        for (auto PropertySpec : SteeringAttr.GetPropertyStack())
        {
            if (auto AttributeSpecHandle = PropertySpec->GetLayer()->GetAttributeAtPath(PropertySpec->GetPath()))
            {
                AttributeSpecHandle->SetAllowedTokens(SteeringAllowedTokens);
            }
        }
    }

    void CreateSimReadyVehicle(const pxr::UsdPrim& Prim, const UPrimitiveComponent& Component)
    {
        ExportCarlaTag(Prim, nullptr, &Component);
        CreateSimReadyVehicleProperties(Prim);
    }

    void CreateSimReadyLight(const pxr::UsdPrim& Prim, const pxr::TfToken& Light)
    {
        auto LightAttr = Prim.CreateAttribute(SimReadyVehicleTokens::LightAttrName, pxr::SdfValueTypeNames->TokenArray);
        LightAttr.Set(pxr::VtTokenArray{ Light });
    }

    bool HasDoorSocket(const UStaticMeshComponent* StaticMeshComponent)
    {
        static const TArray<FName> DoorNames = TArrayBuilder<FName>()
            .Add(TEXT("Door_FL"))
            .Add(TEXT("Door_FR"))
            .Add(TEXT("Door_RL"))
            .Add(TEXT("Door_RR"))
            .Add(TEXT("Door_L"))
            .Add(TEXT("Door_R"));

        if (!StaticMeshComponent)
        {
            return false;
        }

        for (auto Name : DoorNames)
        {
            if (StaticMeshComponent->GetAttachSocketName().IsEqual(Name))
            {
                return true;
            }
        }

        return false;
    }

    bool ParentHasDoorSocket(const UStaticMeshComponent* StaticMeshComponent)
    {
        if (!StaticMeshComponent)
        {
            return false;
        }

        if (StaticMeshComponent->GetAttachParent() && StaticMeshComponent->GetAttachParent()->IsA<UStaticMeshComponent>())
        {
            if (HasDoorSocket(Cast<UStaticMeshComponent>(StaticMeshComponent->GetAttachParent())))
            {
                return true;
            }
        }

        return false;
    }
}


void FUSDCARLAVehicleTools::SplitVehicleSkelByJoints(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdSkelRoot& SkelRoot)
{
    TArray<USDCARLAVehicleToolsInternal::FJointInfo> JointInfos;
    USDCARLAVehicleToolsInternal::GetJointIndices(SkelRoot, JointInfos);

    TArray<pxr::UsdPrim> MeshPrims;
    GetMeshFromSkelRoot(SkelRoot, MeshPrims);

    for (int32 MeshIndex = 0; MeshIndex < MeshPrims.Num(); ++MeshIndex)
    {
        auto MeshPrim = MeshPrims[MeshIndex];

        TArray<USDCARLAVehicleToolsInternal::FIslandInfo> SubmeshIslands;
        for (auto& JointInfo : JointInfos)
        {
            if (JointInfo.JointName.IsNone())
            {
                continue;
            }

            TArray<int32> MeshIndices;
            USDCARLAVehicleToolsInternal::GetAffectedVertices(MeshPrim, JointInfo.Index, MeshIndices);
            if (MeshIndices.Num() > 0)
            {
                auto ParentPath = MeshPrim.GetParent().GetPath();
                auto SubMeshName = JointInfo.JointName.ToString();
                auto Path = ParentPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SubMeshName)));
                if (JointInfo.Index == 0)
                {
                    if (USDStage->GetPrimAtPath(Path))
                    {
                        FString UniqueName = FString::Printf(TEXT("%s_%d"), *SubMeshName, MeshIndex);
                        Path = ParentPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*UniqueName)));
                    }
                }
                SubmeshIslands.Add({ Path, MeshIndices, JointInfo.Offset, JointInfo.Index == 0 ? INDEX_NONE : MeshIndex });
            }
        }

        auto USDMaterial = FUSDExporter::ParsePrimMaterial(MeshPrim).Material;
        if (USDMaterial)
        {
            if (SubmeshIslands.Num() > 1)
            {
                ExportSimReadyVehicleNonVisualRubber(USDMaterial.GetPrim());
            }
            else
            {
                SetNonVisualTags(USDMaterial.GetPrim(), pxr::TfToken(), pxr::VtTokenArray{ SimReadyVehicleTokens::None }, SimReadyVehicleTokens::None);
            }
        }

        if (SubmeshIslands.Num() > 1)
        {
            USDCARLAVehicleToolsInternal::SplitMeshByIslandsWithAttributes(USDStage, MeshPrim, SubmeshIslands, pxr::UsdGeomTokens->vertex);
            for (auto Island : SubmeshIslands)
            {
                if (Island.SubMeshIndex != INDEX_NONE)
                {
                    auto Prim = USDStage->GetPrimAtPath(Island.Path);
                    if (Prim)
                    {
                        USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Wheel);
                    }
                }
            }
        }
    }
}

void FUSDCARLAVehicleTools::ConvertSkelToMesh(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdSkelRoot& SkelRoot)
{
    // Remove skeleton
    pxr::UsdSkelCache USDSkelCache;
    USDSkelCache.Populate(SkelRoot);

    TArray<pxr::UsdPrim> MeshPrims;
    GetMeshFromSkelRoot(SkelRoot, MeshPrims);

    std::vector< pxr::UsdSkelBinding > USDSkeletonBindings;
    USDSkelCache.ComputeSkelBindings(SkelRoot, &USDSkeletonBindings);

    for (auto SkeletonBinding : USDSkeletonBindings)
    {
        USDStage->RemovePrim(SkeletonBinding.GetSkeleton().GetPath());
    }

    for (auto MeshPrim : MeshPrims)
    {
        // Remove SkelBindingAPI from mesh, NOTE: No RemoveAPI at USD 0.20.5 
        pxr::SdfTokenListOp TokenListOp;
        MeshPrim.GetMetadata(pxr::UsdTokens->apiSchemas, &TokenListOp);

        pxr::TfTokenVector ExplicitItems = TokenListOp.GetExplicitItems();

        auto Iter = std::find(ExplicitItems.begin(), ExplicitItems.end(), pxr::TfToken("SkelBindingAPI"));
        if (Iter != ExplicitItems.end())
        {
            ExplicitItems.erase(Iter);
        }

        pxr::SdfTokenListOp ExplicitItemsOp;
        ExplicitItemsOp.SetExplicitItems(ExplicitItems);

        TokenListOp.ComposeOperations(ExplicitItemsOp, pxr::SdfListOpTypeExplicit);
        MeshPrim.SetMetadata(pxr::UsdTokens->apiSchemas, TokenListOp);

        if (MeshPrim)
        {
            MeshPrim.RemoveProperty(pxr::TfToken("primvars:skel:jointWeights"));
            MeshPrim.RemoveProperty(pxr::TfToken("primvars:skel:jointIndices"));
            MeshPrim.RemoveProperty(pxr::TfToken("skel:skeleton"));
        }
    }

    // Convert from SkelRoot to Xform
    auto RootName = SkelRoot.GetPrim().GetName();
    auto DestPath = SkelRoot.GetPrim().GetParent().GetPath().AppendElementString("Temp");
    auto DestRoot = pxr::UsdGeomXform::Define(USDStage, DestPath);
    USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(DestRoot.GetPrim(), SimReadyVehicleTokens::Body);
    for (auto Child : SkelRoot.GetPrim().GetChildren())
    {
        auto Path = DestPath.AppendElementString(Child.GetName());
        SimReadyUsdStageCtrl::CopyPrim(Child, Path);
    }
    SimReadyUsdStageCtrl::RemovePrim(SkelRoot.GetPrim());
    SimReadyUsdStageCtrl::RenamePrim(DestRoot.GetPrim(), RootName);
}

void FUSDCARLAVehicleTools::SplitVehicleLightByTexcoords(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& MeshPrim)
{
    static const TArray<FName> LightNames = TArrayBuilder<FName>()
        .Add(TEXT("Front_Position"))
        .Add(TEXT("Back_Position"))
        .Add(TEXT("Brake"))
        .Add(TEXT("Left_Blinker"))
        .Add(TEXT("Right_Blinker"))
        .Add(TEXT("Front_Fog"))
        .Add(TEXT("Back_Fog"))
        .Add(TEXT("Reverse"))
        .Add(TEXT("High_Beam"))
        .Add(TEXT("Low_Beam"));

    check(LightNames.Num() == ELightType::LightCount);

    TArray<pxr::UsdPrim> USDMeshPrims;
    auto USDMesh = pxr::UsdGeomMesh(MeshPrim);
    if (!USDMesh)
    {
        //Try sub-mesh for the MeshPrim	
        FindAllPrims<pxr::UsdGeomMesh>(MeshPrim, USDMeshPrims);
        if (USDMeshPrims.Num() > 0)
        {
            USDMesh = pxr::UsdGeomMesh(USDMeshPrims[0]);
        }
    }

    if (USDMesh)
    {
        auto FaceVertexIndices = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexIndicesAttr());

        pxr::VtArray<pxr::GfVec2f> UVArray;
        pxr::TfToken UVInterpType;

        auto Primvars = USDMesh.GetPrimvars();
        for (auto Iter = Primvars.begin(); Iter != Primvars.end(); ++Iter)
        {
            auto PrimVar = *Iter;

            // Texture UV
            if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Float2Array.GetCPPTypeName()) == 0)
            {
                ComputeFlattened<pxr::GfVec2f>(PrimVar, &UVArray);
                UVInterpType = PrimVar.GetInterpolation();
                break;
            }
        }

        if (UVInterpType == pxr::UsdGeomTokens->faceVarying)
        {
            TArray<TArray<int32>> SubmeshIndices;

            SubmeshIndices.AddZeroed(LightCount);
            check(UVArray.size() == FaceVertexIndices.size());

            for (int32 Index = 0; Index < FaceVertexIndices.size(); ++Index)
            {
                auto UV = UVArray[Index];
                auto X = UV[0];
                auto Y = 1.0f - UV[1];
                
                ELightType LightType = ELightType::LightCount;
                if (X < 0.5f && Y < 0.5f)
                {
                    // Brake and Position
                    if (Y < 0.5f - X)
                    {
                        //Brake
                        LightType = ELightType::Brake;
                    }
                    else
                    {
                        // Position: Front or Back
                        if (Y > X)
                        {
                            LightType = ELightType::FrontPosition;
                        }
                        else
                        {
                            LightType = ELightType::BackPosition;
                        }
                    }
                }
                else if (X < 0.5f && Y >= 0.5f)
                {
                    // Fog and Reserve
                    if (Y > 0.5f + X)
                    {
                        LightType = ELightType::Reverse;
                    }
                    else
                    {
                        // Fog: Front or Back
                        if (Y < 1 - X)
                        {
                            LightType = ELightType::FrontFog;
                        }
                        else
                        {
                            LightType = ELightType::BackFog;
                        }
                    }

                }
                else if (X >= 0.5f && Y < 0.5f)
                {
                    // Blinker
                    if (Y < X - 0.5f)
                    {
                        LightType = ELightType::LeftBlinker;
                    }
                    else
                    {
                        LightType = ELightType::RightBlinker;
                    }
                }
                else // X >= 0.5f && UV[1] >= 0.5f
                {
                    // Beam
                    if (Y > 1.5f - X)
                    {
                        LightType = ELightType::LowBeam;
                    }
                    else
                    {
                        LightType = ELightType::HighBeam;
                    }
                }
                SubmeshIndices[LightType].Add(Index);
            }

            TArray<USDCARLAVehicleToolsInternal::FIslandInfo> Islands;
            for (int32 LightIndex = 0; LightIndex < (int32)ELightType::LightCount; ++LightIndex)
            {
                auto ParentPath = USDMesh.GetPrim().GetParent().GetPath().AppendElementString("Lights");
                pxr::UsdGeomXform::Define(USDStage, ParentPath);			
                auto Path = ParentPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*LightNames[LightIndex].ToString())));
                Islands.Add({ Path, SubmeshIndices[LightIndex], FVector::ZeroVector, INDEX_NONE });
            }

            USDCARLAVehicleToolsInternal::SplitMeshByIslandsWithAttributes(USDStage, USDMesh.GetPrim(), Islands, pxr::UsdGeomTokens->faceVarying);
        }
    }
}

bool FUSDCARLAVehicleTools::BelongToCarlaVehicle(const USceneComponent* SceneComponent)
{
    if (SceneComponent && SceneComponent->GetOwner() && SceneComponent->GetOwner()->GetClass())
    {
        auto SuperClass = SceneComponent->GetOwner()->GetClass()->GetSuperClass();
        if (SuperClass && (SuperClass->GetName() == CARLA_VEHICLE_BP || SuperClass->GetName() == CARLA_VEHICLE_BP_NW))
        {
            return true;
        }
    }

    return false;
}

bool FUSDCARLAVehicleTools::IsCarlaVehicleSirenLightMaterial(const class UMaterialInterface* MaterialInterface)
{
    if (MaterialInterface)
    {
        auto Material = MaterialInterface->GetMaterial();
        if (Material->GetName() == CARLA_VEHICLELIGHTSIRENS_MAT)
        {
            return true;
        }
    }

    return false;
}

bool FUSDCARLAVehicleTools::IsCarlaVehicleLightMaterial(const class UMaterialInterface* MaterialInterface)
{
    if (MaterialInterface)
    {
        auto Material = MaterialInterface->GetMaterial();
        if (Material->GetName() == CARLA_VEHICLELIGHT_MAT || Material->GetName() == CARLA_VEHICLELIGHT_MAT2)
        {
            return true;
        }
    }

    return false;
}

bool FUSDCARLAVehicleTools::IsCarlaVehiclePlateMaterial(const class UMaterialInterface* MaterialInterface)
{
    if (MaterialInterface)
    {
        auto Material = MaterialInterface->GetMaterial();
        if (Material->GetName() == CARLA_VEHICLEPLATE_MAT || Material->GetName() == CARLA_VEHICLEPLATE_MAT2 || Material->GetName() == CARLA_VEHICLEPLATE_MAT3)
        {
            return true;
        }
    }

    return false;
}

bool FUSDCARLAVehicleTools::IsCarlaVehicleLight(const UStaticMeshComponent* StaticMeshComponent)
{
    if (BelongToCarlaVehicle(StaticMeshComponent))
    {
        return IsCarlaVehicleLightMaterial(StaticMeshComponent->GetMaterial(0));
    }

    return false;
}

bool FUSDCARLAVehicleTools::IsCarlaVehicleSubMesh(const FString& Name, int SectionIndex)
{
    for (int32 Index = 0; Index < VehicleJointNames.Num(); ++Index)
    {
        FString VehicleSubmeshName = VehicleJointNames[Index].ToString();
        if (Name == VehicleSubmeshName || Name == FString::Printf(TEXT("%s_%d"), *VehicleSubmeshName, SectionIndex))
        {
            return true;
        }
    }

    return false;
}

void FUSDCARLAVehicleTools::BindMaterialsToSplitVehicleLight(const pxr::UsdStageRefPtr& USDStage, const TArray<pxr::UsdPrim>& MeshPrims, const UMaterialInterface* MaterialInterface)
{
    static const TArray<FName> LightNames = TArrayBuilder<FName>()
        .Add(TEXT("Front_Position"))
        .Add(TEXT("Back_Position"))
        .Add(TEXT("Brake"))
        .Add(TEXT("Left_Blinker"))
        .Add(TEXT("Right_Blinker"))
        .Add(TEXT("Front_Fog"))
        .Add(TEXT("Back_Fog"))
        .Add(TEXT("Reverse"))
        .Add(TEXT("High_Beam"))
        .Add(TEXT("Low_Beam"));

    static const TArray<FName> EmissiveColorNames = TArrayBuilder<FName>()
        .Add(TEXT("Front Position Color"))
        .Add(TEXT("Back Position Color"))
        .Add(TEXT("Brake Color"))
        .Add(TEXT("Blinker Color")) //Left
        .Add(TEXT("Blinker Color")) //Right
        .Add(TEXT("Front Fog Color"))
        .Add(TEXT("Back Fog Color"))
        .Add(TEXT("Reverse Color"))
        .Add(TEXT("High Beam Color"))
        .Add(TEXT("Low Beam Color"));

    static const TArray<pxr::TfToken> SimReadyLights = TArrayBuilder<pxr::TfToken>()
        .Add(SimReadyVehicleTokens::RunningLight)
        .Add(SimReadyVehicleTokens::TailLight)
        .Add(SimReadyVehicleTokens::BrakeLight)
        .Add(SimReadyVehicleTokens::SignalLLight) //Left
        .Add(SimReadyVehicleTokens::SignalRLight) //Right
        .Add(SimReadyVehicleTokens::FogLight)
        .Add(SimReadyVehicleTokens::FogLight)
        .Add(SimReadyVehicleTokens::ReverseLight)
        .Add(SimReadyVehicleTokens::HighbeamLight)
        .Add(SimReadyVehicleTokens::HeadLight);

    float EmissiveIntensity[ELightType::LightCount] =
    {
        200,
        200,
        700,
        300,
        300,
        600,
        600,
        200,
        600,
        500,
    };

    check(MeshPrims.Num() <= LightNames.Num());
    check(MeshPrims.Num() <= SimReadyLights.Num());
    check(MeshPrims.Num() <= EmissiveColorNames.Num());
    check(MeshPrims.Num() <= ELightType::LightCount);

    pxr::SdfPath ScopePrimPath = pxr::SdfPath::AbsoluteRootPath().AppendElementString(TCHAR_TO_UTF8(*SimReadyRootPrim));
    ScopePrimPath = ScopePrimPath.AppendElementString(pxr::UsdUtilsGetMaterialsScopeName().GetString());

    pxr::SdfPath RemovePrimPath;
    // Duplicate material and setup emissive for different lights
    for (int32 MeshIndex = 0; MeshIndex < MeshPrims.Num(); ++MeshIndex)
    {
        auto MeshPrim = MeshPrims[MeshIndex];
        int32 LightIndex = INDEX_NONE;
        if (!LightNames.Find(FName(MeshPrim.GetName().GetText()), LightIndex))
        {
            continue;
        }
        USDCARLAVehicleToolsInternal::CreateSimReadyLight(MeshPrim, SimReadyLights[LightIndex]);
        pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(MeshPrim);
        auto Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());
        RemovePrimPath = Material.GetPath();
        auto PrimPath = ScopePrimPath.AppendElementToken(MeshPrim.GetName());
        SimReadyUsdStageCtrl::CopyPrim(Material.GetPrim(), PrimPath);

        auto NewMaterial = pxr::UsdShadeMaterial(USDStage->GetPrimAtPath(PrimPath));

        auto MdlSurfaceOutput = NewMaterial.GetSurfaceOutput(USDTokens.mdl);
        if (MdlSurfaceOutput)
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            if (MdlSurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
            {
                auto ShadeShader = pxr::UsdShadeShader(Source);
                // Setup emissive color
                FLinearColor OutValue(EForceInit::ForceInitToZero);
                if (LightIndex < EmissiveColorNames.Num() && MaterialInterface->GetVectorParameterValue(EmissiveColorNames[LightIndex], OutValue))
                {
                    FUSDExporter::ExportParameter<pxr::GfVec4f>(pxr::SdfValueTypeNames->Color3f, TEXT("emissive_color"), ShadeShader, LinearColorToVec(OutValue));
                }

                // Setup emissive intensity
                if (LightIndex < ELightType::LightCount)
                {
                    FUSDExporter::ExportParameter<float>(pxr::SdfValueTypeNames->Float, TEXT("emissive_intensity"), ShadeShader, EmissiveIntensity[LightIndex] * 2560.0f);
                }
            }
        }

        FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualGlass(NewMaterial.GetPrim());
        USDMaterialBinding.Bind(NewMaterial);
    }

    USDStage->RemovePrim(RemovePrimPath);
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleLight(const pxr::UsdPrim& Prim, const ULightComponentBase& LightComponent)
{
    static const TMap<FString, pxr::TfToken> LightNameToToken = {
        {TEXT("front-blinker-r"), SimReadyVehicleTokens::SignalRLight},
        {TEXT("front-blinker-l"), SimReadyVehicleTokens::SignalLLight},
        {TEXT("back-position"), SimReadyVehicleTokens::TailLight},
        {TEXT("back-position-r"), SimReadyVehicleTokens::TailLight},
        {TEXT("back-position-l"), SimReadyVehicleTokens::TailLight},
        {TEXT("back-brake"), SimReadyVehicleTokens::BrakeLight},
        {TEXT("back-brake-r"), SimReadyVehicleTokens::BrakeLight},
        {TEXT("back-brake-l"), SimReadyVehicleTokens::BrakeLight},
        {TEXT("back-reverse"), SimReadyVehicleTokens::ReverseLight},
        {TEXT("back-reverse-r"), SimReadyVehicleTokens::ReverseLight},
        {TEXT("back-reverse-l"), SimReadyVehicleTokens::ReverseLight},
        {TEXT("back-blinker-r"), SimReadyVehicleTokens::SignalRLight},
        {TEXT("back-blinker-l"), SimReadyVehicleTokens::SignalLLight},
        {TEXT("front-position-r"), SimReadyVehicleTokens::RunningLight},
        {TEXT("front-position-l"), SimReadyVehicleTokens::RunningLight},
        {TEXT("front-low_beam-r"), SimReadyVehicleTokens::HeadLight},
        {TEXT("front-low_beam-l"), SimReadyVehicleTokens::HeadLight},
        {TEXT("front-high_beam-r"), SimReadyVehicleTokens::HighbeamLight},
        {TEXT("front-high_beam-l"), SimReadyVehicleTokens::HighbeamLight},
        {TEXT("back-fog-l"), SimReadyVehicleTokens::FogLight},
        {TEXT("back-fog-r"), SimReadyVehicleTokens::FogLight},
        {TEXT("front-fog-l"), SimReadyVehicleTokens::FogLight},
        {TEXT("front-fog-r"), SimReadyVehicleTokens::FogLight},
        {TEXT("plate-position"), SimReadyVehicleTokens::PlateLight},
        {TEXT("special1"), SimReadyVehicleTokens::EmergencyLight},
        {TEXT("interior"), SimReadyVehicleTokens::InteriorLight},
    };
    FString Name = LightComponent.GetName();
    int Index = 0;
    if (Name.FindLastChar('-', Index))
    {
        Name = Name.Left(Index);
        auto Token = LightNameToToken.Find(Name);
        if (Token)
        {
#if PXR_VERSION >= 2111
            pxr::UsdLuxLightAPI LightAPI(Prim);
#else
            pxr::UsdLuxLight LightAPI(Prim);
#endif //PXR_VERSION >= 2111

            USDCARLAVehicleToolsInternal::CreateSimReadyLight(Prim, *Token);
        }
    }
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleComponentType(const pxr::UsdPrim& Prim, const UStaticMeshComponent* StaticMeshComponent)
{
    if (!StaticMeshComponent)
    {
        return;
    }

    if (USDCARLAVehicleToolsInternal::HasDoorSocket(StaticMeshComponent))
    {
        USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Door);
    }
    else if (USDCARLAVehicleToolsInternal::ParentHasDoorSocket(StaticMeshComponent))
    {
        auto Material = StaticMeshComponent->GetMaterial(0);
        if (Material && IsTranslucentBlendMode(Material->GetBlendMode()))
        {
            USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Window);
        }
    }
    else
    {
        USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Body);
    }
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleComponentType(const pxr::UsdPrim& Prim, const UMaterialInterface* MaterialInterface)
{
    if (FUSDCARLAVehicleTools::IsCarlaVehiclePlateMaterial(MaterialInterface))
    {
        USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Plate);
    }
    else if (IsTranslucentBlendMode(MaterialInterface->GetBlendMode()))
    {
        auto Material = MaterialInterface->GetMaterial();
        if (Material->GetName() == CARLA_VEHICLELIGHTGLASS_MAT)
        {
            USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Body);
        }
        else
        {
            USDCARLAVehicleToolsInternal::CreateSimReadyVehicleAttr(Prim, SimReadyVehicleTokens::Window);
        }
    }
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicle(const pxr::UsdPrim& Prim, const UPrimitiveComponent& Component)
{
    USDCARLAVehicleToolsInternal::CreateSimReadyVehicle(Prim, Component);
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualGlass(const pxr::UsdPrim& Prim)
{
    SetNonVisualTags(Prim, SimReadyVehicleTokens::ClearGlass);
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualMaterial(const pxr::UsdPrim& Prim, const UMaterialInterface& MaterialInterface)
{
    static const TMap<FString, pxr::TfToken> TokenMap = {
        {TEXT("plastic"), pxr::TfToken("plastic")},
        {TEXT("rubber"), pxr::TfToken("rubber")},
        {TEXT("fabric"), pxr::TfToken("fabric")},
        {TEXT("aluminium"), pxr::TfToken("aluminum")},
        {TEXT("metal"), pxr::TfToken("steel")},	
    };

    auto Material = MaterialInterface.GetMaterial();

    for (auto Iter : TokenMap)
    {
        if (MaterialInterface.GetName().Contains(Iter.Key) || (Material && Material->GetName().Contains(Iter.Key)))
        {
            SetNonVisualTags(Prim, Iter.Value, pxr::VtTokenArray{ SimReadyVehicleTokens::None }, SimReadyVehicleTokens::None);
            return;
        }
    }
    
    // No matching, leave blank to base materials
    SetNonVisualTags(Prim, pxr::TfToken(), pxr::VtTokenArray{ SimReadyVehicleTokens::None }, SimReadyVehicleTokens::None);
}

void FUSDCARLAVehicleTools::ExportSimReadyVehicleNonVisualRubber(const pxr::UsdPrim& Prim)
{
    SetNonVisualTags(Prim, SimReadyVehicleTokens::Rubber);
}

static TArray<FVector> VehiclePartOffsets;

enum EVehiclePart
{
    FrontLeftDoor,
    FrontRightDoor,
    RearLeftDoor,
    RearRightDoor,
    Trunk,
    Hood,
    FrontLeftWheel,
    FrontRightWheel,
    RearLeftWheel,
    RearRightWheel,
    Body,
    NumParts,
};

void FUSDCARLAVehicleTools::InitPartOffsets(const pxr::UsdStageRefPtr& USDStage)
{
    VehiclePartOffsets.Reset();

    TArray<FString> PartNames =
    {
        "door_0",
        "door_1",
        "door_2",
        "door_3",
        "trunk",
        "hood",
        "wheel_0",
        "wheel_1",
        "wheel_2",
        "wheel_3",
        "body"
    };

    VehiclePartOffsets.AddZeroed(EVehiclePart::NumParts);
    const auto& PrimRange = USDStage->Traverse();
    for (const auto& Prim : PrimRange)
    {
        FString Name = Prim.GetName().GetText();

        for (int Part = 0; Part < EVehiclePart::NumParts; ++Part)
        {
            if (Name == PartNames[Part])
            {
                pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache();
                bool resetsXformStack;
                pxr::GfMatrix4d LocalMatrix = Cache.GetLocalTransformation(Prim, &resetsXformStack);
                VehiclePartOffsets[Part] = USDConvertPosition(USDStage, USDConvert(LocalMatrix.ExtractTranslation()), true);
                break;
            }
        }
    }

    VehiclePartOffsets[EVehiclePart::FrontLeftDoor] += VehiclePartOffsets[EVehiclePart::Body];
    VehiclePartOffsets[EVehiclePart::FrontRightDoor] += VehiclePartOffsets[EVehiclePart::Body];
    VehiclePartOffsets[EVehiclePart::RearLeftDoor] += VehiclePartOffsets[EVehiclePart::Body];
    VehiclePartOffsets[EVehiclePart::RearRightDoor] += VehiclePartOffsets[EVehiclePart::Body];
    VehiclePartOffsets[EVehiclePart::Trunk] += VehiclePartOffsets[EVehiclePart::Body];
    VehiclePartOffsets[EVehiclePart::Hood] += VehiclePartOffsets[EVehiclePart::Body];
}

FVector FUSDCARLAVehicleTools::GetPartOffset(const pxr::UsdGeomMesh& USDMesh)
{
    if (VehiclePartOffsets.Num() != EVehiclePart::NumParts)
    {
        return FVector::ZeroVector;
    }

    FString Name = USDMesh.GetPrim().GetName().GetText();

    pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache();
    bool resetsXformStack;
    pxr::GfMatrix4d LocalMatrix = Cache.GetLocalTransformation(USDMesh.GetPrim(), &resetsXformStack);
    auto ComponentOffset = USDConvertPosition(USDMesh.GetPrim().GetStage(), USDConvert(LocalMatrix.ExtractTranslation()), true);

    FVector Offset;
    if (Name.Contains("_door_0_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::FrontLeftDoor];
    }
    else if (Name.Contains("_door_1_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::FrontRightDoor];
    }
    else if (Name.Contains("_door_2_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::RearLeftDoor];
    }
    else if (Name.Contains("_door_3_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::RearRightDoor];
    }
    else if (Name.Contains("_trunk_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::Trunk];
    }
    else if (Name.Contains("_hood_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::Hood];
    }
    else if (Name.Contains("_suspension_0_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::FrontLeftWheel];
    }
    else if (Name.Contains("_suspension_1_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::FrontRightWheel];
    }
    else if (Name.Contains("_suspension_2"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::RearLeftWheel];
    }
    else if (Name.Contains("_suspension_3_"))
    {
        Offset = VehiclePartOffsets[EVehiclePart::RearRightWheel];
    }
    else
    {
        Offset = VehiclePartOffsets[EVehiclePart::Body];
    }

    return Offset - ComponentOffset;
}