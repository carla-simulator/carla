// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CityMapMeshHolder.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include <vector>
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"



using tag_size_t = std::underlying_type<ECityMapMeshTag>::type;

constexpr static tag_size_t NUMBER_OF_TAGS = CityMapMeshTag::GetNumberOfTags();

// =============================================================================
// -- Construction and update related methods ----------------------------------
// =============================================================================

ACityMapMeshHolder::ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;

  SceneRootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
  SceneRootComponent->SetMobility(EComponentMobility::Static);
  RootComponent = SceneRootComponent;

  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    // Add static mesh holder.
    StaticMeshes.Add(CityMapMeshTag::FromUInt(i));
  }

}

/*void ACityMapMeshHolder::LayoutDetails(IDetailLayoutBuilder& DetailLayout)
{
 IDetailCategoryBuilder& DetailCategory = DetailLayout.EditCategory("Rendering");
 IDetailPropertyRow& row = DetailCategory.AddProperty("Generation", TEXT(""));
 
}*/

void ACityMapMeshHolder::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);

}

void ACityMapMeshHolder::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  if(IsValid(GetLevel())&&!GetLevel()->IsPendingKill())
  {
	if (MeshInstatiators.Num() == 0) {
     ResetInstantiators();
     UpdateMapScale();
     UpdateMap();
    }
	  
  }
  
}

#if WITH_EDITOR
void ACityMapMeshHolder::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) 
  { 
    ResetInstantiators();
    UpdateMapScale();
    UpdateMap();
   }
}
#endif // WITH_EDITOR

// =============================================================================
// -- Other protected methods --------------------------------------------------
// =============================================================================

FVector ACityMapMeshHolder::GetTileLocation(uint32 X, uint32 Y) const
{
  return {X * MapScale, Y * MapScale, 0.0f};
}

void ACityMapMeshHolder::SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh)
{
  StaticMeshes[Tag] = Mesh;
  if (Mesh != nullptr) {
    TagMap.Add(Mesh, Tag);
  }
}

UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag)
{
  return StaticMeshes[Tag];
}

const UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag) const
{
  return StaticMeshes[Tag];
}

ECityMapMeshTag ACityMapMeshHolder::GetTag(const UStaticMesh &StaticMesh) const
{
  const ECityMapMeshTag *Tag = TagMap.Find(&StaticMesh);
  return (Tag != nullptr ? *Tag : ECityMapMeshTag::INVALID);
}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y)
{
  AddInstance(Tag, FTransform(GetTileLocation(X, Y)));
}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y, float Angle)
{
  const FQuat rotation(FVector(0.0f, 0.0f, 1.0f), Angle);
  const FVector location = GetTileLocation(X, Y);
  AddInstance(Tag, FTransform(rotation, location));

}

void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, FTransform Transform)
{
  auto &instantiator = GetInstantiator(Tag);
  static int32 componentnumber = 0;
  FActorSpawnParameters params;
  params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  FVector location = Transform.GetLocation();
  FRotator rotation = Transform.Rotator();
  AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(GetWorld()->SpawnActor(AStaticMeshActor::StaticClass(),&location,&rotation,params));
  StaticMeshActor->AttachToActor(this,FAttachmentTransformRules::KeepRelativeTransform);
  StaticMeshActor->SetMobility(EComponentMobility::Static);
  UStaticMeshComponent* staticmeshcomponent = StaticMeshActor->GetStaticMeshComponent();
  staticmeshcomponent->SetMobility(EComponentMobility::Static);
  staticmeshcomponent->SetStaticMesh(instantiator.GetStaticMesh());
  
  //occlussion 
  StaticMeshActor->Tags.Add(FName("CARLA_ROAD"));
  staticmeshcomponent->bAllowCullDistanceVolume=true;
  staticmeshcomponent->bUseAsOccluder=false;
  staticmeshcomponent->LDMaxDrawDistance = MaxDrawDistance;
  staticmeshcomponent->CastShadow=false;
  StaticMeshActor->bEnableAutoLODGeneration=true;
  
}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

void ACityMapMeshHolder::UpdateMap() {}

void ACityMapMeshHolder::ResetInstantiators()
{
  for (auto *instantiator : MeshInstatiators) {
    if (instantiator != nullptr) {
      instantiator->ClearInstances();
    }
  }
  if (MeshInstatiators.Num() != NUMBER_OF_TAGS) {
    MeshInstatiators.Empty();
    MeshInstatiators.Init(nullptr, NUMBER_OF_TAGS);
  }
  check(MeshInstatiators.Num() == NUMBER_OF_TAGS);
  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    auto &instantiator = GetInstantiator(CityMapMeshTag::FromUInt(i));
    instantiator.SetStaticMesh(GetStaticMesh(CityMapMeshTag::FromUInt(i)));
  }

  const FName roadtag = FName("CARLA_ROAD");
  TArray<AActor*> roadpieces;
  GetAttachedActors(roadpieces);
  
  for(int32 i=roadpieces.Num()-1; i>=0; i--)
  {
	
	//UE_LOG(LogTemp,Display,TEXT("ACityMapMeshHolder::ResetInstantiators Destroyed %s"),*Children[i]->GetName());
	if(roadpieces[i]->Tags.Contains(roadtag))
	{
	   roadpieces[i]->Destroy();
	}
  }
  
}

void ACityMapMeshHolder::UpdateMapScale()
{
  auto Tag = CityMapMeshTag::GetBaseMeshTag();
  auto *mesh = GetStaticMesh(Tag);
  if (mesh == nullptr) {
    UE_LOG(
        LogCarla,
        Error,
        TEXT("Cannot find mesh \"%s\" for computing tile size"),
        *CityMapMeshTag::ToString(Tag));
    MapScale = 1.0f;
  } else {
    FVector size = mesh->GetBoundingBox().GetSize();
    MapScale = size.X;
  }
}

UInstancedStaticMeshComponent &ACityMapMeshHolder::GetInstantiator(ECityMapMeshTag Tag)
{
  UInstancedStaticMeshComponent *instantiator = MeshInstatiators[CityMapMeshTag::ToUInt(Tag)];
  if (instantiator == nullptr) {
    // Create and register an instantiator.
    instantiator = NewObject<UInstancedStaticMeshComponent>(this);
    instantiator->SetMobility(EComponentMobility::Static);
    instantiator->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    instantiator->SetupAttachment(SceneRootComponent);
    instantiator->SetStaticMesh(GetStaticMesh(Tag));
    MeshInstatiators[CityMapMeshTag::ToUInt(Tag)] = instantiator;
    instantiator->RegisterComponent();
  }
  check(instantiator != nullptr);
  return *instantiator;
}
