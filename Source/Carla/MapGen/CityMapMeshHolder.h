// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "CityMapMeshTag.h"
#include "CityMapMeshHolder.generated.h"

class UInstancedStaticMeshComponent;

/// Holds the static meshes and instances necessary for building the city map.
UCLASS(Abstract)
class CARLA_API ACityMapMeshHolder : public AActor
{
  GENERATED_BODY()

public:

  // Sets default values for this actor's properties
  ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer);

protected:

#if WITH_EDITOR
  /// Called after a property change in editor.
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

  /// Return the 3D world location (relative to this actor) of the given 2D
  /// tile.
  FVector GetTileLocation(uint32 X, uint32 Y) const;

  /// Set the static mesh associated with @a Tag.
  void SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh);

  /// Return the static mesh corresponding to @a Tag.
  const UStaticMesh *GetStaticMesh(ECityMapMeshTag Tag) const;

  /// Add an instance of a mesh with a given tile location.
  ///   @param Tag The mesh' tag
  ///   @param X Tile coordinate X
  ///   @param Y Tile coordinate Y
  void AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y);

  /// Add an instance of a mesh with a given tile location and rotation.
  ///   @param Tag The mesh' tag
  ///   @param X Tile coordinate X
  ///   @param Y Tile coordinate Y
  ///   @param Angle Rotation around Z axis
  void AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y, float Angle);

  /// Add an instance of a mesh with a given transform.
  ///   @param Tag The mesh' tag
  ///   @param Transform Transform that will be applied to the mesh
  void AddInstance(ECityMapMeshTag Tag, FTransform Transform);

private:

  /// Clear all instances in the instantiators and update the static meshes.
  void ResetInstantiators();

  /// Set the scale to the dimensions of the base mesh.
  void UpdateMapScale();

  UPROPERTY()
  USceneComponent *SceneRootComponent;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  TMap<ECityMapMeshTag, UStaticMesh *> StaticMeshes;

  UPROPERTY(Category = "Meshes|Debug", VisibleAnywhere)
  float MapScale;

  UPROPERTY(Category = "Meshes|Debug", VisibleAnywhere)
  TArray<UInstancedStaticMeshComponent *> MeshInstatiators;
};
