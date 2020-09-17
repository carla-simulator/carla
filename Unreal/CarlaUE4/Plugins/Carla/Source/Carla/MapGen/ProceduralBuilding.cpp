// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuilding.h"


// Sets default values
AProceduralBuilding::AProceduralBuilding()
{
   // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  SideVisibility.Init(true, 4);
  CornerVisibility.Init(true, 4);
}

// Called when the game starts or when spawned
void AProceduralBuilding::BeginPlay()
{
  Super::BeginPlay();

}

void AProceduralBuilding::CreateBuilding()
{
  Init();

}

void AProceduralBuilding::Reset()
{
  // Discard previous calculation
  SidesLength.Reset();

  // Remove all the instances of each HISMComp
  for(auto& It : HISMComps)
  {
    const FString& MeshName = It.Key;
    UHierarchicalInstancedStaticMeshComponent* HISMComp = It.Value;

    HISMComp->ClearInstances();
  }
  // Empties out the map but preserves all allocations and capacities
  HISMComps.Reset();

}

void AProceduralBuilding::Init()
{
  CalculateSidesLength();
}

void AProceduralBuilding::CreateFloor(
    const FloorMeshCollection& MeshCollection,
    bool IncludeDoors,
    bool IncludeWalls)
{



  // Stores the total length covered. This is needed to place the doors.
  int SideLengthAcumulator = 0;

  for(int i = 0; i < SidesLength.Num(); i++)
  {
    TSet<int> DoorsPosition;
    int SideLength = SidesLength[i];
    bool MainVisibility = true;
    bool CornerVisbility = true;

    if (IncludeDoors)
    {
      DoorsPosition = CalculateDoorsIndexInSide(SideLengthAcumulator, SideLength);
    }

    CalculateSideVisibilities(i, MainVisibility, CornerVisbility);

    CreateSide(MeshCollection, DoorsPosition, SideLength, MainVisibility, CornerVisbility);

    SideLengthAcumulator += SidesLength;
  }
}


float AProceduralBuilding::CreateSide(
    const FloorMeshCollection& MeshCollection,
    const TSet<int>& AuxiliarPositions,
    int SideLength,
    bool MainVisibility,
    bool CornerVisbility)
{
  const TArray<UStaticMesh*>* MainMeshes = MeshCollection.MainMeshes;
  const TArray<UClass*>* MainBPs = MeshCollection.MainBPs;
  const TArray<UStaticMesh*>* CornerMeshes = MeshCollection.CornerMeshes;
  const TArray<UClass*>* CornerBPs = MeshCollection.CornerBPs;
  const TArray<UStaticMesh*>* AuxiliarMeshes = MeshCollection.AuxiliarMeshes;
  const TArray<UClass*>* AuxiliarBPs = MeshCollection.AuxiliarBPs;

  /**
   * Main part
   */

  // Check to know if there are meshes for the main part available
  bool AreMainMeshesAvailable = (MainMeshes && (MainMeshes->Num() > 0)) || (MainBPs && (MainBPs->Num() > 0);
  bool AreAuxMeshesAvailable = (MainMeshes && (MainMeshes->Num() > 0)) || (MainBPs && (MainBPs->Num() > 0);
  for(int i = 0; (i < SideLength) && AreMainMeshesAvailable; i++)
  {
    UStaticMesh* SelectedMesh = nullptr;
    UClass* SelectedBP = nullptr;
    int* AuxiliarPosition = AuxiliarPositions.Find(i);
    if(AreAuxMeshesAvailable && AuxiliarPosition)
    {
      // Add an auxiliar mesh
      ChooseGeometryToSpawn(AuxiliarMeshes, AuxiliarBPs, SelectedMesh, SelectedBP);
    }
    else
    {
      // Add a main mesh
      ChooseGeometryToSpawn(MainMeshes, MainBPs, SelectedMesh, SelectedBP);
    }

    float MaxZ = AddGeometry(SelectedMesh, SelectedBP, MainVisibility);




  }



  return 0.0f;
}

void AProceduralBuilding::CalculateSidesLength()
{
  // Discard previous calculation
  SidesLength.Reset();

  if(UseFullBlocks)
  {
    // The full block configuration covers all the sides of the floor with one mesh
    SidesLength.Emplace(1);
  }
  else
  {
    SidesLength.Emplace(LengthY);
    SidesLength.Emplace(LengthX);
    SidesLength.Emplace(LengthY);
    SidesLength.Emplace(LengthX);
  }

}

TSet<int> AProceduralBuilding::CalculateDoorsIndexInSide(int StartIndex, int Length)
{
  TSet<int> Result;
  int MaxIndex = StartIndex + Length;

  for(int i : DoorsIndexPosition)
  {
    if( StartIndex <= i && i < MaxIndex )
    {
      int RelativePostion = i - StartIndex;
      Result.Emplace(RelativePostion);

  }
  return Result
}

void AProceduralBuilding::CalculateSideVisibilities(int SideIndex, bool& MainVisibility, bool& CornerVisbility)
{
  MainVisibility = UseFullBlocks || SideVisibility[SideIndex];
  CornerVisbility = UseFullBlocks || CornerVisibility[SideIndex]
}

void AProceduralBuilding::ChooseGeometryToSpawn(
    const TArray<UStaticMesh*>* InMeshes,
    const TArray<UClass*>* InBPs,
    UStaticMesh* OutMesh = nullptr,
    UClass* OutBP = nullptr)
{
  int NumMeshes = InMeshes.Num();
  int NumBPs = InBPs.Num();
  int Range = NumMeshes + NumBPs;

  int Choosen = FMath::RandRange(0, Range - 1);

  if(Choosen < NumMeshes)
  {
    OutMesh = InMeshes[Choosen];
  }
  if(NumMeshes <= Choosen && Choosen < NumBPs)
  {
    OutBP = InBPs[Choosen - NumMeshes];
  }
}

float AProceduralBuilding::AddGeometry(
    const UStaticMesh* SelectedMesh,
    const UClass* SelectedBP,
    bool Visible)
{
  if(SelectedMesh)
  {
    if(Visible)
    {
      // AddMeshToBuilding
    }
    // Update Current Transform

  }
}