// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "Carla/Actor/ActorRegistry.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Sensor/Sensor.h"

namespace crp = carla::rpc;

FActorRegistry::IdType FActorRegistry::ID_COUNTER = 0u;

static FActorView::ActorType FActorRegistry_GetActorType(const FActorView &View)
{
  if (!View.IsValid())
  {
    return FActorView::ActorType::INVALID;
  }

  const AActor* Actor = View.GetActor();
  if (nullptr != Cast<ACarlaWheeledVehicle>(Actor))
  {
    return FActorView::ActorType::Vehicle;
  }
  else if (nullptr != Cast<ACharacter>(Actor))
  {
    return FActorView::ActorType::Walker;
  }
  else if (nullptr != Cast<ATrafficLightBase>(Actor))
  {
    return FActorView::ActorType::TrafficLight;
  }
  else if (nullptr != Cast<ATrafficSignBase>(Actor))
  {
    return FActorView::ActorType::TrafficSign;
  }
  else if (nullptr != Cast<ASensor>(Actor))
  {
    return FActorView::ActorType::Sensor;
  }
  else
  {
    return FActorView::ActorType::Other;
  }
}

static FString GetRelevantTagAsString(const TSet<crp::CityObjectLabel> &SemanticTags)
{
  for (auto &&Tag : SemanticTags)
  {
    if ((Tag != crp::CityObjectLabel::None) && (Tag != crp::CityObjectLabel::Other))
    {
      auto Str = ATagger::GetTagAsString(Tag).ToLower();
      return (Str.EndsWith(TEXT("s")) ? Str.LeftChop(1) : Str);
    }
  }
  return TEXT("unknown");
}

FActorView FActorRegistry::Register(AActor &Actor, FActorDescription Description, IdType DesiredId)
{

  FActorView* ActorView = FindPtr(DesiredId);
  bool IsDormant = ActorView && (ActorView->IsDormant());
  if(IsDormant)
  {
    ActorView->TheActor = &Actor;
    Actors.Add(DesiredId, &Actor);
    Ids.Add(&Actor, DesiredId);
    return *ActorView;
  }

  IdType Id = ++FActorRegistry::ID_COUNTER;

  if (DesiredId != 0 && Id != DesiredId) {
    // check if the desired Id is free, then use it instead
    if (!Actors.Contains(DesiredId))
    {
      Id = DesiredId;
      if (ID_COUNTER < Id)
        ID_COUNTER = Id;
    }
  }

  Actors.Emplace(Id, &Actor);
  if (Ids.Contains(&Actor))
  {
    UE_LOG(
        LogCarla,
        Warning,
        TEXT("This actor's memory address is already registered, "
             "either you forgot to deregister the actor "
             "or the actor was garbage collected."));
  }
  Ids.Emplace(&Actor, Id);

  FActorView View = MakeView(Id, Actor, std::move(Description), crp::ActorState::Alive);

  FActorView& Result = ActorDatabase.Emplace(Id, View);

  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());
  return Result;
}

void FActorRegistry::Deregister(IdType Id, bool KeepId)
{
  check(Contains(Id));
  FActorView* ActorView = FindPtr(Id);

  if(!ActorView) return;

  AActor *Actor = ActorView->GetActor();

  // If the ID will be reused again by other actor (like dormant actors)
  // we need to keep the ID <-> FActorView relation
  // but we need to remove all AActor pointers since they will not be valid anymore
  if(KeepId)
  {
    Actors[Id] = nullptr;
  }
  else
  {
    ActorDatabase.Remove(Id);
    Actors.Remove(Id);
  }

  Ids.Remove(Actor);

  ActorView->TheActor = nullptr;

  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());
}

void FActorRegistry::Deregister(AActor *Actor, bool KeepId)
{
  check(Actor != nullptr);
  auto View = Find(Actor);
  check(View.GetActor() == Actor);
  Deregister(View.GetActorId(), KeepId);
}

FActorView FActorRegistry::FindOrFake(AActor *Actor) const
{
  auto View = Find(Actor);
  const bool bFakeActor = (View.GetActor() == nullptr) && (Actor != nullptr);
  // Maybe the state of this view should be Invlaid but I'm not 100% sure
  return bFakeActor ? MakeView(0u, *Actor, FActorDescription{}, crp::ActorState::Alive) : View;
}

FActorView FActorRegistry::MakeView(
    IdType Id,
    AActor &Actor,
    FActorDescription Description,
    crp::ActorState InState) const
{
  auto Info = MakeShared<FActorInfo>();
  Info->Description = std::move(Description);
  ATagger::GetTagsOfTaggedActor(Actor, Info->SemanticTags);
  Info->BoundingBox = UBoundingBoxCalculator::GetActorBoundingBox(&Actor);

  if (Info->Description.Id.IsEmpty())
  {
    // This is a fake actor, let's fake the id based on their semantic tags.
    Info->Description.Id = TEXT("static.") + GetRelevantTagAsString(Info->SemanticTags);
  }

  Info->SerializedData.id = Id;
  Info->SerializedData.description = Info->Description;
  Info->SerializedData.bounding_box = Info->BoundingBox;
  Info->SerializedData.semantic_tags.reserve(Info->SemanticTags.Num());
  for (auto &&Tag : Info->SemanticTags)
  {
    using tag_t = decltype(Info->SerializedData.semantic_tags)::value_type;
    Info->SerializedData.semantic_tags.emplace_back(static_cast<tag_t>(Tag));
  }
  auto *Sensor = Cast<ASensor>(&Actor);
  if (Sensor != nullptr)
  {
    const auto &Token = Sensor->GetToken();
    Info->SerializedData.stream_token = decltype(Info->SerializedData.stream_token)(
        std::begin(Token.data),
        std::end(Token.data));
  }
  auto View = FActorView{Id, &Actor, std::move(Info), InState};
  View.Type = FActorRegistry_GetActorType(View);
  View.BuildActorData();
  return View;
}

void FActorRegistry::PutActorToSleep(FActorView::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  FActorView* ActorView = FindPtr(Id);
  ActorView->PutActorToSleep(CarlaEpisode);
  for (const FActorView::IdType& ChildId : ActorView->GetChildren())
  {
    PutActorToSleep(ChildId, CarlaEpisode);
  }
  // TODO: update id maps
}

void FActorRegistry::WakeActorUp(FActorView::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  FActorView* ActorView = FindPtr(Id);
  ActorView->WakeActorUp(CarlaEpisode);
  if (ActorView->GetParent())
  {
    AActor* Actor = ActorView->GetActor();
    FActorView* ParentView = FindPtr(ActorView->GetParent());
    if (ParentView && !ParentView->IsDormant() && ParentView->GetActor())
    {
      AActor* ParentActor = ParentView->GetActor();
      CarlaEpisode->AttachActors(
          Actor,
          ParentActor,
          static_cast<EAttachmentType>(ActorView->GetAttachmentType()));
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to attach actor %d to %d during wake up"), Id, ActorView->GetParent());
    }
  }
  for (const FActorView::IdType& ChildId : ActorView->GetChildren())
  {
    WakeActorUp(ChildId, CarlaEpisode);
  }
  // TODO: update id maps
}
