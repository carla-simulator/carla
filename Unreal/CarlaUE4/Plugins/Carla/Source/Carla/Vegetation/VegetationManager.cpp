// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ProceduralFoliageVolume.h"
#include "ProceduralFoliageComponent.h"

#include "Carla/Vegetation/VegetationManager.h"
#include "Carla/Vegetation/SpringBasedVegetationComponent.h"
#include "Carla/Game/CarlaStatics.h"
#include "Game/TaggedComponent.h"

static FString GetVersionFromFString(const FString& String)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(GetVersionFromFString);
  auto IsDigit = [](TCHAR charToCheck) {
      if (charToCheck == TCHAR('0')) return true;
      if (charToCheck == TCHAR('1')) return true;
      if (charToCheck == TCHAR('2')) return true;
      if (charToCheck == TCHAR('3')) return true;
      if (charToCheck == TCHAR('4')) return true;
      if (charToCheck == TCHAR('5')) return true;
      if (charToCheck == TCHAR('6')) return true;
      if (charToCheck == TCHAR('7')) return true;
      if (charToCheck == TCHAR('8')) return true;
      if (charToCheck == TCHAR('9')) return true;
      return false;
  };
  int index = String.Find(TEXT("_v"));
  if (index != -1)
  {
    index += 2;
    FString Version = "_v";
    while(IsDigit(String[index]))
    {
      Version += String[index];
      ++index;
      if (index == String.Len())
        return Version;
    }
    return Version;
  }
  return FString();
}

FTileData::~FTileData()
{
}

/********************************************************************************/
/********** POOLED ACTOR STRUCT *************************************************/
/********************************************************************************/
void FPooledActor::EnableActor(const FTransform& Transform, int32 NewIndex, std::shared_ptr<FTileMeshComponent>& NewTileMeshComponent)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPooledActor::EnableActor);
  InUse = true;
  IsActive = false;
  GlobalTransform = Transform;
  Index = NewIndex;
  TileMeshComponent = NewTileMeshComponent;
  TileMeshComponent->IndicesInUse.Emplace(Index);

  Actor->SetActorHiddenInGame(false);
  Actor->SetActorTickEnabled(true);

  USpringBasedVegetationComponent* Component = Actor->FindComponentByClass<USpringBasedVegetationComponent>();
  if (Component)
  {
    Component->ResetComponent();
  }
}

void FPooledActor::ActiveActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPooledActor::ActiveActor);

  IsActive = true;
  Actor->SetActorEnableCollision(true);

  USpringBasedVegetationComponent* Component = Actor->FindComponentByClass<USpringBasedVegetationComponent>();
  if (Component)
  {
    Component->SetComponentTickEnabled(true);
  }

  USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh)
  {
    SkeletalMesh->bNoSkeletonUpdate = false;
  }
}

void FPooledActor::DisableActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPooledActor::DisableActor);

  if (TileMeshComponent)
  {
    if (TileMeshComponent->bIsAlive)
    {
      if (TileMeshComponent->IndicesInUse.Contains(Index))
      {
        TileMeshComponent->IndicesInUse.RemoveSingle(Index);
      }
    }
  }

  InUse = false;
  IsActive = false;
  Index = -1;
  TileMeshComponent = nullptr;

  Actor->SetActorEnableCollision(false);
  Actor->SetActorHiddenInGame(true);
  Actor->SetActorTickEnabled(false);

  USpringBasedVegetationComponent* Component = Actor->FindComponentByClass<USpringBasedVegetationComponent>();
  if (Component)
  {
    Component->SetComponentTickEnabled(false);
  }

  USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
  if (SkeletalMesh)
  {
    SkeletalMesh->bNoSkeletonUpdate = true;
  }
}

/********************************************************************************/
/********** FOLIAGE BLUEPRINT STRUCT ********************************************/
/********************************************************************************/
bool FFoliageBlueprint::IsValid() const
{
  if (BPFullClassName.IsEmpty() || !BPFullClassName.Contains("_C"))
    return false;
  return SpawnedClass != nullptr;
}

bool FFoliageBlueprint::SetBPClassName(const FString& Path)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::SetBPClassName);
  if (Path.IsEmpty())
    return false;
  TArray< FString > ParsedString;
  Path.ParseIntoArray(ParsedString, TEXT("/"), false);
  int Position = ParsedString.Num() - 1;
  const FString FullVersion = GetVersionFromFString(ParsedString[Position]);
  const FString Folder = ParsedString[--Position];
  const FString BPClassName = "BP_" + Folder + FullVersion;
  BPFullClassName = "Blueprint'";
  for (int i = 0; i <= Position; ++i)
  {
    BPFullClassName += ParsedString[i];
    BPFullClassName += '/';
  }
  BPFullClassName += BPClassName;
  BPFullClassName += ".";
  BPFullClassName += BPClassName;
  BPFullClassName += "_C'";
  return true;
}

bool FFoliageBlueprint::SetSpawnedClass()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::SetSpawnedClass);
  UClass* CastedBlueprint = LoadObject< UClass >(nullptr, *BPFullClassName);
  if (CastedBlueprint)
  {
    SpawnedClass = CastedBlueprint;  
    return true;
  }
  SpawnedClass = nullptr;
  return false;
}

/********************************************************************************/
/********** TILE DATA STRUCT ****************************************************/
/********************************************************************************/
void FTileData::UpdateTileMeshComponent(UInstancedStaticMeshComponent* NewInstancedStaticMeshComponent)
{
  UInstancedStaticMeshComponent* Aux { nullptr };
  for (std::shared_ptr<FTileMeshComponent>& ElementPtr
      : TileMeshesCache)
  {
    FTileMeshComponent& Element = *ElementPtr;
    if (Element.InstancedStaticMeshComponent == NewInstancedStaticMeshComponent)
    {
      int32 CurrentCount = Element.InstancedStaticMeshComponent->GetInstanceCount();
      int32 NewCount = NewInstancedStaticMeshComponent->GetInstanceCount();
      Element.bIsAlive = true;
      if (NewCount > CurrentCount)
      {
        Element.InstancedStaticMeshComponent = NewInstancedStaticMeshComponent;
        Element.IndicesInUse.Empty();
      }
    }
  }
}

bool FTileData::ContainsMesh(const UInstancedStaticMeshComponent* Mesh) const
{
  for (const std::shared_ptr<FTileMeshComponent>& Element 
      : TileMeshesCache)
  {
    if (Element->InstancedStaticMeshComponent == Mesh)
      return true;
  }
  return false;
}

void FTileData::UpdateMaterialCache(const FLinearColor& Value, bool DebugMaterials)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FTileData::UpdateMaterialCache);
  for (UMaterialInstanceDynamic* Material : MaterialInstanceDynamicCache)
  {
    if (DebugMaterials)
      Material->SetScalarParameterValue("ActivateDebug", 1);
    else
      Material->SetScalarParameterValue("ActivateDebug", 0);
    Material->SetScalarParameterValue("ActivateOpacity", 1);
    Material->SetVectorParameterValue("VehiclePosition", Value);
  }
}

/********************************************************************************/
/********** OVERRIDE FROM ACTOR *************************************************/
/********************************************************************************/
void AVegetationManager::BeginPlay()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::BeginPlay);
  Super::BeginPlay();
  LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  FWorldDelegates::LevelAddedToWorld.AddUObject(this, &AVegetationManager::OnLevelAddedToWorld);
  FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &AVegetationManager::OnLevelRemovedFromWorld);
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &AVegetationManager::PostWorldOriginOffset);
}

void AVegetationManager::Tick(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::Tick);
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Parent Tick);
    Super::Tick(DeltaTime);
  }
  if (!LargeMap)
    return;
  HeroVehicle = LargeMap->GetHeroVehicle();
  if (!IsValid(HeroVehicle))
      return;
    
  HeroVehicle->UpdateDetectionBox();
  TArray<FString> TilesInUse = GetTilesInUse();
  if (TilesInUse.Num() == 0)
  {
    UE_LOG(LogCarla, Warning, TEXT("No tiles detected."));
    return;
  }

  for (const FString& TileName : TilesInUse)
  {
    FTileData* Tile = TileCache.Find(TileName);
    if (!Tile)
      continue;
    UpdateMaterials(Tile);
    TArray<FElementsToSpawn> ElementsToSpawn = GetElementsToSpawn(Tile);
    SpawnSkeletalFoliages(ElementsToSpawn);
    ActivePooledActors();
    DestroySkeletalFoliages();
  }
  
}

/********************************************************************************/
/********** VEHICLE *************************************************************/
/********************************************************************************/
void AVegetationManager::AddVehicle(ACarlaWheeledVehicle* Vehicle)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::AddVehicle);
  if (!IsValid(Vehicle))
    return;
  HeroVehicle = Vehicle;
  UE_LOG(LogCarla, Display, TEXT("Vehicle added."));
}

void AVegetationManager::RemoveVehicle(ACarlaWheeledVehicle* Vehicle)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::RemoveVehicle);
  if (!IsValid(Vehicle))
    return;
  HeroVehicle = nullptr;
  UE_LOG(LogCarla, Display, TEXT("Vehicle removed."));
}

/********************************************************************************/
/********** CACHES **************************************************************/
/********************************************************************************/
void AVegetationManager::CreateOrUpdateTileCache(ULevel* InLevel)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::CreateOrUpdateTileCache);
  FTileData TileData {};
  for (AActor* Actor : InLevel->Actors)
  {
    AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(Actor);
    if (!IsValid(InstancedFoliageActor))
      continue;
    TileData.InstancedFoliageActor = InstancedFoliageActor;
    break;
  }
  if (!IsValid(TileData.InstancedFoliageActor))
    return;

  for (AActor* Actor : InLevel->Actors)
  {
    AProceduralFoliageVolume* ProceduralFoliageVolume = Cast<AProceduralFoliageVolume>(Actor);
    if (!IsValid(ProceduralFoliageVolume))
      continue;
    TileData.ProceduralFoliageVolume = ProceduralFoliageVolume;
    break;
  }
  if (!IsValid(TileData.ProceduralFoliageVolume))
    return;

  const FString TileName = TileData.InstancedFoliageActor->GetLevel()->GetOuter()->GetName();
  FTileData* ExistingTileData = TileCache.Find(TileName);
  if (ExistingTileData)
  {
    ExistingTileData->InstancedFoliageActor = TileData.InstancedFoliageActor;
    ExistingTileData->ProceduralFoliageVolume = TileData.ProceduralFoliageVolume;
    ExistingTileData->TileMeshesCache.Empty();
    ExistingTileData->MaterialInstanceDynamicCache.Empty();
    SetTileDataInternals(*ExistingTileData);
  }
  else
  {
    SetTileDataInternals(TileData);
    TileCache.Emplace(TileName, TileData);
  }
}

void AVegetationManager::SetTileDataInternals(FTileData& TileData)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::SetTileDataInternals);
  SetInstancedStaticMeshComponentCache(TileData);
  SetMaterialCache(TileData);
}

void AVegetationManager::SetInstancedStaticMeshComponentCache(FTileData& TileData)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::SetInstancedStaticMeshComponentCache);
  const TSet<UActorComponent*>& ActorComponents = TileData.InstancedFoliageActor->GetComponents();
  for (UActorComponent* Component : ActorComponents)
  {
    UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component);
    if (!IsValid(Mesh))
      continue;
    const FString Path = Mesh->GetStaticMesh()->GetPathName();
    const FFoliageBlueprint* BPCache = FoliageBlueprintCache.Find(Path);
    if (!BPCache)
      continue;

    if (TileData.ContainsMesh(Mesh))
    {
      TileData.UpdateTileMeshComponent(Mesh);
    }
    else
    {
      std::shared_ptr<FTileMeshComponent> Aux = 
          std::make_shared<FTileMeshComponent>();
      Aux->InstancedStaticMeshComponent = Mesh;
      Aux->bIsAlive = true;
      TileData.TileMeshesCache.Emplace(Aux);
    }
  }
}

void AVegetationManager::SetMaterialCache(FTileData& TileData)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::SetMaterialCache);
  if (TileData.MaterialInstanceDynamicCache.Num() > 0)
    TileData.MaterialInstanceDynamicCache.Empty();
  
  for (std::shared_ptr<FTileMeshComponent>& ElementPtr 
      : TileData.TileMeshesCache)
  {
    FTileMeshComponent& Element = *ElementPtr;
    UInstancedStaticMeshComponent* Mesh = Element.InstancedStaticMeshComponent;
    if (!IsValid(Mesh))
      continue;
    int32 Index = -1;
    for (UMaterialInterface* Material : Mesh->GetMaterials())
    {
      ++Index;
      if (!IsValid(Material))
        continue;
      UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Material, this);
      if (!MaterialInstanceDynamic)
        continue;
      if (TileData.MaterialInstanceDynamicCache.Contains(MaterialInstanceDynamic))
        continue;
      MaterialInstanceDynamic->SetScalarParameterValue("ActivateOpacity", 0);
      MaterialInstanceDynamic->SetScalarParameterValue("ActivateDebug", 0);
      MaterialInstanceDynamic->SetScalarParameterValue("Distance", HideMaterialDistance);
      Mesh->SetMaterial(Index, MaterialInstanceDynamic);
      TileData.MaterialInstanceDynamicCache.Emplace(MaterialInstanceDynamic);
    }
  }  
}

void AVegetationManager::UpdateFoliageBlueprintCache(ULevel* InLevel)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::UpdateFoliageBlueprintCache);
  for (AActor* Actor : InLevel->Actors)
  {
    AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(Actor);
    if (!IsValid(InstancedFoliageActor))
      continue;
    const FString TileName = InstancedFoliageActor->GetLevel()->GetOuter()->GetName();
    const TSet<UActorComponent*>& ActorComponents = InstancedFoliageActor->GetComponents();
    for (UActorComponent* Component : ActorComponents)
    {
      UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component);
      if (!IsValid(Mesh))
        continue;      
      const FString Path = Mesh->GetStaticMesh()->GetPathName();
      if (!IsFoliageTypeEnabled(Path))
        continue;
      if (FoliageBlueprintCache.Contains(Path))
        continue;
      FFoliageBlueprint NewFoliageBlueprint;
      NewFoliageBlueprint.SetBPClassName(Path);
      NewFoliageBlueprint.SetSpawnedClass();
      
      if (!NewFoliageBlueprint.IsValid())
      {
        UE_LOG(LogCarla, Warning, TEXT("Blueprint %s was invalid."), *NewFoliageBlueprint.BPFullClassName);
      }
      else
      {
        UE_LOG(LogCarla, Display, TEXT("Blueprint %s created."), *NewFoliageBlueprint.BPFullClassName);
        FoliageBlueprintCache.Emplace(Path, NewFoliageBlueprint);
        CreatePoolForBPClass(NewFoliageBlueprint);
      }
    }
  }
}

void AVegetationManager::FreeTileCache(ULevel* InLevel)
{
  if (!IsValid(InLevel))
    return;
  AInstancedFoliageActor* TileInstancedFoliageActor = nullptr;
  for (AActor* Actor : InLevel->Actors)
  {
    if (!IsValid(Actor))
      continue;
    AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(Actor);
    if (!IsValid(InstancedFoliageActor))
      continue;
    TileInstancedFoliageActor = InstancedFoliageActor;
    break;
  }
  if (!IsValid(TileInstancedFoliageActor))
    return;
  const FString TileName = TileInstancedFoliageActor->GetLevel()->GetOuter()->GetName();
  FTileData* ExistingTileData = TileCache.Find(TileName);
  if (ExistingTileData)
  {

    ExistingTileData->MaterialInstanceDynamicCache.Empty();
    for (std::shared_ptr<FTileMeshComponent>& Element 
        : ExistingTileData->TileMeshesCache)
    {
      Element->IndicesInUse.Empty();
      Element->bIsAlive = false;
    }
    ExistingTileData->TileMeshesCache.Empty();
    ExistingTileData->InstancedFoliageActor = nullptr;
    ExistingTileData->ProceduralFoliageVolume = nullptr;

    TileCache.Remove(TileName);
  }
}

/********************************************************************************/
/********** TICK ****************************************************************/
/********************************************************************************/
void AVegetationManager::UpdateMaterials(FTileData* Tile)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::UpdateMaterials);
  const FTransform GlobalTransform = HeroVehicle->GetActorTransform();
  const FLinearColor Position = GlobalTransform.GetLocation();
  Tile->UpdateMaterialCache(Position, DebugMaterials);
}

TArray<FElementsToSpawn> AVegetationManager::GetElementsToSpawn(FTileData* Tile)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::GetElementsToSpawn);
  TArray<FElementsToSpawn> Results;
  int32 i = -1;

  for (std::shared_ptr<FTileMeshComponent>& ElementPtr 
      : Tile->TileMeshesCache)
  {
    FTileMeshComponent& Element = *ElementPtr;
    TRACE_CPUPROFILER_EVENT_SCOPE(Update Foliage Usage);
    ++i;
    UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Element.InstancedStaticMeshComponent;
    const FString Path = InstancedStaticMeshComponent->GetStaticMesh()->GetPathName();
    FFoliageBlueprint* BP = FoliageBlueprintCache.Find(Path);
    if (!BP)
      continue;
    TArray<int32> Indices = HeroVehicle->GetFoliageInstancesCloseToVehicle(InstancedStaticMeshComponent);
    if (Indices.Num() == 0)
      continue;
    TArray<int32> NewIndices;
    for (int32 Index : Indices)
    {
      if (Element.IndicesInUse.Contains(Index))
      {
        continue;
      }
      NewIndices.Emplace(Index);
    }
    
    FElementsToSpawn NewElement {};
    NewElement.TileMeshComponent = Tile->TileMeshesCache[i];
    NewElement.BP = *BP;
    for (int32 Index : NewIndices)
    {
      FTransform Transform;
      InstancedStaticMeshComponent->GetInstanceTransform(Index, Transform, true);
      NewElement.TransformIndex.Emplace(TPair<FTransform, int32>(Transform, Index));
    }
    if (NewElement.TransformIndex.Num() > 0)
      Results.Emplace(NewElement);
  }
  return Results;
}

void AVegetationManager::SpawnSkeletalFoliages(TArray<FElementsToSpawn>& ElementsToSpawn)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::SpawnSkeletalFoliages);
  const FTransform HeroTransform = HeroVehicle->GetActorTransform();
  const FVector HeroLocation = HeroTransform.GetLocation();
  const float HeroDetectionSizeSquared = HeroVehicle->GetDetectionSize() * HeroVehicle->GetDetectionSize();

  for (FElementsToSpawn& Element : ElementsToSpawn)
  {
    TArray<FPooledActor>* Pool = ActorPool.Find(Element.BP.BPFullClassName);

    if (Pool == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("Pool not valid"));
      continue;
    }
    for (const TPair<FTransform, int32>& TransformIndex : Element.TransformIndex)
    {
      const FTransform& Transform = TransformIndex.Key;
      int32 Index = TransformIndex.Value;
      if (Element.TileMeshComponent->IndicesInUse.Contains(Index))
      {
        continue;        
      }
      const float Distance = FMath::Abs(FVector::DistSquared(Transform.GetLocation(), HeroLocation));
      if (Distance > HeroDetectionSizeSquared)
      {
        continue;
      }
      bool Ok = EnableActorFromPool(Transform, Index, Element.TileMeshComponent, *Pool);
      if (Ok)
      {        
      }
      else
      {
        FPooledActor NewElement;
        NewElement.Actor = CreateFoliage(Element.BP, {});
        if (IsValid(NewElement.Actor))
        {
          NewElement.Actor->SetTickableWhenPaused(false);
          NewElement.EnableActor(Transform, Index, Element.TileMeshComponent);
          Pool->Emplace(NewElement);
        }
      }
    }
  }
}

void AVegetationManager::ActivePooledActors()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::ActivePooledActors);
  const FTransform HeroTransform = HeroVehicle->GetActorTransform();
  const FVector HeroLocation = HeroTransform.GetLocation();
  const float SquaredActiveActorDistance = ActiveActorDistance * ActiveActorDistance;

  for (TPair<FString, TArray<FPooledActor>>& Element : ActorPool)
  {
    TArray<FPooledActor>& Pool = Element.Value;
    for (FPooledActor& Actor : Pool)
    {
      if (!Actor.InUse)
        continue;
      if (Actor.IsActive)
        continue;
      const FVector Location = Actor.GlobalTransform.GetLocation();
      const float Distance = FMath::Abs(FVector::DistSquared(Location, HeroLocation));
      if (Distance < SquaredActiveActorDistance)
      {
        Actor.ActiveActor();
      }
    }
  }
}

void AVegetationManager::DestroySkeletalFoliages()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::DestroySkeletalFoliages);
  const FTransform HeroTransform = HeroVehicle->GetActorTransform();
  const FVector HeroLocation = HeroTransform.GetLocation();
  const float HeroDetectionSizeSquared = HeroVehicle->GetDetectionSize() * HeroVehicle->GetDetectionSize();

  for (TPair<FString, TArray<FPooledActor>>& Element : ActorPool)
  {
    TArray<FPooledActor>& Pool = Element.Value;
    for (FPooledActor& Actor : Pool)
    {
      if (!Actor.InUse)
          continue;
      const FVector Location = Actor.GlobalTransform.GetLocation();
      const float Distance = FMath::Abs(FVector::DistSquared(Location, HeroLocation));
      if (Distance > HeroDetectionSizeSquared)
      {
        Actor.DisableActor();
      }
    }
  }
}

bool AVegetationManager::EnableActorFromPool(
    const FTransform& Transform,
    int32 Index,
    std::shared_ptr<FTileMeshComponent>& TileMeshComponent,
    TArray<FPooledActor>& Pool)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::EnableActorFromPool);
  for (FPooledActor& PooledActor : Pool)
  {
    if (PooledActor.InUse)
      continue;
    PooledActor.EnableActor(Transform, Index, TileMeshComponent);
    PooledActor.Actor->SetActorLocationAndRotation(Transform.GetLocation(), Transform.Rotator(), true, nullptr, ETeleportType::ResetPhysics);
    if (SpawnScale <= 1.01f && SpawnScale >= 0.99f)
      PooledActor.Actor->SetActorScale3D(Transform.GetScale3D());
    else
      PooledActor.Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
    return true;
  }
  return false;
}

/********************************************************************************/
/********** POOLS ***************************************************************/
/********************************************************************************/
void AVegetationManager::CreatePoolForBPClass(const FFoliageBlueprint& BP)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::CreatePoolForBPClass);
  TArray<FPooledActor> AuxPool;
  const FTransform Transform {};
  for (int32 i = 0; i < InitialPoolSize; ++i)
  {
    FPooledActor NewElement;
    NewElement.Actor = CreateFoliage(BP, Transform);
    if (IsValid(NewElement.Actor))
    {
      UE_LOG(LogCarla, Display, TEXT("Created actor for pool"));
      NewElement.Actor->SetTickableWhenPaused(false);
      NewElement.DisableActor();
      AuxPool.Emplace(NewElement);
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to create actor for pool"));
    }
  }
  ActorPool.Emplace(BP.BPFullClassName, AuxPool);
  UE_LOG(LogCarla, Display, TEXT("CreatePoolForBPClass: %s"), *BP.BPFullClassName);
}

AActor* AVegetationManager::CreateFoliage(const FFoliageBlueprint& BP, const FTransform& Transform) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::CreateFoliage);

  AActor* Actor = GetWorld()->SpawnActor<AActor>(BP.SpawnedClass,
    Transform.GetLocation(), Transform.Rotator());
  
  TArray<UTaggedComponent*> TaggedComponents;
  Actor->GetComponents(TaggedComponents);
  for (UTaggedComponent* Component: TaggedComponents)
  {
    Component->DestroyComponent();
  }

  if (SpawnScale <= 1.01f && SpawnScale >= 0.99f)
    Actor->SetActorScale3D(Transform.GetScale3D());
  else
    Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
  return Actor;
}

void AVegetationManager::UpdatePoolBasePosition()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::UpdatePoolBasePosition);
  for (TPair<FString, TArray<FPooledActor>>& Element : ActorPool)
  {
    TArray<FPooledActor>& Pool = Element.Value;
    for (FPooledActor& PooledActor : Pool)
    {
      if (PooledActor.InUse)
        continue;
      PooledActor.Actor->SetActorTransform(InactivePoolTransform, true, nullptr, ETeleportType::ResetPhysics);
    }
  }
}

/********************************************************************************/
/********** EVENTS **************************************************************/
/********************************************************************************/
void AVegetationManager::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::OnLevelAddedToWorld);
  UpdateFoliageBlueprintCache(InLevel);
  CreateOrUpdateTileCache(InLevel);
}

void AVegetationManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::OnLevelRemovedFromWorld);
  FreeTileCache(InLevel);
}

void AVegetationManager::PostWorldOriginOffset(UWorld*, FIntVector, FIntVector)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::PostWorldOriginOffset);
  InactivePoolTransform.SetLocation(FVector(0.0f, 0.0f, 0.0f));
  UpdatePoolBasePosition();
}

/********************************************************************************/
/********** TILES ***************************************************************/
/********************************************************************************/
bool AVegetationManager::IsFoliageTypeEnabled(const FString& Path) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::IsFoliageTypeEnabled);
  if (!SpawnRocks)
    if (Path.Contains("/Rock"))
      return false;
  if (!SpawnTrees)
    if (Path.Contains("/Tree"))
      return false;
  if (!SpawnBushes)
    if (Path.Contains("/Bush"))
      return false;
  if (!SpawnPlants)
    if (Path.Contains("/Plant"))
      return false;
  return true;
}

bool AVegetationManager::CheckForNewTiles() const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::CheckForNewTiles);
  const UObject* World = GetWorld();
  TArray<AActor*> ActorsInLevel;
  UGameplayStatics::GetAllActorsOfClass(World, AInstancedFoliageActor::StaticClass(), ActorsInLevel);
  for (AActor* Actor : ActorsInLevel)
  {
    AInstancedFoliageActor* InstancedFoliageActor = Cast<AInstancedFoliageActor>(Actor);
    if (!IsValid(InstancedFoliageActor))
      continue;
    const FString TileName = InstancedFoliageActor->GetLevel()->GetOuter()->GetName();
    if (!TileCache.Contains(TileName))
      return true;
  }
  return false;
}

TArray<FString> AVegetationManager::GetTilesInUse()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationManager::GetTilesInUse);
  TArray<FString> Results;
  
  for (const TPair<FString, FTileData>& Element : TileCache)
  {
    const FTileData& TileData = Element.Value;
    if (!IsValid(TileData.InstancedFoliageActor) || !IsValid(TileData.ProceduralFoliageVolume))
    {
      TileCache.Remove(Element.Key);
      return Results;
    }
    if (Results.Contains(Element.Key))
      continue;
    const AProceduralFoliageVolume* Procedural = TileData.ProceduralFoliageVolume;
    if (!IsValid(Procedural))
      continue;
    if (!IsValid(Procedural->ProceduralComponent))
      continue;
    const FBox Box = Procedural->ProceduralComponent->GetBounds();
    if (!Box.IsValid)
      continue;
    if (Box.IsInside(HeroVehicle->GetActorTransform().GetLocation()))
        Results.Emplace(Element.Key);
  }
  return Results;
}
