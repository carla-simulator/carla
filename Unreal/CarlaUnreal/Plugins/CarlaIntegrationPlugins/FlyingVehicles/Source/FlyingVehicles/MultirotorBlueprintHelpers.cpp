#include "MultirotorBlueprintHelpers.h"

static void AddRecommendedValuesForActorRoleName(
    FActorDefinition &Definition,
    TArray<FString> &&RecommendedValues)
{
  for (auto &&ActorVariation: Definition.Variations)
  {
    if (ActorVariation.Id == "role_name")
    {
      ActorVariation.RecommendedValues = RecommendedValues;
      return;
    }
  }
}

template <typename T, typename Functor>
static void FillActorDefinitionArray(
    const TArray<T> &ParameterArray,
    TArray<FActorDefinition> &Definitions,
    Functor Maker)
{
  for (auto &Item : ParameterArray)
  {
    FActorDefinition Definition;
    bool Success = false;
    Maker(Item, Success, Definition);
    if (Success)
    {
      Definitions.Emplace(std::move(Definition));
    }
  }
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

template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}

static FString ColorToFString(const FColor &Color)
{
  return JoinStrings(
      TEXT(","),
      FString::FromInt(Color.R),
      FString::FromInt(Color.G),
      FString::FromInt(Color.B));
}

void UMultirotorBlueprintHelpers::MakeMultirotorDefinitions(
    const TArray<FMultirotorParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakeMultirotorDefinition);
}

void UMultirotorBlueprintHelpers::MakeMultirotorDefinition(
    const FMultirotorParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("multirotor"), Parameters.Make, Parameters.Model);
  AddRecommendedValuesForActorRoleName(Definition,
      {TEXT("autopilot"), TEXT("scenario"), TEXT("ego_vehicle")});
  Definition.Class = Parameters.Class;

  if (Parameters.RecommendedColors.Num() > 0)
  {
    FActorVariation Colors;
    Colors.Id = TEXT("color");
    Colors.Type = EActorAttributeType::RGBColor;
    Colors.bRestrictToRecommended = false;
    for (auto &Color : Parameters.RecommendedColors)
    {
      Colors.RecommendedValues.Emplace(ColorToFString(Color));
    }
    Definition.Variations.Emplace(Colors);
  }

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("object_type"),
    EActorAttributeType::String,
    Parameters.ObjectType});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("number_of_rotors"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.NumberOfRotors)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("generation"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.Generation)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("control_type"),
    EActorAttributeType::String,
    "multirotor"});

  Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
}
