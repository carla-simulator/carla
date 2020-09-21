// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuilding.h"


// Sets default values
AProceduralBuilding::AProceduralBuilding()
{
   // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  SideVisibility.Init(true, 4);
  CornerVisibility.Init(true, 4);

  USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = SceneComponent;

}

void AProceduralBuilding::CreateBuilding()
{
  Init();

  // Base Floor
  CreateFloor(
    { &BaseMeshes, &BaseBPs, &CornerBaseMeshes, &CornerBaseBPs, &DoorMeshes, &DoorBPs },
    true,
    false);

  // Body floors

  // Top floor

  // Roof

}

void AProceduralBuilding::Reset()
{
  CurrentTransform = FTransform::Identity;

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
  Reset();

  CalculateSidesLength();
}

void AProceduralBuilding::CreateFloor(
    const FloorMeshCollection& MeshCollection,
    bool IncludeDoors,
    bool IncludeWalls)
{
  float MaxZ = 0.0f;

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

    // Update Max Z
    float SideMaxZ = CreateSide(MeshCollection, DoorsPosition, SideLength, MainVisibility, CornerVisbility);
    MaxZ = (MaxZ < SideMaxZ) ? SideMaxZ : MaxZ;

    // Update the acumulator to calculate doors index in next sides
    SideLengthAcumulator += SideLength;

    // Update transform rotation for the next side
    if(!UseFullBlocks)
    {
      const FQuat RotationToAdd = FRotator(0.0f, 90.0f, 0.0f).Quaternion();
      CurrentTransform.ConcatenateRotation(RotationToAdd);
    }
  }

  // Update transform for the next floor
  FVector NewLocation = CurrentTransform.GetTranslation() + FVector(0.0f, 0.0f, MaxZ);
  CurrentTransform.SetTranslation(NewLocation);

}


float AProceduralBuilding::CreateSide(
    const FloorMeshCollection& MeshCollection,
    const TSet<int>& AuxiliarPositions,
    int SideLength,
    bool MainVisibility,
    bool CornerVisbility)
{

  UE_LOG(LogCarla, Warning, TEXT("AProceduralBuilding::CreateSide of length %d"), SideLength);

  const TArray<UStaticMesh*>* MainMeshes = MeshCollection.MainMeshes;
  const TArray<TSubclassOf<AActor>>* MainBPs = MeshCollection.MainBPs;
  const TArray<UStaticMesh*>* CornerMeshes = MeshCollection.CornerMeshes;
  const TArray<TSubclassOf<AActor>>* CornerBPs = MeshCollection.CornerBPs;
  const TArray<UStaticMesh*>* AuxiliarMeshes = MeshCollection.AuxiliarMeshes;
  const TArray<TSubclassOf<AActor>>* AuxiliarBPs = MeshCollection.AuxiliarBPs;

  /**
   * Main part
   */

  UStaticMesh* SelectedMesh = nullptr;
  TSubclassOf<AActor> SelectedBP = nullptr;
  FBox SelectedMeshBounds;
  float MaxZ = 0.0f;

  // Check to know if there are meshes for the main part available
  bool AreMainMeshesAvailable = (MainMeshes && (MainMeshes->Num() > 0)) || (MainBPs && (MainBPs->Num() > 0));
  bool AreAuxMeshesAvailable = (MainMeshes && (MainMeshes->Num() > 0)) || (MainBPs && (MainBPs->Num() > 0));

  for(int i = 0; (i < SideLength) && AreMainMeshesAvailable; i++)
  {
    const int* AuxiliarPosition = AuxiliarPositions.Find(i);
    if(AreAuxMeshesAvailable && AuxiliarPosition)
    {
      // Choose an auxiliar mesh
      ChooseGeometryToSpawn(*AuxiliarMeshes, *AuxiliarBPs, &SelectedMesh, &SelectedBP);
    }
    else
    {
      // Choose a main mesh
      ChooseGeometryToSpawn(*MainMeshes, *MainBPs, &SelectedMesh, &SelectedBP);
    }

    float ChunkZ = AddChunck(SelectedMesh, SelectedBP, MainVisibility, SelectedMeshBounds);
    MaxZ = (MaxZ < ChunkZ) ? ChunkZ : MaxZ;
  }

  /**
   * Corner part
   */
  bool AreCornerMeshesAvailable = (CornerMeshes && (CornerMeshes->Num() > 0)) || (CornerBPs && (CornerBPs->Num() > 0));
  if(AreCornerMeshesAvailable)
  {
    // Choose a corner mesh
    ChooseGeometryToSpawn(*CornerMeshes, *CornerBPs, &SelectedMesh, &SelectedBP);
    float ChunkZ = AddChunck(SelectedMesh, SelectedBP, MainVisibility, SelectedMeshBounds);
    MaxZ = (MaxZ < ChunkZ) ? ChunkZ : MaxZ;

    // Move the Transform location to the next side of the building
    // because corners can be in two sides
    UpdateTransformPositionToNextSide(SelectedMeshBounds);
  }

  return MaxZ;
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
  }
  return Result;
}

void AProceduralBuilding::CalculateSideVisibilities(int SideIndex, bool& MainVisibility, bool& CornerVisbility)
{
  MainVisibility = UseFullBlocks || SideVisibility[SideIndex];
  CornerVisbility = UseFullBlocks || CornerVisibility[SideIndex];
}

void AProceduralBuilding::ChooseGeometryToSpawn(
    const TArray<UStaticMesh*>& InMeshes,
    const TArray<TSubclassOf<AActor>>& InBPs,
    UStaticMesh** OutMesh = nullptr,
    TSubclassOf<AActor>* OutBP = nullptr)
{
  int NumMeshes = InMeshes.Num();
  int NumBPs = InBPs.Num();
  int Range = NumMeshes + NumBPs;

  int Choosen = FMath::RandRange(0, Range - 1);

  if(Choosen < NumMeshes)
  {
    *OutMesh = InMeshes[Choosen];
  }
  if(NumMeshes <= Choosen && Choosen < NumBPs)
  {
    *OutBP = InBPs[Choosen - NumMeshes];
  }
}

float AProceduralBuilding::AddChunck(
    const UStaticMesh* SelectedMesh,
    const TSubclassOf<AActor> SelectedBP,
    bool Visible,
    FBox& OutSelectedMeshBounds)
{
  float Result = 0.0f;

  UE_LOG(LogCarla, Warning, TEXT("AddChunck"));

  // Static Mesh
  if(SelectedMesh)
  {
    if(Visible)
    {
      AddMeshToBuilding(SelectedMesh);
    }
    FVector MeshBound = GetMeshBound(SelectedMesh);
    Result = MeshBound.Z;

    UpdateTransformPositionToNextChunk(MeshBound);
    OutSelectedMeshBounds = SelectedMesh->GetBoundingBox();
  }
  // BP
  else if(SelectedBP)
  {

    UE_LOG(LogCarla, Warning, TEXT("AddChunck BP"));

    // Create a new ChildActorComponent
    UChildActorComponent* ChildActorComp = NewObject<UChildActorComponent>(this,
      FName(*FString::Printf(TEXT("ChildActorComp_%d"), ChildActorComps.Num())));
    ChildActorComp->SetupAttachment(GetRootComponent());
    ChildActorComp->RegisterComponent();

    // Set the class that it will use
    ChildActorComp->SetChildActorClass(SelectedBP);

    // Look for all the SMComps
    TArray<UStaticMeshComponent*> SMComps;
    UStaticMeshComponent* PivotSMComp = nullptr;

    GetComponents<UStaticMeshComponent>(SMComps);

    int NumSMs = SMComps.Num();

    if(Visible && NumSMs > 0)
    {
      ChildActorComps.Emplace(ChildActorComp);

      // The first mesh on the BP is the pivot to continue creating the floor
      PivotSMComp = SMComps[0];
      const UStaticMesh* SM = PivotSMComp->GetStaticMesh();

      AddMeshToBuilding(SM);
      FVector MeshBound = GetMeshBound(SM);
      Result = MeshBound.Z;

      UpdateTransformPositionToNextChunk(MeshBound);

      // Make it invisible on the child actor to avoid duplication with the HISMComp
      PivotSMComp->SetVisibility(false, false);
      OutSelectedMeshBounds = SM->GetBoundingBox();
    }
    else
    {
      ChildActorComp->DestroyComponent();
    }

    /*
    // TODO: add the other meshes to a HISMComp
    for(int i = 1; i < NumSMs; i++)
    {
      UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(SMComps[i]);
      const UStaticMesh* SM = SMComp->GetStaticMesh();
    }
    */
  }

  return Result;
}

void AProceduralBuilding::AddMeshToBuilding(const UStaticMesh* SM)
{

  UHierarchicalInstancedStaticMeshComponent* HISMComp = GetHISMComp(SM);
  HISMComp->AddInstance(CurrentTransform);
}

UHierarchicalInstancedStaticMeshComponent* AProceduralBuilding::GetHISMComp(
    const UStaticMesh* SM)
{

  FString SMName = SM->GetName();

  UHierarchicalInstancedStaticMeshComponent** HISMCompPtr = HISMComps.Find(SMName);

  if(HISMCompPtr) return *HISMCompPtr;

  UHierarchicalInstancedStaticMeshComponent* HISMComp = *HISMCompPtr;

  // If it doesn't exist, create the component
  HISMComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this,
    FName(*FString::Printf(TEXT("HISMComp_%d"), HISMComps.Num())));
  HISMComp->SetupAttachment(GetRootComponent());
  HISMComp->RegisterComponent();

  // Set the mesh that will be used
  HISMComp->SetStaticMesh(const_cast<UStaticMesh*>(SM));

  // Add to the map
  HISMComps.Emplace(SMName, HISMComp);

  return HISMComp;
}

FVector AProceduralBuilding::GetMeshBound(const UStaticMesh* SM)
{
  FBox Box = SM->GetBoundingBox();
  return (Box.Max - Box.Min);
}

void AProceduralBuilding::UpdateTransformPositionToNextChunk(const FVector& Box)
{
  // Update Current Transform to the right side of the added chunk
  // Nothing to change if the chunk is the size of a floor
  if(!UseFullBlocks)
  {
    FQuat Rotation = CurrentTransform.GetRotation();
    FVector ForwardVector = -Rotation.GetForwardVector();

    FVector NewLocation = CurrentTransform.GetTranslation() + ForwardVector * Box.X;
    CurrentTransform.SetTranslation(NewLocation);
  }
}

void AProceduralBuilding::UpdateTransformPositionToNextSide(const FBox& Box)
{
  // Update Current Transform to the right side of the added chunk
  // Nothing to change if the chunk is the size of a floor
  if(!UseFullBlocks)
  {
    FQuat Rotation = CurrentTransform.GetRotation();
    FVector RightVector = -Rotation.GetRightVector();
    FVector Extent = (Box.Max - Box.Min);

    FVector NewLocation = CurrentTransform.GetTranslation() + RightVector * Extent.Y;
    CurrentTransform.SetTranslation(NewLocation);
  }
}






