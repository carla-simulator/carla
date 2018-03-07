// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "CityMapMeshTag.h"
#include "CityMapMeshHolder.generated.h"

class IDetailLayoutBuilder;
class UInstancedStaticMeshComponent;
class AStaticMeshActor;


/// Holds the static meshes and instances necessary for building a city map.
UCLASS(Abstract)
class CARLA_API ACityMapMeshHolder : public AActor
{
  GENERATED_BODY()

  // ===========================================================================
  // -- Construction and update related methods --------------------------------
  // ===========================================================================
public:

  /// Initializes the mesh holders. It is safe to call SetStaticMesh after this.
  /// However, instances cannot be added until OnConstruction is called.
  ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer);
  //void LayoutDetails( IDetailLayoutBuilder& DetailLayout );
protected:

  /// Initializes the instantiators.
  virtual void OnConstruction(const FTransform &Transform) override;
  virtual void PostInitializeComponents() override;

#if WITH_EDITOR
  /// Clears and updates the instantiators.
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

  // ===========================================================================
  // -- Other protected methods ------------------------------------------------
  // ===========================================================================
protected:

  float GetMapScale() const
  {
	  
    return MapScale;
  }

  /// Return the 3D world location (relative to this actor) of the given 2D
  /// tile.
  FVector GetTileLocation(uint32 X, uint32 Y) const;

  /// Set the static mesh associated with @a Tag.
  void SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh);

  /// Return the static mesh corresponding to @a Tag.
  UStaticMesh *GetStaticMesh(ECityMapMeshTag Tag);

  /// Return the static mesh corresponding to @a Tag.
  const UStaticMesh *GetStaticMesh(ECityMapMeshTag Tag) const;

  /// Return the tag corresponding to @a StaticMesh.
  ECityMapMeshTag GetTag(const UStaticMesh &StaticMesh) const;

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

  // ===========================================================================
  // -- Private methods and members --------------------------------------------
  // ===========================================================================
private:

  /// Here does nothing, implement in derived classes.
  virtual void UpdateMap();

  /// Clear all instances of the static mesh actors.
  void DeletePieces();

  /// Set the scale to the dimensions of the base mesh.
  void UpdateMapScale();

 private:


  UPROPERTY()
  USceneComponent *SceneRootComponent;

  UPROPERTY(Category = "Map Generation", VisibleAnywhere)
  float MapScale = 1.0f;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  TMap<ECityMapMeshTag, UStaticMesh *> StaticMeshes;

  UPROPERTY()
  TMap<UStaticMesh *, ECityMapMeshTag> TagMap;


};
