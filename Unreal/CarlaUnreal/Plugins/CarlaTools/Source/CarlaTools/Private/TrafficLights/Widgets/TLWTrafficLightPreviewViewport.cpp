#include "TrafficLights/Widgets/TLWTrafficLightPreviewViewport.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreGlobals.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Light.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "Math/MathFwd.h"
#include "PreviewScene.h"
#include "SEditorViewport.h"
#include "TrafficLights/TrafficLightActor.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/SViewport.h"

void STrafficLightPreviewViewport::Construct(const FArguments& InArgs)
{
	PreviewScene = MakeUnique<FPreviewScene>(FPreviewScene::ConstructionValues());

	float WorldSunIntensity{1.0f};
	FLinearColor WorldSunColor{FLinearColor::White};
	float WorldSkyIntensity{1.0f};
	FLinearColor WorldSkyColor{FLinearColor::White};

	UWorld* EditorWorld{GEditor->GetEditorWorldContext().World()};
	if (IsValid(EditorWorld))
	{
		APostProcessVolume* GlobalPPV{nullptr};
		for (TActorIterator<APostProcessVolume> It(EditorWorld); It; ++It)
		{
			if (It->bUnbound)
			{
				GlobalPPV = *It;
				break;
			}
		}
		if (!GlobalPPV)
		{
			for (TActorIterator<APostProcessVolume> It(EditorWorld); It; ++It)
			{
				GlobalPPV = *It;
				break;
			}
		}

		if (IsValid(GlobalPPV) && IsValid(PreviewScene->GetWorld()))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.ObjectFlags |= RF_Transient;

			APostProcessVolume* PreviewPPV{PreviewScene->GetWorld()->SpawnActor<APostProcessVolume>(
				APostProcessVolume::StaticClass(), FTransform::Identity, SpawnParams)};
			PreviewPPV->bUnbound = true;
			PreviewPPV->BlendWeight = GlobalPPV->BlendWeight;
			PreviewPPV->Settings = GlobalPPV->Settings;
		}

		for (TObjectIterator<UDirectionalLightComponent> It; It; ++It)
		{
			if (It->GetWorld() == EditorWorld)
			{
				WorldSunIntensity = It->Intensity;
				WorldSunColor = It->LightColor;
				break;
			}
		}
		for (TObjectIterator<USkyLightComponent> It; It; ++It)
		{
			if (It->GetWorld() == EditorWorld)
			{
				WorldSkyIntensity = It->Intensity;
				WorldSkyColor = It->LightColor;
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PreviewViewport] No editor world found for preview scene"));
	}

	if (IsValid(PreviewScene->DirectionalLight))
	{
		PreviewScene->DirectionalLight->SetIntensity(WorldSunIntensity);
		PreviewScene->DirectionalLight->SetLightColor(WorldSunColor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PreviewViewport] No directional light found in preview scene"));
	}
	if (IsValid(PreviewScene->SkyLight))
	{
		PreviewScene->SkyLight->SetIntensity(WorldSkyIntensity);
		PreviewScene->SkyLight->SetLightColor(WorldSkyColor);
		PreviewScene->SkyLight->RecaptureSky();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PreviewViewport] No sky light found in preview scene"));
	}

	{
		UWorld* PreviewWorld{PreviewScene->GetWorld()};
		if (PreviewWorld)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.ObjectFlags |= RF_Transient;
			PreviewTrafficLight =
				PreviewWorld->SpawnActor<ATrafficLightActor>(ATrafficLightActor::StaticClass(), FTransform::Identity, SpawnParams);
		}
	}

	ViewportClient = MakeShareable(new FEditorViewportClient(nullptr, PreviewScene.Get(), nullptr));
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->SetRealtime(false);
	ViewportClient->SetViewLocation(FVector(-300, 0, 150));
	ViewportClient->SetViewRotation(FRotator(0, 0, 0));
	ViewportClient->SetViewMode(VMI_Lit);
	ViewportClient->SetAllowCinematicControl(true);
	ViewportClient->VisibilityDelegate.BindLambda([]() { return true; });
	ViewportClient->EngineShowFlags.SetGrid(true);

	SAssignNew(ViewportWidget, SViewport).EnableGammaCorrection(false).EnableBlending(true);

	SceneViewport = MakeShareable(new FSceneViewport(ViewportClient.Get(), ViewportWidget));
	ViewportClient->Viewport = SceneViewport.Get();
	ViewportWidget->SetViewportInterface(SceneViewport.ToSharedRef());

	ChildSlot[ViewportWidget.ToSharedRef()];
}

STrafficLightPreviewViewport::~STrafficLightPreviewViewport()
{
	if (PreviewTrafficLight)
	{
		PreviewTrafficLight->Destroy();
		PreviewTrafficLight = nullptr;
	}
	if (ViewportClient.IsValid())
	{
		ViewportClient->Viewport = nullptr;
		FlushRenderingCommands();
		PreviewScene.Reset();
	}
}

void STrafficLightPreviewViewport::Reload()
{
	if (SceneViewport.IsValid())
	{
		SceneViewport->Invalidate();
	}
	if (ViewportClient.IsValid())
	{
		ViewportClient->Invalidate();
	}
}

void STrafficLightPreviewViewport::ResetFrame()
{
	check(IsValid(PreviewTrafficLight));
	check(ViewportClient.IsValid());

	const FBox Bounds{PreviewTrafficLight->GetComponentsBoundingBox(true)};
	const FVector Center{Bounds.GetCenter()};
	const double Radius{Bounds.GetExtent().GetMax()};
	const double Distance{Radius * -10.0};
	const FVector Forward{FVector::ForwardVector.Rotation().RotateVector(FVector(0, 1, 0))};
	const FVector Up{FVector::UpVector};
	const FVector CamPos{Center - Forward * Distance + Up * (Radius * 0.5)};
	const FRotator CamRot(0.0, -90.0, 0.0);

	ViewportClient->SetViewLocation(CamPos);
	ViewportClient->SetViewRotation(CamRot);
}
