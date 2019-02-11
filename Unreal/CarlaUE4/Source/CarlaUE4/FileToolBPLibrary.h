// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FileToolBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CARLAUE4_API UFileToolBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:
	/** Saves text to filename */
	UFUNCTION(BlueprintCallable, Category = "FileToolBPLibrary|File IO")
	static bool SaveStringTextToFile(FString SaveDirectory, FString FileName, FString SaveText, bool AllowOverWriting = false);

	UFUNCTION(BlueprintCallable, Category = "FileToolBPLibrary|File IO")
	static bool SaveStringArrayTextToFile(FString SaveDirectory, FString FileName, TArray<FString> SaveTextArray, bool AllowOverWriting = false);
};
