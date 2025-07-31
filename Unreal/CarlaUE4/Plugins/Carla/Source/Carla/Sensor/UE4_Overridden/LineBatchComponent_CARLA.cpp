// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Sensor/UE4_Overridden/LineBatchComponent_CARLA.h"
#include "Sensor/CosmosControlSensor.h"

FLineBatcherSceneProxy_CARLA::FLineBatcherSceneProxy_CARLA(const ULineBatchComponent_CARLA* InComponent) :
	FLineBatcherSceneProxy(InComponent), Lines(InComponent->BatchedLines),
	Points(InComponent->BatchedPoints), Meshes(InComponent->BatchedMeshes)
{
	bWillEverBeLit = false;
	//SetCustomDepthEnabled_GameThread(true);
	//SetCustomDepthStencilValue_GameThread(250);
}


FPrimitiveSceneProxy* ULineBatchComponent_CARLA::CreateSceneProxy()
{
	return new FLineBatcherSceneProxy_CARLA(this);
}

SIZE_T FLineBatcherSceneProxy_CARLA::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}
/**
*  Returns a struct that describes to the renderer when to draw this proxy.
*	@param		Scene view to use to determine our relevence.
*  @return		View relevance struct
*/
FPrimitiveViewRelevance FLineBatcherSceneProxy_CARLA::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance ViewRelevance;

	ViewRelevance.bDrawRelevance = Cast<ACosmosControlSensor>(View->ViewActor) != nullptr;//|| View->ViewActor->IsA(ACosmosControlSensor::StaticClass());
	ViewRelevance.bDynamicRelevance = true;
	// ideally the TranslucencyRelevance should be filled out by the material, here we do it conservative
	ViewRelevance.bSeparateTranslucency = ViewRelevance.bNormalTranslucency = true;
	return ViewRelevance;
}