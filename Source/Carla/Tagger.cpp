// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "Tagger.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"

enum class CityObjectLabel : uint8
{
  None         =   0u,
  Buildings    =   1u,
  Fences       =   2u,
  Other        =   3u,
  Pedestrians  =   4u,
  Poles        =   5u,
  RoadLines    =   6u,
  Roads        =   7u,
  Sidewalks    =   8u,
  Vegetation   =   9u,
  Vehicles     =  10u,
  Walls        =  11u,
};

#ifdef CARLA_TAGGER_EXTRA_LOG
static FString GetLabelAsString(const CityObjectLabel Label)
{
  switch (Label) {
#define CARLA_GET_LABEL_STR(lbl) case CityObjectLabel:: lbl : return #lbl;
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

static CityObjectLabel GetLabelByFolderName(const FString &String) {
  if      (String == "Buildings")       return CityObjectLabel::Buildings;
  else if (String == "Fences")          return CityObjectLabel::Fences;
  else if (String == "Pedestrians")     return CityObjectLabel::Pedestrians;
  else if (String == "Pole")            return CityObjectLabel::Poles;
  else if (String == "Props")           return CityObjectLabel::Other;
  else if (String == "Road")            return CityObjectLabel::Roads;
  else if (String == "RoadLines")       return CityObjectLabel::RoadLines;
  else if (String == "SideWalk")        return CityObjectLabel::Sidewalks;
  else if (String == "Vegetation")      return CityObjectLabel::Vegetation;
  else if (String == "Vehicles")        return CityObjectLabel::Vehicles;
  else if (String == "Walls")           return CityObjectLabel::Walls;
  else                                  return CityObjectLabel::None;
}

template <typename T>
static CityObjectLabel GetLabelByPath(const T *Object)
{
  const FString Path = Object->GetPathName();
  TArray<FString> StringArray;
  Path.ParseIntoArray(StringArray, TEXT("/"), false);
  return (StringArray.Num() > 3 ? GetLabelByFolderName(StringArray[3]) : CityObjectLabel::None);
}

static void SetStencilValue(UPrimitiveComponent *comp, const CityObjectLabel &Label) {
  if (Label != CityObjectLabel::None) {
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
