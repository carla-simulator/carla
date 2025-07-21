// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDDerivedDataCache.h"
#include "DerivedDataCacheInterface.h"
#include "HAL/IConsoleManager.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "StaticMeshResources.h"
#include "Components.h"
#include "Rendering/SkeletalMeshLODImporterData.h"

namespace UsdDDCType
{
    enum Type
    {
        StaticMesh,
        SkeletalMesh,
        Invalid,
    };
}

static TAutoConsoleVariable<int32> CVarLocalMeshCache(TEXT("SimReady.LocalMeshCache"), 1, TEXT(""));

#define USDMESH_DERIVEDDATA_VER TEXT("CBA7FD3AB2A44BDCAD47900D8F211D93")
#define USDSKELETAL_DERIVEDDATA_VER TEXT("D2C2D39A1A7744BC94443764F46B1F8C")

static FString GetDDCKey(UsdDDCType::Type CachedType, const FString& USDHash)
{
    if (CVarLocalMeshCache.GetValueOnAnyThread() == 0)
    {
        return TEXT("");
    }

    FString KeyPrefix;
    switch (CachedType)
    {
    case UsdDDCType::StaticMesh:
        KeyPrefix = FString::Printf(TEXT("MeshCache_%s_%s"), USDMESH_DERIVEDDATA_VER, *USDHash);
        break;
    case UsdDDCType::SkeletalMesh:
        KeyPrefix = FString::Printf(TEXT("SkeletalCache_%s_%s"), USDSKELETAL_DERIVEDDATA_VER, *USDHash);
        break;
    }

    auto& DerivedDataCache = GetDerivedDataCacheRef();
    FString DDCVersion = FString::Printf(TEXT("wegrr6534r_%d_%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
    return DerivedDataCache.BuildCacheKey(TEXT("SimReadyDerivedData"), *DDCVersion, *KeyPrefix);
}

static void operator << (FArchive& Ar, FStaticMeshBuildVertex& Vertex)
{
    Ar << Vertex.Position;
    Ar << Vertex.TangentX;
    Ar << Vertex.TangentY;
    Ar << Vertex.TangentZ;
    for (auto& UV : Vertex.UVs)
    {
        Ar << UV;
    }
    Ar << Vertex.Color;
}

void Serialize(FArchive& Ar, FStaticMeshRenderData& StaticMeshRenderData)
{
    FStaticMeshLODResources& MeshRes = StaticMeshRenderData.LODResources[0];

    auto& PositionBuffer = MeshRes.VertexBuffers.PositionVertexBuffer;
    auto& VertexBuffer = MeshRes.VertexBuffers.StaticMeshVertexBuffer;
    auto& ColorBuffer = MeshRes.VertexBuffers.ColorVertexBuffer;

    if (Ar.IsSaving())
    {
        Ar << StaticMeshRenderData.Bounds;

        int32 SectionNum = MeshRes.Sections.Num();
        Ar << SectionNum;

        for (int32 Index = 0; Index < SectionNum; ++Index)
        {
            auto& Section = MeshRes.Sections[Index];

            Ar << Section.FirstIndex;
            Ar << Section.NumTriangles;
            Ar << Section.MinVertexIndex;
            Ar << Section.MaxVertexIndex;
        }

        TArray<uint32> Indices;
        MeshRes.IndexBuffer.GetCopy(Indices);
        Ar << Indices;

        uint32 TexCoordsNum = VertexBuffer.GetNumTexCoords();
        Ar << TexCoordsNum;

        TArray<FStaticMeshBuildVertex> Vertices;
        Vertices.SetNum(VertexBuffer.GetNumVertices());
        for (uint32 Index = 0; Index < VertexBuffer.GetNumVertices(); ++Index)
        {
            auto& Vertex = Vertices[Index];

            Vertex.Position = PositionBuffer.VertexPosition(Index);

            for (uint32 UVIndex = 0; UVIndex < FMath::Min<uint32>(VertexBuffer.GetNumTexCoords(), MAX_STATIC_TEXCOORDS); ++UVIndex)
            {
                Vertex.UVs[UVIndex] = VertexBuffer.GetVertexUV(Index, UVIndex);
            }

            Vertex.TangentX = VertexBuffer.VertexTangentX(Index);
            Vertex.TangentY = VertexBuffer.VertexTangentY(Index);
            Vertex.TangentZ = VertexBuffer.VertexTangentZ(Index);

            Vertex.Color = Index < ColorBuffer.GetNumVertices() ? ColorBuffer.VertexColor(Index) : FColor::White;
        }

        Ar << Vertices;
    }
    else
    {
        Ar << StaticMeshRenderData.Bounds;

        int32 SectionNum = 0;
        Ar << SectionNum;
        MeshRes.Sections.Empty();
        MeshRes.Sections.AddDefaulted(SectionNum);

        for (int32 Index = 0; Index < SectionNum; ++Index)
        {
            auto& Section = MeshRes.Sections[Index];

            Ar << Section.FirstIndex;
            Ar << Section.NumTriangles;
            Ar << Section.MinVertexIndex;
            Ar << Section.MaxVertexIndex;
        }

        TArray<uint32> Indices;
        Ar << Indices;
        MeshRes.IndexBuffer.SetIndices(Indices, EIndexBufferStride::AutoDetect);

        uint32 TexCoordsNum = 0;
        Ar << TexCoordsNum;

        TArray<FStaticMeshBuildVertex> Vertices;
        Ar << Vertices;

        PositionBuffer.Init(Vertices);
        VertexBuffer.Init(Vertices, FMath::Max<uint32>(1, TexCoordsNum));
        ColorBuffer.Init(Vertices);
        MeshRes.bHasColorVertexData = ColorBuffer.GetNumVertices() > 0;
    }
}

void Serialize(FArchive& Ar, FSkeletalMeshImportData& SkeletalMeshData)
{
    Ar << SkeletalMeshData.NumTexCoords;
    Ar << SkeletalMeshData.MaxMaterialIndex;
    Ar << SkeletalMeshData.bHasVertexColors;
    Ar << SkeletalMeshData.bHasNormals;
    Ar << SkeletalMeshData.bHasTangents;
    Ar << SkeletalMeshData.Materials;
    Ar << SkeletalMeshData.Points;
    Ar << SkeletalMeshData.Wedges;
    Ar << SkeletalMeshData.Faces;
    Ar << SkeletalMeshData.RefBonesBinary;
    Ar << SkeletalMeshData.Influences;
}

void FUSDDerivedDataCache::Save(const FString& HashKey, FStaticMeshRenderData& StaticMeshRenderData)
{
    FString CacheKey = GetDDCKey(UsdDDCType::StaticMesh, HashKey);

    if (CacheKey.IsEmpty())
    {
        return;
    }

    TArray<uint8> CacheData;
    FMemoryWriter Ar(CacheData, true);

    Serialize(Ar, StaticMeshRenderData);

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
    GetDerivedDataCacheRef().Put(*CacheKey, CacheData);
#else
    GetDerivedDataCacheRef().Put(*CacheKey, CacheData, TEXT("SimReady USD Import Static Mesh"), false);
#endif
}

bool FUSDDerivedDataCache::Load(const FString& HashKey, FStaticMeshRenderData& StaticMeshRenderData)
{
    TArray<uint8> CacheData;
    FString CacheKey = GetDDCKey(UsdDDCType::StaticMesh, HashKey);

    if (CacheKey.IsEmpty())
    {
        return false;
    }

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
    if (GetDerivedDataCacheRef().GetSynchronous(*CacheKey, CacheData))
#else
    if (GetDerivedDataCacheRef().GetSynchronous(*CacheKey, CacheData, TEXT("SimReady USD Import Static Mesh")))
#endif
    {
        if (CacheData.Num() > 0)
        {
            FMemoryReader Ar(CacheData, true);
            Serialize(Ar, StaticMeshRenderData);
            return true;
        }
    }

    return false;
}

void FUSDDerivedDataCache::Save(const FString& HashKey, FSkeletalMeshImportData& SkeletalMeshData)
{
    FString CacheKey = GetDDCKey(UsdDDCType::SkeletalMesh, HashKey);
    if (CacheKey.IsEmpty())
    {
        return;
    }

    TArray<uint8> CacheData;
    FMemoryWriter Ar(CacheData, true);

    Serialize(Ar, SkeletalMeshData);

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
    GetDerivedDataCacheRef().Put(*CacheKey, CacheData);
#else
    GetDerivedDataCacheRef().Put(*CacheKey, CacheData, TEXT("SimReady USD Import Skeletal Mesh"), false);
#endif
}

bool FUSDDerivedDataCache::Load(const FString& HashKey, FSkeletalMeshImportData& SkeletalMeshData)
{
    TArray<uint8> CacheData;
    FString CacheKey = GetDDCKey(UsdDDCType::SkeletalMesh, HashKey);

    if (CacheKey.IsEmpty())
    {
        return false;
    }

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 24
    if (GetDerivedDataCacheRef().GetSynchronous(*CacheKey, CacheData))
#else
    if (GetDerivedDataCacheRef().GetSynchronous(*CacheKey, CacheData, TEXT("SimReady USD Import Skeletal Mesh")))
#endif
    {
        if (CacheData.Num() > 0)
        {
            FMemoryReader Ar(CacheData, true);
            Serialize(Ar, SkeletalMeshData);
            return true;
        }
    }

    return false;
}
