#pragma once
#include "CoreMinimal.h"
#include "EditorCamera.generated.h"




UCLASS(BlueprintType)
class CARLATOOLS_API UEditorCameraUtils :
	public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Get();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Set();



	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTransform CameraTransform;

};