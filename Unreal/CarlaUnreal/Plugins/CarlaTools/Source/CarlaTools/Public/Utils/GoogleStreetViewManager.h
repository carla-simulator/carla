#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Utils/GoogleStreetViewFetcher.h"
#include "GoogleStreetViewManager.generated.h"

UCLASS()
class CARLATOOLS_API AGoogleStreetViewManager : public AActor
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "GoogleStreetView")
    void Initialize(UWorld* World, FTransform CameraTransform, FVector2D InOriginGeoCoordinates, const FString& InGoogleAPIKey);

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    ACameraActor* CameraActor;

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    FVector2D OriginGeoCoordinates;

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    FString GoogleAPIKey;

    UPROPERTY(EditAnywhere, Category = "GoogleStreetView")
    UGoogleStreetViewFetcher* Fetcher;

    FVector LastCameraLocation;

    FRotator LastCameraRotation;

    float MovementThreshold;

    float RotationThreshold;
};
