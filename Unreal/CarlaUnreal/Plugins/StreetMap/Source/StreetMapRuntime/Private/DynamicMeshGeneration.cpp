// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DynamicMeshGeneration.h"

// Engine headers
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstance.h"
#include "StaticMeshAttributes.h"
#include "RenderingThread.h"
#include "DynamicMeshGeneration.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMeshActor.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "Engine/StaticMesh.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"

// Carla C++ headers

// Carla plugin headers

DEFINE_LOG_CATEGORY(LogCarlaDynamicMeshGeneration);
static const float OSMToCentimetersScaleFactor = 100.0f;

UStaticMesh* UDynamicMeshGeneration::CreateMeshFromPoints(
    TArray<FVector> Points3D,
    FName MeshName,
    const FString& AssetPath,
    bool bFlipped,
    FVector Offset)
{
    TArray<FVector2D> Points;
    TMap<FVector2D, float> Heights;
    Points.Reserve(Points3D.Num());
    for (const FVector& Point : Points3D)
    {
        // Convert FVector to FVector2D by ignoring Z coordinate
        FVector2D Point2D(Point.X, Point.Y);
        Points.Add(Point2D);
        // Store height for each point
        Heights.Add(Point2D, Point.Z);
    }

    if (Points.Num() < 3)
    {
        UE_LOG(LogCarlaDynamicMeshGeneration, Warning, TEXT("Not enough points to create a mesh"));
        return nullptr;
    }

    // Step 1: Build a DynamicMesh
    UDynamicMesh* DynamicMesh = NewObject<UDynamicMesh>();
    FGeometryScriptPrimitiveOptions Options;
    Options.bFlipOrientation = bFlipped;
    FTransform Transform;

    // Example: 3D extruded polygon, change height/thickness as needed
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSimpleExtrudePolygon(
        DynamicMesh,
        Options,
        Transform,
        Points,
        100.0f,  // Extrude height
        1.0f,    // Wall thickness
        true
    );


    // Step 2: Apply height (Z) from original Points3D + Offset
    DynamicMesh->EditMesh([&](FDynamicMesh3& Mesh)
      {
        for (int32 vid : Mesh.VertexIndicesItr())
        {
          FVector3d Pos = Mesh.GetVertex(vid);

          FVector2D XY(Pos.X, Pos.Y);
          float Z = Pos.Z;

          // Try to replace Z if a height was stored for this XY
          if (const float* FoundZ = Heights.Find(XY))
          {
            Z = *FoundZ;
          }

          Mesh.SetVertex(vid, FVector3d(Pos.X + Offset.X, Pos.Y + Offset.Y, Z + Offset.Z));
        }
      });


    // Step 2: Construct full package name (path + mesh name)
    FString CleanAssetPath = AssetPath;
    MeshName = FName(*FString::Printf(TEXT("SM_%s"), *MeshName.ToString()));
    FString PackageName = CleanAssetPath / MeshName.ToString();
    FString UniquePackageName;
    if (!FPackageName::TryConvertFilenameToLongPackageName(PackageName, UniquePackageName))
    {
      UE_LOG(LogCarlaDynamicMeshGeneration, Error, TEXT("Invalid package name: %s"), *PackageName);
      return nullptr;
    }
    // Step 3: Create the package
    UPackage* Package = CreatePackage(*UniquePackageName);
    if (!Package)
    {
        UE_LOG(LogCarlaDynamicMeshGeneration, Error, TEXT("Failed to create package for mesh at: %s"), *UniquePackageName);
        return nullptr;
    }

    UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>(
      Package,
      MeshName,
      RF_Public | RF_Standalone
    );

    if (!NewStaticMesh)
    {
      UE_LOG(LogCarlaDynamicMeshGeneration, Error, TEXT("Failed to create StaticMesh asset"));
      return nullptr;
    }

    // Step 4: Copy mesh data into the StaticMesh using Geometry Script
    FGeometryScriptCopyMeshToAssetOptions CopyOptions;
    //CopyOptions.bRecomputeNormals = true;
    //CopyOptions.bRecomputeTangents = true;

    FGeometryScriptMeshWriteLOD TargetLOD;
    TargetLOD.LODIndex = 0;

    EGeometryScriptOutcomePins Outcome;
    UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshToStaticMesh(
      DynamicMesh,
      NewStaticMesh,
      CopyOptions,
      TargetLOD,
      Outcome,
      nullptr  // Debug
    );

    if (Outcome != EGeometryScriptOutcomePins::Success)
    {
      UE_LOG(LogCarlaDynamicMeshGeneration, Error, TEXT("Failed to copy mesh into StaticMesh"));
      return nullptr;
    }

    if (!NewStaticMesh)
    {
        UE_LOG(LogCarlaDynamicMeshGeneration, Error, TEXT("Failed to create StaticMesh from DynamicMesh"));
        return nullptr;
    }

    // Step 5: Register and save asset
    FAssetRegistryModule::AssetCreated(NewStaticMesh);
    Package->MarkPackageDirty();

    FString PackageFileName = FPackageName::LongPackageNameToFilename(UniquePackageName, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_None;
    UPackage::SavePackage(Package, NewStaticMesh, *PackageFileName, SaveArgs);

    UE_LOG(LogCarlaDynamicMeshGeneration, Log, TEXT("Created StaticMesh asset: %s"), *PackageName);

    return NewStaticMesh;
}
