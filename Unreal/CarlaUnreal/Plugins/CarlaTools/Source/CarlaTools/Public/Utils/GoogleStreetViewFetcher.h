#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "GoogleStreetViewFetcher.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGoogleStreetView, Log, All);

UCLASS(Blueprintable)
class CARLATOOLS_API UGoogleStreetViewFetcher : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GoogleStreetView")
    void Initialize(ACameraActor* InCameraActor, FVector2D OriginGeoCoordinates, const FString& InGoogleAPIKey);

    UFUNCTION(BlueprintCallable, Category = "GoogleStreetView")
    void SetCamera(ACameraActor* InCameraActor);

    UFUNCTION(BlueprintCallable, Category = "GoogleStreetView")
    void RequestGoogleStreetViewImage();

    UFUNCTION(BlueprintCallable, Category = "GoogleStreetView")
    void ApplyCameraTexture();

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    ACameraActor* CameraActor;

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    UTexture2D* StreetViewTexture;

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    UStaticMeshComponent* TargetMeshComponent = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* StreetViewPostProcessMaterial;

private:
    void OnStreetViewResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    FVector2D OriginGeoCoordinates;
    FString GoogleAPIKey;
};
