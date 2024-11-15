// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CityMapMeshHolder.h"
#include "Carla.h"
#include "Settings/CarlaSettings.h"
#include "Game/CarlaGameInstance.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Level.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include <util/ue-header-guard-end.h>

#include <vector>


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

  if (IsValid(GetLevel()) && IsValidChecked(GetLevel()))
  {
	 TArray<AActor*> roadpieces;
     GetAttachedActors(roadpieces);
	 if (roadpieces.Num()==0)
	 {
	   UE_LOG(LogCarla, Error, TEXT("Please regenerate the road in edit mode for '%s' actor"), *UKismetSystemLibrary::GetDisplayName(this));
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
    DeletePieces();
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
  FActorSpawnParameters params;
  params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  FVector location = Transform.GetLocation();
  FRotator rotation = Transform.Rotator();
  AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(GetWorld()->SpawnActor(AStaticMeshActor::StaticClass(),&location,&rotation,params));
  StaticMeshActor->AttachToActor(this,FAttachmentTransformRules::KeepRelativeTransform);
  StaticMeshActor->SetMobility(EComponentMobility::Static);
  UStaticMeshComponent* staticmeshcomponent = StaticMeshActor->GetStaticMeshComponent();
  staticmeshcomponent->SetMobility(EComponentMobility::Static);
  staticmeshcomponent->SetStaticMesh(GetStaticMesh(Tag));
  StaticMeshActor->Tags.Add(UCarlaSettings::CARLA_ROAD_TAG);
  StaticMeshActor->bEnableAutoLODGeneration = true;
}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

void ACityMapMeshHolder::UpdateMap() {}

void ACityMapMeshHolder::DeletePieces()
{
  //this part will be deprecated: remove the instanced static mesh components
  TArray<UInstancedStaticMeshComponent*> oldcomponents;
  GetComponents(oldcomponents);
  for (int32 i=0;i<oldcomponents.Num();i++)
  {
	  oldcomponents[i]->DestroyComponent();
  }

  TArray<AActor*> roadpieces;
  GetAttachedActors(roadpieces);

  for (int32 i=roadpieces.Num()-1; i>=0; i--)
  {
	if (roadpieces[i]->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG))
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


