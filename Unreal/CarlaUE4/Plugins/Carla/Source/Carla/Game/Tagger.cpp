// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Tagger.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"

template <typename T>
static auto CastEnum(T label)
{
  return static_cast<typename std::underlying_type<T>::type>(label);
}

static ECityObjectLabel GetLabelByFolderName(const FString &String) {
  if      (String == "Buildings")       return ECityObjectLabel::Buildings;
  else if (String == "Fences")          return ECityObjectLabel::Fences;
  else if (String == "Pedestrians")     return ECityObjectLabel::Pedestrians;
  else if (String == "Pole")            return ECityObjectLabel::Poles;
  else if (String == "Props")           return ECityObjectLabel::Other;
  else if (String == "Road")            return ECityObjectLabel::Roads;
  else if (String == "RoadLines")       return ECityObjectLabel::RoadLines;
  else if (String == "SideWalk")        return ECityObjectLabel::Sidewalks;
  else if (String == "TrafficSigns")    return ECityObjectLabel::TrafficSigns;
  else if (String == "Vegetation")      return ECityObjectLabel::Vegetation;
  else if (String == "Vehicles")        return ECityObjectLabel::Vehicles;
  else if (String == "Walls")           return ECityObjectLabel::Walls;
  else                                  return ECityObjectLabel::None;
}

template <typename T>
static ECityObjectLabel GetLabelByPath(const T *Object)
{
  const FString Path = Object->GetPathName();
  TArray<FString> StringArray;
  Path.ParseIntoArray(StringArray, TEXT("/"), false);
  return (StringArray.Num() > 4 ? GetLabelByFolderName(StringArray[4]) : ECityObjectLabel::None);
}

static void SetStencilValue(
    UPrimitiveComponent &Component,
    const ECityObjectLabel &Label,
    const bool bSetRenderCustomDepth) {
  Component.SetCustomDepthStencilValue(CastEnum(Label));
  Component.SetRenderCustomDepth(
      bSetRenderCustomDepth &&
      (Label != ECityObjectLabel::None));
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
  for (UStaticMeshComponent *Component : StaticMeshComponents) {
    const auto Label = GetLabelByPath(Component->GetStaticMesh());
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + StaticMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetTagAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG
  }

  // Iterate skeletal meshes.
  TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
  Actor.GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
  for (USkeletalMeshComponent *Component : SkeletalMeshComponents) {
    const auto Label = GetLabelByPath(Component->GetPhysicsAsset());
    SetStencilValue(*Component, Label, bTagForSemanticSegmentation);
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

void ATagger::GetTagsOfTaggedActor(const AActor &Actor, TSet<ECityObjectLabel> &Tags)
{
  TArray<UPrimitiveComponent *> Components;
  Actor.GetComponents<UPrimitiveComponent>(Components);
  for (auto *Component : Components) {
    if (Component != nullptr) {
      const auto Tag = GetTagOfTaggedComponent(*Component);
      if (Tag != ECityObjectLabel::None) {
        Tags.Add(Tag);
      }
    }
  }
}

FString ATagger::GetTagAsString(const ECityObjectLabel Label)
{
  switch (Label) {
#define CARLA_GET_LABEL_STR(lbl) case ECityObjectLabel:: lbl : return TEXT(#lbl);
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
    CARLA_GET_LABEL_STR(Vehicles)
    CARLA_GET_LABEL_STR(Walls)
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
