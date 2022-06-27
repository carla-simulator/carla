// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#include "Carla/Vegetation/VegetationSpawner.h"

#include "ProceduralFoliageVolume.h"
#include "Kismet/GameplayStatics.h"

void FPooledFoliage::EnableActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(EnableActor);
  Actor->SetActorTickEnabled(true);
  Actor->SetActorEnableCollision(true);
  Actor->SetActorHiddenInGame(false);
}

void FPooledFoliage::DisableActor()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(DisableActor);
  Actor->SetActorTickEnabled(false);
  Actor->SetActorEnableCollision(false);
  Actor->SetActorHiddenInGame(true);
}

FFoliageBlueprintCache::FFoliageBlueprintCache(const FString& Path)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::FFoliageBlueprintCache);
  if (Path.IsEmpty())
    return;
  Init(Path);
}

bool FFoliageBlueprintCache::IsValid() const
{
  return SpawnedClass;
}

void FFoliageBlueprintCache::Init(const FString& Path)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FoliageBlueprintCache::Init);
  TArray< FString > ParsedString;
  Path.ParseIntoArray(ParsedString, TEXT("/"), false);
  int Position = ParsedString.Num() - 1;
  const FString Version = ParsedString[Position];
  const FString FullVersion = GetVersionFromFString(Version);
  if (FullVersion.IsEmpty())
    return;
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

  UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *BPClassName);
  UBlueprint* CastedBlueprint = Cast<UBlueprint>(LoadedObject);

  FString Right;
  BPClassName.Split(".BP", nullptr, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
  if (Right.IsEmpty())
    return;
  Right = "BP" + Right;
  Right.RemoveFromEnd("'", ESearchCase::IgnoreCase);
  BPClassName = Right;

  if (CastedBlueprint && CastedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
    SpawnedClass = *CastedBlueprint->GeneratedClass;  
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

void AVegetationSpawner::BeginPlay()
{
  Super::BeginPlay();
  
  GetFoliageTypesInLevel();
  //CreatePools();
}

void AVegetationSpawner::Tick(float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::Tick);
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Parent Tick);
    Super::Tick(DeltaTime);
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Get Vehicles);
    GetVehiclesInLevel();
    if (VehiclesInLevel.Num() == 0)
      return;
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Update Vehicles Detection Boxes);
    for (AActor* Vehicle : VehiclesInLevel)
    {
      ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
      if (IsValid(CarlaVehicle))
        CarlaVehicle->UpdateDetectionBox();
    }
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Get Foliage Usage);
    for (FLevelFoliage& Foliage : FoliageTypesInLevel)
    {
      for (AActor* Vehicle : VehiclesInLevel)
      {
        ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
        if (!IsValid(CarlaVehicle))
          continue;
        //Para un coche solo valdría con Foliage.IndicesInUse = CarlaVehicle->GetFoliageInstancesCloseToVehicle(Foliage.Mesh);
        Foliage.IndicesInUse = CarlaVehicle->GetFoliageInstancesCloseToVehicle(Foliage.Mesh);
        //TArray<int32> FrameIndices = CarlaVehicle->GetFoliageInstancesCloseToVehicle(Foliage.Mesh);
        //for (int32 i : FrameIndices)
        //  Foliage.IndicesInUse[i] = true;       
      }
    }
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Spawn Skeletal Foliage);
    for (FLevelFoliage& Foliage : FoliageTypesInLevel)
    {
      const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
      if (!IsFoliageTypeEnabled(Path))
        continue;
      const FFoliageBlueprintCache BlueprintCache = GetBlueprintFromCache(Path);
      if (!BlueprintCache.IsValid())
        continue;
      /*for (int32 Index : Foliage.IndicesInUse)
      {
        if (IsValid(Foliage.SpawnedActors[Index]))
          continue;
        
        AActor* Actor = GetFoliageFromPool(BlueprintCache);
        if (!Actor)
        {
          GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("New Actor is NULL"));
          continue;
        }
        //moves the actor to the correct position.
        SpawnFoliage(Actor, Foliage.Transforms[Index]);
        //Assign
        Foliage.SpawnedActors[Index] = Actor;
        {
          TRACE_CPUPROFILER_EVENT_SCOPE(Hide Instance Transform);
          Foliage.Mesh->UpdateInstanceTransform(Index, {}, true, true, false);
        }
      }*/
      //int32 Index = -1;
      for (int32 Index : Foliage.IndicesInUse)
      {
        //++Index;
        //if (!InUse)
          //continue;
        if (IsValid(Foliage.SpawnedActors[Index]))
          continue;
        /*AActor* Actor = GetFoliageFromPool(BlueprintCache);
        if (!Actor)
        {
          GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("New Actor is NULL"));
          continue;
        }
        //moves the actor to the correct position.
        SpawnFoliage(Actor, Foliage.Transforms[Index]);
        //Assign
        Foliage.SpawnedActors[Index] = Actor;*/
        Foliage.SpawnedActors[Index] = CreateFoliage(BlueprintCache, Foliage.Transforms[Index]);
        {
          TRACE_CPUPROFILER_EVENT_SCOPE(Hide Instance Transform);
          Foliage.Mesh->UpdateInstanceTransform(Index, {}, true, true, false);
        }
      }
    }
  }
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(Delete Foliages);
    for (FLevelFoliage& Foliage : FoliageTypesInLevel)
    {
      const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
      if (!IsFoliageTypeEnabled(Path))
        continue;
      bool Found = FindInCache(Path);
      if (!Found)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("BlueprintCache Not Found in Cache"));
        continue;
      }
      const FFoliageBlueprintCache BlueprintCache = GetBlueprintFromCache(Path);
      for (int32 Index : Foliage.IndicesInUse)
      {
        if (!IsValid(Foliage.SpawnedActors[Index]))
          continue;
        const FVector Location = Foliage.Transforms[Index].GetLocation();
        bool StillInUse = false;
        for (AActor* Vehicle : VehiclesInLevel)
        {
          ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
          if (!IsValid(CarlaVehicle))
            continue;
          StillInUse = CarlaVehicle->IsInVehicleRange(Location);
          if (StillInUse)
            break;
        }
        if (!StillInUse)
        {
          TRACE_CPUPROFILER_EVENT_SCOPE(Destroy Spawned Actor);
          Foliage.SpawnedActors[Index]->Destroy();
          Foliage.SpawnedActors[Index] = nullptr;
          Foliage.Mesh->UpdateInstanceTransform(Index, Foliage.Transforms[Index], true, true, false);
        }
      }

      /*
      int32 Index = -1;
      for (AActor* Actor : Foliage.SpawnedActors)
      {
        ++Index;
        if ((Actor == nullptr) || Foliage.IndicesInUse.Contains(Index))
          continue;
        //HideFoliage(BlueprintCache, Actor);
        const FVector Location = Foliage.Transforms[Index].GetLocation();
        bool StillInUse = false;
        for (AActor* Vehicle : VehiclesInLevel)
        {
          ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
          if (!IsValid(CarlaVehicle))
            continue;
          StillInUse = CarlaVehicle->IsInVehicleRange(Location);
          if (StillInUse)
            break;
        }
        if (!StillInUse)
        {
          Foliage.SpawnedActors[Index]->Destroy();
          Foliage.SpawnedActors[Index] = nullptr;
        }
        //Actor = nullptr;
        {
          TRACE_CPUPROFILER_EVENT_SCOPE(Show Instance Transform);
          Foliage.Mesh->UpdateInstanceTransform(Index, Foliage.Transforms[Index], true, true, false);
        }
      }
      */
    }
  }
}

AActor* AVegetationSpawner::GetFoliageFromPool(const FFoliageBlueprintCache& FoliageBlueprint)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetFoliageFromPool);
  TArray<FPooledFoliage>* Pool = FoliagePool.Find(FoliageBlueprint.BPClassName);
  if (!Pool)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("FoliageBlueprintCache not found!"));
    return nullptr;
  }
  //Get the first unusued actor.
  for (FPooledFoliage& PooledFoliage : *Pool)
  {
    if (!PooledFoliage.InUse)
    {
      PooledFoliage.InUse = true;
      PooledFoliage.EnableActor();
      return PooledFoliage.Actor;
    }
  }
  
  FPooledFoliage Aux = CreatePooledFoliage(FoliageBlueprint);
  if (!Aux.Actor)
    return nullptr;
  Pool->Add(Aux);
  return Aux.Actor;
}

void AVegetationSpawner::HideFoliage(const FFoliageBlueprintCache& FoliageBlueprint, AActor* Actor)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::HideFoliage);
  TArray<FPooledFoliage>* Pool = FoliagePool.Find(FoliageBlueprint.BPClassName);
  if (!Pool)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString("FoliageBlueprintCache not found!"));
    return;
  }
  
  for (FPooledFoliage& PooledFoliage : *Pool)
  {
    if (PooledFoliage.Actor == Actor)
    {
      PooledFoliage.InUse = false;
      PooledFoliage.DisableActor();
      return;
    }
  }
  Actor->Destroy();
}

void AVegetationSpawner::SpawnFoliage(AActor* Actor, const FTransform& FoliageTransform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::SpawnFoliage);
  Actor->SetActorLocationAndRotation(FoliageTransform.GetLocation(), FoliageTransform.Rotator(), false, 0, ETeleportType::None);
  if (SpawnScale <= 1.01f && SpawnScale >= 0.99f)
    Actor->SetActorScale3D(FoliageTransform.GetScale3D());
  else
    Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
}

AActor* AVegetationSpawner::CreateFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CreateFoliage);
    AActor* Actor = GetWorld()->SpawnActor<AActor>(CacheBPClass.SpawnedClass,
      FoliageTransform.GetLocation(), FoliageTransform.Rotator());
    if (SpawnScale <= 1.01f && SpawnScale >= 0.99f)
      Actor->SetActorScale3D(FoliageTransform.GetScale3D());
    else
      Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
    return Actor;
  }

void AVegetationSpawner::CreatePools()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CreatePools);
  for (const auto& Element : FoliageCache)
  {
    if (!Element.IsValid())
      continue;

    TArray<FPooledFoliage>* Exists = FoliagePool.Find(Element.BPClassName);
    if (Exists)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, Element.BPClassName + " - Pool ya existe");
      continue;
    }
    TArray<FPooledFoliage> NewPool;
    for (int32 i = 0; i < InitialPoolSize; ++i)
    {
      FPooledFoliage NewElement = CreatePooledFoliage(Element);
      if (NewElement.Actor == nullptr)
        break;
      NewElement.DisableActor();
      NewPool.Add(NewElement);
    }
    if (NewPool.Num() == 0)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Element.BPClassName + " Pool vacio");
    else if (NewPool.Num() == 1)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Element.BPClassName + " Pool 1");
    else if (NewPool.Num() == 2)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Element.BPClassName + " Pool 2");
    else if (NewPool.Num() == 3)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Element.BPClassName + " Pool 3");
    else if (NewPool.Num() == 4)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Element.BPClassName + " Pool 4");
    else if (NewPool.Num() == InitialPoolSize)
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, Element.BPClassName + " - Pool creado");
    else
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, Element.BPClassName + FString(" - Pool a mitad ( ") + FString::FromInt(NewPool.Num()) + FString(" )"));
    FoliagePool.Add(Element.BPClassName, NewPool);
  }
}

FPooledFoliage AVegetationSpawner::CreatePooledFoliage(const FFoliageBlueprintCache& FoliageBlueprint) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::CreatePooledFoliage);
  AActor* Actor = GetWorld()->SpawnActor<AActor>(FoliageBlueprint.SpawnedClass, FVector(), FRotator());
  return { Actor, false };
}

//NOT USED
bool AVegetationSpawner::StillInUse(const FTransform& Transform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::StillInUse);
  const FVector Location = Transform.GetLocation();
  for (const AActor* Vehicle : VehiclesInLevel)
  {
    const ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
    if (!IsValid(CarlaVehicle))
      continue;
    if (CarlaVehicle->IsInVehicleRange(Location))
      return true;
  }
  return false;
}

bool AVegetationSpawner::FindInCache(const FString& Path) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::FindInCache);
  const FFoliageBlueprintCache Aux(Path);
  for (const auto& Element : FoliageCache)
  {
    if (Aux.BPClassName == Element.BPClassName)
      return true;
  }
  return false;
}

FFoliageBlueprintCache AVegetationSpawner::GetBlueprintFromCache(const FString& Path) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetBlueprintFromCache);
  const FFoliageBlueprintCache Aux(Path);
  for (const auto& Element : FoliageCache)
  {
    if (Aux.BPClassName == Element.BPClassName)
      return Aux;
  }
  return FFoliageBlueprintCache();
}

bool AVegetationSpawner::IsFoliageTypeEnabled(const FString& Path)
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

void AVegetationSpawner::GetVehiclesInLevel()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetVehiclesInLevel);
  TArray<AActor*> FoundVehicles;
  const UObject* World = GetWorld();
  UGameplayStatics::GetAllActorsOfClass(World, ACarlaWheeledVehicle::StaticClass(), FoundVehicles);
  VehiclesInLevel = std::move(FoundVehicles);
}

bool AVegetationSpawner::AddFoliageToCache(const FFoliageBlueprintCache& NewElement)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::AddFoliageToCache);
  for (const FFoliageBlueprintCache& Foliage : FoliageCache)
  {
    if (Foliage.BPClassName == NewElement.BPClassName)
      return false;
  }
  FoliageCache.Add(NewElement);
  return true;
}

void AVegetationSpawner::GetFoliageTypesInLevel()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AVegetationSpawner::GetFoliageTypesInLevel);
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
      FFoliageBlueprintCache NewFoliageBlueprint(Path);
      if (!NewFoliageBlueprint.IsValid())
        continue;
      
      bool Added = AddFoliageToCache(NewFoliageBlueprint);
      if (!Added)
        continue;
      
      const int32 NumOfInstances = Mesh->GetInstanceCount();
      {
        TRACE_CPUPROFILER_EVENT_SCOPE(Init Foliage);        
        FLevelFoliage Foliage;
        Foliage.Mesh = Mesh;
        Foliage.Transforms.Init(FTransform(), NumOfInstances);
        Foliage.SpawnedActors.Init(nullptr, NumOfInstances);
        Foliage.IndicesInUse.Init(false, NumOfInstances);
        for (int32 i = 0; i < NumOfInstances; ++i)
          Mesh->GetInstanceTransform(i, Foliage.Transforms[i], true);
        FoliageTypesInLevel.Add(Foliage);        
      }
    }
  }

  for (const auto& Element : FoliageCache)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Element.BPClassName);
  }
  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Foliage Cache Begin");
}
