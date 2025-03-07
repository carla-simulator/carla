// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaActor.h"
#include "CarlaTemplateExternalInterfaceActor.h"
#include "CarlaTemplateExternalInterfaceActorData.h"

const FString FCarlaTemplateExternalInterfaceActor::CustomType = "CarlaTemplateExternalInterface_CarlaTemplateExternalInterface";

FCarlaTemplateExternalInterfaceActor::FCarlaTemplateExternalInterfaceActor(
    IdType ActorId,
    AActor* Actor,
    TSharedPtr<const FActorInfo> Info,
    carla::rpc::ActorState InState,
    UWorld* World)
    : FCarlaActor(ActorId, Actor, Info, InState, World)
{
  Type = ActorType::Custom;
  ActorData = MakeShared<FCarlaTemplateExternalInterfaceData>();
}

TSharedPtr<FCarlaTemplateExternalInterfaceActor> FCarlaTemplateExternalInterfaceActor::CreateInstance(
    FCarlaActor::IdType ActorId, 
    AActor* Actor, 
    TSharedPtr<const FActorInfo> Info, 
    carla::rpc::ActorState InState, 
    UWorld* World)
  {
    return MakeShared<FCarlaTemplateExternalInterfaceActor>(ActorId, Actor, std::move(Info), InState, World);
  }

void FCarlaTemplateExternalInterfaceActor::RegisterClassWithFactory()
  {
    CarlaActorConstructorFactory::Instance().Register(TEXT("FCarlaTemplateExternalInterfaceActor"), ACarlaTemplateExternalInterfaceActor::StaticClass(), CreateInstance);
  }
