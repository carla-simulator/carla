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

	// UE5.8: FSceneViewport(FViewportClient*, ...) is deprecated; Create also
	// wires the widget's viewport interface.
	SceneViewport = FSceneViewport::Create(ViewportClient, ViewportWidget);
	ViewportClient->Viewport = SceneViewport.Get();

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
	LastFramedBounds = Bounds;
	if (!Bounds.IsValid)
	{
		// Nothing to frame yet (a pole without meshes): keep the default view.
		ViewportClient->SetViewLocation(FVector(-300, 0, 150));
		ViewportClient->SetViewRotation(FRotator(0, 0, 0));
		return;
	}

	// Three-quarter front view: the heads face +Y, so look back at the centre of the bounds
	// from +Y turned 12 degrees towards -X (more and the visors hide the lamps), from the distance at which the bounds fill the
	// frame (projected half width or half height over the tangent of the half field of view,
	// plus the projected half depth), with a 15 % margin.
	const FVector Center{Bounds.GetCenter()};
	const FVector Extent{Bounds.GetExtent()};
	const double ViewYaw{FMath::DegreesToRadians(12.0)};
	const FVector ToCamera{-FMath::Sin(ViewYaw), FMath::Cos(ViewYaw), 0.0};
	const double HalfWidth{FMath::Abs(Extent.X * ToCamera.Y) + FMath::Abs(Extent.Y * ToCamera.X)};
	const double HalfDepth{FMath::Abs(Extent.X * ToCamera.X) + FMath::Abs(Extent.Y * ToCamera.Y)};
	const double HalfFOV{FMath::DegreesToRadians(ViewportClient->ViewFOV * 0.5)};
	const double HalfSize{FMath::Max(HalfWidth, Extent.Z)};
	const double Distance{HalfSize / FMath::Tan(HalfFOV) * 1.15 + HalfDepth};
	const FVector CamPos{Center + ToCamera * Distance};
	const FRotator CamRot{(-ToCamera).Rotation()};

	ViewportClient->SetViewLocation(CamPos);
	ViewportClient->SetViewRotation(CamRot);
}

void STrafficLightPreviewViewport::ResetFrameIfBoundsChanged()
{
	if (!IsValid(PreviewTrafficLight) || !ViewportClient.IsValid())
	{
		return;
	}
	const FBox Bounds{PreviewTrafficLight->GetComponentsBoundingBox(true)};
	if (Bounds.IsValid != LastFramedBounds.IsValid ||
		(Bounds.IsValid && !Bounds.GetCenter().Equals(LastFramedBounds.GetCenter(), 1.0)) ||
		(Bounds.IsValid && !Bounds.GetExtent().Equals(LastFramedBounds.GetExtent(), 1.0)))
	{
		ResetFrame();
	}
}
