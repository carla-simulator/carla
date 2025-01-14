// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include <util/ue-header-guard-end.h>

#include "ProceduralBuilding.generated.h"


// TODO: support n-sides building

struct FloorMeshCollection
{
  TArray<UStaticMesh*>* MainMeshes = nullptr;
  TArray<TSubclassOf<AActor>>* MainBPs = nullptr;
  TArray<UStaticMesh*>* CornerMeshes = nullptr;
  TArray<TSubclassOf<AActor>>* CornerBPs = nullptr;
  TArray<UStaticMesh*>* AuxiliarMeshes = nullptr;
  TArray<TSubclassOf<AActor>>* AuxiliarBPs = nullptr;
};

UCLASS()
class CARLA_API AProceduralBuilding : public AActor
{
  GENERATED_BODY()

public:

  // Sets default values for this actor's properties
  AProceduralBuilding();

  // Looks for the HISMComp on the HISMComps Map that uses the SelectedMesh and returns it.
  // If doesn't exist its created
  UFUNCTION(BlueprintCallable, Category="Procedural Building")
  UHierarchicalInstancedStaticMeshComponent* GetHISMComp(const UStaticMesh* SM);

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Procedural Building")
  void ConvertOldBP_ToNativeCodeObject(AActor* BP_Building);

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Procedural Building")
  void HideAllChildren();

  UFUNCTION(BlueprintCallable, Category="Procedural Building|Conversion")
  void SetBaseParameters(
    const TSet<int>& InDoorsIndexPosition,
    const TArray<bool>& InUseWallMesh,
    int InNumFloors,
    int InLengthX,
    int InLengthY,
    bool InCorners,
    bool InUseFullBlocks);

  UFUNCTION(BlueprintCallable, Category="Procedural Building|Conversion")
  void SetVisibilityParameters(
    const TArray<bool>& InSideVisibility,
    const TArray<bool>& InCornerVisibility,
    bool InRoofVisibility);

  UFUNCTION(BlueprintCallable, Category="Procedural Building|Conversion")
  void SetBaseMeshes(
    const TArray<UStaticMesh*>& InBaseMeshes,
    const TArray<TSubclassOf<AActor>>& InBaseBPs,
    const TArray<UStaticMesh*>& InCornerBaseMeshes,
    const TArray<TSubclassOf<AActor>>& InCornerBaseBPs,
    const TArray<UStaticMesh*>& InDoorMeshes,
    const TArray<TSubclassOf<AActor>>& InDoorBPs);

  UFUNCTION(BlueprintCallable, Category="Procedural Building|Conversion")
  void SetBodyMeshes(
    const TArray<UStaticMesh*>& InBodyMeshes,
    const TArray<TSubclassOf<AActor>>& InBodyBPs,
    const TArray<UStaticMesh*>& InCornerBodyMeshes,
    const TArray<TSubclassOf<AActor>>& InCornerBodyBPs,
    const TArray<UStaticMesh*>& InWallMeshes,
    const TArray<TSubclassOf<AActor>>& InWallBPs);

  UFUNCTION(BlueprintCallable, Category="Procedural Building|Conversion")
  void SetTopMeshes(
    const TArray<UStaticMesh*>& InTopMeshes,
    const TArray<TSubclassOf<AActor>>& InTopBPs,
    const TArray<UStaticMesh*>& InCornerTopMeshes,
    const TArray<TSubclassOf<AActor>>& InCornerTopBPs,
    const TArray<UStaticMesh*>& InRoofMeshes,
    const TArray<TSubclassOf<AActor>>& InRoofBPs);

protected:

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Procedural Building")
  void CreateBuilding();

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Procedural Building")
  void Reset();

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Procedural Building")
  void FixMobility();

  // TODO: AdvancedDisplay
  // Map containing the pair with the name of the mesh and the component that uses it
  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Procedural Building|Debug")
  TMap<FString, UHierarchicalInstancedStaticMeshComponent*> HISMComps;

  // Contains all the ChildActorComps spawned for this Actor
  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Procedural Building|Debug")
  TArray<UChildActorComponent*> ChildActorComps;

  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Procedural Building")
  UStaticMeshComponent* RootSMComp = nullptr;

  /**
   *  Base Parameters
   */

  // Set to contain the index of the doors that need to be placed on the base floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters", meta=(ToolTip="Index to indicate where the door is placed"))
  TSet<int> DoorsIndexPosition;

  // Indicates if the wall has to be filled with the wall-mesh or the body-mesh
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters", meta=(ToolTip="True to use wall-mesh instead of body-mesh"))
  TArray<bool> UseWallMesh;

  // Number of floors for the building, base and roof not included
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters",  meta=(ClampMin="0", UIMin="0", ToolTip="Number of floors for the building, base and roof not included"))
  int NumFloors = 0;

  // Number of elements in X Axis
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters")
  int LengthX = 0;

  // Number of elements in Y Axis
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters")
  int LengthY = 0;

  // Enables the corners of the building
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters")
  bool Corners = false;

  // Indicates if the meshes that are used on this buildings fill the whole floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters")
  bool UseFullBlocks = false;

  /**
   *  Base Parameters | Visibility
   */

  // True to render the side of the building. Each index represents the side
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters|Visibility")
  TArray<bool> SideVisibility;

  // True to render the corner of the building. Each index represents the corner
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters|Visibility")
  TArray<bool> CornerVisibility;

  // True to render the roof
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Base Parameters|Visibility")
  bool RoofVisibility = true;


  /**
   *  Meshes
   */

  /**
   *  Meshes | Base
   */

  // Pool of meshes to be randomly selected for the base floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<UStaticMesh*> BaseMeshes;

  // Pool of BPs to be randomly selected for the base floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<TSubclassOf<AActor>> BaseBPs;

  // Pool of meshes to be randomly selected for doors
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<UStaticMesh*> DoorMeshes;

  // Pool of BPs to be randomly selected for doors
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<TSubclassOf<AActor>> DoorBPs;

  // Pool of meshes to be randomly selected for the corners of the base floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<UStaticMesh*> CornerBaseMeshes;

  // Pool of BPs to be randomly selected for the corners of the base floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Base")
  TArray<TSubclassOf<AActor>> CornerBaseBPs;

  /**
   *  Meshes | Body
   */

  // Pool of meshes to be randomly selected for the body
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<UStaticMesh*> BodyMeshes;

  // Pool of BPs to be randomly selected for the body
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<TSubclassOf<AActor>> BodyBPs;

  // Pool of meshes to be randomly selected for the walls of the body
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<UStaticMesh*> WallMeshes;

  // Pool of BPs to be randomly selected for the body walls of the body
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<TSubclassOf<AActor>> WallBPs;

  // Pool of meshes to be randomly selected for the corners of the body floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<UStaticMesh*> CornerBodyMeshes;

  // Pool of BPs to be randomly selected for the corners of the body floor
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Body")
  TArray<TSubclassOf<AActor>> CornerBodyBPs;

  /**
   *  Meshes | Top
   */

  // Pool of meshes to be randomly selected for the top
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<UStaticMesh*> TopMeshes;

  // Pool of BPs to be randomly selected for the top
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<TSubclassOf<AActor>> TopBPs;

  // Pool of meshes to be randomly selected for the corners of the top
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<UStaticMesh*> CornerTopMeshes;

  // Pool of BPs to be randomly selected for the corners of the top
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<TSubclassOf<AActor>> CornerTopBPs;

  // Pool of meshes to be randomly selected for the roof
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<UStaticMesh*> RoofMeshes;

  // Pool of BPs to be randomly selected for the roof
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Procedural Building|Meshes|Top")
  TArray<TSubclassOf<AActor>> RoofBPs;

private:

  void Init();

  void CreateFloor(const FloorMeshCollection& MeshCollection, bool IncludeDoors, bool IncludeWalls);

  void CreateRoof();

  // Creates the side of a floor of the building and returns the maximun height of the side
  float CreateSide(
    const FloorMeshCollection& MeshCollection,
    const TSet<int>& AuxiliarPositions,
    int SideLength,
    bool MainVisibility,
    bool CornerVisbility);

  void CalculateSidesLength();

  // Calculates the doors of DoorsIndexPosition inside the range between [StartIndex, StartIndex + Length)
  // The set returns the position (index) of the doors relative to the side, ie: [0, Length)
  TSet<int> CalculateDoorsIndexInSide(int StartIndex, int Length);

  //
  TSet<int> GenerateWallsIndexPositions(int Length);

  // Check if the main and corner part of the side is visible
  void CalculateSideVisibilities(int SideIndex, bool& MainVisibility, bool& CornerVisbility);

  // Choose randomly between the Mesh and BP containers, only one option will be returned.
  void ChooseGeometryToSpawn(
    const TArray<UStaticMesh*>& InMeshes,
    const TArray<TSubclassOf<AActor>>& InMainBPs,
    UStaticMesh** OutMesh,
    TSubclassOf<AActor>* OutBP);

  // Add one part on the side
  float AddChunck(
    const UStaticMesh* SelectedMesh,
    const TSubclassOf<AActor> SelectedBP,
    bool Visible,
    FBox& OutSelectedMeshBounds);

  // Add the Static Mesh on the transform location with the transform orientation
  void AddMeshToBuilding(const UStaticMesh* SM);

  // Calculate the Bounds for the Static Mesh
  FVector GetMeshSize(const UStaticMesh* SM);

  void UpdateTransformPositionToNextChunk(const FVector& Box);

  void UpdateTransformPositionToNextSide(const FBox& Box);

  // The lengths of each side of the building. The length of the array is the number of sides
  // Helper to iterate throught all the sides of the building
  TArray<int> SidesLength;

  // Transform used to spawn the meshes of the building. It is updated (moved) with each new mesh
  FTransform CurrentTransform = FTransform::Identity;

  FBox LastSelectedMeshBounds;

};
