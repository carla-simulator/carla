// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Tagger.h"
#include "Carla.h"
#include "Vehicle/CarlaWheeledVehicle.h"

#include <util/ue-header-guard-begin.h>
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include <util/ue-header-guard-end.h>

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
  else if (String == "Rock")         return crp::CityObjectLabel::Rock;
  else if (String == "Stone")        return crp::CityObjectLabel::Rock;
  else if (String == "Bush")         return crp::CityObjectLabel::Vegetation;
  else                               return crp::CityObjectLabel::None;
}

void ATagger::SetStencilValue( UPrimitiveComponent &Component, const uint32 ActorID, const crp::CityObjectLabel &Label, const bool bSetRenderCustomDepth) 
{

static TMap<unsigned short, unsigned int> TruncatedIDUsedDebug;

#ifdef CARLA_TAGGER_EXTRA_LOG
  unsigned short truncated_id = static_cast<unsigned short>(ActorID);
  if(TruncatedIDUsedDebug.Contains(truncated_id)){
    UE_LOG(LogCarla, Warning, TEXT("Warning: The Instance ID %d has been used %d time/s previously on this scene"), truncated_id, TruncatedIDUsedDebug[truncated_id]);
    TruncatedIDUsedDebug[truncated_id]++;
  }
  else
  {
    TruncatedIDUsedDebug.Emplace(truncated_id, 1);
  }
#endif //CARLA_TAGGER_EXTRA_LOG

  Component.SetCustomPrimitiveDataVector4(4, FVector4((float) ActorID, (float) CastEnum(Label), 0.0f, 0.0f));
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

FLinearColor ATagger::GetActorLabelColor(const AActor &Actor, const crp::CityObjectLabel &Label)
{
  uint32 id = Actor.GetUniqueID();
  // TODO: Warn if id > 0xffff.

  // Encode label and id like semantic segmentation does
  // TODO: Steal bits from R channel and maybe A channel?
  FLinearColor Color(0.0f, 0.0f, 0.0f, 1.0f);
  Color.R = CastEnum(Label) / 255.0f;
  Color.G = ((id & 0x00ff) >> 0) / 255.0f;
  Color.B = ((id & 0xff00) >> 8) / 255.0f;

  return Color;
}


// =============================================================================
// -- static ATagger functions -------------------------------------------------
// =============================================================================

void ATagger::TagActor(const AActor &Actor, bool bTagForSemanticSegmentation)
{
#ifdef CARLA_TAGGER_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Actor: %s"), *Actor.GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

  // Iterate static meshes.
  TArray<UStaticMeshComponent *> StaticMeshComponents;
  Actor.GetComponents<UStaticMeshComponent>(StaticMeshComponents);

  for (UStaticMeshComponent *Component : StaticMeshComponents) 
  {
    auto Label = GetLabelByPath(Component->GetStaticMesh());
    if (Label == crp::CityObjectLabel::Pedestrians && Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }

    SetStencilValue(*Component, Actor.GetUniqueID(), Label, bTagForSemanticSegmentation);
    Component->ComponentTags.Add(FName(*GetTagAsString(Label)));
    #ifdef CARLA_TAGGER_EXTRA_LOG
        UE_LOG(LogCarla, Log, TEXT("  + StaticMeshComponent: %s"), *Component->GetName());
        UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
    #endif // CARLA_TAGGER_EXTRA_LOG
  }
  
  // Iterate skeletal meshes.
  TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
  Actor.GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

  for (USkeletalMeshComponent *Component : SkeletalMeshComponents) {
    auto Label = GetLabelByPath(Component->GetPhysicsAsset());
    if (Label == crp::CityObjectLabel::Pedestrians && Cast<ACarlaWheeledVehicle>(&Actor))
    {
      Label = crp::CityObjectLabel::Rider;
    }

    SetStencilValue(*Component, Actor.GetUniqueID(), Label, bTagForSemanticSegmentation);
    Component->ComponentTags.Add(FName(*GetTagAsString(Label)));
    #ifdef CARLA_TAGGER_EXTRA_LOG
        UE_LOG(LogCarla, Log, TEXT("  + SkeletalMeshComponent: %s"), *Component->GetName());
        UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
    #endif // CARLA_TAGGER_EXTRA_LOG
  }
  
}

void ATagger::TagActorsInLevel(UWorld &World, bool bTagForSemanticSegmentation)
{
  for (TActorIterator<AActor> it(&World); it; ++it) {
    TagActor(**it, bTagForSemanticSegmentation);
  }
}


void ATagger::TagActorsInLevel(ULevel &Level, bool bTagForSemanticSegmentation)
{
  for (AActor * Actor : Level.Actors) {
    TagActor(*Actor, bTagForSemanticSegmentation);
  }
}

crp::CityObjectLabel ATagger::GetTagOfTaggedComponent(const UPrimitiveComponent &Component)
{
  if (Component.ComponentTags.Num() > 0) {
    return GetTagFromString(Component.ComponentTags[0].ToString());
  }
  return static_cast<crp::CityObjectLabel>(Component.CustomDepthStencilValue);
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

crp::CityObjectLabel ATagger::GetTagFromString(FString Tag)
{
  if(Tag.Contains("Building")) return crp::CityObjectLabel::Buildings;
  if(Tag.Contains("Fence")) return crp::CityObjectLabel::Fences;
  if(Tag.Contains("Pedestrian")) return crp::CityObjectLabel::Pedestrians;
  if(Tag.Contains("Pole")) return crp::CityObjectLabel::Poles;
  if(Tag.Contains("Other")) return crp::CityObjectLabel::Other;
  if(Tag.Contains("Roads")) return crp::CityObjectLabel::Roads;
  if(Tag.Contains("RoadLine")) return crp::CityObjectLabel::RoadLines;
  if(Tag.Contains("SideWalk")) return crp::CityObjectLabel::Sidewalks;
  if(Tag.Contains("TrafficSign")) return crp::CityObjectLabel::TrafficSigns;
  if(Tag.Contains("Vegetation")) return crp::CityObjectLabel::Vegetation;
  if(Tag.Contains("Car")) return crp::CityObjectLabel::Car;
  if(Tag.Contains("Wall")) return crp::CityObjectLabel::Walls;
  if(Tag.Contains("Sky")) return crp::CityObjectLabel::Sky;
  if(Tag.Contains("Ground")) return crp::CityObjectLabel::Ground;
  if(Tag.Contains("Bridge")) return crp::CityObjectLabel::Bridge;
  if(Tag.Contains("RailTrack")) return crp::CityObjectLabel::RailTrack;
  if(Tag.Contains("GuardRail")) return crp::CityObjectLabel::GuardRail;
  if(Tag.Contains("TrafficLight")) return crp::CityObjectLabel::TrafficLight;
  if(Tag.Contains("Static")) return crp::CityObjectLabel::Static;
  if(Tag.Contains("Dynamic")) return crp::CityObjectLabel::Dynamic;
  if(Tag.Contains("Water")) return crp::CityObjectLabel::Water;
  if(Tag.Contains("Terrain")) return crp::CityObjectLabel::Terrain;
  if(Tag.Contains("Truck")) return crp::CityObjectLabel::Truck;
  if(Tag.Contains("Motorcycle")) return crp::CityObjectLabel::Motorcycle;
  if(Tag.Contains("Bicycle")) return crp::CityObjectLabel::Bicycle;
  if(Tag.Contains("Bus")) return crp::CityObjectLabel::Bus;
  if(Tag.Contains("Rider")) return crp::CityObjectLabel::Rider;
  if(Tag.Contains("Train")) return crp::CityObjectLabel::Train;
  if(Tag.Contains("Rock")) return crp::CityObjectLabel::Rock;
  return crp::CityObjectLabel::None;
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
    CARLA_GET_LABEL_STR(Rock)

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
