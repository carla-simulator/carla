// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "WheelParentClassFixer.generated.h"

/**
 * 
 */
UCLASS()
class CARLATOOLS_API AWheelParentClassFixer : public AEditorUtilityActor
{
	GENERATED_BODY()
	
public:
	AWheelParentClassFixer();

	UPROPERTY(EditAnywhere)
	UClass* NewParentClass;

	UFUNCTION(CallInEditor, Category = "Settings")
	void FixWheelClassesParentActor();

private:
	UPROPERTY()
	USceneComponent* SceneComponent;
};
