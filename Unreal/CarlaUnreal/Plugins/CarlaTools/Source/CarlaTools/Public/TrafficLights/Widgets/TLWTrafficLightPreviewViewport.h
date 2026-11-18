// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "Engine/DataTable.h"
#include "PreviewScene.h"
#include "Slate/SceneViewport.h"
#include "TrafficLights/TLHead.h"
#include "TrafficLights/TLModule.h"
#include "TrafficLights/TLPole.h"
#include "TrafficLights/TrafficLightActor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SViewport.h"

class STrafficLightPreviewViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STrafficLightPreviewViewport)
	{
	}
	SLATE_END_ARGS()

	/** Construct the preview viewport widget */
	void Construct(const FArguments& InArgs);

	/** Destructor: clear viewport reference to avoid crash */
	virtual ~STrafficLightPreviewViewport();

	inline ATrafficLightActor* GetPreviewTrafficLight() const
	{
		return PreviewTrafficLight;
	}

	void Reload();
	void ResetFrame();
	/** Frame the preview again when its bounds changed since the last framing. */
	void ResetFrameIfBoundsChanged();

private:
	ATrafficLightActor* PreviewTrafficLight{nullptr};
	FBox LastFramedBounds{ForceInit};

	TUniquePtr<FPreviewScene> PreviewScene;
	TSharedPtr<FEditorViewportClient> ViewportClient;
	TSharedPtr<FSceneViewport> SceneViewport;
	TSharedPtr<SViewport> ViewportWidget;
};
