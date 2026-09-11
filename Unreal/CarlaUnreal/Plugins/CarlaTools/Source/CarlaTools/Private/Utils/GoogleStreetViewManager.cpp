#include "Utils/GoogleStreetViewManager.h"

void AGoogleStreetViewManager::Initialize(UWorld* World, FTransform CameraTransform, FVector2D InOriginGeoCoordinates, const FString& InGoogleAPIKey)
{
    CameraActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraTransform);
    CameraActor->SetActorLabel(TEXT("GoogleStreetViewCamera"));

    LastCameraLocation = CameraActor->GetActorLocation();
    LastCameraRotation = CameraActor->GetActorRotation();

    OriginGeoCoordinates = InOriginGeoCoordinates;
    GoogleAPIKey = InGoogleAPIKey;

    PrimaryActorTick.bCanEverTick = true;
    MovementThreshold = 50.f;
    RotationThreshold = 5.0f;
}

void AGoogleStreetViewManager::BeginPlay()
{
    Super::BeginPlay();

    Fetcher = NewObject<UGoogleStreetViewFetcher>();
    Fetcher->AddToRoot();

    Fetcher->Initialize(CameraActor, OriginGeoCoordinates, GoogleAPIKey);
    Fetcher->RequestGoogleStreetViewImage();
}

void AGoogleStreetViewManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!CameraActor || !Fetcher) return;

    FVector CurrentLocation = CameraActor->GetActorLocation();
    FRotator CurrentRotation = CameraActor->GetActorRotation();

    if (FVector::Dist(CurrentLocation, LastCameraLocation) > MovementThreshold ||
        FMath::Abs(CurrentRotation.Yaw - LastCameraRotation.Yaw) > RotationThreshold)
    {
        LastCameraLocation = CurrentLocation;
        LastCameraRotation = CurrentRotation;

        Fetcher->SetCamera(CameraActor);
        Fetcher->RequestGoogleStreetViewImage();
    }
}
