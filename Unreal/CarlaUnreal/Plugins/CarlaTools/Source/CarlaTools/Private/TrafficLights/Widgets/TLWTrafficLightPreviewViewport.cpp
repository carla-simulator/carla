#include "TrafficLights/Widgets/TLWTrafficLightPreviewViewport.h"

#include "CarlaTools.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreGlobals.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Light.h"
#include "Engine/TextureCube.h"
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
	// The preview has its own daylight and a fixed exposure, independent of the map open in the
	// editor. Copying the map's sun, sky light and post process (as it used to) breaks with the
	// CARLA maps: their sky light captures the map's sky, which the preview scene lacks (black),
	// and their exposure is tuned for that sky. Physical values, so the tool's lamps (emissive
	// 50000 nits when lit) read as lamps: a 50000 lux sun, a sky light from the engine's daylight
	// cubemap, and EV100 13, a bright overcast day.
	PreviewScene = MakeUnique<FPreviewScene>(FPreviewScene::ConstructionValues()
		.SetLightBrightness(50000.0f)
		.SetSkyBrightness(8000.0f)
		.SetLightRotation(FRotator(-40.0f, -60.0f, 0.0f)));
	if (UTextureCube* const SkyCubemap{LoadObject<UTextureCube>(
			nullptr, TEXT("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap"))})
	{
		PreviewScene->SetSkyCubemap(SkyCubemap);
	}
	else
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("Traffic Light Tool preview: daylight cubemap not found; the preview has no sky light"));
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
	ViewportClient->ExposureSettings.bFixed = true;
	ViewportClient->ExposureSettings.FixedEV100 = 13.0f;

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
