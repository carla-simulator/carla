// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#include "Carla/Vegetation/VegetationSpawner.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
 	

#include "ProceduralFoliageVolume.h"
#include "Kismet/GameplayStatics.h"

void AVegetationSpawner::BeginPlay()
{
  Super::BeginPlay();
  CheckForProcedurals();
}

void AVegetationSpawner::CheckForProcedurals()
{
  const UObject* World = GetWorld();
  TArray<AActor*> Spawners;
  TArray<FSpawnedFoliage> FrameFound;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Spawners);
  for (const AActor* Spawner : Spawners)
  {
    const TSet<UActorComponent*>& Components = Spawner->GetComponents();
    for (UActorComponent* Component : Components)
    {
      UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component);
      if (Mesh == nullptr)
        continue;
      if (IsInstancedStaticMeshComponentLoaded(Mesh))
        continue;
      const int32 NumOfInstances = Mesh->GetInstanceCount();
      TArray<int32> SpawnerOverlappingInstances;
      TArray<FTransform> SpawnerOverlappingTransforms;
      TArray<AActor*> SpawnedActors;
      SpawnerOverlappingInstances.Init(0, NumOfInstances);
      SpawnerOverlappingTransforms.SetNum(NumOfInstances);
      SpawnedActors.Init(nullptr, NumOfInstances);
      
      for (int32 i = 0; i < NumOfInstances; ++i)
      {
        FTransform Transform;
        Mesh->GetInstanceTransform(i, Transform, true);
        SpawnerOverlappingTransforms[i] = Transform;
      }
      FSpawnedFoliage Aux;
      Aux.InUse = true;
      Aux.Mesh = Mesh;
      Aux.Indices = SpawnerOverlappingInstances;
      Aux.Transforms = SpawnerOverlappingTransforms;
      Aux.SpawnedActors = SpawnedActors;
      ProceduralInstances.Add(Aux);
      FrameFound.Add(Aux);
    }
  }

  for (auto& Element : ProceduralInstances)
  {
    if (Element.InUse == false)
      continue;
    bool Found = false;
    for (const FSpawnedFoliage& NewElement : FrameFound)
    {
      if (Element.Mesh == NewElement.Mesh)
      {
        Found = true;
        break;
      }
    }
    Element.InUse = Found;
  }
}

void AVegetationSpawner::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  UpdateVehiclesInLevel();
  if (VehiclesInLevel.Num() == 0)
  {
    if (Defaulted == false)
      Default();
    Defaulted = true;
    return;
  }
  CheckForProcedurals();
  Defaulted = false;
  UpdateProceduralInstanceCount();

}

void AVegetationSpawner::UpdateVehiclesInLevel()
{
  TArray<AActor*> FoundVehicles;
  const UObject* World = GetWorld();
  UGameplayStatics::GetAllActorsOfClass(World, ACarlaWheeledVehicle::StaticClass(), FoundVehicles);
  VehiclesInLevel = FoundVehicles;
}

void AVegetationSpawner::UpdateProceduralInstanceCount()
{
  TArray<int32> ProceduralInstanceCount;
  for (FSpawnedFoliage& Foliage : ProceduralInstances)
  {
    if (!Foliage.InUse)
      continue;
    const int32 NumOfInstances = Foliage.Mesh->GetInstanceCount();
    TArray<int32> SpawnerOverlappingInstances;
    SpawnerOverlappingInstances.Init(0, NumOfInstances);

    for (int32 i = 0; i < NumOfInstances; ++i)
    {
      const FVector Location = Foliage.Transforms[i].GetLocation();      
      for (const AActor* Vehicle : VehiclesInLevel)
      {
        const ACarlaWheeledVehicle* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Vehicle);
        if (CarlaVehicle == nullptr)
          continue;
        if (CarlaVehicle->IsInVehicleRange(Location))
        {
          SpawnerOverlappingInstances[i]++;
        }           
      }
    }    
    UpdateFoliage(Foliage, SpawnerOverlappingInstances);
  }
}

void AVegetationSpawner::UpdateFoliage(FSpawnedFoliage& Foliage, TArray<int32>& VehiclesDetection)
{
  auto FoliageIterator = Foliage.Indices.CreateIterator();
  auto VehicleDetectionIterator = VehiclesDetection.CreateConstIterator();
  for (int32 i = 0; FoliageIterator && VehicleDetectionIterator; ++i, ++FoliageIterator, ++VehicleDetectionIterator)
  {
    if (*FoliageIterator > 0 && *VehicleDetectionIterator == 0)
    {
      *FoliageIterator = 0;
      if (Foliage.SpawnedActors[i])
      {
        Foliage.SpawnedActors[i]->Destroy();
        Foliage.SpawnedActors[i] = nullptr;
      }
      Foliage.Mesh->UpdateInstanceTransform(i, Foliage.Transforms[i], true, true, true);
    }
    else if (*FoliageIterator == 0 && *VehicleDetectionIterator > 0)
    {
      const FString Path = Foliage.Mesh->GetStaticMesh()->GetPathName();
      if (!IsFoliageTypeEnabled(Path))
        continue;
      const FFoliageBlueprintCache FullClassName = GetClassFromPath(Path);
      if (FullClassName.SpawnedClass == nullptr)
        continue;
      *FoliageIterator = *VehicleDetectionIterator;
      Foliage.Mesh->UpdateInstanceTransform(i, {}, true, true, true);
      Foliage.SpawnedActors[i] = SpawnFoliage(FullClassName, Foliage.Transforms[i]);
    }
  }
}

  FFoliageBlueprintCache AVegetationSpawner::GetClassFromPath(const FString& Path)
  {    
    FFoliageBlueprintCache Result = FindInCache(Path);
    if (!Result.Path.IsEmpty())
      return Result;

    Result.Path = Path;

    TArray< FString > ParsedString;
    Path.ParseIntoArray(ParsedString, TEXT("/"), false);
    int Position = ParsedString.Num() - 1;
    const FString Version = ParsedString[Position];
    --Position;
    const FString Folder = ParsedString[Position];
    ++Position;
    const FString FullVersion = GetVersionFromFString(Version);
    if (FullVersion.IsEmpty())
    {
      Result.BPClassName = Path;
      FoliageCache.Add(Result);
      return Result;
    }
    FString ClassPath = "BP_" + Folder + FullVersion;
    FString FullClassPath = "Blueprint'";
    for (int i = 0; i < Position; ++i)
    {
      FullClassPath += ParsedString[i];
      FullClassPath += '/';
    }
    FullClassPath += ClassPath;
    FullClassPath += ".";
    FullClassPath += ClassPath;
    FullClassPath += "'";

    UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FullClassPath);
    UBlueprint* CastedBlueprint = Cast<UBlueprint>(LoadedObject);

    Result.GetBPName();
             
    if (CastedBlueprint && CastedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
    {
      Result.SpawnedClass = *CastedBlueprint->GeneratedClass;
    }

    FoliageCache.Add(Result);

    return Result;
  }

AActor* AVegetationSpawner::SpawnFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform)
  {
    AActor* Actor = GetWorld()->SpawnActor<AActor>(CacheBPClass.SpawnedClass, FoliageTransform.GetLocation(), FoliageTransform.Rotator());
    if (Actor)
    {
      if (SpawnScale > 1.001f || SpawnScale < 0.999f)
        Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
      else
        Actor->SetActorScale3D(FoliageTransform.GetScale3D());
    }
    return Actor;
  }

void AVegetationSpawner::Default()
{
  for (FSpawnedFoliage& Foliage : ProceduralInstances)
  {
    const int32 NumOfInstances = Foliage.Mesh->GetInstanceCount();
    Foliage.InUse = false;
    Foliage.Indices.Init(0, NumOfInstances);
    Foliage.SpawnedActors.Init(nullptr, NumOfInstances);
  }
}

FFoliageBlueprintCache AVegetationSpawner::FindInCache(const FString& Path)
{
  for (const FFoliageBlueprintCache& Element : FoliageCache)
  {
    if (Element.Path == Path)
    {
      return Element;
    }
  }
  return FFoliageBlueprintCache();
}

bool AVegetationSpawner::IsFoliageTypeEnabled(const FString& Path)
{
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

FString AVegetationSpawner::GetVersionFromFString(const FString& string)
{
  auto IsDigit = [](TCHAR charToTest) {
      if (charToTest == TCHAR('0')) return true;
      if (charToTest == TCHAR('1')) return true;
      if (charToTest == TCHAR('2')) return true;
      if (charToTest == TCHAR('3')) return true;
      if (charToTest == TCHAR('4')) return true;
      if (charToTest == TCHAR('5')) return true;
      if (charToTest == TCHAR('6')) return true;
      if (charToTest == TCHAR('7')) return true;
      if (charToTest == TCHAR('8')) return true;
      if (charToTest == TCHAR('9')) return true;
      return false;
  };
  int index = string.Find(TEXT("_v"));
  if (index != -1)
  {
    index += 2;
    FString Version = "_v";
    while(IsDigit(string[index]))
    {
      Version += string[index];
      ++index;
      if (index == string.Len())
        return Version;
    }
    return Version;
  }
  return FString();
}

bool AVegetationSpawner::IsInstancedStaticMeshComponentLoaded(const UInstancedStaticMeshComponent* Mesh)
{
  for(const FSpawnedFoliage& Instance : ProceduralInstances)
  {
    if (Instance.Mesh == Mesh && Instance.InUse == true)
    {
      return true;
    }
  }
  return false;
}

