// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include <util/ue-header-guard-end.h>

#include "BlueprintParentClassFixer.generated.h"

/**
 * 
 */

UENUM()
enum class EFixClass : uint8 {
	BLUEPRINT = 0 UMETA(DisplayName = "BLUEPRINTS"),
	ANIMBLUEPRINT = 1  UMETA(DisplayName = "ANIMBLUEPRINT")
};

UCLASS()
class CARLATOOLS_API ABlueprintParentClassFixer : public AEditorUtilityActor
{
	GENERATED_BODY()
	
public:
	ABlueprintParentClassFixer();

	UPROPERTY(EditAnywhere, Category = "Settings")
	EFixClass ClassToFix;

	UPROPERTY(EditAnywhere, Category = "Settings")
	UClass* NewParentClass;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ToolTip = "Includes /Game/. The folder route must finish in '/'."))
	FString PathToSearchClassOn = "/Carla/Blueprints/";

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FString> ContainsStringCondition;

	UFUNCTION(CallInEditor, Category = "Settings")
	void FixBlueprints();

private:
	UPROPERTY()
	USceneComponent* SceneComponent;
};
