#include "Utils/GoogleStreetViewFetcher.h"
#include "HttpModule.h"
#include "ImageUtils.h"
#include "Misc/Base64.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogGoogleStreetView);

void UGoogleStreetViewFetcher::Initialize(ACameraActor* InCameraActor, FVector2D InOriginGeoCoordinates, const FString& InGoogleAPIKey)
{
    CameraActor = InCameraActor;
    OriginGeoCoordinates = InOriginGeoCoordinates;
    GoogleAPIKey = InGoogleAPIKey;
}

void UGoogleStreetViewFetcher::SetCamera(ACameraActor* InCameraActor)
{
    CameraActor = InCameraActor;
}

void UGoogleStreetViewFetcher::RequestGoogleStreetViewImage()
{
    if (!CameraActor)
    {
        UE_LOG(LogGoogleStreetView, Warning, TEXT("Camera actor is null."));
        return;
    }

    double OriginLat = OriginGeoCoordinates.X;
    double OriginLon = OriginGeoCoordinates.Y;

    FVector CameraLocation = CameraActor->GetActorLocation();
    FRotator CameraRotation = CameraActor->GetActorRotation();

    FVector2D latlon = UMapGenFunctionLibrary::InverseTransverseMercatorProjection( CameraLocation.X, CameraLocation.Y, OriginLat, OriginLon );
    double lat = latlon.X;
    double lon = latlon.Y;
    int heading = static_cast<int>(FMath::Fmod(CameraRotation.Yaw + 360.0f, 360.0f));

    FString URL = FString::Printf(
        TEXT("https://maps.googleapis.com/maps/api/streetview?size=640x480&location=%.6f,%.6f&heading=%d&pitch=0&key=%s"),
        lat, lon, heading, *GoogleAPIKey);

    UE_LOG(LogGoogleStreetView, Log, TEXT("Requesting Google Street View image from URL: %s"), *URL);

    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb("GET");
    Request->OnProcessRequestComplete().BindUObject(this, &UGoogleStreetViewFetcher::OnStreetViewResponseReceived);
    Request->ProcessRequest();
}

void UGoogleStreetViewFetcher::OnStreetViewResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        const TArray<uint8>& ImageData = Response->GetContent();
        UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(ImageData);

        if (Texture)
        {
            UE_LOG(LogGoogleStreetView, Log, TEXT("Successfully created Google Street View texture."));
            StreetViewTexture = Texture;

            if (CameraActor)
            {
                ApplyCameraTexture();
            }
            else{
                UE_LOG(LogGoogleStreetView, Error, TEXT("No Camera Actor available."));
            }
        }
        else
        {
            UE_LOG(LogGoogleStreetView, Warning, TEXT("Failed to convert Google Street View image to texture."));
        }
    }
    else
    {
        UE_LOG(LogGoogleStreetView, Error, TEXT("Failed to retrieve Google Street View image. HTTP code: %d"),
            Response.IsValid() ? Response->GetResponseCode() : -1);
    }
}

void UGoogleStreetViewFetcher::ApplyCameraTexture()
{
    // Create a dynamic material instance from the post process material
    UMaterialInterface* BasePPMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CarlaDigitalTwinsTool/digitalTwins/Static/Utils/M_GoogleStreetViewPost.M_GoogleStreetViewPost"));

    if (!StreetViewPostProcessMaterial && BasePPMat)
    {
        StreetViewPostProcessMaterial = UMaterialInstanceDynamic::Create(BasePPMat, this);
    }

    // Assign the fetched texture
    if (StreetViewTexture)
    {
        StreetViewPostProcessMaterial->SetTextureParameterValue("StreetViewTex", StreetViewTexture);
    }
    else{
        UE_LOG(LogGoogleStreetView, Error, TEXT("No StreetViewTexture."));
    }

    // Apply the post-process material to the camera
    if (CameraActor && CameraActor->FindComponentByClass<UCameraComponent>())
    {
        UCameraComponent* Cam = CameraActor->FindComponentByClass<UCameraComponent>();
        // Cam->PostProcessSettings.bOverride_WeightedBlendables = true;    // Needed for UE5

        FWeightedBlendable Blendable;
        Blendable.Object = StreetViewPostProcessMaterial;
        Blendable.Weight = 1.0f;
        Cam->PostProcessSettings.WeightedBlendables.Array.Add(Blendable);
    }
    else{
        UE_LOG(LogGoogleStreetView, Error, TEXT("No camera actor or camera component."));
    }
}
