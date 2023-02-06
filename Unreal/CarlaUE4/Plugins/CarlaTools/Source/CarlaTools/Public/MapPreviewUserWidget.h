// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapPreviewUserWidget.generated.h"

class FSocket;
class UTexture2D;

UCLASS()
class CARLATOOLS_API UMapPreviewUserWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	FSocket* Socket;

	bool SendStr(FString Msg);
	//void PaintMapToTexture(TArray<);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* MapTexture;

	UFUNCTION(BlueprintCallable)
	void ConnectToSocket();

	UFUNCTION(BlueprintCallable)
	void RenderMap();

	UFUNCTION(BlueprintCallable)
	void Shutdown();
	
};
