// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Tagger.generated.h"

enum class ECityObjectLabel : uint8
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
  TrafficSigns =  12u,
  Vegetation   =   9u,
  Vehicles     =  10u,
  Walls        =  11u,
};

/// Sets actors' custom depth stencil value for semantic segmentation according
/// to their meshes.
///
/// Non-static functions present so it can be dropped into the scene for testing
/// purposes.
UCLASS()
class CARLA_API ATagger : public AActor
{
  GENERATED_BODY()

public:

  /// Set the tag of an actor.
  ///
  /// If bTagForSemanticSegmentation true, activate the custom depth pass. This
  /// pass is necessary for rendering the semantic segmentation. However, it may
  /// add a performance penalty since occlusion doesn't seem to be applied to
  /// objects having this value active.
  static void TagActor(const AActor &Actor, bool bTagForSemanticSegmentation);

  /// Set the tag of every actor in level.
  ///
  /// If bTagForSemanticSegmentation true, activate the custom depth pass. This
  /// pass is necessary for rendering the semantic segmentation. However, it may
  /// add a performance penalty since occlusion doesn't seem to be applied to
  /// objects having this value active.
  static void TagActorsInLevel(UWorld &World, bool bTagForSemanticSegmentation);

  /// Retrieve the tag of an already tagged component.
  static ECityObjectLabel GetTagOfTaggedComponent(const UPrimitiveComponent &Component)
  {
    return static_cast<ECityObjectLabel>(Component.CustomDepthStencilValue);
  }

  /// Retrieve the tags of an already tagged actor. ECityObjectLabel::None is
  /// not added to the array.
  static void GetTagsOfTaggedActor(const AActor &Actor, TSet<ECityObjectLabel> &Tags);

  /// Return true if @a Component has been tagged with the given @a Tag.
  static bool MatchComponent(const UPrimitiveComponent &Component, ECityObjectLabel Tag)
  {
    return (Tag == GetTagOfTaggedComponent(Component));
  }

  static FString GetTagAsString(ECityObjectLabel Tag);

  ATagger();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTriggerTagObjects = false;

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTagForSemanticSegmentation = false;
};
