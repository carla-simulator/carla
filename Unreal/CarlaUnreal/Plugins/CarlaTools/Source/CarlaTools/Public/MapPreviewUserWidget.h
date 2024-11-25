// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <boost/asio.hpp>
#include <util/enable-ue4-macros.h>

#include <memory>

#include "MapPreviewUserWidget.generated.h"

class FSocket;
class UTexture2D;

UCLASS()
class CARLATOOLS_API UMapPreviewUserWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	// Boost socket
  boost::asio::io_service io_service;
  std::unique_ptr<boost::asio::ip::tcp::socket> SocketPtr;


	bool SendStr(FString Msg);
	FString RecvCornersLatLonCoords();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* MapTexture;

	UPROPERTY(BlueprintReadOnly)
	float TopRightLat = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float TopRightLon = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float BottomLeftLat = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float BottomLeftLon = 0.f;

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

	UFUNCTION(BlueprintCallable)
	void UpdateLatLonCoordProperties();
	
};
