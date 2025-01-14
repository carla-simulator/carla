// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include <util/ue-header-guard-end.h>

#include "Tagger.generated.h"

namespace crp = carla::rpc;

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

  static void TagActorsInLevel(ULevel &Level, bool bTagForSemanticSegmentation);

  /// Retrieve the tag of an already tagged component.
  static crp::CityObjectLabel GetTagOfTaggedComponent(const UPrimitiveComponent &Component);
  
  /// Retrieve the tags of an already tagged actor. CityObjectLabel::None is
  /// not added to the array.
  static void GetTagsOfTaggedActor(const AActor &Actor, TSet<crp::CityObjectLabel> &Tags);

  /// Return true if @a Component has been tagged with the given @a Tag.
  static bool MatchComponent(const UPrimitiveComponent &Component, crp::CityObjectLabel Tag)
  {
    return (Tag == GetTagOfTaggedComponent(Component));
  }

  /// Transform a string into a CityObjectLabel.
  static crp::CityObjectLabel GetTagFromString(FString Tag);

  /// Transform a CityObjectLabel into a string.
  static FString GetTagAsString(crp::CityObjectLabel Tag);

  /// Method that computes the label corresponding to a folder path
  static crp::CityObjectLabel GetLabelByFolderName(const FString &String);

  /// Method that computes the label corresponding to an specific object
  /// using the folder path in which it is stored
  template <typename T>
  static crp::CityObjectLabel GetLabelByPath(const T* Object)
  {
    const FString Path = Object->GetPathName();
    TArray<FString> StringArray;
    Path.ParseIntoArray(StringArray, TEXT("/"), false);
    if(Path.Contains("UE5UseOnly"))
    {
      return (StringArray.Num() > 5 ? GetLabelByFolderName(StringArray[5]) : crp::CityObjectLabel::None);
    }
    else
    {
      return (StringArray.Num() > 4 ? GetLabelByFolderName(StringArray[4]) : crp::CityObjectLabel::None);
    }
  }

  /// Method that computes the label corresponding to an specific object
  /// using the folder path in which it is stored
  template <typename T>
  static crp::CityObjectLabel GetLabelByPath(TObjectPtr<T> Object)
  {
      return GetLabelByPath(Object.Get());
  }

  //TODO: Change function name to reflect the new functionality
  static void SetStencilValue(UPrimitiveComponent &Component, const uint32 ActorID,
    const crp::CityObjectLabel &Label, const bool bSetRenderCustomDepth);

  static FLinearColor GetActorLabelColor(const AActor &Actor, const crp::CityObjectLabel &Label);

  static bool IsThing(const crp::CityObjectLabel &Label);

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
