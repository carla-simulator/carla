// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ProceduralBuilding.h"


// Sets default values
AProceduralBuilding::AProceduralBuilding()
{
   // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  SideVisibility.Init(true, 4);
  CornerVisibility.Init(true, 4);
  UseWallMesh.Init(false, 4);

  RootSMComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
  RootComponent = RootSMComp;
  RootComponent->SetMobility(EComponentMobility::Static);
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
  HISMComp->SetupAttachment(RootComponent);
  HISMComp->RegisterComponent();

  // Set the mesh that will be used
  HISMComp->SetStaticMesh(const_cast<UStaticMesh*>(SM));

  // Add to the map
  HISMComps.Emplace(SMName, HISMComp);

  return HISMComp;
}

void AProceduralBuilding::FixMobility() {

  for (UChildActorComponent *ChildComp : ChildActorComps) {

    ChildComp->SetMobility(EComponentMobility::Type::Static);
  }
}

void AProceduralBuilding::ConvertOldBP_ToNativeCodeObject(AActor* BP_Building)
{
  AProceduralBuilding* ProceduralBuilding = nullptr;

  // Look for all the HISMComps
  TArray<UHierarchicalInstancedStaticMeshComponent*> OtherHISMComps;
  BP_Building->GetComponents<UHierarchicalInstancedStaticMeshComponent>(OtherHISMComps);

  for(UHierarchicalInstancedStaticMeshComponent* OtherHISMComp : OtherHISMComps)
  {
    const UStaticMesh* SM = OtherHISMComp->GetStaticMesh();

    // Create a new HISMComp and set the SM
    UHierarchicalInstancedStaticMeshComponent* NewHISMComp = GetHISMComp(SM);

    // Create the instances
    const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData =  OtherHISMComp->PerInstanceSMData;

    for(const FInstancedStaticMeshInstanceData& InstSMIData : PerInstanceSMData)
    {
      FTransform Transform = FTransform(InstSMIData.Transform);

      NewHISMComp->AddInstance(Transform);
    }
  }

  // TODO: Look for all ChildActors -> Add new Child
  TArray<UChildActorComponent*> OtherChildComps;
  BP_Building->GetComponents<UChildActorComponent>(OtherChildComps);

  for(const UChildActorComponent* OtherChildActor : OtherChildComps)
  {
    // Create a new ChildActorComponent
    UChildActorComponent* ChildActorComp = NewObject<UChildActorComponent>(this,
      FName(*FString::Printf(TEXT("ChildActorComp_%d"), ChildActorComps.Num() )));
    ChildActorComp->SetMobility(EComponentMobility::Type::Static);
    ChildActorComp->SetupAttachment(RootComponent);

    // Set the class that it will use
    ChildActorComp->SetChildActorClass(OtherChildActor->GetChildActorClass());
    ChildActorComp->SetRelativeTransform(OtherChildActor->GetRelativeTransform());

    // Spawns the actor referenced by UChildActorComponent
    ChildActorComp->RegisterComponent();

    AActor* NewChildActor = ChildActorComp->GetChildActor();

#if WITH_EDITOR
    // Add the child actor to a subfolder of the actor's name
    NewChildActor->SetFolderPath(FName( *FString::Printf(TEXT("/Buildings/%s"), *GetName())));
 #endif

    // Look for all the SMComps
    TArray<UStaticMeshComponent*> NewSMComps;
    NewChildActor->GetComponents<UStaticMeshComponent>(NewSMComps);

    // Make it invisible on the child actor to avoid duplication with the HISMComp
    UStaticMeshComponent* PivotSMComp = NewSMComps[0];
    PivotSMComp->SetVisibility(false, false);

    ChildActorComps.Emplace(ChildActorComp);

  }
}

void AProceduralBuilding::HideAllChildren()
{
  for(UChildActorComponent* ChildActorComp : ChildActorComps)
  {
    AActor* ChildActor = ChildActorComp->GetChildActor();
    TArray<UStaticMeshComponent*> SMComps;
    ChildActor->GetComponents<UStaticMeshComponent>(SMComps);
    if(SMComps.Num() > 0)
    {
      SMComps[0]->SetVisibility(false, false);
    }
  }
}

void AProceduralBuilding::SetBaseParameters(
  const TSet<int>& InDoorsIndexPosition,
  const TArray<bool>& InUseWallMesh,
  int InNumFloors,
  int InLengthX,
  int InLengthY,
  bool InCorners,
  bool InUseFullBlocks)
{
  DoorsIndexPosition = InDoorsIndexPosition;
  UseWallMesh = InUseWallMesh;
  NumFloors = InNumFloors;
  LengthX = InLengthX;
  LengthY = InLengthY;
  Corners = InCorners;
  UseFullBlocks = InUseFullBlocks;
}

void AProceduralBuilding::SetVisibilityParameters(
  const TArray<bool>& InSideVisibility,
  const TArray<bool>& InCornerVisibility,
  bool InRoofVisibility)
{
  SideVisibility = InSideVisibility;
  CornerVisibility = InCornerVisibility;
  RoofVisibility = InRoofVisibility;
}

void AProceduralBuilding::SetBaseMeshes(
  const TArray<UStaticMesh*>& InBaseMeshes,
  const TArray<TSubclassOf<AActor>>& InBaseBPs,
  const TArray<UStaticMesh*>& InCornerBaseMeshes,
  const TArray<TSubclassOf<AActor>>& InCornerBaseBPs,
  const TArray<UStaticMesh*>& InDoorMeshes,
  const TArray<TSubclassOf<AActor>>& InDoorBPs)
{
  BaseMeshes = InBaseMeshes;
  BaseBPs = InBaseBPs;
  CornerBaseMeshes = InCornerBaseMeshes;
  CornerBaseBPs = InCornerBaseBPs;
  DoorMeshes = InDoorMeshes;
  DoorBPs = InDoorBPs;
}

void AProceduralBuilding::SetBodyMeshes(
  const TArray<UStaticMesh*>& InBodyMeshes,
  const TArray<TSubclassOf<AActor>>& InBodyBPs,
  const TArray<UStaticMesh*>& InCornerBodyMeshes,
  const TArray<TSubclassOf<AActor>>& InCornerBodyBPs,
  const TArray<UStaticMesh*>& InWallMeshes,
  const TArray<TSubclassOf<AActor>>& InWallBPs)
{
  BodyMeshes = InBodyMeshes;
  BodyBPs = InBodyBPs;
  CornerBodyMeshes = InCornerBodyMeshes;
  CornerBodyBPs = InCornerBodyBPs;
  WallMeshes = InWallMeshes;
  WallBPs = InWallBPs;
}

void AProceduralBuilding::SetTopMeshes(
  const TArray<UStaticMesh*>& InTopMeshes,
  const TArray<TSubclassOf<AActor>>& InTopBPs,
  const TArray<UStaticMesh*>& InCornerTopMeshes,
  const TArray<TSubclassOf<AActor>>& InCornerTopBPs,
  const TArray<UStaticMesh*>& InRoofMeshes,
  const TArray<TSubclassOf<AActor>>& InRoofBPs)
{
  TopMeshes = InTopMeshes;
  TopBPs = InTopBPs;
  CornerTopMeshes = InCornerTopMeshes;
  CornerTopBPs = InCornerTopBPs;
  RoofMeshes = InRoofMeshes;
  RoofBPs = InRoofBPs;
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
  const FloorMeshCollection BodyMeshCollection =
      { &BodyMeshes, &BodyBPs, &CornerBodyMeshes, &CornerBodyBPs, &WallMeshes, &WallBPs };
  for(int i = 0; i < NumFloors; i++)
  {
    CreateFloor(BodyMeshCollection, false, true);
  }

  // Top floor
  CreateFloor(
    { &TopMeshes, &TopBPs, &CornerTopMeshes, &CornerTopBPs },
    false,
    false);

  // Roof
  CreateRoof();

}

void AProceduralBuilding::Reset()
{
  CurrentTransform = FTransform::Identity;

  // Discard previous calculation
  SidesLength.Reset();

  const TSet <UActorComponent*> Comps = GetComponents();

  // Remove all the instances of each HISMComp
  for(auto& It : HISMComps)
  {
    const FString& MeshName = It.Key;
    UHierarchicalInstancedStaticMeshComponent* HISMComp = It.Value;

    HISMComp->ClearInstances();
  }
  // Empties out the map but preserves all allocations and capacities
  HISMComps.Reset();

  // Remove all child actors
  for(UChildActorComponent* ChildActorComp : ChildActorComps)
  {
    if(ChildActorComp)
    {
      ChildActorComp->DestroyComponent();
    }
  }
  ChildActorComps.Reset();

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
    TSet<int> AuxiliarPositions;
    int SideLength = SidesLength[i];
    bool MainVisibility = true;
    bool CornerVisbility = true;

    if (IncludeDoors)
    {
      AuxiliarPositions = CalculateDoorsIndexInSide(SideLengthAcumulator, SideLength);
    }
    if(IncludeWalls && UseWallMesh[i])
    {
      AuxiliarPositions = GenerateWallsIndexPositions(SideLength);
    }

    CalculateSideVisibilities(i, MainVisibility, CornerVisbility);

    // Update Max Z
    float SideMaxZ = CreateSide(MeshCollection, AuxiliarPositions, SideLength, MainVisibility, CornerVisbility);
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

void AProceduralBuilding::CreateRoof()
{
  UStaticMesh* SelectedMesh = nullptr;
  TSubclassOf<AActor> SelectedBP = nullptr;
  FBox SelectedMeshBounds;

  bool AreRoofMeshesAvailable = (RoofMeshes.Num() > 0) || (RoofBPs.Num() > 0);

  // Hack for top meshes. Perhaps the top part has a little part of the roof
  FVector BoxSize = LastSelectedMeshBounds.GetSize();
  BoxSize = FVector(0.0f, -BoxSize.Y, 0.0f);

  CurrentTransform.SetTranslation(CurrentTransform.GetTranslation() + BoxSize);

  if(AreRoofMeshesAvailable)
  {

    for(int i = 0; i < LengthY; i++)
    {
      FVector PivotLocation = CurrentTransform.GetTranslation();
      for(int j = 0; j < LengthX; j++)
      {
        // Choose a roof mesh
        ChooseGeometryToSpawn(RoofMeshes, RoofBPs, &SelectedMesh, &SelectedBP);

        AddChunck(SelectedMesh, SelectedBP, RoofVisibility, SelectedMeshBounds);

      }
      // Move the Transform location to the beginning of the next row
      CurrentTransform.SetTranslation(PivotLocation);
      UpdateTransformPositionToNextSide(SelectedMeshBounds);
    }
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
  if(Corners && AreCornerMeshesAvailable)
  {
    // Choose a corner mesh
    ChooseGeometryToSpawn(*CornerMeshes, *CornerBPs, &SelectedMesh, &SelectedBP);
    float ChunkZ = AddChunck(SelectedMesh, SelectedBP, CornerVisbility, SelectedMeshBounds);
    MaxZ = (MaxZ < ChunkZ) ? ChunkZ : MaxZ;

    // Move the Transform location to the next side of the building
    // because corners can be in two sides
    UpdateTransformPositionToNextSide(SelectedMeshBounds);
  }

  LastSelectedMeshBounds = SelectedMeshBounds;

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
    SidesLength.Emplace(LengthX);
    SidesLength.Emplace(LengthY);
    SidesLength.Emplace(LengthX);
    SidesLength.Emplace(LengthY);
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

TSet<int> AProceduralBuilding::GenerateWallsIndexPositions(int Length)
{
  TSet<int> Result;
  for(int i = 0; i < Length; i++)
  {
    Result.Emplace(i);
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

  // Static Mesh
  if(SelectedMesh)
  {
    if(Visible)
    {
      AddMeshToBuilding(SelectedMesh);
    }
    FVector MeshBound = GetMeshSize(SelectedMesh);
    Result = MeshBound.Z;

    UpdateTransformPositionToNextChunk(MeshBound);
    OutSelectedMeshBounds = SelectedMesh->GetBoundingBox();
  }
  // BP
  else if(SelectedBP)
  {
    // Create a new ChildActorComponent
    UChildActorComponent* ChildActorComp = NewObject<UChildActorComponent>(this,
      FName(*FString::Printf(TEXT("ChildActorComp_%d"), ChildActorComps.Num() )));
    ChildActorComp->SetMobility(EComponentMobility::Type::Static);
    ChildActorComp->SetupAttachment(RootComponent);

    // Set the class that it will use
    ChildActorComp->SetChildActorClass(SelectedBP);
    ChildActorComp->SetRelativeTransform(CurrentTransform);

    // Spawns the actor referenced by UChildActorComponent
    ChildActorComp->RegisterComponent();

    AActor* ChildActor = ChildActorComp->GetChildActor();

#if WITH_EDITOR
    // Add the child actor to a subfolder of the actor's name
    ChildActor->SetFolderPath(FName( *FString::Printf(TEXT("/Buildings/%s"), *GetName())));
 #endif

    // Look for all the SMComps
    TArray<UStaticMeshComponent*> SMComps;
    UStaticMeshComponent* PivotSMComp = nullptr;

    ChildActor->GetComponents<UStaticMeshComponent>(SMComps);

    // The first mesh on the BP is the pivot to continue creating the floor
    PivotSMComp = SMComps[0];
    const UStaticMesh* SM = PivotSMComp->GetStaticMesh();

    if(Visible)
    {
      ChildActorComps.Emplace(ChildActorComp);
      AddMeshToBuilding(SM);
    }
    else
    {
      ChildActorComp->DestroyComponent();
    }

    FVector MeshBound = GetMeshSize(SM);
    Result = MeshBound.Z;

    UpdateTransformPositionToNextChunk(MeshBound);

    // Make it invisible on the child actor to avoid duplication with the HISMComp
    PivotSMComp->SetVisibility(false, false);
    OutSelectedMeshBounds = SM->GetBoundingBox();

  }

  return Result;
}

void AProceduralBuilding::AddMeshToBuilding(const UStaticMesh* SM)
{

  UHierarchicalInstancedStaticMeshComponent* HISMComp = GetHISMComp(SM);
  HISMComp->AddInstance(CurrentTransform);
}

FVector AProceduralBuilding::GetMeshSize(const UStaticMesh* SM)
{
  FBox Box = SM->GetBoundingBox();
  return Box.GetSize();
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
    FVector Size = Box.GetSize();

    FVector NewLocation = CurrentTransform.GetTranslation() + RightVector * Size.Y;
    CurrentTransform.SetTranslation(NewLocation);
  }
}
