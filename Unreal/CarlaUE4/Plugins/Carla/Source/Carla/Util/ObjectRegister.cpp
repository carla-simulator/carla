// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Util/ObjectRegister.h"

#include "Carla/Game/Tagger.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "InstancedFoliageActor.h"
#include "GameFramework/Character.h"

#if WITH_EDITOR
#include "FileHelper.h"
#include "Paths.h"
#endif // WITH_EDITOR

namespace crp = carla::rpc;

TArray<FEnvironmentObject> UObjectRegister::GetEnvironmentObjects(uint8 InTagQueried) const
{
  TArray<FEnvironmentObject> Result;

  crp::CityObjectLabel TagQueried = (crp::CityObjectLabel)InTagQueried;
  bool FilterByTagEnabled = (TagQueried != crp::CityObjectLabel::Any);

  for(const FEnvironmentObject& It : EnvironmentObjects)
  {
    if(!FilterByTagEnabled || (It.ObjectLabel == TagQueried))
    {
      Result.Emplace(It);
    }
  }

  return Result;
}

void UObjectRegister::RegisterObjects(TArray<AActor*> Actors)
{
  // Empties the array but doesn't change memory allocations
  EnvironmentObjects.Reset();

  for(AActor* Actor : Actors)
  {

    FString ClassName = Actor->GetClass()->GetName();
    // Discard Sky to not break the global ilumination
    if(ClassName.Equals("BP_Sky_C")) continue;

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

#if WITH_EDITOR
  // To help debug
  FString FileContent;
  FileContent += FString::Printf(TEXT("Num actors %d\n"), Actors.Num());
  FileContent += FString::Printf(TEXT("Num registered objects %d\n\n"), EnvironmentObjects.Num());

  for(const FEnvironmentObject& Object : EnvironmentObjects)
  {
    FileContent += FString::Printf(TEXT("%llu\t"), Object.Id);
    FileContent += FString::Printf(TEXT("%s\t"), *Object.Name);
    FileContent += FString::Printf(TEXT("%s\t"), *Object.IdStr);
    FileContent += FString::Printf(TEXT("%d\n"), static_cast<int32>(Object.Type));
  }


  FString FilePath = FPaths::ProjectSavedDir() + "RegisteredObjects.txt";
  FFileHelper::SaveStringToFile(
    FileContent,
    *FilePath,
    FFileHelper::EEncodingOptions::AutoDetect,
    &IFileManager::Get(),
    EFileWrite::FILEWRITE_Silent);

#endif // WITH_EDITOR

}

void UObjectRegister::EnableEnvironmentObjects(const TSet<uint64>& EnvObjectIds, bool Enable)
{
  for(uint64 It : EnvObjectIds)
  {
    bool found = false;
    for(FEnvironmentObject& EnvironmentObject : EnvironmentObjects)
    {
      if(It == EnvironmentObject.Id)
      {
        EnableEnvironmentObject(EnvironmentObject, Enable);
        found = true;
        break;
      }
    }
    if(!found)
    {
      UE_LOG(LogCarla, Error, TEXT("EnableEnvironmentObjects id not found %llu"), It);
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
  EnvironmentObjects.Emplace(std::move(EnvironmentObject));
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
    const FBoundingBox& BB = BBs[i];
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
    // But this is only calculated when the level is loaded
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
  int InstanceCount = 0;
  bool IsActorTickEnabled = Actor->IsActorTickEnabled();

  // Foliage actor is a special case, it can appear more than once while traversing the actor list
  if(Cast<AInstancedFoliageActor>(Actor))
  {
    InstanceCount = FoliageActorInstanceCount;
  }

  for(UInstancedStaticMeshComponent* Comp : ISMComps)
  {
    const TArray<FInstancedStaticMeshInstanceData>& PerInstanceSMData = Comp->PerInstanceSMData;
    const FTransform CompTransform = Comp->GetComponentTransform();

    TArray<FBoundingBox> BoundingBoxes;
    UBoundingBoxCalculator::GetISMBoundingBox(Comp, BoundingBoxes);

    FString CompName = Comp->GetName();
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

      const FString InstanceName = FString::Printf(TEXT("%s_Inst_%d_%d"), *ActorName, InstanceCount, i);
      const FString InstanceIdStr = FString::Printf(TEXT("%s_%s_%d_%d_%d_%d"), *ActorName, *CompName, X, Y, Z, InstanceCount);
      uint64 InstanceId = CityHash64(TCHAR_TO_ANSI(*InstanceIdStr), InstanceIdStr.Len());

      FEnvironmentObject EnvironmentObject;
      EnvironmentObject.Transform = InstanceTransform * CompTransform;
      EnvironmentObject.Id = InstanceId;
      EnvironmentObject.Name = InstanceName;
      EnvironmentObject.IdStr = InstanceIdStr;
      EnvironmentObject.Actor = Actor;
      EnvironmentObject.CanTick = IsActorTickEnabled;
      if( i < BoundingBoxes.Num())
      {
        EnvironmentObject.BoundingBox = BoundingBoxes[i];
      }

      EnvironmentObject.Type = EnvironmentObjectType::ISMComp;
      EnvironmentObject.ObjectLabel = static_cast<crp::CityObjectLabel>(Tag);
      EnvironmentObjects.Emplace(EnvironmentObject);

      ObjectIdToComp.Emplace(InstanceId, Comp);
      InstanceCount++;
    }
  }

  if(Cast<AInstancedFoliageActor>(Actor))
  {
    FoliageActorInstanceCount = InstanceCount;
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
  case EnvironmentObjectType::SMComp:
  case EnvironmentObjectType::SKMComp:
    EnableActor(EnvironmentObject, Enable);
    break;
  case EnvironmentObjectType::TrafficLight:
    EnableTrafficLight(EnvironmentObject, Enable);
    break;
  case EnvironmentObjectType::ISMComp:
    EnableISMComp(EnvironmentObject, Enable);
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
  // We need to look for the component(s) that form the EnvironmentObject
  // i.e.: The light box is composed by various SMComponents, one per light,
  //       we need to enable/disable all of them

  TArray<const UStaticMeshComponent*> ObjectComps;
  ObjectIdToComp.MultiFind(EnvironmentObject.Id, ObjectComps);

  for(const UStaticMeshComponent* Comp : ObjectComps)
  {
    UStaticMeshComponent* SMComp = const_cast<UStaticMeshComponent*>(Comp);
    SMComp->SetHiddenInGame(!Enable);
    ECollisionEnabled::Type CollisionType = Enable ? ECollisionEnabled::Type::QueryAndPhysics : ECollisionEnabled::Type::NoCollision;
    SMComp->SetCollisionEnabled(CollisionType);
  }

}

void UObjectRegister::EnableISMComp(FEnvironmentObject& EnvironmentObject, bool Enable)
{
  TArray<const UStaticMeshComponent*> ObjectComps;
  TArray<FString> InstanceName;
  FTransform InstanceTransform = EnvironmentObject.Transform;

  ObjectIdToComp.MultiFind(EnvironmentObject.Id, ObjectComps);
  EnvironmentObject.Name.ParseIntoArray(InstanceName, TEXT("_"), false);

  int Index = FCString::Atoi(*InstanceName[InstanceName.Num() - 1]);

  if(!Enable)
  {
    InstanceTransform.SetTranslation(FVector(1000000.0f));
    InstanceTransform.SetScale3D(FVector(0.0f));
  }

  UStaticMeshComponent* SMComp = const_cast<UStaticMeshComponent*>(ObjectComps[0]);
  UInstancedStaticMeshComponent* ISMComp = Cast<UInstancedStaticMeshComponent>(SMComp);
  bool Result = ISMComp->UpdateInstanceTransform(Index, InstanceTransform, true, true);

}
