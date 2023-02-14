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

	double TopRightLat = 0.f;
	double TopRightLon = 0.f;
	double BottomLeftLat = 0.f;
	double BottomLeftLon = 0.f;

	bool SendStr(FString Msg);
	void RecvCornersLatLonCoords();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* MapTexture;

	UFUNCTION(BlueprintCallable)
	void ConnectToSocket(FString DatabasePath, FString StylesheetPath, int Size);

	UFUNCTION(BlueprintCallable)
	void RenderMap(FString Latitude, FString Longitude, FString Zoom);

	UFUNCTION(BlueprintCallable)
	void Shutdown();

	UFUNCTION(BlueprintCallable)
	void CreateTexture();

	UFUNCTION(BlueprintCallable)
	void OpenServer();

	UFUNCTION(BlueprintCallable)
	void CloseServer();
	
};
