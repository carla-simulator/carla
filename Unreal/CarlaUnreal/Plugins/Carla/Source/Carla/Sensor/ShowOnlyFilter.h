// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include <util/ue-header-guard-end.h>

class AActor;
class UCarlaEpisode;
class USceneCaptureComponent2D;
class UWorld;

/// Show-only ("render layer") filter shared by every scene-capture camera.
///
/// Selected through three blueprint attributes:
///
///   show_only_tags      comma separated CARLA semantic label names
///                       ("Car,Truck,Bus,Motorcycle,Bicycle,Pedestrians").
///   shadow_catcher_tags same format; actors carrying these labels are
///                       rendered too so they can receive the cast shadows
///                       of the show-only actors ("Roads,Sidewalks").
///   show_only_actor_ids comma separated CARLA actor ids, added to the list
///                       whatever their labels.
///
/// When show_only_tags or show_only_actor_ids is non-empty the capture
/// components switch to PRM_UseShowOnlyList and their ShowOnlyActors list is
/// the set of world actors carrying at least one of the labels (any
/// primitive component, ATagger label tag or custom-depth stencil), plus the
/// id list. Everything else is neither rendered nor casts shadows into the
/// capture (the renderer filters shadow casters through the same list), so
/// the empty pixels carry only the sky / atmosphere in colour captures,
/// stencil 0 in the segmentation captures and the far plane in depth.
///
/// The actor scan runs only when the world's actor count changes (spawn or
/// destroy) so newly spawned vehicles show up on the next capture; every
/// other tick costs one integer compare plus copying the cached list into
/// the capture components.
struct CARLA_API FCarlaShowOnlyFilter
{
  using Label = carla::rpc::CityObjectLabel;

  /// Parse a comma separated list of label names into @a OutLabels using the
  /// tagger's own name mapping. Returns false and fills @a OutUnknown with
  /// the tokens that map to no label (they are skipped).
  static bool ParseLabels(const FString &CommaSeparated, TSet<Label> &OutLabels, FString &OutUnknown);

  void SetShowOnlyTags(const FString &CommaSeparated);

  void SetShadowCatcherTags(const FString &CommaSeparated);

  void SetShowOnlyActorIds(const FString &CommaSeparated);

  bool IsEnabled() const
  {
    return ShowOnlyTags.Num() > 0 || ActorIds.Num() > 0;
  }

  const TSet<Label> &GetShowOnlyTags() const { return ShowOnlyTags; }
  const TSet<Label> &GetShadowCatcherTags() const { return ShadowCatcherTags; }
  const TSet<uint32> &GetShowOnlyActorIds() const { return ActorIds; }

  /// Rebuild the cached actor list if the world changed since the last call.
  /// Cheap when nothing changed. @a Episode may be null (ids are then
  /// ignored).
  void Refresh(UWorld *World, const UCarlaEpisode *Episode);

  /// Push the current list into @a Capture (or restore full-scene rendering
  /// when the filter is disabled).
  void ApplyTo(USceneCaptureComponent2D &Capture) const;

  /// Force a rescan on the next Refresh.
  void MarkDirty() { bDirty = true; }

  int32 GetCachedActorCount() const { return CachedActors.Num(); }

private:

  void Rescan(UWorld *World, const UCarlaEpisode *Episode);

  TSet<Label> ShowOnlyTags;
  TSet<Label> ShadowCatcherTags;
  TSet<uint32> ActorIds;

  TArray<TWeakObjectPtr<AActor>> CachedActors;
  int32 LastActorCount = -1;
  bool bDirty = true;
};
