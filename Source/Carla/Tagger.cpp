// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "Tagger.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"

#ifdef CARLA_TAGGER_EXTRA_LOG
static FString GetLabelAsString(const ECityObjectLabel Label)
{
  switch (Label) {
#define CARLA_GET_LABEL_STR(lbl) case ECityObjectLabel:: lbl : return #lbl;
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
    CARLA_GET_LABEL_STR(Vegetation)
    CARLA_GET_LABEL_STR(Vehicles)
    CARLA_GET_LABEL_STR(Walls)
#undef CARLA_GET_LABEL_STR
  }
}
#endif // CARLA_TAGGER_EXTRA_LOG

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
  return (StringArray.Num() > 3 ? GetLabelByFolderName(StringArray[3]) : ECityObjectLabel::None);
}

static void SetStencilValue(UPrimitiveComponent *comp, const ECityObjectLabel &Label) {
  if (Label != ECityObjectLabel::None) {
    comp->SetRenderCustomDepth(true);
    comp->SetCustomDepthStencilValue(CastEnum(Label));
  }
}

// =============================================================================
// -- static ATagger functions -------------------------------------------------
// =============================================================================

void ATagger::TagActor(const AActor &Actor)
{
#ifdef CARLA_TAGGER_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("Actor: %s"), *Actor.GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

  // Iterate static meshes.
  TArray<UStaticMeshComponent *> StaticMeshComponents;
  Actor.GetComponents<UStaticMeshComponent>(StaticMeshComponents);
  for (UStaticMeshComponent *Component : StaticMeshComponents) {
    const auto Label = GetLabelByPath(Component->GetStaticMesh());
    SetStencilValue(Component, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + StaticMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetLabelAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG
  }

  // Iterate skeletal meshes.
  TArray<USkeletalMeshComponent *> SkeletalMeshComponents;
  Actor.GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
  for (USkeletalMeshComponent *Component : SkeletalMeshComponents) {
    const auto Label = GetLabelByPath(Component->GetPhysicsAsset());
    SetStencilValue(Component, Label);
#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("  + SkeletalMeshComponent: %s"), *Component->GetName());
    UE_LOG(LogCarla, Log, TEXT("    - Label: \"%s\""), *GetLabelAsString(Label));
#endif // CARLA_TAGGER_EXTRA_LOG
  }
}

void ATagger::TagActorsInLevel(UWorld &World)
{
  for (TActorIterator<AActor> it(&World); it; ++it) {
    TagActor(**it);
  }
}

ECityObjectLabel ATagger::GetTagOfTaggedComponent(const UPrimitiveComponent &Component)
{
  return
      (Component.bRenderCustomDepth ?
          static_cast<ECityObjectLabel>(Component.CustomDepthStencilValue) :
          ECityObjectLabel::None);
}

void ATagger::GetTagsOfTaggedActor(const AActor &Actor, TArray<ECityObjectLabel> &Tags)
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
      TagActorsInLevel(*GetWorld());
    }
  }
  bTriggerTagObjects = false;
}
#endif // WITH_EDITOR
