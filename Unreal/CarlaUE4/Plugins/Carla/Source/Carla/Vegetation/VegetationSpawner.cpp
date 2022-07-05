// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#include "Carla/Vegetation/VegetationSpawner.h"

#include "ProceduralFoliageVolume.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Vegetation/SpringBasedVegetationComponent.h"
#include "Kismet/GameplayStatics.h"

void FPooledActor::EnableActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPooledActor::EnableActor);
  Actor->SetActorHiddenInGame(false);
  Actor->SetActorEnableCollision(true);
  Actor->SetActorTickEnabled(true);

  USpringBasedVegetationComponent* Component = Actor->FindComponentByClass<USpringBasedVegetationComponent>();
  if (Component)
    Component->SetComponentTickEnabled(true);
}

void FPooledActor::DisableActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPooledActor::DisableActor);
  Actor->SetActorHiddenInGame(true);
  Actor->SetActorEnableCollision(false);
  Actor->SetActorTickEnabled(false);

  USpringBasedVegetationComponent* Component = Actor->FindComponentByClass<USpringBasedVegetationComponent>();
  if (Component)
    Component->SetComponentTickEnabled(false);
}

FFoliageBlueprintCache::FFoliageBlueprintCache() = default;

bool FFoliageBlueprintCache::IsValid() const
{
  return SpawnedClass;
}

FString FFoliageBlueprintCache::GetVersionFromFString(const FString& String)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetVersionFromFString);
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

bool FFoliageBlueprintCache::SetBPClassName()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::SetBPClassName);
  TArray< FString > ParsedString;
  Path.ParseIntoArray(ParsedString, TEXT("/"), false);
  int Position = ParsedString.Num() - 1;
  const FString Version = ParsedString[Position];
  const FString FullVersion = GetVersionFromFString(Version);
  if (FullVersion.IsEmpty())
    return false;
  const FString Folder = ParsedString[--Position];
  FString ClassPath = "BP_" + Folder + FullVersion;
  BPClassName = "Blueprint'";
  for (int i = 0; i <= Position; ++i)
  {
    BPClassName += ParsedString[i];
    BPClassName += '/';
  }
  BPClassName += ClassPath;
  BPClassName += ".";
  BPClassName += ClassPath;
  BPClassName += "'";
  return true;
}

bool FFoliageBlueprintCache::SetSpawnedClass()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::SetSpawnedClass);
  UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *BPClassName);
  UBlueprint* CastedBlueprint = Cast<UBlueprint>(LoadedObject);
  if (CastedBlueprint && CastedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
  {
    SpawnedClass = *CastedBlueprint->GeneratedClass;  
    return true;
  }
  return false;
}


void AVegetationSpawner::BeginPlay()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::BeginPlay);
  Super::BeginPlay();

  CacheFoliageTypesInLevel();
  CreatePools();
}

void AVegetationSpawner::Tick(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::Tick);
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Parent Tick);
    Super::Tick(DeltaTime);
  }
  GetVehiclesInLevel();
  if (VehiclesInLevel.Num() == 0)
    return;
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Update Vehicles Detection Boxes);
    for (AActor* Vehicle : VehiclesInLevel)
    {
      ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
      if (IsValid(CarlaVehicle))
        CarlaVehicle->UpdateDetectionBox();
    }
  }
  for (FLevelFoliage& Foliage : LevelFoliages)
  {
    TArray<int32> IndicesInUse;
    const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
    FFoliageBlueprintCache* Blueprint = FoliageCache.Find(Path);
    if (!Blueprint->IsValid())
      continue;
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(Get Foliage Usage);
      for (const AActor* Vehicle : VehiclesInLevel)
      {
        const ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
        if (!IsValid(CarlaVehicle))
          continue;
        const TArray<int32> Aux = CarlaVehicle->GetFoliageInstancesCloseToVehicle(Foliage.Mesh);
        AddFoliageIndicesInUse(IndicesInUse, Aux);
      }
    }

    /*
    SetmMin draw distance.
    virtual bool UpdateInstances
    (
        const TArray< int32 > & UpdateInstanceIds,
        const TArray< FTransform > & UpdateInstanceTransforms,
        const TArray< FTransform > & UpdateInstancePreviousTransforms,
        int32 NumCustomFloats,
        const TArray< float > & CustomFloatData
    ) 
    */
    {
      for (int32 Index : IndicesInUse)
      {
        if (IsFoliageIndexInUse(Foliage, Index))
          continue;
        if (GetFoliageFromPool(Foliage, Index))
        {
          TRACE_CPUPROFILER_EVENT_SCOPE(Hide Static Foliage);
          //Foliage.Mesh->UpdateInstanceTransform(Index, FTransform(), true, false, false);
        }
      }
    }
    {
      for (FPooledActor& PooledActor : Foliage.FoliagePool)
      {
        if (!PooledActor.InUse)
          continue;

        const FVector Location = PooledActor.Actor->GetActorLocation();
        bool StillInUse = false;
        for (AActor* Vehicle : VehiclesInLevel)
        {
          ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
          if (!CarlaVehicle)
            continue;
          StillInUse = CarlaVehicle->IsInVehicleRange(Location);
          if (StillInUse)
            break;
        }
        if (!StillInUse)
        {
          PooledActor.InUse = false;
          PooledActor.DisableActor();
          {
            TRACE_CPUPROFILER_EVENT_SCOPE(Show Static Foliage);
            //Foliage.Mesh->UpdateInstanceTransform(PooledActor.OriginalIndex, PooledActor.OriginalTransform, true, false, false);
          }
        }
      }
    }
  }
}

void AVegetationSpawner::GetVehiclesInLevel()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetVehiclesInLevel);
  TArray<AActor*> FoundVehicles;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarlaWheeledVehicle::StaticClass(), FoundVehicles);
  VehiclesInLevel = std::move(FoundVehicles);
}

void AVegetationSpawner::AddFoliageIndicesInUse(TArray<int32>& Global, const TArray<int32>& Local)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::AddFoliageIndicesInUse);
  for (int32 Index : Local)
  {
    if (!Global.Contains(Index))
      Global.Add(Index);
  }
}

bool AVegetationSpawner::IsFoliageIndexInUse(const FLevelFoliage& Foliage, int32 Index) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::IsFoliageIndexInUse);
  for (const FPooledActor& Actor : Foliage.FoliagePool)
  {
    if (Actor.InUse && Actor.OriginalIndex == Index)
      return true;
  }
  return false;
}


/************* FOLIAGE CREATION AND DESTRUCTION *******************************/
AActor* AVegetationSpawner::CreateFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CreateFoliage);
  AActor* Actor = GetWorld()->SpawnActor<AActor>(CacheBPClass.SpawnedClass,
    FoliageTransform.GetLocation(), FoliageTransform.Rotator());
  if (SpawnScale <= 1.01f && SpawnScale >= 0.99f)
    Actor->SetActorScale3D(FoliageTransform.GetScale3D());
  else
    Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
  Actor->SetTickableWhenPaused(false);
  return Actor;
}

bool AVegetationSpawner::GetFoliageFromPool(FLevelFoliage& Foliage, int32 Index)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetFoliageFromPool);
  for (FPooledActor& PooledActor : Foliage.FoliagePool)
  {
    if (PooledActor.InUse)
      continue;
    bool Ok = false;
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(GetInstanceTransform);
      Ok = Foliage.Mesh->GetInstanceTransform(Index, PooledActor.OriginalTransform, true);
    }
    if (Ok)
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(SetActorLocationAndRotation);
      PooledActor.InUse = true;
      PooledActor.OriginalIndex = Index;
      PooledActor.Actor->SetActorLocationAndRotation(PooledActor.OriginalTransform.GetLocation(), PooledActor.OriginalTransform.Rotator(), true, nullptr, ETeleportType::ResetPhysics);
      PooledActor.EnableActor();
      return true;
    }
    return false;
  }
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Pool lleno");
    TRACE_CPUPROFILER_EVENT_SCOPE(Add New Foliage to pool);
    const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
    FFoliageBlueprintCache* Blueprint = FoliageCache.Find(Path);
    if (!Blueprint->IsValid())
      return false;
    bool Ok = false;
    FPooledActor NewElement;
    {
      TRACE_CPUPROFILER_EVENT_SCOPE(GetInstanceTransform);
      Ok = Foliage.Mesh->GetInstanceTransform(Index, NewElement.OriginalTransform, true);
    }
    if (!Ok)
      return false;
    NewElement.Actor = CreateFoliage(*Blueprint, NewElement.OriginalTransform);
    if (!NewElement.Actor)
      return false;
    NewElement.InUse = true;
    NewElement.OriginalIndex = Index;
    NewElement.EnableActor();
    Foliage.FoliagePool.Emplace(NewElement);
  }
  return true;
}

/************* FOLIAGE CACHE *******************************/

void AVegetationSpawner::CacheFoliageTypesInLevel()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CacheFoliageTypesInLevel);
  const UObject* World = GetWorld();
  TArray<AActor*> ActorsInLevel;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInLevel);
  for (const AActor* Actor : ActorsInLevel)
  {
    const TSet<UActorComponent*>& ActorComponents = Actor->GetComponents();
    for (UActorComponent* Component : ActorComponents)
    {
      UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component);
      if (!IsValid(Mesh))
        continue;
      const FString Path = Mesh->GetStaticMesh()->GetPathName();
      if (FoliageCache.Contains(Path))
        continue;
      FFoliageBlueprintCache NewFoliageBlueprint;
      NewFoliageBlueprint.Path = Path;
      if (!IsFoliageTypeEnabled(Path))
      {
        FoliageCache.Emplace(Path, NewFoliageBlueprint);
        continue;
      }
      if (!NewFoliageBlueprint.SetBPClassName())
      {
        FoliageCache.Emplace(Path, NewFoliageBlueprint);
        continue;
      }
      if (!NewFoliageBlueprint.SetSpawnedClass())
      {
        FoliageCache.Emplace(Path, NewFoliageBlueprint);
        continue;
      }
      FoliageCache.Emplace(Path, NewFoliageBlueprint);

      FLevelFoliage Foliage;
      Foliage.Mesh = Mesh;
      LevelFoliages.Emplace(Foliage);
      //TODO: Remove, only for debug.
      //Foliage.Mesh->SetCullDistances(MinDistance, MaxDistance);
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, NewFoliageBlueprint.BPClassName);
    }
  }
}

void AVegetationSpawner::CreatePools()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CreatePools);
  for (FLevelFoliage& Foliage : LevelFoliages)
  {
    const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
    FFoliageBlueprintCache* Blueprint = FoliageCache.Find(Path);
    if (!Blueprint->IsValid())
      continue;    
    for (int32 i = 0; i < InitialPoolSize; ++i)
    {
      FPooledActor NewElement;
      NewElement.Actor = CreateFoliage(*Blueprint, FTransform());
      if (NewElement.Actor == nullptr)
          break;
      NewElement.DisableActor();
      Foliage.FoliagePool.Emplace(NewElement);
    }
    if (Foliage.FoliagePool.Num() == InitialPoolSize)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, Blueprint->BPClassName + " - Pool creado");
    else if (Foliage.FoliagePool.Num() == 0)      
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Blueprint->BPClassName + " Pool vacio");
    else
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Blueprint->BPClassName + FString(" - Pool a mitad ( ") + FString::FromInt(Foliage.FoliagePool.Num()) + FString(" )"));
  }
}

bool AVegetationSpawner::IsFoliageTypeEnabled(const FString& Path) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::IsFoliageTypeEnabled);
  if (!SpawnRocks)
    if (Path.Contains("Rock"))
      return false;
  if (!SpawnTrees)
    if (Path.Contains("Tree"))
      return false;
  if (!SpawnBushes)
    if (Path.Contains("Bush"))
      return false;
  if (!SpawnPlants)
    if (Path.Contains("Plant"))
      return false;
  return true;
}
