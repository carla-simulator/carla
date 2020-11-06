// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/ObjectRegister.h"

#include "Carla/Game/Tagger.h"

namespace crp = carla::rpc;

void UObjectRegister::RegisterObjects(TArray<AActor*> Actors)
{
  // Empties the array but doesn't change memory allocations
  EnvironmentObjects.Reset();

  for(AActor* Actor : Actors)
  {

    ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
    if (Vehicle)
    {
      RegisterVehicle(Vehicle);
      continue;
    }

    ACharacter* Character = Cast<ACharacter>(Actor);
    if (Character)
    {
      RegisterCharacter(Character);
      continue;
    }

    ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
    if(TrafficLight)
    {
      RegisterTrafficLight(TrafficLight);
      continue;
    }

    RegisterISMComponents(Actor);

    RegisterSMComponents(Actor);

    RegisterSKMComponents(Actor);
  }
}

void UObjectRegister::EnableEnvironmentObjects(const TSet<uint64>& EnvObjectIds, bool Enable)
{
  for(FEnvironmentObject& EnvironmentObject : EnvironmentObjects)
  {
    if(EnvObjectIds.Contains(EnvironmentObject.Id))
    {
      EnableEnvironmentObject(EnvironmentObject, Enable);
    }
  }
}

void UObjectRegister::RegisterEnvironmentObject(
    AActor* Actor,
    FBoundingBox& BoundingBox,
    EnvironmentObjectType Type,
    uint8 Tag)
{
  const FString ActorName = Actor->GetName();
  const char* ActorNameChar = TCHAR_TO_ANSI(*ActorName);

  FEnvironmentObject EnvironmentObject;
  EnvironmentObject.Transform = Actor->GetActorTransform();
  EnvironmentObject.Id = CityHash64(ActorNameChar, ActorName.Len());
  EnvironmentObject.Name = ActorName;
  EnvironmentObject.Actor = Actor;
  EnvironmentObject.CanTick = Actor->IsActorTickEnabled();
  EnvironmentObject.BoundingBox = BoundingBox;
  EnvironmentObject.ObjectLabel = static_cast<crp::CityObjectLabel>(Tag);
  EnvironmentObject.Type = Type;
  EnvironmentObjects.Emplace(EnvironmentObject);
}

void UObjectRegister::RegisterVehicle(ACarlaWheeledVehicle* Vehicle)
{
  check(Vehicle);
  FBoundingBox BB = UBoundingBoxCalculator::GetVehicleBoundingBox(Vehicle);
  RegisterEnvironmentObject(Vehicle, BB, EnvironmentObjectType::Vehicle, static_cast<uint8>(crp::CityObjectLabel::Vehicles));
}

void UObjectRegister::RegisterCharacter(ACharacter* Character)
{
  check(Character);
  FBoundingBox BB = UBoundingBoxCalculator::GetCharacterBoundingBox(Character);
  RegisterEnvironmentObject(Character, BB, EnvironmentObjectType::Character, static_cast<uint8>(crp::CityObjectLabel::Pedestrians));
}

void UObjectRegister::RegisterTrafficLight(ATrafficLightBase* TrafficLight)
{
  check(TrafficLight);

  TArray<FBoundingBox> BBs;
  TArray<uint8> Tags;

  UBoundingBoxCalculator::GetTrafficLightBoundingBox(TrafficLight, BBs, Tags);
  check(BBs.Num() == Tags.Num());

  const FTransform Transform = TrafficLight->GetTransform();
  const FString ActorName = TrafficLight->GetName();
  const bool IsActorTickEnabled = TrafficLight->IsActorTickEnabled();

  for(int i = 0; i < BBs.Num(); i++)
  {
    const FBoundingBox& BB = BBs
    const uint8 Tag = Tags[i];

    crp::CityObjectLabel ObjectLabel = static_cast<crp::CityObjectLabel>(Tag);

    const FString TagString = ATagger::GetTagAsString(ObjectLabel);
    const FString SMName = FString::Printf(TEXT("%s_%s_%d"), *ActorName, *TagString, i);

    FEnvironmentObject EnvironmentObject;
    EnvironmentObject.Transform = Transform;
    EnvironmentObject.Id = CityHash64(TCHAR_TO_ANSI(*SMName), SMName.Len());
    EnvironmentObject.Name = SMName;
    EnvironmentObject.Actor = TrafficLight;
    EnvironmentObject.CanTick = IsActorTickEnabled;
    EnvironmentObject.BoundingBox = BB;
    EnvironmentObject.Type = EnvironmentObjectType::TrafficLight;
    EnvironmentObject.ObjectLabel = ObjectLabel;
    EnvironmentObjects.Emplace(EnvironmentObject);

    // Register components with its ID; it's not the best solution since we are recalculating the BBs
    TArray<UStaticMeshComponent*> StaticMeshComps;
    UBoundingBoxCalculator::GetMeshCompsFromActorBoundingBox(TrafficLight, BB, StaticMeshComps);
    for(const UStaticMeshComponent* Comp : StaticMeshComps)
    {
      ObjectIdToComp.Emplace(EnvironmentObject.Id, Comp);
    }
  }
}

void UObjectRegister::RegisterISMComponents(AActor* Actor)
{
  check(Actor);

  TArray<UInstancedStaticMeshComponent*> ISMComps;
  Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComps);

  const FString ActorName = Actor->GetName();
  bool IsActorTickEnabled = Actor->IsActorTickEnabled();

  if(ISMComps.Num() > 0)
  {
    UE_LOG(LogCarla, Warning, TEXT("ISM ActorName = %s"), *ActorName);
  }

  for(UInstancedStaticMeshComponent* Comp : ISMComps)
  {
    const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData = Comp->PerInstanceSMData;

    TArray<FBoundingBox> BoundingBoxes;
    UBoundingBoxCalculator::GetISMBoundingBox(Comp, BoundingBoxes);

    const crp::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*Comp);

    for(int i = 0; i < PerInstanceSMData.Num(); i++)
    {
      const FInstancedStaticMeshInstanceData& It = PerInstanceSMData[i];
      const FTransform InstanceTransform = FTransform(It.Transform);
      const FVector InstanceLocation = InstanceTransform.GetLocation();

      // Discard decimal part
      const int32 X = static_cast<int32>(InstanceLocation.X);
      const int32 Y = static_cast<int32>(InstanceLocation.Y);
      const int32 Z = static_cast<int32>(InstanceLocation.Z);

      const FString InstanceName = FString::Printf(TEXT("%s_Inst_%d"), *ActorName, i);
      const FString InstanceId = FString::Printf(TEXT("%s_%d_%d_%d"), *ActorName, X, Y, Z);

      FEnvironmentObject EnvironmentObject;
      EnvironmentObject.Transform = InstanceTransform;
      EnvironmentObject.Id = CityHash64(TCHAR_TO_ANSI(*InstanceId), InstanceId.Len());
      EnvironmentObject.Name = InstanceName;
      EnvironmentObject.Actor = Actor;
      EnvironmentObject.CanTick = IsActorTickEnabled;
      EnvironmentObject.BoundingBox = BoundingBoxes[i];
      EnvironmentObject.Type = EnvironmentObjectType::ISMComp;
      EnvironmentObject.ObjectLabel = static_cast<crp::CityObjectLabel>(Tag);
      EnvironmentObjects.Emplace(EnvironmentObject);
    }
  }
}

void UObjectRegister::RegisterSMComponents(AActor* Actor)
{
  check(Actor);

  TArray<UStaticMeshComponent*> StaticMeshComps;
  Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);

  TArray<FBoundingBox> BBs;
  TArray<uint8> Tags;
  UBoundingBoxCalculator::GetBBsOfStaticMeshComponents(StaticMeshComps, BBs, Tags);
  check(BBs.Num() == Tags.Num());

  const FTransform Transform = Actor->GetTransform();
  const FString ActorName = Actor->GetName();
  const bool IsActorTickEnabled = Actor->IsActorTickEnabled();

  for(int i = 0; i < BBs.Num(); i++)
  {
    const FString SMName = FString::Printf(TEXT("%s_SM_%d"), *ActorName, i);

    FEnvironmentObject EnvironmentObject;
    EnvironmentObject.Transform = Transform;
    EnvironmentObject.Id = CityHash64(TCHAR_TO_ANSI(*SMName), SMName.Len());
    EnvironmentObject.Name = SMName;
    EnvironmentObject.Actor = Actor;
    EnvironmentObject.CanTick = IsActorTickEnabled;
    EnvironmentObject.BoundingBox = BBs[i];
    EnvironmentObject.Type = EnvironmentObjectType::SMComp;
    EnvironmentObject.ObjectLabel = static_cast<crp::CityObjectLabel>(Tags[i]);
    EnvironmentObjects.Emplace(EnvironmentObject);

  }
}

void UObjectRegister::RegisterSKMComponents(AActor* Actor)
{
  check(Actor);

  TArray<USkeletalMeshComponent*> SkeletalMeshComps;
  Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);

  TArray<FBoundingBox> BBs;
  TArray<uint8> Tags;
  UBoundingBoxCalculator::GetBBsOfSkeletalMeshComponents(SkeletalMeshComps, BBs, Tags);
  check(BBs.Num() == Tags.Num());

  const FTransform Transform = Actor->GetTransform();
  const FString ActorName = Actor->GetName();
  const bool IsActorTickEnabled = Actor->IsActorTickEnabled();

  for(int i = 0; i < BBs.Num(); i++)
  {
    const FString SKMName = FString::Printf(TEXT("%s_SKM_%d"), *ActorName, i);

    FEnvironmentObject EnvironmentObject;
    EnvironmentObject.Transform = Transform;
    EnvironmentObject.Id = CityHash64(TCHAR_TO_ANSI(*SKMName), SKMName.Len());
    EnvironmentObject.Name = SKMName;
    EnvironmentObject.Actor = Actor;
    EnvironmentObject.CanTick = IsActorTickEnabled;
    EnvironmentObject.BoundingBox = BBs[i];
    EnvironmentObject.Type = EnvironmentObjectType::SKMComp;
    EnvironmentObject.ObjectLabel = static_cast<crp::CityObjectLabel>(Tags[i]);
    EnvironmentObjects.Emplace(EnvironmentObject);

  }

}

void UObjectRegister::EnableEnvironmentObject(
  FEnvironmentObject& EnvironmentObject,
  bool Enable)
{
  switch (EnvironmentObject.Type)
  {
  case EnvironmentObjectType::Vehicle:
  case EnvironmentObjectType::Character:
    EnableActor(EnvironmentObject, Enable);
    break;
  case EnvironmentObjectType::TrafficLight:
    EnableTrafficLight(EnvironmentObject, Enable);
    break;
  case EnvironmentObjectType::ISMComp:
    break;
  case EnvironmentObjectType::SMComp:
    break;
  case EnvironmentObjectType::SKMComp:
    break;
  default:
    check(false);
    break;
  }

}

void UObjectRegister::EnableActor(FEnvironmentObject& EnvironmentObject, bool Enable)
{
  AActor* Actor = EnvironmentObject.Actor;

  Actor->SetActorHiddenInGame(!Enable);
  Actor->SetActorEnableCollision(Enable);
  if(EnvironmentObject.CanTick)
  {
    Actor->SetActorTickEnabled(Enable);
  }
}

void UObjectRegister::EnableTrafficLight(FEnvironmentObject& EnvironmentObject, bool Enable)
{



}