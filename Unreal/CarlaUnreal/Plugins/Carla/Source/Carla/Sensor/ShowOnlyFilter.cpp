// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/ShowOnlyFilter.h"
#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/Tagger.h"

#include <util/ue-header-guard-begin.h>
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

bool FCarlaShowOnlyFilter::ParseLabels(
    const FString &CommaSeparated,
    TSet<Label> &OutLabels,
    FString &OutUnknown)
{
  OutLabels.Reset();
  OutUnknown.Reset();
  TArray<FString> Tokens;
  CommaSeparated.ParseIntoArray(Tokens, TEXT(","), true);
  bool bAllKnown = true;
  for (FString &Token : Tokens)
  {
    Token.TrimStartAndEndInline();
    if (Token.IsEmpty())
    {
      continue;
    }
    // ATagger::GetTagFromString is the mapping the tagger itself uses for
    // component tags; it is a case-insensitive substring match, so "car",
    // "Car" and "Cars" all name CityObjectLabel::Car.
    const Label Parsed = ATagger::GetTagFromString(Token);
    if (Parsed == Label::None)
    {
      bAllKnown = false;
      OutUnknown += (OutUnknown.IsEmpty() ? TEXT("") : TEXT(",")) + Token;
      continue;
    }
    OutLabels.Add(Parsed);
  }
  return bAllKnown;
}

static void ShowOnlyFilter_WarnUnknown(const TCHAR *Attribute, const FString &Unknown)
{
  if (!Unknown.IsEmpty())
  {
    UE_LOG(LogCarla, Warning,
        TEXT("%s: unknown semantic label(s) \"%s\" ignored"), Attribute, *Unknown);
  }
}

void FCarlaShowOnlyFilter::SetShowOnlyTags(const FString &CommaSeparated)
{
  FString Unknown;
  ParseLabels(CommaSeparated, ShowOnlyTags, Unknown);
  ShowOnlyFilter_WarnUnknown(TEXT("show_only_tags"), Unknown);
  bDirty = true;
}

void FCarlaShowOnlyFilter::SetShadowCatcherTags(const FString &CommaSeparated)
{
  FString Unknown;
  ParseLabels(CommaSeparated, ShadowCatcherTags, Unknown);
  ShowOnlyFilter_WarnUnknown(TEXT("shadow_catcher_tags"), Unknown);
  bDirty = true;
}

void FCarlaShowOnlyFilter::SetShowOnlyActorIds(const FString &CommaSeparated)
{
  ActorIds.Reset();
  TArray<FString> Tokens;
  CommaSeparated.ParseIntoArray(Tokens, TEXT(","), true);
  for (FString &Token : Tokens)
  {
    Token.TrimStartAndEndInline();
    if (Token.IsEmpty())
    {
      continue;
    }
    if (Token.IsNumeric())
    {
      ActorIds.Add(static_cast<uint32>(FCString::Atoi64(*Token)));
    }
    else
    {
      UE_LOG(LogCarla, Warning,
          TEXT("show_only_actor_ids: \"%s\" is not an actor id, ignored"), *Token);
    }
  }
  bDirty = true;
}

void FCarlaShowOnlyFilter::Refresh(UWorld *World, const UCarlaEpisode *Episode)
{
  if (!IsEnabled() || World == nullptr)
  {
    return;
  }
  const int32 ActorCount = World->GetActorCount();
  if (bDirty || ActorCount != LastActorCount)
  {
    Rescan(World, Episode);
    LastActorCount = ActorCount;
    bDirty = false;
  }
}

void FCarlaShowOnlyFilter::Rescan(UWorld *World, const UCarlaEpisode *Episode)
{
  const double Start = FPlatformTime::Seconds();
  CachedActors.Reset();

  TSet<Label> Wanted = ShowOnlyTags;
  Wanted.Append(ShadowCatcherTags);

  if (Wanted.Num() > 0)
  {
    TInlineComponentArray<UPrimitiveComponent *> Primitives;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
      AActor *Actor = *It;
      if (!IsValid(Actor))
      {
        continue;
      }
      Primitives.Reset();
      Actor->GetComponents<UPrimitiveComponent>(Primitives);
      for (UPrimitiveComponent *Primitive : Primitives)
      {
        if (Primitive != nullptr &&
            Wanted.Contains(ATagger::GetTagOfTaggedComponent(*Primitive)))
        {
          CachedActors.Add(Actor);
          break;
        }
      }
    }
  }

  if (ActorIds.Num() > 0 && Episode != nullptr)
  {
    const FActorRegistry &Registry = Episode->GetActorRegistry();
    for (const uint32 Id : ActorIds)
    {
      const FCarlaActor *View = Registry.FindCarlaActor(Id);
      if (View == nullptr)
      {
        continue;
      }
      AActor *Actor = const_cast<AActor *>(View->GetActor());
      if (IsValid(Actor))
      {
        CachedActors.AddUnique(Actor);
      }
    }
  }

  UE_LOG(LogCarla, Verbose,
      TEXT("ShowOnlyFilter: %d actor(s) selected out of %d in %.2f ms"),
      CachedActors.Num(), World->GetActorCount(),
      (FPlatformTime::Seconds() - Start) * 1000.0);
}

void FCarlaShowOnlyFilter::ApplyTo(USceneCaptureComponent2D &Capture) const
{
  if (!IsEnabled())
  {
    if (Capture.PrimitiveRenderMode == ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList)
    {
      Capture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
      Capture.ShowOnlyActors.Reset();
      Capture.ClearShowOnlyComponents();
    }
    return;
  }
  Capture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  Capture.ShowOnlyActors.Reset(CachedActors.Num());
  for (const TWeakObjectPtr<AActor> &Weak : CachedActors)
  {
    if (AActor *Actor = Weak.Get())
    {
      Capture.ShowOnlyActors.Add(Actor);
    }
  }
}
