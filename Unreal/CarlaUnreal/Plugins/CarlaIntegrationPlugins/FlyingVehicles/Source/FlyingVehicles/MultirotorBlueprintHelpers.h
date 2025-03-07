#pragma once

#include "CoreMinimal.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "MultirotorBlueprintHelpers.generated.h"

UCLASS()
class UMultirotorBlueprintHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    template <typename T>
    static T* GetActorComponent(AActor* actor, FString name)
    {
        TArray<T*> components;
        actor->GetComponents(components);
        T* found = nullptr;
        for (T* component : components) {
            if (component->GetName().Compare(name) == 0) {
                found = component;
                break;
            }
        }
        return found;
    };

    UFUNCTION(Category = "Carla Actor", BlueprintCallable)
    static void MakeMultirotorDefinition(
        const FMultirotorParameters &Parameters,
        bool &Success,
        FActorDefinition &Definition);

    UFUNCTION(Category = "Carla Actor", BlueprintCallable)
    static void MakeMultirotorDefinitions(
        const TArray<FMultirotorParameters> &ParameterArray,
        TArray<FActorDefinition> &Definitions);

};
