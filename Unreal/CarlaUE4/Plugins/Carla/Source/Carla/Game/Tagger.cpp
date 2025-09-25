// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Tagger.h"
#include "TaggedComponent.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Actor/CarlaActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"

namespace crp = carla::rpc;

template <typename T>
static auto CastEnum(T label)
{
  return static_cast<typename std::underlying_type<T>::type>(label);
}

crp::CityObjectLabel ATagger::GetLabelByFolderName(const FString &String) {
  if      (String == "Building")     return crp::CityObjectLabel::Buildings;
  else if (String == "Fence")        return crp::CityObjectLabel::Fences;
  else if (String == "Pedestrian")   return crp::CityObjectLabel::Pedestrians;
  else if (String == "Pole")         return crp::CityObjectLabel::Poles;
  else if (String == "Other")        return crp::CityObjectLabel::Other;
  else if (String == "Road")         return crp::CityObjectLabel::Roads;
  else if (String == "RoadLine")     return crp::CityObjectLabel::RoadLines;
  else if (String == "SideWalk")     return crp::CityObjectLabel::Sidewalks;
  else if (String == "TrafficSign")  return crp::CityObjectLabel::TrafficSigns;
  else if (String == "Vegetation")   return crp::CityObjectLabel::Vegetation;
  else if (String == "Car")          return crp::CityObjectLabel::Car;
  else if (String == "Wall")         return crp::CityObjectLabel::Walls;
  else if (String == "Sky")          return crp::CityObjectLabel::Sky;
  else if (String == "Ground")       return crp::CityObjectLabel::Ground;
  else if (String == "Bridge")       return crp::CityObjectLabel::Bridge;
  else if (String == "RailTrack")    return crp::CityObjectLabel::RailTrack;
  else if (String == "GuardRail")    return crp::CityObjectLabel::GuardRail;
  else if (String == "TrafficLight") return crp::CityObjectLabel::TrafficLight;
  else if (String == "Static")       return crp::CityObjectLabel::Static;
  else if (String == "Dynamic")      return crp::CityObjectLabel::Dynamic;
  else if (String == "Water")        return crp::CityObjectLabel::Water;
  else if (String == "Terrain")      return crp::CityObjectLabel::Terrain;
  else if (String == "Truck")        return crp::CityObjectLabel::Truck;
  else if (String == "Motorcycle")   return crp::CityObjectLabel::Motorcycle;
  else if (String == "Bicycle")      return crp::CityObjectLabel::Bicycle;
  else if (String == "Bus")          return crp::CityObjectLabel::Bus;
  else if (String == "Rider")        return crp::CityObjectLabel::Rider;
  else if (String == "Train")        return crp::CityObjectLabel::Train;
  else                               return crp::CityObjectLabel::None;
}

void ATagger::SetStencilValue(
    UPrimitiveComponent &Component,
    const crp::CityObjectLabel &Label,
    const bool bSetRenderCustomDepth) {
  Component.SetCustomDepthStencilValue(CastEnum(Label));
  Component.SetRenderCustomDepth(
      bSetRenderCustomDepth &&
      (Label != crp::CityObjectLabel::None));
}

bool ATagger::IsThing(const crp::CityObjectLabel &Label)
{
  return (Label == crp::CityObjectLabel::Pedestrians ||
          Label == crp::CityObjectLabel::TrafficSigns ||
          Label == crp::CityObjectLabel::Car ||
          Label == crp::CityObjectLabel::Train ||
          Label == crp::CityObjectLabel::Bicycle ||
          Label == crp::CityObjectLabel::Motorcycle ||
          Label == crp::CityObjectLabel::Bus ||
          Label == crp::CityObjectLabel::Rider ||
          Label == crp::CityObjectLabel::Truck ||
          Label == crp::CityObjectLabel::TrafficLight);
}

FLinearColor ATagger::GetLabelColor(const uint32_t ActorID, const crp::CityObjectLabel &Label)
{
  // TODO: Warn if id > 0xffff.

  // Encode label and id like semantic segmentation does
  // TODO: Steal bits from R channel and maybe A channel?
  FLinearColor Color(0.0f, 0.0f, 0.0f, 1.0f);
  Color.R = CastEnum(Label) / 255.0f;
  Color.G = ((ActorID & 0x000000ff) >> 0) / 255.0f;
  Color.B = ((ActorID & 0x0000ff00) >> 8) / 255.0f;

  return Color;
}


// =============================================================================
// -- static ATagger functions -------------------------------------------------
// =============================================================================

template<class T /* = UTaggedComponent*/>
T* ATagger::FindTaggedComponent(const USceneComponent* Component) {
  TArray<USceneComponent *> AttachedComponents = Component->GetAttachChildren();
  for (USceneComponent *SceneComponent : AttachedComponents) {
    T *TaggedSceneComponent = Cast<T>(SceneComponent);
    if (IsValid(TaggedSceneComponent)) {
        return TaggedSceneComponent;
    }
  }
  return NULL;
}

void ATagger::TagActor(const AActor &Actor, bool bTagForSemanticSegmentation, uint32_t ActorID)
{
#ifdef CARLA_TAGGER_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Actor: %s %d %d"), *Actor.GetName(), Actor.GetUniqueID(), ActorID);
#endif // CARLA_TAGGER_EXTRA_LOG

  // Iterate static meshes.
  TArray<UStaticMeshComponent *> StaticMeshComponents;
  Actor.GetComponents<UStaticMeshComponent>(StaticMeshComponents);
  for (UStaticMeshComponent *Component : StaticMeshComponents) {
    auto Label = GetLabelByPath(Component->GetStaticMesh());
    if (Label == crp::CityObjectLabel::Pedestrians &&
        Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + StaticMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG

    if(!Component->IsVisible() || !Component->GetStaticMesh())
    {
      continue;
    }

    // Find a tagged component that is attached to this component
    UTaggedComponent *TaggedComponent = FindTaggedComponent(Component);
    if (TaggedComponent) {
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Found Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    } else {
      // If not found, then create new tagged component and attach it to this component
      TaggedComponent = NewObject<UTaggedComponent>(Component);
      TaggedComponent->SetupAttachment(Component);
      TaggedComponent->RegisterComponent();
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Added Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    }

    // Set tagged component color
    FLinearColor Color = GetLabelColor(ActorID, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("    - Color: %s"), *Color.ToString());
#endif // CARLA_TAGGER_EXTRA_LOG

    TaggedComponent->SetColor(Color);
    TaggedComponent->MarkRenderStateDirty();
  }

  // Iterate skeletal meshes.
  TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
  Actor.GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
  for (USkeletalMeshComponent *Component : SkeletalMeshComponents) {
    auto Label = GetLabelByPath(Component->GetPhysicsAsset());
    if (Label == crp::CityObjectLabel::Pedestrians &&
        Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + SkeletalMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG

    if(!Component->IsVisible() || !Component->GetSkeletalMeshRenderData())
    {
      continue;
    }

    // Find a tagged component that is attached to this component
    UTaggedComponent *TaggedComponent = FindTaggedComponent(Component);
    if (TaggedComponent) {
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Found Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    } else {
      // If not found, then create new tagged component and attach it to this component
      TaggedComponent = NewObject<UTaggedComponent>(Component);
      TaggedComponent->SetupAttachment(Component);
      TaggedComponent->RegisterComponent();
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Added Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    }

    // Set tagged component color
    FLinearColor Color = GetLabelColor(ActorID, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("    - Color: %s"), *Color.ToString());
#endif // CARLA_TAGGER_EXTRA_LOG

    TaggedComponent->SetColor(Color);
    TaggedComponent->MarkRenderStateDirty();
    TaggedComponent->SetComponentTickEnabled(true);

  }

  // Iterate landscape components.
  TArray<ULandscapeComponent *> LandscapeComponents;
  Actor.GetComponents<ULandscapeComponent>(LandscapeComponents);
  for (ULandscapeComponent *Component : LandscapeComponents) {
    auto Label = crp::CityObjectLabel::Terrain;
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + LandscapeComponent: %s"), *Component->GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

    // Find a tagged component that is attached to this component
    UTaggedLandscapeComponent *TaggedComponent = FindTaggedComponent<UTaggedLandscapeComponent>(Component);
    if (TaggedComponent) {
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Found Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    } else {
      // If not found, then create new tagged component and attach it to this component
      TaggedComponent = NewObject<UTaggedLandscapeComponent>(Component);
      TaggedComponent->SetupAttachment(Component);
      TaggedComponent->RegisterComponent();
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Log, TEXT("    - Added Tag"));
#endif // CARLA_TAGGER_EXTRA_LOG
    }

    TaggedComponent->MarkRenderStateDirty();
  }
}

void ATagger::TagActorsInLevel(UWorld &World, const UCarlaEpisode &Episode, bool bTagForSemanticSegmentation)
{
  for (TActorIterator<AActor> it(&World); it; ++it) {
    uint32_t cActorId = (*it)->GetUniqueID();
    FCarlaActor* cActor = Episode.FindCarlaActor(*it);
    if (cActor) {
      cActorId = cActor->GetActorId();
    }
    TagActor(**it, bTagForSemanticSegmentation, cActorId);
  }
}

void ATagger::TagActorsInLevel(UWorld &World, bool bTagForSemanticSegmentation)
{
  for (TActorIterator<AActor> it(&World); it; ++it) {
    TagActor(**it, bTagForSemanticSegmentation, (*it)->GetUniqueID());
  }
}

void ATagger::TagActorsInLevel(ULevel &Level, const UCarlaEpisode &Episode, bool bTagForSemanticSegmentation)
{
  for (AActor * Actor : Level.Actors) {
    uint32_t cActorId = Actor->GetUniqueID();
    FCarlaActor* cActor = Episode.FindCarlaActor(Actor);
    if (cActor) {
      cActorId = cActor->GetActorId();
    }
    TagActor(*Actor, bTagForSemanticSegmentation, cActorId);
  }
}

void ATagger::GetTagsOfTaggedActor(const AActor &Actor, TSet<crp::CityObjectLabel> &Tags)
{
  TArray<UPrimitiveComponent *> Components;
  Actor.GetComponents<UPrimitiveComponent>(Components);
  for (auto *Component : Components) {
    if (Component != nullptr) {
      const auto Tag = GetTagOfTaggedComponent(*Component);
      if (Tag != crp::CityObjectLabel::None) {
        Tags.Add(Tag);
      }
    }
  }
}

FString ATagger::GetTagAsString(const crp::CityObjectLabel Label)
{
  switch (Label) {
#define CARLA_GET_LABEL_STR(lbl) case crp::CityObjectLabel:: lbl : return TEXT(#lbl);
    default:
    CARLA_GET_LABEL_STR(None)
    CARLA_GET_LABEL_STR(Buildings)
    CARLA_GET_LABEL_STR(Fences)
    CARLA_GET_LABEL_STR(Other)
    CARLA_GET_LABEL_STR(Pedestrians)
    CARLA_GET_LABEL_STR(Poles)
    CARLA_GET_LABEL_STR(RoadLines)
    CARLA_GET_LABEL_STR(Roads)
    CARLA_GET_LABEL_STR(Sidewalks)
    CARLA_GET_LABEL_STR(TrafficSigns)
    CARLA_GET_LABEL_STR(Vegetation)
    CARLA_GET_LABEL_STR(Car)
    CARLA_GET_LABEL_STR(Walls)
    CARLA_GET_LABEL_STR(Sky)
    CARLA_GET_LABEL_STR(Ground)
    CARLA_GET_LABEL_STR(Bridge)
    CARLA_GET_LABEL_STR(RailTrack)
    CARLA_GET_LABEL_STR(GuardRail)
    CARLA_GET_LABEL_STR(TrafficLight)
    CARLA_GET_LABEL_STR(Static)
    CARLA_GET_LABEL_STR(Dynamic)
    CARLA_GET_LABEL_STR(Water)
    CARLA_GET_LABEL_STR(Terrain)
    CARLA_GET_LABEL_STR(Truck)
    CARLA_GET_LABEL_STR(Motorcycle)
    CARLA_GET_LABEL_STR(Bicycle)
    CARLA_GET_LABEL_STR(Bus)
    CARLA_GET_LABEL_STR(Train)
    CARLA_GET_LABEL_STR(Rider)

#undef CARLA_GET_LABEL_STR
  }
}

// =============================================================================
// -- non-static ATagger functions ---------------------------------------------
// =============================================================================

ATagger::ATagger()
{
  PrimaryActorTick.bCanEverTick = false;
}

#if WITH_EDITOR
void ATagger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bTriggerTagObjects && (GetWorld() != nullptr)) {
      TagActorsInLevel(*GetWorld(), bTagForSemanticSegmentation);
    }
  }
  bTriggerTagObjects = false;
}
#endif // WITH_EDITOR
