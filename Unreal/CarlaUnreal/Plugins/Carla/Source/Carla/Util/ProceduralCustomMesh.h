// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "ProceduralCustomMesh.generated.h"

/// A definition of a Carla Mesh.
USTRUCT(Blueprintable)
struct CARLA_API FProceduralCustomMesh
{
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="VertexData")
  TArray<FVector> Vertices;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="VertexData")
  TArray<int32> Triangles;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="VertexData")
  TArray<FVector> Normals;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="VertexData")
  TArray<FVector2D> UV0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="VertexData")
  TArray<FLinearColor> VertexColor;

  // This is commented due to an strange bug including ProceduralMeshComponent.h
  // UPROPERTY()
  // TArray<FProcMeshTangent> Tangents;
};
