#pragma once

#include "CarlaTemplateExternalInterfaceBlueprintLibrary.h"
#include "CarlaTemplateExternalInterfaceActor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/Sensor.h"

#if WITH_EDITOR
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr)                    \
  if (!IsValid(ActorPtr))     \
  {                                                           \
    UE_LOG(LogCarlaTemplateExternalInterface, Error, TEXT("Cannot set empty actor!")); \
    return;                                                   \
  }
#else
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr) \
  IsValid(ActorPtr);
#endif // WITH_EDITOR

FActorDefinition UCarlaTemplateExternalInterfaceBlueprintLibrary::MakeCarlaTemplateExternalInterfaceDefinition(TSubclassOf<ACarlaTemplateExternalInterfaceActor> Class)
{
  FActorDefinition Definition;
  bool Success;
  MakeCarlaTemplateExternalInterfaceDefinition(Success, Definition, Class);
  check(Success);
  return Definition;
}

template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}

template <typename ... TStrs>
static void FillIdAndTags(FActorDefinition &Def, TStrs && ... Strings)
{
  Def.Id = JoinStrings(TEXT("."), std::forward<TStrs>(Strings) ...).ToLower();
  Def.Tags = JoinStrings(TEXT(","), std::forward<TStrs>(Strings) ...).ToLower();

  // each actor gets an actor role name attribute (empty by default)
  FActorVariation ActorRole;
  ActorRole.Id = TEXT("role_name");
  ActorRole.Type = EActorAttributeType::String;
  ActorRole.RecommendedValues = { TEXT("default") };
  ActorRole.bRestrictToRecommended = false;
  Def.Variations.Emplace(ActorRole);

  // ROS2
  FActorVariation Var;
  Var.Id = TEXT("ros_name");
  Var.Type = EActorAttributeType::String;
  Var.RecommendedValues = { Def.Id };
  Var.bRestrictToRecommended = false;
  Def.Variations.Emplace(Var);
}

void UCarlaTemplateExternalInterfaceBlueprintLibrary::MakeCarlaTemplateExternalInterfaceDefinition(
    bool &Success,
    FActorDefinition &Definition,
    TSubclassOf<ACarlaTemplateExternalInterfaceActor> Class)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("template"));
  
  Definition.Class = Class;

  FActorVariation Tick;
  Tick.Id = TEXT("sensor_tick");
  Tick.Type = EActorAttributeType::Float;
  Tick.RecommendedValues = { TEXT("0.0") };
  Tick.bRestrictToRecommended = false;

  FActorVariation TemplateActorValue;
  TemplateActorValue.Id = TEXT("template_value");
  TemplateActorValue.Type = EActorAttributeType::Float;
  TemplateActorValue.RecommendedValues = { TEXT("0.25") };
  TemplateActorValue.bRestrictToRecommended = false;

  Definition.Variations.Append({
    Tick,
    TemplateActorValue});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("sensor_type"),
    EActorAttributeType::String,
    "template"});

  Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
}

void UCarlaTemplateExternalInterfaceBlueprintLibrary::SetCarlaTemplateExternalInterface(
    const FActorDescription &Description,
    ACarlaTemplateExternalInterfaceActor *CarlaTemplateExternalInterface)
{
  CARLA_ABFL_CHECK_ACTOR(CarlaTemplateExternalInterface);

  CarlaTemplateExternalInterface->SetTemplateValue(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("template_value", Description.Variations, 0.25f)
  );

  CarlaTemplateExternalInterface->OutputName = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString("ros_name", Description.Variations, "ABC");

}