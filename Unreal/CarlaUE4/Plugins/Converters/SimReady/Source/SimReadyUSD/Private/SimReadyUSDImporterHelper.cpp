// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDImporterHelper.h"
#include "USDConverter.h"
#include "UnrealEd/Private/GeomFitUtils.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshLODImporterData.h"

#include "USDHashGenerator.h"
#include "USDDerivedDataCache.h"
#include "USDGeometryCache.h"
#include "Extractors/TimeSamplesData.h"
#include "SimReadyTexture.h"
#include "SimReadyPathHelper.h"
#include "SimReadyUSDLog.h"
#include "SimReadyMDL.h"
#include "SimReadyUSDTokens.h"
#include "SimReadyMaterialReparentUtility.h"
#include "mikktspace.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Logging/LogMacros.h"
#include "MaterialEditorInstanceNotifier.h"
#include "IMaterialEditor.h"

#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "PhysicsEngine/BodySetup.h"

pxr::TfToken GetGeometryOrientation(const pxr::UsdGeomGprim& USDMesh)
{
    static pxr::TfToken RightHanded("rightHanded");

    if (USDMesh)
    {
        pxr::UsdAttribute Orientation = USDMesh.GetOrientationAttr();
        if (Orientation)
        {
            pxr::TfToken OrientationValue;
            Orientation.Get(&OrientationValue);

            return OrientationValue;
        }
    }

    return RightHanded;
}

bool FSimReadyUSDImporterHelper::USDImportMesh(const pxr::UsdGeomMesh& USDMesh, UStaticMesh& Mesh, const FVector& Offset)
{
    // Validate
    if(!USDMesh)
    {
        return false;
    }

    // Create render data
    if(!Mesh.RenderData)
    {
        Mesh.RenderData.Reset(new(FMemory::Malloc(sizeof(FStaticMeshRenderData)))FStaticMeshRenderData());
    }

    // Release resources
    Mesh.ReleaseResources();

    // Create necessary LOD and vertex factory
    if(Mesh.RenderData->LODResources.Num() <= 0)
    {
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
        FStaticMeshLODResources* NewLODResources = new(FMemory::Malloc(sizeof(FStaticMeshLODResources))) FStaticMeshLODResources();
        Mesh.RenderData->LODResources.Add(NewLODResources);
        FStaticMeshVertexFactories* NewVertexFactories = new(FMemory::Malloc(sizeof(FStaticMeshVertexFactories))) FStaticMeshVertexFactories(ERHIFeatureLevel::SM5);
        Mesh.RenderData->LODVertexFactories.Add(NewVertexFactories);
#else
        Mesh.RenderData->AllocateLODResources(1);
#endif
    }

    FString HashKey = FUSDHashGenerator::ComputeSHAHash(USDMesh).ToString();
    if(!FUSDDerivedDataCache::Load(HashKey, *Mesh.RenderData))
    {
        if(!USDImportMesh(USDMesh, Mesh.RenderData->LODResources[0], Offset))
        {
            return false;
        }

        // Bounding box
        auto& MeshRenderData = *Mesh.RenderData;

        // Calculate the bounding box.
        FBox BoundingBox(ForceInit);
        FPositionVertexBuffer& PositionVertexBuffer = MeshRenderData.LODResources[0].VertexBuffers.PositionVertexBuffer;
        for(uint32 VertexIndex = 0; VertexIndex < PositionVertexBuffer.GetNumVertices(); VertexIndex++)
        {
            BoundingBox += PositionVertexBuffer.VertexPosition(VertexIndex);
        }
        BoundingBox.GetCenterAndExtents(MeshRenderData.Bounds.Origin, MeshRenderData.Bounds.BoxExtent);

        // Calculate the bounding sphere using the center of the bounding box as the origin
        float SphereRadiusSquared = 0.0f;
        for(uint32 VertexIndex = 0; VertexIndex < PositionVertexBuffer.GetNumVertices(); VertexIndex++)
        {
            SphereRadiusSquared = FMath::Max(
                (PositionVertexBuffer.VertexPosition(VertexIndex) - MeshRenderData.Bounds.Origin).SizeSquared(), 
                SphereRadiusSquared
            );
        }
        // Determine the actual radius from the squared radius
        MeshRenderData.Bounds.SphereRadius = FMath::Sqrt(SphereRadiusSquared);

        // Save to cache
        if(!HashKey.IsEmpty())
        {
            FUSDDerivedDataCache::Save(HashKey, *Mesh.RenderData);
        }
    }

    // Bounds
    Mesh.CalculateExtendedBounds();

    // 4.26 requires this to avoid checking when we don't have LODResources
    Mesh.NeverStream = true;

    // Recreate resources
    Mesh.InitResources();

    // Other setup
    Mesh.CreateBodySetup();

    return true;
}

bool FSimReadyUSDImporterHelper::USDImportMesh(const pxr::UsdGeomMesh& USDMesh, const FTransform& GeomTransform, TArray<FStaticMeshBuildVertex>& Vertices, TArray<FVector>& UE4Points, TArray<uint32>& Indices, TArray<uint32>& FacePosInIndices, TArray<uint32>& FacePosInVertices, int32& TriangleCount, int32& UVCount, const FVector& Offset)
{
    // Add triangles
    auto FaceVertexCounts = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexCountsAttr());
    auto Points = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDMesh.GetPointsAttr());
    auto FaceVertexIndices = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexIndicesAttr());
    bool bLeftHanded = GetGeometryOrientation(USDMesh) == USDTokens.leftHanded;

    TriangleCount = 0;
    for (auto FaceVertexCount : FaceVertexCounts)
    {
        TriangleCount += FaceVertexCount - 2;
    }

    Indices.Reserve(TriangleCount * 3);
    UE4Points.AddZeroed(Points.size());
    Vertices.AddZeroed(FaceVertexIndices.size());	// One vertex for one wedge
    FacePosInIndices.AddZeroed(FaceVertexCounts.size());
    FacePosInVertices.AddZeroed(FaceVertexCounts.size());
    auto Stage = USDMesh.GetPrim().GetStage();

    for (int32 PointIndex = 0; PointIndex < Points.size(); ++PointIndex)
    {
        UE4Points[PointIndex] = GeomTransform.TransformPosition(USDConvertPosition(Stage, USDConvert(Points[PointIndex]), true)) - Offset;
    }

    for (int FaceIdx = 0, WedgeIdx = 0;
        FaceIdx < FaceVertexCounts.size();
        WedgeIdx += FaceVertexCounts[FaceIdx], ++FaceIdx
        )
    {
        if (FaceVertexCounts[FaceIdx] < 3)
        {
            return false;
        }

        for (auto Idx = 0; Idx < FaceVertexCounts[FaceIdx]; ++Idx)
        {
            int VertIdx = WedgeIdx + Idx;
            if (VertIdx >= FaceVertexIndices.size())
            {
                return false;
            }

            if (FaceVertexIndices[VertIdx] >= Points.size())
            {
                return false;
            }

            FVector& VertexPos = Vertices[VertIdx].Position;
            VertexPos = USDConvert(Points[FaceVertexIndices[VertIdx]]);
            if (VertexPos.ContainsNaN())
            {
                VertexPos = FVector(0, 0, 0);

                const FString MeshPath = USDMesh.GetPath().GetText();
                UE_LOG(LogTemp, Warning, TEXT("Mesh at %s contains NaN vertex at index %d"), *MeshPath, VertIdx);
            }
        }

        FacePosInIndices[FaceIdx] = Indices.Num();
        FacePosInVertices[FaceIdx] = WedgeIdx;

        for (auto Idx = 0; Idx < FaceVertexCounts[FaceIdx] - 2; ++Idx)
        {
            Indices.Add(WedgeIdx + (bLeftHanded ? (Idx + 2) : 0));
            Indices.Add(WedgeIdx + Idx + 1);
            Indices.Add(WedgeIdx + (bLeftHanded ? 0 : (Idx + 2)));
        }
    }

    for (auto& Vertex : Vertices)
    {
        Vertex.Position = USDConvertPosition(Stage, Vertex.Position, true) - Offset;
    }

    UVCount = 0;
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

            const pxr::UsdAttribute& Attr = PrimVar.GetAttr();
            pxr::VtValue CustomData = Attr.GetCustomDataByKey(pxr::TfToken("Maya"));

            int32 UVIndex = -1;
            if (!CustomData.IsEmpty() && CustomData.IsHolding<pxr::VtDictionary>()) {
                pxr::VtDictionary NewDict = CustomData.UncheckedGet<pxr::VtDictionary>();
                pxr::VtDictionary::iterator Itor = NewDict.find("UVSetIndex");
                if (Itor != NewDict.end())
                {
                    UVIndex = Itor->second.Get<int32>();
                }
            }

            // If can't find custom index, still use the output index.
            if (UVIndex < 0)
            {
                UVIndex = UVCount;
            }

            // Get interpolation type
            auto USDInterpType = PrimVar.GetInterpolation();

            // Copy data
            pxr::VtArray<pxr::GfVec2f> UVArray;
            ComputeFlattened<pxr::GfVec2f>(PrimVar, &UVArray);

            if (USDInterpType == pxr::UsdGeomTokens->faceVarying && UVArray.size() >= Vertices.Num())
            {
                for (int WedgeIdx = 0; WedgeIdx < Vertices.Num(); ++WedgeIdx)
                {
                    Vertices[WedgeIdx].UVs[UVIndex] = USDConvert(UVArray[WedgeIdx]);
                }
            }
            else if (USDInterpType == pxr::UsdGeomTokens->vertex && UVArray.size() >= Points.size())
            {
                for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
                {
                    Vertices[VertexIdx].UVs[UVIndex] = USDConvert(UVArray[FaceVertexIndices[VertexIdx]]);
                }
            }
            else if (USDInterpType == pxr::UsdGeomTokens->uniform)
            {
                if (FaceVertexCounts.size() >= UVArray.size())
                {
                    for (int Index = 0; Index < UVArray.size(); ++Index)
                    {
                        int32 VertBegin = FacePosInVertices[Index];
                        int32 VertEnd = (Index == (UVArray.size() - 1)) ? Vertices.Num() : FacePosInVertices[Index + 1];

                        for (; VertBegin < VertEnd; ++VertBegin)
                        {
                            Vertices[VertBegin].UVs[UVIndex] = USDConvert(UVArray[Index]);
                        }
                    }
                }
            }

            for (auto& Vertex : Vertices)
            {
                auto& UV = Vertex.UVs[UVIndex];
                UV.Y = 1 - UV.Y;
            }

            if (++UVCount >= MAX_STATIC_TEXCOORDS)
            {
                break;
            }
        }
        // Vertex Color
        else if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Color4fArray.GetCPPTypeName()) == 0)
        {
            // Get interpolation type
            auto USDInterpType = PrimVar.GetInterpolation();

            // Copy data
            pxr::VtArray<pxr::GfVec4f> ColorArray;
            ComputeFlattened<pxr::GfVec4f>(PrimVar, &ColorArray);

            if (USDInterpType == pxr::UsdGeomTokens->faceVarying && ColorArray.size() >= Vertices.Num())
            {
                for (int WedgeIdx = 0; WedgeIdx < Vertices.Num(); ++WedgeIdx)
                {
                    Vertices[WedgeIdx].Color = USDConvertToLinearColor(ColorArray[WedgeIdx]).ToFColor(false);
                }
            }
            else if (USDInterpType == pxr::UsdGeomTokens->vertex && ColorArray.size() >= Points.size())
            {
                for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
                {
                    Vertices[VertexIdx].Color = USDConvertToLinearColor(ColorArray[FaceVertexIndices[VertexIdx]]).ToFColor(false);
                }
            }
            else if (USDInterpType == pxr::UsdGeomTokens->uniform)
            {
                if (FaceVertexCounts.size() >= ColorArray.size())
                {
                    for (int Index = 0; Index < ColorArray.size(); ++Index)
                    {
                        int32 VertBegin = FacePosInVertices[Index];
                        int32 VertEnd = (Index == (ColorArray.size() - 1)) ? Vertices.Num() : FacePosInVertices[Index + 1];

                        for (; VertBegin < VertEnd; ++VertBegin)
                        {
                            Vertices[VertBegin].Color = USDConvertToLinearColor(ColorArray[Index]).ToFColor(false);
                        }
                    }
                }
            }
            else if (USDInterpType == pxr::UsdGeomTokens->constant)
            {
                for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
                {
                    Vertices[VertexIdx].Color = ColorArray.size() > 0 ? USDConvertToLinearColor(ColorArray.front()).ToFColor(false) : FColor::White;
                }
            }
        }
    }

    // Add normal
    bool bAddedNormal = false;

    if (auto NormalAttri = USDMesh.GetNormalsAttr())
    {
        // Get interpolation type
        auto USDInterpType = USDMesh.GetNormalsInterpolation();

        // Copy data
        auto NormalArray = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(NormalAttri);

        if (NormalArray.size() > 0)
        {
            if (USDInterpType == pxr::UsdGeomTokens->faceVarying && NormalArray.size() >= Vertices.Num())
            {
                for (int WedgeIdx = 0; WedgeIdx < Vertices.Num(); ++WedgeIdx)
                {
                    Vertices[WedgeIdx].TangentZ = USDConvert(NormalArray[WedgeIdx]).GetSafeNormal();
                }

                bAddedNormal = true;
            }
            else if (USDInterpType == pxr::UsdGeomTokens->vertex && NormalArray.size() >= Points.size())
            {
                for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
                {
                    Vertices[VertexIdx].TangentZ = USDConvert(NormalArray[FaceVertexIndices[VertexIdx]]).GetSafeNormal();
                }

                bAddedNormal = true;
            }
            else if (USDInterpType == pxr::UsdGeomTokens->uniform)
            {
                if (FaceVertexCounts.size() >= NormalArray.size())
                {
                    for (int Index = 0; Index < NormalArray.size(); ++Index)
                    {
                        int32 VertBegin = FacePosInVertices[Index];
                        int32 VertEnd = (Index == (NormalArray.size() - 1)) ? Vertices.Num() : FacePosInVertices[Index + 1];

                        for (; VertBegin < VertEnd; ++VertBegin)
                        {
                            Vertices[VertBegin].TangentZ = USDConvert(NormalArray[Index]).GetSafeNormal();
                        }
                    }
                }

                bAddedNormal = true;
            }
        }

        // Conversion
        if (bAddedNormal)
        {
            for (auto& Vertex : Vertices)
            {
                Vertex.TangentZ = GeomTransform.TransformVector(USDConvertVector(Stage, Vertex.TangentZ));
            }
        }
    }

    // Compute normal if necessary
    if (!bAddedNormal)
    {
        // compute polygon normal at first
        for (int TriangleIdx = 0; TriangleIdx < TriangleCount; ++TriangleIdx)
        {
            FVector P[3];
            P[0] = Vertices[Indices[TriangleIdx * 3]].Position;
            P[1] = Vertices[Indices[TriangleIdx * 3 + 1]].Position;
            P[2] = Vertices[Indices[TriangleIdx * 3 + 2]].Position;

            FVector Edge1 = P[1] - P[0];
            FVector Edge2 = P[2] - P[0];
            FVector Normal = FVector::CrossProduct(Edge2, Edge1).GetSafeNormal();

            Vertices[Indices[TriangleIdx * 3]].TangentZ = Normal;
            Vertices[Indices[TriangleIdx * 3 + 1]].TangentZ = Normal;
            Vertices[Indices[TriangleIdx * 3 + 2]].TangentZ = Normal;
        }

        // compute each vertex normal
        TArray<FVector> GroupNormals;
        TArray<bool> bNormalizedNormals;
        GroupNormals.AddZeroed(Points.size());
        bNormalizedNormals.AddZeroed(Points.size());

        for (int32 Index = 0; Index < Vertices.Num(); ++Index)
        {
            GroupNormals[FaceVertexIndices[Index]] += Vertices[Index].TangentZ;
        }
        for (int32 Index = 0; Index < Vertices.Num(); ++Index)
        {
            if (!bNormalizedNormals[FaceVertexIndices[Index]])
            {
                GroupNormals[FaceVertexIndices[Index]].Normalize();
                bNormalizedNormals[FaceVertexIndices[Index]] = true;
            }
            Vertices[Index].TangentZ = GroupNormals[FaceVertexIndices[Index]];
        }
    }

    // Compute tangents
    {
        struct FMeshDesc
        {
            pxr::VtArray<int>* FaceVertexCounts;
            TArray<FStaticMeshBuildVertex>* Vertices;
            TArray<uint32>* FacePosInVertices;
        };

        FMeshDesc MeshDesc;
        MeshDesc.FaceVertexCounts = &FaceVertexCounts;
        MeshDesc.Vertices = &Vertices;
        MeshDesc.FacePosInVertices = &FacePosInVertices;

        // we can use mikktspace to calculate the tangents
        SMikkTSpaceInterface MikkTInterface;
        MikkTInterface.m_getNormal = [](const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;

            const FVector& Normal = (*MeshDesc.Vertices)[(*MeshDesc.FacePosInVertices)[iFace] + iVert].TangentZ;
            fvNormOut[0] = Normal.X;
            fvNormOut[1] = Normal.Y;
            fvNormOut[2] = Normal.Z;
        };
        MikkTInterface.m_getNumFaces = [](const SMikkTSpaceContext * pContext)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;

            return MeshDesc.FacePosInVertices->Num();
        };
        MikkTInterface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext * pContext, const int iFace)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;
            return (*MeshDesc.FaceVertexCounts)[iFace];
        };
        MikkTInterface.m_getPosition = [](const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;

            const FVector& Position = (*MeshDesc.Vertices)[(*MeshDesc.FacePosInVertices)[iFace] + iVert].Position;
            fvPosOut[0] = Position.X;
            fvPosOut[1] = Position.Y;
            fvPosOut[2] = Position.Z;
        };
        MikkTInterface.m_getTexCoord = [](const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;

            const FVector2D& UV = (*MeshDesc.Vertices)[(*MeshDesc.FacePosInVertices)[iFace] + iVert].UVs[0];
            fvTexcOut[0] = UV.X;
            fvTexcOut[1] = UV.Y;
        };
        MikkTInterface.m_setTSpaceBasic = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
        {
            const FMeshDesc& MeshDesc = *(FMeshDesc*)pContext->m_pUserData;

            FStaticMeshBuildVertex& Vertex = (*MeshDesc.Vertices)[(*MeshDesc.FacePosInVertices)[iFace] + iVert];
            Vertex.TangentX = FVector(fvTangent[0], fvTangent[1], fvTangent[2]).GetSafeNormal();
            Vertex.TangentY = FVector::CrossProduct(Vertex.TangentZ, Vertex.TangentX).GetSafeNormal() * -fSign;
        };
        MikkTInterface.m_setTSpace = nullptr;

        SMikkTSpaceContext MikkTContext;
        MikkTContext.m_pInterface = &MikkTInterface;
        MikkTContext.m_pUserData = &MeshDesc;
        MikkTContext.m_bIgnoreDegenerates = false;
        genTangSpaceDefault(&MikkTContext);
    }

    // Vertex color
    auto ColorPrimvar = USDMesh.GetDisplayColorPrimvar();
    if (ColorPrimvar)
    {
        pxr::VtArray<pxr::GfVec3f> USDColors;
        ComputeFlattened<pxr::GfVec3f>(ColorPrimvar, &USDColors);

        auto USDInterpType = ColorPrimvar.GetInterpolation();

        if (USDInterpType == pxr::UsdGeomTokens->faceVarying && USDColors.size() >= Vertices.Num())
        {
            for (int Index = 0; Index < Vertices.Num(); ++Index)
            {
                Vertices[Index].Color = USDConvertToLinearColor(USDColors[Index]).ToFColor(false);
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->vertex && USDColors.size() >= Points.size())
        {
            for (int Index = 0; Index < Vertices.Num(); ++Index)
            {
                Vertices[Index].Color = USDConvertToLinearColor(USDColors[FaceVertexIndices[Index]]).ToFColor(false);
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->uniform)
        {
            if (FaceVertexCounts.size() >= USDColors.size())
            {
                for (int Index = 0; Index < USDColors.size(); ++Index)
                {
                    int32 VertBegin = FacePosInVertices[Index];
                    int32 VertEnd = (Index == (USDColors.size() - 1)) ? Vertices.Num() : FacePosInVertices[Index + 1];

                    for (; VertBegin < VertEnd; ++VertBegin)
                    {
                        Vertices[VertBegin].Color = USDConvertToLinearColor(USDColors[Index]).ToFColor(false);
                    }
                }
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->constant)
        {
            for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
            {
                Vertices[VertexIdx].Color = USDColors.size() > 0 ? USDConvertToLinearColor(USDColors.front()).ToFColor(false) : FColor::White;
            }
        }
    }

    // Vertex color opacity
    auto OpacityPrimvar = USDMesh.GetDisplayOpacityPrimvar();
    if (OpacityPrimvar)
    {
        pxr::VtArray<float> USDOpacities;
        ComputeFlattened<float>(OpacityPrimvar, &USDOpacities);

        auto USDInterpType = OpacityPrimvar.GetInterpolation();

        if (USDInterpType == pxr::UsdGeomTokens->faceVarying && USDOpacities.size() >= Vertices.Num())
        {
            for (int Index = 0; Index < Vertices.Num(); ++Index)
            {
                Vertices[Index].Color.A = FMath::FloorToInt(FMath::Clamp(USDOpacities[Index], 0.0f, 1.0f) * 255.999f);
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->vertex && USDOpacities.size() >= Points.size())
        {
            for (int Index = 0; Index < Vertices.Num(); ++Index)
            {
                Vertices[Index].Color.A = FMath::FloorToInt(FMath::Clamp(USDOpacities[FaceVertexIndices[Index]], 0.0f, 1.0f) * 255.999f);
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->uniform)
        {
            if (FaceVertexCounts.size() >= USDOpacities.size())
            {
                for (int Index = 0; Index < USDOpacities.size(); ++Index)
                {
                    int32 VertBegin = FacePosInVertices[Index];
                    int32 VertEnd = (Index == (USDOpacities.size() - 1)) ? Vertices.Num() : FacePosInVertices[Index + 1];

                    for (; VertBegin < VertEnd; ++VertBegin)
                    {
                        Vertices[VertBegin].Color.A = FMath::FloorToInt(FMath::Clamp(USDOpacities[Index], 0.0f, 1.0f) * 255.999f);
                    }
                }
            }
        }
        else if (USDInterpType == pxr::UsdGeomTokens->constant)
        {
            for (int VertexIdx = 0; VertexIdx < Vertices.Num(); ++VertexIdx)
            {
                Vertices[VertexIdx].Color.A = USDOpacities.size() > 0 ? FMath::FloorToInt(FMath::Clamp(USDOpacities.front(), 0.0f, 1.0f) * 255.999f) : 255;
            }
        }
    }

    return true;
}

bool FSimReadyUSDImporterHelper::USDImportMesh(const pxr::UsdGeomMesh& USDMesh, FStaticMeshLODResources& MeshRes, const FVector& Offset)
{
    TArray<uint32> Indices;
    TArray<FStaticMeshBuildVertex> Vertices;
    TArray<FVector> Points;
    TArray<uint32> FacePosInIndices;
    TArray<uint32> FacePosInVertices;
    int32 TriangleCount;
    int32 UVCount;
    if (!USDImportMesh(USDMesh, FTransform::Identity, Vertices, Points, Indices, FacePosInIndices, FacePosInVertices, TriangleCount, UVCount, Offset))
    {
        return false;
    }

    // Initialize index buffer
    MeshRes.IndexBuffer.SetIndices(Indices, EIndexBufferStride::AutoDetect);
    // Initialize vertex buffer
    MeshRes.VertexBuffers.StaticMeshVertexBuffer.Init(Vertices, FMath::Max(UVCount, 1));
    MeshRes.VertexBuffers.PositionVertexBuffer.Init(Vertices);
    MeshRes.VertexBuffers.ColorVertexBuffer.Init(Vertices);
    MeshRes.bHasColorVertexData = MeshRes.VertexBuffers.ColorVertexBuffer.GetNumVertices() > 0;

    // Handle subsets
    MeshRes.Sections.Empty(1);
    auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
    if(!USDGeomSubsets.empty())
    {
        auto FaceVertexCounts = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexCountsAttr());
        // Re-arrange index buffer
        FStaticMeshSection Section;

        TArray<uint32> NewIndices;
        NewIndices.Reserve(Indices.Num());

        for(auto USDGeomSubset : USDGeomSubsets)
        {
            Section.FirstIndex += Section.NumTriangles * 3;
            Section.NumTriangles = 0;
            Section.MinVertexIndex = UINT32_MAX;
            Section.MaxVertexIndex = 0;

            auto FaceIndices = GetUSDValue<pxr::VtArray<int>>(USDGeomSubset.GetIndicesAttr());

            for(auto FaceIndex : FaceIndices)
            {
                if(FaceIndex >= FaceVertexCounts.size())
                {
                    return false;
                }

                auto VertexCount = FaceVertexCounts[FaceIndex];

                Section.NumTriangles += VertexCount - 2;

                if(FaceIndex >= FacePosInIndices.Num())
                {
                    return false;
                }

                auto StartPosInIndices = FacePosInIndices[FaceIndex];
                for(auto WedgeIndex = 0; WedgeIndex < (VertexCount - 2) * 3; ++WedgeIndex)
                {
                    auto Index = Indices[StartPosInIndices + WedgeIndex];
                    NewIndices.Add(Index);

                    Section.MinVertexIndex = FMath::Min(Section.MinVertexIndex, Index);
                    Section.MaxVertexIndex = FMath::Max(Section.MaxVertexIndex, Index);
                }
            }

            Section.MaterialIndex = MeshRes.Sections.Num();
            MeshRes.Sections.Add(Section);
        }

        MeshRes.IndexBuffer.SetIndices(NewIndices, EIndexBufferStride::AutoDetect);
    }
    else
    {
        FStaticMeshSection Section;
        Section.MaterialIndex = 0;
        Section.FirstIndex = 0;
        Section.NumTriangles = TriangleCount;
        Section.MinVertexIndex = 0;
        if(Vertices.Num() > 0)
        {
            Section.MaxVertexIndex = Vertices.Num() - 1;
        }
        MeshRes.Sections.Add(Section);
    }

    return true;
}

bool FSimReadyUSDImporterHelper::USDImportSkeleton(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, FSkeletalMeshImportData& SkeletalMeshImportData)
{
    TArray<FString> JointNames;
    TArray<int32> ParentJointIndices;

    pxr::VtArray<pxr::TfToken> JointOrder = USDSkeletonQuery.GetJointOrder();
    const pxr::UsdSkelTopology& Topology = USDSkeletonQuery.GetTopology();
    for (uint32 Index = 0; Index < Topology.GetNumJoints(); ++Index)
    {
        pxr::SdfPath JointPath(JointOrder[Index]);
        JointNames.Add(JointPath.GetName().c_str());
        int ParentIndex = Topology.GetParent(Index);
        ParentJointIndices.Add(ParentIndex);
    }

    if (JointNames.Num() > 0)
    {
        TArray<FTransform> BindTransforms;
        pxr::VtArray<pxr::GfMatrix4d> WorldBindTransforms;
        if (USDSkeletonQuery.GetJointWorldBindTransforms(&WorldBindTransforms))
        {
            for (uint32 Index = 0; Index < WorldBindTransforms.size(); ++Index)
            {
                BindTransforms.Add(USDConvert(USDSkeletonQuery.GetPrim().GetStage(), WorldBindTransforms[Index]));
            }
        }

        for (int32 Index = 0; Index < JointNames.Num(); ++Index)
        {
            SkeletalMeshImportData::FBone& Bone = SkeletalMeshImportData.RefBonesBinary.Add_GetRef(SkeletalMeshImportData::FBone());

            Bone.Name = JointNames[Index];
            Bone.ParentIndex = ParentJointIndices[Index];
            if (Bone.ParentIndex >= 0)
            {
                SkeletalMeshImportData::FBone& ParentBone = SkeletalMeshImportData.RefBonesBinary[Bone.ParentIndex];
                ++ParentBone.NumChildren;
            }

            SkeletalMeshImportData::FJointPos& JointMatrix = Bone.BonePos;
            JointMatrix.Transform = Index < BindTransforms.Num() ? BindTransforms[Index] : FTransform::Identity;
        }

        // Convert from world to local
        for (int32 Index = JointNames.Num() - 1; Index >= 0; --Index)
        {
            SkeletalMeshImportData::FBone& Bone = SkeletalMeshImportData.RefBonesBinary[Index];
            SkeletalMeshImportData::FJointPos& JointMatrix = Bone.BonePos;
            if (Bone.ParentIndex >= 0)
            {
                SkeletalMeshImportData::FBone& ParentBone = SkeletalMeshImportData.RefBonesBinary[Bone.ParentIndex];
                SkeletalMeshImportData::FJointPos& ParentJointMatrix = ParentBone.BonePos;
                JointMatrix.Transform *= ParentJointMatrix.Transform.Inverse();
            }
        }

        return true;
    }

    return false;
}

bool FSimReadyUSDImporterHelper::USDImportSkinning(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, const pxr::UsdSkelSkinningQuery& USDSkinningQuery, FSkeletalMeshImportData& SkeletalMeshImportData)
{
    const pxr::UsdPrim& SkinningPrim = USDSkinningQuery.GetPrim();
    pxr::UsdSkelBindingAPI SkelBinding(SkinningPrim);

    pxr::GfMatrix4d GeomBindingTransform(1);
    pxr::UsdAttribute GeomBindingAttribute = SkelBinding.GetGeomBindTransformAttr();
    if (GeomBindingAttribute)
    {
        GeomBindingAttribute.Get(&GeomBindingTransform);
    }

    auto USDStage = SkinningPrim.GetStage();
    FTransform GeomTransform = USDConvert(USDStage, GeomBindingTransform);
    pxr::UsdGeomMesh USDMesh = pxr::UsdGeomMesh(SkinningPrim);

    TArray<uint32> Indices;
    TArray<FStaticMeshBuildVertex> Vertices;
    TArray<FVector> Points;
    TArray<uint32> FacePosInIndices;
    TArray<uint32> FacePosInVertices;
    int32 TriangleCount;
    int32 UVCount;
    if (!USDImportMesh(USDMesh, GeomTransform, Vertices, Points, Indices, FacePosInIndices, FacePosInVertices, TriangleCount, UVCount, FVector::ZeroVector))
    {
        return false;
    }


    auto FaceVertexIndices = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexIndicesAttr());

    int32 NumExistPoints = SkeletalMeshImportData.Points.Num();
    int32 NumExistWedges = SkeletalMeshImportData.Wedges.Num();
    int32 NumExistFaces = SkeletalMeshImportData.Faces.Num();

    SkeletalMeshImportData.Wedges.Reserve(NumExistWedges + Indices.Num());
    SkeletalMeshImportData.Faces.Reserve(NumExistFaces + Indices.Num() / 3);
    SkeletalMeshImportData.Points.Append(Points);

    auto CreateWedgeAndTriangle = [&](int32 Index, int32 MaterialIndex)
    {
        const uint32 WedgeIndex = SkeletalMeshImportData.Wedges.AddUninitialized();
        SkeletalMeshImportData::FVertex& Wedge = SkeletalMeshImportData.Wedges[WedgeIndex];
        Wedge.VertexIndex = NumExistPoints + FaceVertexIndices[Indices[Index]];
        Wedge.Color = Vertices[Indices[Index]].Color;
        Wedge.MatIndex = MaterialIndex;
        Wedge.Reserved = 0;
        for (int32 UVIndex = 0; UVIndex < MAX_TEXCOORDS; ++UVIndex)
        {
            Wedge.UVs[UVIndex] = Vertices[Indices[Index]].UVs[UVIndex];
        }
        int32 CornerIndex = Index % 3;
        int32 TriangleFaceIndex = CornerIndex == 0 ? SkeletalMeshImportData.Faces.AddUninitialized() : SkeletalMeshImportData.Faces.Num() - 1;
        SkeletalMeshImportData::FTriangle& Triangle = SkeletalMeshImportData.Faces[TriangleFaceIndex];
        Triangle.WedgeIndex[CornerIndex] = WedgeIndex;
        Triangle.TangentX[CornerIndex] = Vertices[Indices[Index]].TangentX;
        Triangle.TangentY[CornerIndex] = Vertices[Indices[Index]].TangentY;
        Triangle.TangentZ[CornerIndex] = Vertices[Indices[Index]].TangentZ;
        Triangle.SmoothingGroups = 255;
        Triangle.MatIndex = MaterialIndex;
        Triangle.AuxMatIndex = 0;
    };

    auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
    if (!USDGeomSubsets.empty())
    {
        auto FaceVertexCounts = GetUSDValue<pxr::VtArray<int>>(USDMesh.GetFaceVertexCountsAttr());

        for (auto USDGeomSubset : USDGeomSubsets)
        {
            SkeletalMeshImportData::FMaterial NewMaterial;
            NewMaterial.Material = nullptr;
            NewMaterial.MaterialImportName = TEXT("");

            pxr::TfToken FamilyName;
            if (USDGeomSubset.GetFamilyNameAttr().Get(&FamilyName) && FamilyName == pxr::UsdShadeTokens->materialBind)
            {
                FUSDConversion::FMaterialBinding MaterialBinding = FUSDConversion::ParsePrimMaterial(USDGeomSubset.GetPrim());
                if (MaterialBinding.Material)
                {
                    FString Path = MaterialBinding.Material.GetPath().GetText();
                    NewMaterial.MaterialImportName = FPaths::GetBaseFilename(Path);
                }
            }
            uint32 MaterialIndex = SkeletalMeshImportData.Materials.Add(NewMaterial);

            auto FaceIndices = GetUSDValue<pxr::VtArray<int>>(USDGeomSubset.GetIndicesAttr());

            for (auto FaceIndex : FaceIndices)
            {
                if (FaceIndex >= FaceVertexCounts.size())
                {
                    return false;
                }

                auto VertexCount = FaceVertexCounts[FaceIndex];
                if (FaceIndex >= FacePosInIndices.Num())
                {
                    return false;
                }

                auto StartPosInIndices = FacePosInIndices[FaceIndex];
                for (auto WedgeIndex = 0; WedgeIndex < (VertexCount - 2) * 3; ++WedgeIndex)
                {
                    CreateWedgeAndTriangle(StartPosInIndices + WedgeIndex, MaterialIndex);
                }
            }
        }
    }
    else
    {
        SkeletalMeshImportData::FMaterial NewMaterial;
        NewMaterial.Material = nullptr;
        NewMaterial.MaterialImportName = TEXT("");

        FUSDConversion::FMaterialBinding MaterialBinding = FUSDConversion::ParsePrimMaterial(USDMesh.GetPrim());
        if (MaterialBinding.Material)
        {
            FString Path = MaterialBinding.Material.GetPath().GetText();
            NewMaterial.MaterialImportName = FPaths::GetBaseFilename(Path);
        }
        uint32 MaterialIndex = SkeletalMeshImportData.Materials.Add(NewMaterial);

        for (int32 Index = 0; Index < Indices.Num(); ++Index)
        {
            CreateWedgeAndTriangle(Index, MaterialIndex);
        }
    }

    SkeletalMeshImportData.bHasNormals = true;
    SkeletalMeshImportData.bHasTangents = true;
    SkeletalMeshImportData.bHasVertexColors = true;
    SkeletalMeshImportData.NumTexCoords = UVCount;
    SkeletalMeshImportData.MaxMaterialIndex = SkeletalMeshImportData.Materials.Num() > 0 ? FMath::Max((uint32)(SkeletalMeshImportData.Materials.Num() - 1), SkeletalMeshImportData.MaxMaterialIndex) : 0;

    // Joint
    // Check if there's skinning joint order, it should be mapped to skeleton joint order
    bool bNeedMapper = false;
    pxr::VtArray<int> SkinningJointMap;
    auto Mapper = USDSkinningQuery.GetMapper();
    if (Mapper)
    {
        pxr::VtArray<int> SkeletonJointMap;
        pxr::VtArray<pxr::TfToken> SkeletonJointOrder = USDSkeletonQuery.GetJointOrder();
        pxr::VtArray<int> SkeletonJointIndices;
        for (int Index = 0; Index < SkeletonJointOrder.size(); ++Index)
        {
            SkeletonJointIndices.push_back(Index);
        }

        if (Mapper->Remap(SkeletonJointIndices, &SkinningJointMap)) {
            bNeedMapper = true;
        }
    }

    pxr::VtArray<int> JointIndices;
    pxr::VtArray<float> JointWeights;
    USDSkinningQuery.ComputeVaryingJointInfluences(Points.Num(), &JointIndices, &JointWeights);

    uint32 NumInfluencesPerComponent = USDSkinningQuery.GetNumInfluencesPerComponent();
    if (NumInfluencesPerComponent > MAX_TOTAL_INFLUENCES)
    {
        pxr::UsdSkelResizeInfluences(&JointIndices, NumInfluencesPerComponent, MAX_TOTAL_INFLUENCES);
        pxr::UsdSkelResizeInfluences(&JointWeights, NumInfluencesPerComponent, MAX_TOTAL_INFLUENCES);
        NumInfluencesPerComponent = MAX_TOTAL_INFLUENCES;
    }

    uint32 JointIndex = 0;
    SkeletalMeshImportData.Influences.Reserve(Points.Num());
    for (int32 PointIndex = 0; PointIndex < Points.Num(); ++PointIndex)
    {
        for (uint32 InfluenceIndex = 0; InfluenceIndex < NumInfluencesPerComponent; ++InfluenceIndex, ++JointIndex)
        {
            float BoneWeight = JointWeights[JointIndex];
            if (BoneWeight != 0.f)
            {
                int32 Index = SkeletalMeshImportData.Influences.AddUninitialized();
                SkeletalMeshImportData.Influences[Index].BoneIndex = bNeedMapper ? SkinningJointMap[JointIndices[JointIndex]] : JointIndices[JointIndex];
                SkeletalMeshImportData.Influences[Index].Weight = BoneWeight;
                SkeletalMeshImportData.Influences[Index].VertexIndex = NumExistPoints + PointIndex;
            }
        }
    }

    return true;
}

void FSimReadyUSDImporterHelper::BuildSourceModel(UStaticMesh* Mesh)
{
    const int32 LODIndex = 0;
    if (Mesh->GetNumSourceModels() == 0)
    {
        Mesh->AddSourceModel();
    }

    FMeshDescription* MeshDesc = Mesh->GetMeshDescription(LODIndex);
    if (MeshDesc == nullptr)
    {
        MeshDesc = Mesh->CreateMeshDescription(LODIndex);
    }
    ExportStaticMeshLOD(Mesh->RenderData->LODResources[LODIndex], *MeshDesc, Mesh->StaticMaterials);
    FStaticMeshSourceModel& SrcModel = Mesh->GetSourceModel(LODIndex);
    SrcModel.BuildSettings.bGenerateLightmapUVs = false; // Don't need to generate lightmap UV for usd mesh
    SrcModel.BuildSettings.bRecomputeNormals = false;
    SrcModel.BuildSettings.bRecomputeTangents = false;

    Mesh->CommitMeshDescription(LODIndex);
    Mesh->PostEditChange();
}

UStaticMesh* FSimReadyUSDImporterHelper::CreateStaticMesh(const pxr::UsdGeomMesh& USDMesh, UObject* Parent, FName Name, EObjectFlags Flags, const FVector& Offset)
{
    // Create static mesh to avoid breaking existing mesh
    auto StaticMesh = NewObject<UStaticMesh>(Parent, Name, Flags);

    // Construct mesh
    if(!USDImportMesh(USDMesh, *StaticMesh, Offset))
    {
        return nullptr;
    }

    // TODO: If there's the collision from usd, do not auto-generate bodysetup
    if (StaticMesh->bCustomizedCollision == false && StaticMesh->BodySetup)
    {
        /*FKAggregateGeom & AggGeom = StaticMesh->BodySetup->AggGeom;
        AggGeom.ConvexElems.Empty(1);

        const int32 NumDirs = 18;
        TArray<FVector> Dirs;
        Dirs.AddUninitialized(NumDirs);
        for (int32 DirIdx = 0; DirIdx < NumDirs; ++DirIdx)
        { 
            Dirs[DirIdx] = KDopDir18[DirIdx]; 
        }
        GenerateKDopAsSimpleCollision(StaticMesh, Dirs);*/
        StaticMesh->BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
    }

    // Bind default material
    if(UMaterial* DefaultMaterial = LoadObject<UMaterial>(nullptr, *SimReadyDefaultMaterial))
    {
        StaticMesh->StaticMaterials.Empty();

        auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
        if (!USDGeomSubsets.empty())
        {
            for (auto USDGeomSubset : USDGeomSubsets)
            {
                FStaticMaterial NewMaterial(DefaultMaterial);
                pxr::TfToken FamilyName;
                if (USDGeomSubset.GetFamilyNameAttr().Get(&FamilyName) && FamilyName == pxr::UsdShadeTokens->materialBind)
                {
                    FUSDConversion::FMaterialBinding MaterialBinding = FUSDConversion::ParsePrimMaterial(USDGeomSubset.GetPrim());
                    if (MaterialBinding.Material)
                    {
                        FString Path = MaterialBinding.Material.GetPath().GetText();
                        NewMaterial.MaterialSlotName = *FPaths::GetBaseFilename(Path);
                        NewMaterial.ImportedMaterialSlotName = *FPaths::GetBaseFilename(Path);
                    }
                }

                StaticMesh->StaticMaterials.Add(NewMaterial);
            }
        }
        else
        {
            FStaticMaterial NewMaterial(DefaultMaterial);

            FUSDConversion::FMaterialBinding MaterialBinding = FUSDConversion::ParsePrimMaterial(USDMesh.GetPrim());
            if (MaterialBinding.Material)
            {
                FString Path = MaterialBinding.Material.GetPath().GetText();
                NewMaterial.MaterialSlotName = *FPaths::GetBaseFilename(Path);
                NewMaterial.ImportedMaterialSlotName = *FPaths::GetBaseFilename(Path);
            }
            StaticMesh->StaticMaterials.Add(NewMaterial);
        }

        StaticMesh->UpdateUVChannelData(false);
    }

    // Add to cache
    if (StaticMesh->GetOutermost() != GetTransientPackage())
    {
        // We should create static materials before building source model
        BuildSourceModel(StaticMesh);
        FAssetRegistryModule::AssetCreated(StaticMesh);
    }

    return StaticMesh;
}

UAnimSequence* FSimReadyUSDImporterHelper::CreateAnimSequence(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, UObject* Parent, FName Name, EObjectFlags Flags, USkeleton* Skeleton, USkeletalMesh* PreviewMesh, double TimeCodesPerSecond, double StartTimeCode, double EndTimeCode, FBox* BoundingBox)
{
    UAnimSequence* AnimSequence = nullptr;
    if (Skeleton == nullptr)
    {
        return nullptr;
    }
    pxr::UsdSkelAnimQuery AnimQuery = USDSkeletonQuery.GetAnimQuery();
    std::vector<double> TimeSamples;
    if (AnimQuery.IsValid() && AnimQuery.GetJointTransformTimeSamples(&TimeSamples))
    {
        AnimSequence = NewObject<UAnimSequence>(Parent, Name, Flags);
        if (AnimSequence)
        {
            auto USDStage = USDSkeletonQuery.GetPrim().GetStage();
            auto Prim = USDSkeletonQuery.GetSkeleton().GetPrim();
            pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache();
            bool resetsXformStack;
            pxr::GfMatrix4d LocalMatrix = Cache.GetLocalTransformation(Prim, &resetsXformStack);
            FTransform SkeletonToComponent = USDConvert(USDStage, LocalMatrix);

            AnimSequence->SetSkeleton(Skeleton);
            AnimSequence->ImportFileFramerate = TimeCodesPerSecond;
            AnimSequence->ImportResampleFramerate = TimeCodesPerSecond;
            double TotalTimeSample = EndTimeCode - StartTimeCode;
            int32 NumberOfFrame = FMath::CeilToInt(FMath::Max(TotalTimeSample + 1.0, 1.0));
            AnimSequence->SequenceLength = TotalTimeSample / TimeCodesPerSecond;
            AnimSequence->SetRawNumberOfFrame(NumberOfFrame);

            TArray<FName> BoneNames;
            BoneNames.Empty(Skeleton->GetReferenceSkeleton().GetNum());
            BoneNames.AddUninitialized(Skeleton->GetReferenceSkeleton().GetNum());
            for (int32 i = 0; i < Skeleton->GetReferenceSkeleton().GetNum(); i++)
            {
                BoneNames[i] = Skeleton->GetReferenceSkeleton().GetBoneName(i);
            }

            for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
            {
                // add tracks for the bone existing
                AnimSequence->AddNewRawTrack(BoneNames[BoneIndex]);
            }

            auto AddKeyToSequence = [&](pxr::UsdTimeCode TimeCode)
            {
                pxr::VtArray<pxr::GfMatrix4d> USDJointLocalTransforms;
                USDSkeletonQuery.ComputeJointLocalTransforms(&USDJointLocalTransforms, TimeCode);

                pxr::VtArray<pxr::GfMatrix4d> USDJointWorldTransforms;
                if (BoundingBox)
                {
                    //Get world joint transform to update bounding box				
                    pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache(TimeCode);
                    USDSkeletonQuery.ComputeJointWorldTransforms(&USDJointWorldTransforms, &Cache);
                }

                for (int32 JointIndex = 0; JointIndex < BoneNames.Num(); ++JointIndex)
                {
                    FTransform LocalTransform = USDConvert(USDStage, USDJointLocalTransforms[JointIndex]);
                    // Applying the transform of skeleton to root of the animation.
                    if (JointIndex == 0)
                    {
                        LocalTransform *= SkeletonToComponent;
                    }
                    
                    FRawAnimSequenceTrack& RawTrack = AnimSequence->GetRawAnimationTrack(JointIndex);
                    RawTrack.PosKeys.Add(LocalTransform.GetTranslation());
                    RawTrack.RotKeys.Add(LocalTransform.GetRotation());
                    RawTrack.ScaleKeys.Add(LocalTransform.GetScale3D());

                    if (BoundingBox)
                    {
                        // Update animation Bounding box for skeletal mesh
                        // TODO: if physcis asset can be created, this can be ignored.
                        FTransform WorldTransform = USDConvert(USDStage, USDJointWorldTransforms[JointIndex]);
                        *BoundingBox += WorldTransform.GetLocation();
                    }
                }
            };

            if (TimeSamples.size() == 0)
            {
                AddKeyToSequence(pxr::UsdTimeCode::Default());
            }
            else
            {
                for (double TimeSample = 0.0; TimeSample <= TotalTimeSample; TimeSample += 1.0)
                {
                    AddKeyToSequence(StartTimeCode + TimeSample);
                }
            }

            AnimSequence->MarkRawDataAsModified();
            AnimSequence->PostProcessSequence();
            AnimSequence->PostEditChange();
            AnimSequence->SetPreviewMesh(PreviewMesh);
        }
    }

    return AnimSequence;
}

bool FSimReadyUSDImporterHelper::GetMdlPathAndName(pxr::UsdShadeShader ShadeShader, FString& MdlPath, FString& MaterialName, bool& bRelativePath)
{
    pxr::SdfAssetPath SdfMdlPath;
    if (ShadeShader.GetSourceAsset(&SdfMdlPath, USDTokens.mdl))
    {
        // Material name is optional, it's not required
        pxr::TfToken MaterialNameToken; 
        if (ShadeShader.GetSourceAssetSubIdentifier(&MaterialNameToken, USDTokens.mdl))
        {
            MaterialName = MaterialNameToken.GetText();
            if (!MaterialName.Contains(TEXT("(")) && MaterialName.Contains(TEXT("::")))
            {
                FString InvalidSubIdentifier = ShadeShader.GetPath().GetText();
                UE_LOG(LogSimReadyUsd, Warning, TEXT("Invalid subIdentifier: %s on prim %s. Do not prepend namespace to material name."), *MaterialName, *InvalidSubIdentifier);
                FString(MoveTemp(MaterialName)).Split("::", nullptr, &MaterialName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
            }
        }
        
        MdlPath = SdfMdlPath.GetAssetPath().c_str();
        if (!USimReadyMDL::IsLocalBaseMDL(MdlPath) && !USimReadyMDL::IsLocalMaterialGraphMDL(MdlPath))
        {
            const auto& ResolvedPath = SdfMdlPath.GetResolvedPath();
            if (ResolvedPath.empty())
            {
                auto ShaderPrim = ShadeShader.GetPrim();
                for (const auto& ShaderPrimSpec : ShaderPrim.GetPrimStack())
                {
                    for (const auto& PrimProperty : ShaderPrimSpec->GetProperties())
                    {
                        if (PrimProperty->GetName() == "info:mdl:sourceAsset")
                        {
                            if (PrimProperty->GetValueType().IsA<PXR_NS::SdfAssetPath>() && PrimProperty->HasDefaultValue())
                            {
                                auto AssetPath = PrimProperty->GetDefaultValue().Get<PXR_NS::SdfAssetPath>();
                                if (AssetPath.GetAssetPath() == SdfMdlPath.GetAssetPath())
                                {
                                    MdlPath = PrimProperty->GetLayer()->ComputeAbsolutePath(AssetPath.GetAssetPath()).c_str();
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        bRelativePath = FPaths::IsRelative(MdlPath);
        
        if(bRelativePath)
        {
            FString RelativePath = SdfMdlPath.GetResolvedPath().c_str();
            if (!RelativePath.IsEmpty())
            {
                MdlPath = RelativePath;
            }
            else // local base mdls
            {
                bRelativePath = false;
            }
        }

        return true;
    }

    return false;
}

void FSimReadyUSDImporterHelper::LinkNodeGraphShader(const pxr::UsdShadeShader& ShadeShader, FString& InstanceName)
{
    if (ShadeShader)
    {
        FString MdlPath, MaterialName;
        bool bRelativePath = false;
        GetMdlPathAndName(ShadeShader, MdlPath, MaterialName, bRelativePath);

        FString ModuleName = FPaths::GetPath(MdlPath) / FPaths::GetBaseFilename(MdlPath);
        ModuleName.ReplaceInline(TEXT("/"), TEXT("::"));
        if (!ModuleName.StartsWith(TEXT("::")))
        {
            ModuleName = TEXT("::") + ModuleName;
        }

        static int32 InstanceID = 0;
        InstanceName = FString::Printf(TEXT("Instance%d"), InstanceID++);
        USimReadyMDL::CreateMdlInstance(ModuleName, MaterialName, InstanceName);

        std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
        for (auto Input : Inputs)
        {
            FString InputName = Input.GetBaseName().GetText();
            pxr::VtValue VtValue;
            if (Input.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI Source;
                pxr::TfToken SourceName;
                pxr::UsdShadeAttributeType SourceType;
                Input.GetConnectedSource(&Source, &SourceName, &SourceType);
        
                if (Source.GetPrim().IsA<pxr::UsdShadeShader>())
                {
                    // input link to shader
                    FString ArgumentInstance;
                    LinkNodeGraphShader(pxr::UsdShadeShader(Source.GetPrim()), ArgumentInstance);
                    USimReadyMDL::SetCall(InstanceName, InputName, ArgumentInstance);
                }
                else if (Source.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
                {
                    // input link to node graph
                    LinkNodeGraphShader(pxr::UsdShadeNodeGraph(Source.GetPrim()), SourceName, InputName, InstanceName);
                }
            }
        }
    }
}

void FSimReadyUSDImporterHelper::LinkNodeGraphShader(const pxr::UsdShadeNodeGraph& NodeGraph, const pxr::TfToken& SourceName, const FString& InputName, const FString& InstanceName)
{
    if (NodeGraph)
    {
        auto NodeOutput = NodeGraph.GetOutput(SourceName);
        auto NodeInput = NodeGraph.GetInput(SourceName);
        if (NodeOutput && NodeOutput.HasConnectedSource())
        {
            // output of node graph link to shader
            pxr::UsdShadeConnectableAPI OutputSource;
            pxr::TfToken OutputSourceName;
            pxr::UsdShadeAttributeType OutputSourceType;
            NodeOutput.GetConnectedSource(&OutputSource, &OutputSourceName, &OutputSourceType);

            FString ArgumentInstance;
            LinkNodeGraphShader(pxr::UsdShadeShader(OutputSource.GetPrim()), ArgumentInstance);
            USimReadyMDL::SetCall(InstanceName, InputName, ArgumentInstance);
        }
        else if (NodeInput && NodeInput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI InputSource;
            pxr::TfToken InputSourceName;
            pxr::UsdShadeAttributeType InputSourceType;
            NodeInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

            // input of node graph link to shader
            if (InputSource.GetPrim().IsA<pxr::UsdShadeShader>())
            {
                FString ArgumentInstance;
                LinkNodeGraphShader(pxr::UsdShadeShader(InputSource.GetPrim()), ArgumentInstance);
                USimReadyMDL::SetCall(InstanceName, InputName, ArgumentInstance);
            }
            // input of node graph link to node graph
            else if (InputSource.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
            {
                LinkNodeGraphShader(pxr::UsdShadeNodeGraph(InputSource.GetPrim()), InputSourceName, InputName, InstanceName);
            }
        }
    }
}

bool FSimReadyUSDImporterHelper::LoadMaterialGraph(const pxr::UsdShadeShader& ShadeShader, UMaterial* Material)
{
    if (Material == nullptr)
    {
        return false;
    }

    bool Result = false;
    USimReadyMDL::LoadMaterialGraphDefinitions();
    FString MaterialInstanceName;
    LinkNodeGraphShader(ShadeShader, MaterialInstanceName);
    Result = USimReadyMDL::DistillMaterialInstance(Material, MaterialInstanceName, false);
    USimReadyMDL::UnloadMaterialGraphDefinitions();

    return Result;
}

void SetParameterByInputValue(UMaterialInstanceConstant* MaterialInst, UMaterialEditorInstanceConstant* EditorInst, const FString& Parameter, const pxr::UsdShadeInput& Input)
{
    pxr::VtValue VtValue;
    if (Input.Get(&VtValue))
    {
        auto Type = VtValue.GetType();
        if (Type == pxr::SdfValueTypeNames->Float.GetType())
        {
            float Value = VtValue.Get<float>();
            MaterialInst->SetScalarParameterValueEditorOnly(*Parameter, Value);
            FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInst, *Parameter, Value);
        }
        else if(Type == pxr::SdfValueTypeNames->Int.GetType())
        {
            int32 Value = VtValue.Get<int>();
            MaterialInst->SetScalarParameterValueEditorOnly(*Parameter, (float)Value);
            FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInst, *Parameter, (float)Value);
        }
        else if(Type == pxr::SdfValueTypeNames->Color3f.GetType()
            || Type == pxr::SdfValueTypeNames->Normal3f.GetType())
        {
            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec3f>());
            MaterialInst->SetVectorParameterValueEditorOnly(*Parameter, Value);
            FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInst, *Parameter, Value);
        }
        else if(Type == pxr::SdfValueTypeNames->Float2.GetType())
        {
            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec2f>());
            MaterialInst->SetVectorParameterValueEditorOnly(*Parameter, Value);
            FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInst, *Parameter, Value);
        }
    }
}

void GetScalarValueByName(const pxr::UsdShadeShader& ShadeShader, const pxr::TfToken &name, float DefaultTextureOut, float& Out)
{
    auto Input = ShadeShader.GetInput(name);
    if (Input)
    {
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            Input.GetConnectedSource(&Source, &SourceName, &SourceType);
                                    
            pxr::UsdShadeShader ConnectedShader = pxr::UsdShadeShader(Source);
            if (ConnectedShader)
            {
                Out = DefaultTextureOut;
            }
            else
            {
                auto ConnectInput = Source.GetInput(SourceName);
                ConnectInput.Get<float>(&Out);
            }
        }
        else
        {
            Input.Get<float>(&Out);
        }
    }
}

void FSimReadyUSDImporterHelper::UpdatePreviewSurfaceInputs(
    UMaterialInstanceConstant* MaterialInst,
    const pxr::UsdShadeShader& ShadeShader,
    const TMap<FString, int32>* UVSets,
    TFunction<void(const FString&)> Callback,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    if (!ShadeShader)
    {
        return;
    }

    if (GetUSDValue<pxr::TfToken>(ShadeShader.GetIdAttr()) != USDTokens.previewSurface)
    {
        // Try to get the usd shader for preview surface
        auto USDMaterial = FindShadeMaterial(ShadeShader.GetPrim());
        if (USDMaterial)
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            auto SurfaceOutput = USDMaterial.GetSurfaceOutput();
            if (SurfaceOutput)
            {
                if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                {
                    auto PreviewSurfaceShader = pxr::UsdShadeShader(Source);
                    UpdatePreviewSurfaceInputs(MaterialInst, PreviewSurfaceShader, UVSets, Callback, CreateCallback);
                }
            }	
        }
        return;
    }

    UMaterialEditorInstanceConstant* EditorInstanceInUse = nullptr;
    auto* MaterialEditor = (IMaterialEditor*)GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(MaterialInst, false);
    if (MaterialEditor)
    {
        for( FObjectIterator Iter( UMaterialEditorInstanceConstant::StaticClass() ); Iter; ++Iter )
        {
            UMaterialEditorInstanceConstant* EditorInstance = Cast<UMaterialEditorInstanceConstant>( *Iter );
            if (EditorInstance->SourceInstance == MaterialInst)
            {
                EditorInstanceInUse = EditorInstance;
            }
        }
    }

    FString MaterialName;
    int32 MaterialReparentIndex = FSimReadyMaterialReparentUtility::FindOmniMaterial(MaterialInst->Parent, MaterialName, ESimReadyReparentUsage::Import);

    // try getting opacity/opacityThreshold at first to decide the model
    float opacity = 1.0f;
    GetScalarValueByName(ShadeShader, pxr::TfToken("opacity"), 0.5f, opacity);

    float opacityThreshold = 0.0f;
    GetScalarValueByName(ShadeShader, pxr::TfToken("opacityThreshold"), 0.5f, opacityThreshold);

    if (opacity < 1.0f && opacityThreshold == 0.0f)
    {
        MaterialInst->BasePropertyOverrides.bOverride_BlendMode = true;
        MaterialInst->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Translucent;
        MaterialInst->BasePropertyOverrides.bOverride_TwoSided = true;
        MaterialInst->BasePropertyOverrides.TwoSided = false;
        MaterialInst->PostEditChange();
    }
    else if (opacityThreshold > 0.0f)
    {
        MaterialInst->BasePropertyOverrides.bOverride_OpacityMaskClipValue = true;
        MaterialInst->BasePropertyOverrides.OpacityMaskClipValue = opacityThreshold;
        MaterialInst->PostEditChange();
    }

    auto TextureCoord = [&](const pxr::UsdShadeShader& Shader, const FString& PropertyName)
    {
        auto VarNameInput = Shader.GetInput(USDTokens.varname);
        if (UVSets && VarNameInput)
        {
            //Version 2.3 Changes type of UsdPrimvarReader.varname input from token to string
            // so try getting TfToken at first
            FString TexCoordName;
            pxr::TfToken TexCoord = GetUSDValue<pxr::TfToken>(VarNameInput.GetAttr());
            if (TexCoord.IsEmpty())
            {
                std::string TexCoordNameStr;
                VarNameInput.Get<std::string>(&TexCoordNameStr);
                TexCoordName = TexCoordNameStr.c_str();
            }
            else
            {
                TexCoordName = TexCoord.GetText();
            }

            const int32* UVIndex = UVSets->Find(TexCoordName);

            FString DisplayName = PropertyName + TEXT("UV");
            if (MaterialReparentIndex != INDEX_NONE)
            {
                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
            }

            if (UVIndex)
            {
                MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, (float)(*UVIndex));
                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, (float)(*UVIndex));
            }
            else
            {
                MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, 0.0f);
                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, 0.0f);
            }
        }
    };

    auto Transform2d = [&](const pxr::UsdShadeShader& Shader, const FString& PropertyName)
    {
        auto InInput = Shader.GetInput(pxr::TfToken("in"));
        if (InInput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            InInput.GetConnectedSource(&Source, &SourceName, &SourceType);
            pxr::UsdShadeShader TexCoordShader = pxr::UsdShadeShader(Source);
            if (Callback)
            {
                Callback(TexCoordShader.GetPath().GetText());
            }
            const auto TexCoordShaderID = GetUSDValue<pxr::TfToken>(TexCoordShader.GetIdAttr());

            if (TexCoordShaderID == USDTokens.PrimvarReader_float2)
            {
                TextureCoord(TexCoordShader, PropertyName);
            }
        }

        FString RotationDisplayName = PropertyName + TEXT("Transform2dRotation");
        FString ScaleDisplayName = PropertyName + TEXT("Transform2dScale");
        FString TranslationDisplayName = PropertyName + TEXT("Transform2dTranslation");
        if (MaterialReparentIndex != INDEX_NONE)
        {
            FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(RotationDisplayName), RotationDisplayName);
            FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(ScaleDisplayName), ScaleDisplayName);
            FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TranslationDisplayName), TranslationDisplayName);
        }

        auto RotationInput = Shader.GetInput(pxr::TfToken("rotation"));
        SetParameterByInputValue(MaterialInst, EditorInstanceInUse, RotationDisplayName, RotationInput);
        auto ScaleInput = Shader.GetInput(pxr::TfToken("scale"));
        SetParameterByInputValue(MaterialInst, EditorInstanceInUse, ScaleDisplayName, ScaleInput);
        auto TranslationInput = Shader.GetInput(pxr::TfToken("translation"));
        SetParameterByInputValue(MaterialInst, EditorInstanceInUse, TranslationDisplayName, TranslationInput);
    };

    std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
    for (auto Input : Inputs)
    {
        auto ConnectInput = Input;
        const FString PropertyName = ConnectInput.GetBaseName().GetText();
        FString PropertyDisplayName = PropertyName;
        if (MaterialReparentIndex != INDEX_NONE)
        {
            FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(PropertyDisplayName), PropertyDisplayName);
        }

        if (ConnectInput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            ConnectInput.GetConnectedSource(&Source, &SourceName, &SourceType);
                                    
            FString Mask = SourceName.GetText();
            pxr::UsdShadeShader ConnectedShader = pxr::UsdShadeShader(Source);
            if (ConnectedShader)
            {
                if (Callback)
                {
                    Callback(ConnectedShader.GetPath().GetText());
                }
                bool IsColor = PropertyName == TEXT("diffuseColor") || PropertyName == TEXT("emissiveColor");
                if (GetUSDValue<pxr::TfToken>(ConnectedShader.GetIdAttr()) == USDTokens.uvTexture)
                {
                    bool bTextureLoaded = false;
                    auto FileInput = ConnectedShader.GetInput(USDTokens.file);
                    if (FileInput)
                    {
                        pxr::SdfAssetPath SdfTexturePath;
                        if (FileInput.Get<pxr::SdfAssetPath>(&SdfTexturePath))
                        {
                            FString TextureDisplayName = PropertyName + TEXT("Texture");
                            if (MaterialReparentIndex != INDEX_NONE)
                            {
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureDisplayName), TextureDisplayName);
                            }

                            bTextureLoaded = LoadTextureByShadeInput(FileInput, ConnectedShader.GetInput(USDTokens.sourceColorSpace), *MaterialInst, EditorInstanceInUse, TextureDisplayName, true, IsColor, CreateCallback);
                            if (bTextureLoaded)
                            {
                                FString DisplayName = PropertyName + TEXT("TextureEnable");
                                if (MaterialReparentIndex != INDEX_NONE)
                                {
                                    FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
                                }
                                MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, 1.0f);
                                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, 1.0f);
                            }
                        }

                        auto STInput = ConnectedShader.GetInput(USDTokens.st);
                        if (STInput && STInput.HasConnectedSource())
                        {
                            pxr::UsdShadeConnectableAPI STSource;
                            pxr::TfToken STSourceName;
                            pxr::UsdShadeAttributeType STSourceType;
                            STInput.GetConnectedSource(&STSource, &STSourceName, &STSourceType);

                            pxr::UsdShadeShader TexCoordShader = pxr::UsdShadeShader(STSource);
                            if (Callback)
                            {
                                Callback(TexCoordShader.GetPath().GetText());
                            }
                            const auto TexCoordShaderID = GetUSDValue<pxr::TfToken>(TexCoordShader.GetIdAttr());
                            if (TexCoordShaderID == USDTokens.PrimvarReader_float2)
                            {
                                TextureCoord(TexCoordShader, PropertyName);
                            }
                            else if (TexCoordShaderID == USDTokens.Transform2d)
                            {
                                Transform2d(TexCoordShader, PropertyName);
                            }
                        }

                        auto ScaleInput = ConnectedShader.GetInput(pxr::TfToken("scale"));
                        if (ScaleInput)
                        {
                            pxr::GfVec4f Scale;
                            if (ScaleInput.Get<pxr::GfVec4f>(&Scale))
                            {
                                FString DisplayName = PropertyName + TEXT("TextureScale");
                                if (MaterialReparentIndex != INDEX_NONE)
                                {
                                    FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
                                }

                                MaterialInst->SetVectorParameterValueEditorOnly(*DisplayName, USDConvertToLinearColor(Scale));
                                FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, USDConvertToLinearColor(Scale));
                            }
                        }

                        auto BiasInput = ConnectedShader.GetInput(pxr::TfToken("bias"));
                        if (BiasInput)
                        {
                            pxr::GfVec4f Bias;
                            if (BiasInput.Get<pxr::GfVec4f>(&Bias))
                            {
                                FString DisplayName = PropertyName + TEXT("TextureBias");
                                if (MaterialReparentIndex != INDEX_NONE)
                                {
                                    FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
                                }
                                MaterialInst->SetVectorParameterValueEditorOnly(*DisplayName, USDConvertToLinearColor(Bias));
                                FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, USDConvertToLinearColor(Bias));
                            }
                        }

                        if (!Mask.Equals("rgb"))
                        {
                            FString ComponentMaskParameter = PropertyName + "Mask";
                            if (MaterialReparentIndex != INDEX_NONE)
                            {
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(ComponentMaskParameter), ComponentMaskParameter);
                            }

                            if (Mask.Equals("r"))
                            {
                                MaterialInst->SetScalarParameterValueEditorOnly(*ComponentMaskParameter, 0.0f);
                                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *ComponentMaskParameter, 0.0f);
                            }
                            else if (Mask.Equals("g"))
                            {
                                MaterialInst->SetScalarParameterValueEditorOnly(*ComponentMaskParameter, 1.0f);
                                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *ComponentMaskParameter, 1.0f);
                            }
                            else if (Mask.Equals("b"))
                            {
                                MaterialInst->SetScalarParameterValueEditorOnly(*ComponentMaskParameter, 2.0f);
                                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *ComponentMaskParameter, 2.0f);
                            }
                            else //a
                            {
                                MaterialInst->SetScalarParameterValueEditorOnly(*ComponentMaskParameter, 3.0f);
                                FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *ComponentMaskParameter, 3.0f);
                            }
                        }
                    }
                                
                    if (!bTextureLoaded)// If not get texture file, setting the fallback color
                    {
                        auto FallbackInput = ConnectedShader.GetInput(pxr::TfToken("fallback"));
                        if (FallbackInput)
                        {
                            pxr::GfVec4f Fallback;
                            if (FallbackInput.Get<pxr::GfVec4f>(&Fallback))
                            {
                                FLinearColor FallbackColor = USDConvertToLinearColor(Fallback);

                                FString DisplayName = PropertyName;

                                if (MaterialReparentIndex != INDEX_NONE)
                                {
                                    FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
                                }
                                if (Mask.Equals("r"))
                                {
                                    MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, FallbackColor.R);
                                    FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, FallbackColor.R);
                                }
                                else if (Mask.Equals("g"))
                                {
                                    MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, FallbackColor.G);
                                    FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, FallbackColor.G);
                                }
                                else if (Mask.Equals("b"))
                                {
                                    MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, FallbackColor.B);
                                    FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, FallbackColor.B);
                                }
                                else if (Mask.Equals("a"))
                                {
                                    MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, FallbackColor.A);
                                    FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, FallbackColor.A);
                                }
                                else //rgb
                                {
                                    MaterialInst->SetVectorParameterValueEditorOnly(*DisplayName, FallbackColor);
                                    FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, FallbackColor);
                                }
                            }
                        }
                    }
                }
                else
                {
                    const auto Varname = GetUSDValue<pxr::TfToken>(ConnectedShader.GetInput(USDTokens.varname));
                    if (Varname == USDTokens.displayColor && IsColor)
                    {
                        FString DisplayName = PropertyName + TEXT("DisplayColor");
                        if (MaterialReparentIndex != INDEX_NONE)
                        {
                            FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(DisplayName), DisplayName);
                        }
                        MaterialInst->SetScalarParameterValueEditorOnly(*DisplayName, 1.0f);
                        FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, 1.0f);
                    }
                }
            }
            else
            {
                ConnectInput = Source.GetInput(SourceName);
                SetParameterByInputValue(MaterialInst, EditorInstanceInUse, PropertyDisplayName, ConnectInput);
            }
        }
        else
        {
            SetParameterByInputValue(MaterialInst, EditorInstanceInUse, PropertyDisplayName, ConnectInput);
        }
    }
}

void FSimReadyUSDImporterHelper::UpdateNodeGraphInput(
    UMaterialInstanceConstant* Instance,
    const pxr::UsdShadeNodeGraph& NodeGraph,
    const pxr::TfToken& SourceName,
    const FString& ParentInputName,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    if (NodeGraph)
    {
        auto NodeOutput = NodeGraph.GetOutput(SourceName);
        auto NodeInput = NodeGraph.GetInput(SourceName);
        if (NodeOutput && NodeOutput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI OutputSource;
            pxr::TfToken OutputSourceName;
            pxr::UsdShadeAttributeType OutputSourceType;
            NodeOutput.GetConnectedSource(&OutputSource, &OutputSourceName, &OutputSourceType);

            UpdateNodeGraphInput(Instance, pxr::UsdShadeShader(OutputSource.GetPrim()), ParentInputName, CreateCallback);
        }
        else if (NodeInput && NodeInput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI InputSource;
            pxr::TfToken InputSourceName;
            pxr::UsdShadeAttributeType InputSourceType;
            NodeInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

            // input of node graph link to shader
            if (InputSource.GetPrim().IsA<pxr::UsdShadeShader>())
            {
                UpdateNodeGraphInput(Instance, pxr::UsdShadeShader(InputSource.GetPrim()), ParentInputName, CreateCallback);
            }
            // input of node graph link to node graph
            else if (InputSource.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
            {
                UpdateNodeGraphInput(Instance, pxr::UsdShadeNodeGraph(InputSource.GetPrim()), InputSourceName, ParentInputName, CreateCallback);
            }
        }
        else if (NodeInput)
        {
            LoadMdlInput(*Instance, NodeInput, ParentInputName.IsEmpty() ? NodeInput.GetBaseName().GetText() : ParentInputName, false, CreateCallback);
        }
    }
}

void FSimReadyUSDImporterHelper::UpdateNodeGraphInput(
    UMaterialInstanceConstant* Instance,
    const pxr::UsdShadeShader& ShadeShader,
    const FString& ParentInputName,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    if (ShadeShader)
    {
        std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
        for (auto Input : Inputs)
        {
            FString InputName = Input.GetBaseName().GetText();
            FString UEInputName = ParentInputName.IsEmpty() ? InputName : (ParentInputName + TEXT(".") + InputName);
            if (Input.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI Source;
                pxr::TfToken SourceName;
                pxr::UsdShadeAttributeType SourceType;
                Input.GetConnectedSource(&Source, &SourceName, &SourceType);

                if (Source.GetPrim().IsA<pxr::UsdShadeShader>())
                {
                    UpdateNodeGraphInput(Instance, pxr::UsdShadeShader(Source.GetPrim()), UEInputName, CreateCallback);
                }
                else if (Source.GetPrim().IsA<pxr::UsdShadeNodeGraph>())
                {
                    UpdateNodeGraphInput(Instance, pxr::UsdShadeNodeGraph(Source.GetPrim()), SourceName, UEInputName, CreateCallback);
                }
            }
            else
            {
                LoadMdlInput(*Instance, Input, ParentInputName.IsEmpty() ? InputName : UEInputName, false, CreateCallback);
            }
        }
    }
}

bool FSimReadyUSDImporterHelper::UpdateMaterialGraphInputs(
    const pxr::UsdShadeShader& ShadeShader,
    UMaterialInstanceConstant* Instance,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    if (Instance == nullptr)
    {
        return false;
    }

    UpdateNodeGraphInput(Instance, ShadeShader, TEXT(""), CreateCallback);
    return true;
}

bool FSimReadyUSDImporterHelper::LoadMdlInput(
    UMaterialInstanceConstant& InstanceConstant,
    const pxr::UsdShadeInput& Input,
    const FString& DisplayName,
    bool bCheckParameter,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    UMaterialEditorInstanceConstant* EditorInstanceInUse = nullptr;
    auto* MaterialEditor = (IMaterialEditor*)GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(&InstanceConstant, false);
    if (MaterialEditor)
    {
        for( FObjectIterator Iter( UMaterialEditorInstanceConstant::StaticClass() ); Iter; ++Iter )
        {
            UMaterialEditorInstanceConstant* EditorInstance = Cast<UMaterialEditorInstanceConstant>( *Iter );
            if (EditorInstance->SourceInstance == &InstanceConstant)
            {
                EditorInstanceInUse = EditorInstance;
            }
        }
    }

    pxr::VtValue VtValue;
    if (Input.Get(&VtValue))
    {
        auto Type = VtValue.GetType();
        if (Type == pxr::SdfValueTypeNames->Float.GetType())
        {	
            if (bCheckParameter)
            {
                float CurrentValue;
                if (!InstanceConstant.GetScalarParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            float Value = VtValue.Get<float>();
            InstanceConstant.SetScalarParameterValueEditorOnly(*DisplayName, Value);
            FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, Value);
        }
        else if (Type == pxr::SdfValueTypeNames->Int.GetType())
        {
            if (bCheckParameter)
            {
                float CurrentValue;
                if (!InstanceConstant.GetScalarParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            int32 Value = VtValue.Get<int>();
            InstanceConstant.SetScalarParameterValueEditorOnly(*DisplayName, (float)Value);
            FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, (float)Value);
        }
        else if (Type == pxr::SdfValueTypeNames->Bool.GetType())
        {
            if (bCheckParameter)
            {
                float CurrentValue;
                if (!InstanceConstant.GetScalarParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            bool Value = VtValue.Get<bool>();
            //SwitchParameters.Add(*DisplayName, Value);
            InstanceConstant.SetScalarParameterValueEditorOnly(*DisplayName, Value ? 1.0f : 0.0f);
            FMaterialEditorInstanceNotifier::SetScalarParameterValue(EditorInstanceInUse, *DisplayName, Value ? 1.0f : 0.0f);
        }
        else if (Type == pxr::SdfValueTypeNames->Float2.GetType())
        {
            if (bCheckParameter)
            {
                FLinearColor CurrentValue;
                if (!InstanceConstant.GetVectorParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec2f>());
            InstanceConstant.SetVectorParameterValueEditorOnly(*DisplayName, Value);
            FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, Value);
        }
        else if (Type == pxr::SdfValueTypeNames->Float3.GetType())
        {
            if (bCheckParameter)
            {
                FLinearColor CurrentValue;
                if (!InstanceConstant.GetVectorParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec3f>());
            InstanceConstant.SetVectorParameterValueEditorOnly(*DisplayName, Value);
            FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, Value);
        }
        else if (Type == pxr::SdfValueTypeNames->Float4.GetType())
        {
            if (bCheckParameter)
            {
                FLinearColor CurrentValue;
                if (!InstanceConstant.GetVectorParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec4f>());
            InstanceConstant.SetVectorParameterValueEditorOnly(*DisplayName, Value);
            FMaterialEditorInstanceNotifier::SetVectorParameterValue(EditorInstanceInUse, *DisplayName, Value);
        }
        else if (Type == pxr::SdfValueTypeNames->Asset.GetType())
        {
            if (bCheckParameter)
            {
                UTexture* CurrentValue;
                if (!InstanceConstant.GetTextureParameterValue(*DisplayName, CurrentValue))
                {
                    return false;
                }
            }

            pxr::UsdShadeInput SourceColorSpace;
            LoadTextureByShadeInput(Input, SourceColorSpace, InstanceConstant, EditorInstanceInUse, DisplayName, false, false, CreateCallback);
        }

        return true;
    }

    return false;
}

FString ResolveUDIMPath(const pxr::SdfLayerHandle& LayerHandle, const FString& AssetPath)
{
    FString AssetPathToResolve = AssetPath;
    AssetPathToResolve.ReplaceInline(TEXT("<UDIM>"), TEXT("1001"));

    pxr::ArResolver& Resolver = pxr::ArGetResolver();

    const std::string AbsolutePathToResolve =
        LayerHandle
        ? LayerHandle->ComputeAbsolutePath(TCHAR_TO_UTF8(*AssetPathToResolve))
        : Resolver.Resolve(TCHAR_TO_UTF8(*AssetPathToResolve));

    return AbsolutePathToResolve.c_str();
}

void OverrideTextureSRGB(UTexture* Texture, bool SRGB)
{
    if (!(Texture && Texture->IsValidLowLevel()))
    {
        return;
    }

    if (Texture && Texture->SRGB != SRGB)
    {
        Texture->SRGB = SRGB;

        if (Texture->SRGB && Texture->CompressionSettings == TC_Normalmap)
        {
            Texture->CompressionSettings = TC_Default;
        }

        Texture->PostEditChange();
        Texture->MarkPackageDirty();
    }
}

bool FSimReadyUSDImporterHelper::LoadTextureByShadeInput(
    const pxr::UsdShadeInput& FileInput,
    const pxr::UsdShadeInput& SourceColorSpace,
    UMaterialInstanceConstant& MaterialInstance,
    UMaterialEditorInstanceConstant* EditorInst,
    const FString& PropertyName,
    bool bSetDefaultColorSpace,
    bool bDefaultSRGB,
    TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
)
{
    pxr::SdfAssetPath SdfTexturePath;
    FileInput.Get<pxr::SdfAssetPath>(&SdfTexturePath);
    FString TexturePath = SdfTexturePath.GetAssetPath().c_str();
    if(FPaths::IsRelative(TexturePath))
    {
        TexturePath = SdfTexturePath.GetResolvedPath().c_str();
    }

    FString ResolvedPath = SdfTexturePath.GetResolvedPath().c_str();
    if (!ResolvedPath.IsEmpty() && FSimReadyPathHelper::IsPackagePath(ResolvedPath))
    {
        UTexture* Texture = nullptr;
        {
            pxr::ArResolver& resolver = pxr::ArGetResolver();
            auto Asset = resolver.OpenAsset(SdfTexturePath.GetResolvedPath());
            if (Asset && Asset->GetSize() > 0)
            {
                uint8* Buffer = new uint8[Asset->GetSize()];
                Asset->Read(Buffer, Asset->GetSize(), 0);
                if (CreateCallback)
                {
                    CreateCallback(Buffer, Asset->GetSize(), ResolvedPath, Texture);
                }

                if (Texture)
                {
                    pxr::TfToken ColorSpaceToken;
                    if (SourceColorSpace)
                    {
                        SourceColorSpace.Get(&ColorSpaceToken);
                    }
                    else
                    {
                        ColorSpaceToken = FileInput.GetAttr().GetColorSpace();
                    }

                    if (ColorSpaceToken == USDTokens.raw_deprecated || ColorSpaceToken == USDTokens.raw || ColorSpaceToken == USDTokens.sRGB)
                    {
                        bool SRGB = (ColorSpaceToken == USDTokens.sRGB);
                        OverrideTextureSRGB(Texture, SRGB);
                    }
                    else if (bSetDefaultColorSpace)
                    {
                        OverrideTextureSRGB(Texture, bDefaultSRGB);
                    }
                    else if (ColorSpaceToken == USDTokens.auto_colorspace)
                    {
                        OverrideTextureSRGB(Texture, true);
                    }
                }

                delete[] Buffer;
            }
        }

        if (Texture)
        {
            FMaterialEditorInstanceNotifier::SetTextureParameterValue(EditorInst, *PropertyName, Texture);
            MaterialInstance.SetTextureParameterValueEditorOnly(*PropertyName, Texture);
        }

        return true;
    }

    if (TexturePath.IsEmpty())
    {
        TexturePath = SdfTexturePath.GetAssetPath().c_str();
        if (!TexturePath.IsEmpty())
        {
            pxr::SdfLayerHandle LayerHandle = GetLayerHandle(FileInput.GetAttr());
            if (LayerHandle)
            {
                if (TexturePath.Contains(TEXT("<UDIM>")))
                {
                    TexturePath = ResolveUDIMPath(LayerHandle, TexturePath);
                }
            }
        }
    }

    if (!TexturePath.IsEmpty())
    {
        UTexture* Texture = nullptr;
        USimReadyTexture* TextureAsset = nullptr;

        if (Texture == nullptr && FPaths::FileExists(TexturePath))
        {
            TArray64<uint8> Content;
            if (FFileHelper::LoadFileToArray(Content, *TexturePath, EFileRead::FILEREAD_Silent))
            {
                if (CreateCallback)
                {
                    CreateCallback(Content.GetData(), Content.Num(), ResolvedPath, Texture);
                }
            }
        }

        if (Texture)
        {	
            pxr::TfToken ColorSpaceToken;
            if (SourceColorSpace)
            {
                SourceColorSpace.Get(&ColorSpaceToken);
            }
            else
            {
                ColorSpaceToken = FileInput.GetAttr().GetColorSpace();
            }

            TOptional<bool> OverrideSRGB;
            if (ColorSpaceToken == USDTokens.raw_deprecated || ColorSpaceToken == USDTokens.raw || ColorSpaceToken == USDTokens.sRGB)
            {
                OverrideSRGB = (ColorSpaceToken == USDTokens.sRGB);
            }
            else if (bSetDefaultColorSpace)
            {
                OverrideSRGB = bDefaultSRGB;
            }
            else if (ColorSpaceToken == USDTokens.auto_colorspace)
            {
                OverrideSRGB = true;
            }

            if (OverrideSRGB.IsSet())
            {
                if (!TextureAsset)
                {
                    OverrideTextureSRGB(Texture, OverrideSRGB.GetValue());
                }
            }

            FMaterialEditorInstanceNotifier::SetTextureParameterValue(EditorInst, *PropertyName, Texture);
            MaterialInstance.SetTextureParameterValueEditorOnly(*PropertyName, Texture);			

            return true;
        }
    }

    return false;
}
