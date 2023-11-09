// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorData.h"
#include "Carla/Actor/ActorRegistry.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Sensor/Sensor.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/streaming/Token.h"
#include "carla/streaming/detail/Token.h"
#include <compiler/enable-ue4-macros.h>


namespace crp = carla::rpc;

FActorRegistry::IdType FActorRegistry::ID_COUNTER = 0u;

static FCarlaActor::ActorType FActorRegistry_GetActorType(const AActor *Actor)
{
  if (!Actor)
  {
    return FCarlaActor::ActorType::INVALID;
  }

  if (nullptr != Cast<ACarlaWheeledVehicle>(Actor))
  {
    return FCarlaActor::ActorType::Vehicle;
  }
  else if (nullptr != Cast<ACharacter>(Actor))
  {
    return FCarlaActor::ActorType::Walker;
  }
  else if (nullptr != Cast<ATrafficLightBase>(Actor))
  {
    return FCarlaActor::ActorType::TrafficLight;
  }
  else if (nullptr != Cast<ATrafficSignBase>(Actor))
  {
    return FCarlaActor::ActorType::TrafficSign;
  }
  else if (nullptr != Cast<ASensor>(Actor))
  {
    return FCarlaActor::ActorType::Sensor;
  }
  else
  {
    return FCarlaActor::ActorType::Other;
  }
}

FString CarlaGetRelevantTagAsString(const TSet<crp::CityObjectLabel> &SemanticTags)
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

FCarlaActor* FActorRegistry::Register(AActor &Actor, FActorDescription Description, IdType DesiredId)
{

  FCarlaActor* CarlaActor = FindCarlaActor(DesiredId);
  bool IsDormant = CarlaActor && (CarlaActor->IsDormant());
  if(IsDormant)
  {
    CarlaActor->TheActor = &Actor;
    Actors.Add(DesiredId, &Actor);
    Ids.Add(&Actor, DesiredId);
    return CarlaActor;
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

  TSharedPtr<FCarlaActor> View =
      MakeCarlaActor(Id, Actor, std::move(Description), crp::ActorState::Active);

  TSharedPtr<FCarlaActor>& Result = ActorDatabase.Emplace(Id, MoveTemp(View));

  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());
  return Result.Get();
}

void FActorRegistry::Deregister(IdType Id)
{
  FCarlaActor* CarlaActor = FindCarlaActor(Id);

  if(!CarlaActor) return;

  AActor *Actor = CarlaActor->GetActor();

  ActorDatabase.Remove(Id);
  Actors.Remove(Id);
  Ids.Remove(Actor);

  CarlaActor->TheActor = nullptr;

  check(static_cast<size_t>(Actors.Num()) == ActorDatabase.Num());
}

void FActorRegistry::Deregister(AActor *Actor)
{
  check(Actor != nullptr);
  FCarlaActor* CarlaActor = FindCarlaActor(Actor);
  check(CarlaActor->GetActor() == Actor);
  Deregister(CarlaActor->GetActorId());
}

TSharedPtr<FCarlaActor> FActorRegistry::MakeCarlaActor(
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
    Info->Description.Id = TEXT("static.") + CarlaGetRelevantTagAsString(Info->SemanticTags);
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
  auto Type = FActorRegistry_GetActorType(&Actor);
  TSharedPtr<FCarlaActor> CarlaActor =
      FCarlaActor::ConstructCarlaActor(
        Id, &Actor,
        std::move(Info), Type,
        InState, Actor.GetWorld());
  return CarlaActor;
}

void FActorRegistry::PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{
  FCarlaActor* CarlaActor = FindCarlaActor(Id);

  // update id maps
  Actors[Id] = nullptr;
  AActor* Actor = CarlaActor->GetActor();
  if(Actor)
  {
    Ids.Remove(Actor);
  }

  CarlaActor->PutActorToSleep(CarlaEpisode);
  for (const FCarlaActor::IdType& ChildId : CarlaActor->GetChildren())
  {
    PutActorToSleep(ChildId, CarlaEpisode);
  }
  // TODO: update id maps
}

void FActorRegistry::WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode)
{

  FCarlaActor* CarlaActor = FindCarlaActor(Id);
  CarlaActor->WakeActorUp(CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  if (Actor)
  {
    // update ids
    Actors[Id] = Actor;
    Ids.Add(Actor, Id);
    if (CarlaActor->GetParent())
    {
      FCarlaActor* ParentView = FindCarlaActor(CarlaActor->GetParent());
      if (ParentView && !ParentView->IsDormant() && ParentView->GetActor())
      {
        AActor* ParentActor = ParentView->GetActor();
        CarlaEpisode->AttachActors(
            Actor,
            ParentActor,
            static_cast<EAttachmentType>(CarlaActor->GetAttachmentType()));
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to attach actor %d to %d during wake up"), Id, CarlaActor->GetParent());
      }
    }
  }
  for (const FCarlaActor::IdType& ChildId : CarlaActor->GetChildren())
  {
    WakeActorUp(ChildId, CarlaEpisode);
  }
}

FString FActorRegistry::GetDescriptionFromStream(carla::streaming::detail::stream_id_type Id)
{
  for (auto &Item : ActorDatabase)
  {
    // check for a sensor
    ASensor *Sensor = Cast<ASensor>(Item.Value->GetActor());
    if (Sensor == nullptr) continue;

    carla::streaming::detail::token_type token(Sensor->GetToken());
    if (token.get_stream_id() == Id)
    {
      const FActorInfo *Info = Item.Value->GetActorInfo();
      return Info->Description.Id;
    }
  }
  return FString("");
}
