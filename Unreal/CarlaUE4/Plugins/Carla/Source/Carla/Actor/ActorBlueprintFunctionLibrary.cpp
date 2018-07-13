// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "ActorBlueprintFunctionLibrary.h"

#include <algorithm>

/// @todo Improve all the check here.
namespace ActorDetail {

  /// Iterate every item and validate it with @a Validator.
  template <typename T, typename F>
  static bool ForEach(const TArray<T> &Array, F Validator)
  {
    bool result = true;
    for (auto &&Item : Array)
    {
      result &= Validator(Item);
    }
    return result;
  }

  static bool IsValid(const FString &String)
  {
    return !String.IsEmpty();
  }

  static bool IdIsValid(const FString &Id)
  {
    return IsValid(Id);
  }

  static bool TagsAreValid(const FString &Tags)
  {
    return IsValid(Tags);
  }

  static bool IsValid(const EActorAttributeType Type)
  {
    return Type < EActorAttributeType::SIZE;
  }

  static bool ValueIsValid(const EActorAttributeType Type, const FString &Value)
  {
    return true;
  }

  static bool IsValid(const FActorVariation &Variation)
  {
    return
        IdIsValid(Variation.Id) &&
        IsValid(Variation.Type) &&
        ForEach(Variation.RecommendedValues, [&](auto &Value) {
          return ValueIsValid(Variation.Type, Value);
        });
  }

  static bool IsValid(const FActorAttribute &Attribute)
  {
    return
        IdIsValid(Attribute.Id) &&
        IsValid(Attribute.Type) &&
        ValueIsValid(Attribute.Type, Attribute.Value);
  }

  static bool IsValid(const FActorDefinition &Definition);

  template <typename T>
  static bool AreValid(const TArray<T> &Array)
  {
    return ForEach(Array, [](const auto &Item){ return ActorDetail::IsValid(Item); });
  }

  static bool IsValid(const FActorDefinition &Definition)
  {
    return
        IdIsValid(Definition.Id) &&
        TagsAreValid(Definition.Tags) &&
        AreValid(Definition.Variations) &&
        AreValid(Definition.Attributes);
  }

} // namespace ActorDetail

bool UActorBlueprintFunctionLibrary::CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions)
{
  return ActorDetail::AreValid(ActorDefinitions);
}

void UActorBlueprintFunctionLibrary::ValidateActorDefinitions(TArray<FActorDefinition> &ActorDefinitions)
{
  /// @todo
  if (!CheckActorDefinitions(ActorDefinitions))
  {
    ActorDefinitions.Empty(); // lol
  }
}
