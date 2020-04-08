// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "ProceduralCustomMesh.generated.h"

/// A definition of a Carla Mesh.
USTRUCT()
struct CARLA_API FProceduralCustomMesh
{
  GENERATED_BODY()

  UPROPERTY()
  TArray<FVector> Vertices;

  UPROPERTY()
  TArray<int32> Triangles;

  UPROPERTY()
  TArray<FVector> Normals;

  UPROPERTY()
  TArray<FVector2D> UV0;

  UPROPERTY()
  TArray<FLinearColor> VertexColor;

  // This is commented due to an strange bug including ProceduralMeshComponent.h
  // UPROPERTY()
  // TArray<FProcMeshTangent> Tangents;
};
