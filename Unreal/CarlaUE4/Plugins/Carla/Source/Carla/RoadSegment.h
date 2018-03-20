// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "MapGen/StaticMeshCollection.h"
#include "RoadSegment.generated.h"

UENUM(BlueprintType)
enum class ELaneMarkingType : uint8
{
  None        UMETA(DisplayName = "None"),
  Solid       UMETA(DisplayName = "Solid Lane Marking"),
  Broken      UMETA(DisplayName = "Broken Lane Marking")
};

/// Description of a road segment piece.
USTRUCT(BlueprintType)
struct CARLA_API FRoadSegmentPiece
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bHasLeftSidewalk = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bHasRightSidewalk = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  ELaneMarkingType LaneMarking = ELaneMarkingType::Solid;
};

/// A straight segment of road.
///
/// Please call GenerateRoad after modifying it.
UCLASS()
class CARLA_API ARoadSegment : public AStaticMeshCollection
{
  GENERATED_BODY()

public:

  ARoadSegment(const FObjectInitializer& ObjectInitializer);

  virtual void OnConstruction(const FTransform &Transform) override;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

  UFUNCTION(BlueprintCallable, Category="Road Description")
  void GenerateRoad();

  UFUNCTION(BlueprintCallable, Category="Road Description")
  int32 GetNumberOfPieces() const
  {
    return RoadDescription.Num();
  }

  UFUNCTION(BlueprintCallable, Category="Road Description")
  void AppendPiece(const FRoadSegmentPiece &RoadSegmentPiece)
  {
    RoadDescription.Add(RoadSegmentPiece);
  }

  UFUNCTION(BlueprintCallable, Category="Road Description")
  void RemoveAllPieces()
  {
    RoadDescription.Empty();
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_LaneLeft(UStaticMesh *StaticMesh)
  {
    LaneLeft = StaticMesh;
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_LaneRight(UStaticMesh *StaticMesh)
  {
    LaneRight = StaticMesh;
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_SidewalkLeft(UStaticMesh *StaticMesh)
  {
    SidewalkLeft = StaticMesh;
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_SidewalkRight(UStaticMesh *StaticMesh)
  {
    SidewalkRight = StaticMesh;
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_LaneMarkingSolid(UStaticMesh *StaticMesh)
  {
    LaneMarkingSolid = StaticMesh;
  }

  UFUNCTION(BlueprintCallable, Category="Set Static Mesh")
  void SetStaticMesh_LaneMarkingBroken(UStaticMesh *StaticMesh)
  {
    LaneMarkingBroken = StaticMesh;
  }

private:

  void UpdateMeshes();

  void UpdateRoad();

  UPROPERTY(Category = "Road Description", EditAnywhere)
  TArray<FRoadSegmentPiece> RoadDescription;

  UPROPERTY(Category = "Road Description", AdvancedDisplay, EditAnywhere)
  float Scale = 1.0f;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *LaneLeft;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *LaneRight;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *SidewalkLeft;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *SidewalkRight;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *LaneMarkingSolid;

  UPROPERTY(Category = "Meshes", EditAnywhere)
  UStaticMesh *LaneMarkingBroken;
};
