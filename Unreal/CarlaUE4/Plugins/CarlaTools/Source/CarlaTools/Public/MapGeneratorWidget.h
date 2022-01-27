// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


// #if WITH_EDITOR

// #pragma once 

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
//#include "Editor/Blutility/Classes/EditorUtilityWidget.h"

#include "MapGeneratorWidget.generated.h"

// /**
//  * 
//  */
UCLASS(BlueprintType)
//UCLASS()
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    // UMapGeneratorWidget();
    // ~UMapGeneratorWidget();

    UFUNCTION(BlueprintCallable)
    void HelloWorld();
};
// #endif