
#pragma once

#include "Engine.h"
#include "Logging/LogMacros.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "StructDefinitions.generated.h"

USTRUCT(BlueprintType)
struct FTwist
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Linear;

    UPROPERTY(BlueprintReadWrite)
    FVector Angular;

    FTwist(){}

};