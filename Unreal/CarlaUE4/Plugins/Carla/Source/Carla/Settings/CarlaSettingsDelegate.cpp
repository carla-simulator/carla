#include "Carla.h"
#include "Engine/World.h"
#include "Game/CarlaGameModeBase.h"
#include "Settings/CarlaSettings.h"
#include "CarlaSettingsDelegate.h"

UCarlaSettingsDelegate::UCarlaSettingsDelegate() :
  ActorSpawnedDelegate(FOnActorSpawned::FDelegate::CreateUObject(this, &UCarlaSettingsDelegate::OnActorSpawned))
{
}

void UCarlaSettingsDelegate::RegisterSpawnHandler(UWorld *InWorld)
{
  check(InWorld!=nullptr);
  auto GameInstance  = Cast<UCarlaGameInstance>(InWorld->GetGameInstance());
  check(GameInstance!=nullptr);
  CarlaSettings = &GameInstance->GetCarlaSettings();
  check(CarlaSettings!=nullptr);
  InWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
}

void UCarlaSettingsDelegate::OnActorSpawned(AActor* InActor)
{
  check(CarlaSettings!=nullptr);
  if (InActor != nullptr && IsValid(InActor) && !InActor->IsPendingKill()) 
  {
     //apply settings for this actor for the current quality level
	   TArray<UActorComponent*> components = InActor->GetComponentsByClass(UPrimitiveComponent::StaticClass());
	   float dist = CarlaSettings->LowStaticMeshMaxDrawDistance;
     const float maxscale = InActor->GetActorScale().GetMax();
     #define _MAX_SCALE_SIZE 50.0f
     
	   if(maxscale>_MAX_SCALE_SIZE) {
        dist *= 100.0f;
     }

	   for(int32 i=0; i<components.Num(); i++)
  	 {
		  UPrimitiveComponent* primitivecomponent = Cast<UPrimitiveComponent>(components[i]);
		  if(IsValid(primitivecomponent))
		  {
			 primitivecomponent->SetCullDistance(dist);
		  }
	   }
  }
}


void UCarlaSettingsDelegate::ApplyQualitySettingsLevelPostRestart() const
{
	switch(CarlaSettings->GetQualitySettingsLevel())
	{
	  case EQualitySettingsLevel::Low: 
	  	ApplyLowQualitySettings();
		  break;
	  case EQualitySettingsLevel::Medium: 
      /** @todo */
      break;
	  case EQualitySettingsLevel::High: 
      /** @todo */
      break;
	  default: case EQualitySettingsLevel::Epic: 
      /** @todo */
      break;
	}
}


void UCarlaSettingsDelegate::LaunchLowQualityCommands(UWorld * world) const
{
  //launch commands to lower quality settings
  GEngine->Exec(world,TEXT("r.DefaultFeature.MotionBlur 0"));
  GEngine->Exec(world,TEXT("r.DefaultFeature.Bloom 0"));
  GEngine->Exec(world,TEXT("r.DefaultFeature.AmbientOcclusion 0"));
  GEngine->Exec(world,TEXT("r.AmbientOcclusionLevels 0"));
  GEngine->Exec(world,TEXT("r.DefaultFeature.AmbientOcclusionStaticFraction 0"));
  GEngine->Exec(world,TEXT("r.DefaultFeature.AutoExposure 0"));
  GEngine->Exec(world,TEXT("r.RHICmdBypass 0"));
  GEngine->Exec(world,TEXT("r.DefaultFeature.AntiAliasing 2"));
  GEngine->Exec(world,TEXT("r.Streaming.PoolSize 2000"));
  GEngine->Exec(world,TEXT("r.HZBOcclusion 0"));
  GEngine->Exec(world,TEXT("r.MinScreenRadiusForLights 0.01"));
  GEngine->Exec(world,TEXT("r.SeparateTranslucency 0"));
  GEngine->Exec(world,TEXT("r.FinishCurrentFrame 0"));
  GEngine->Exec(world,TEXT("r.MotionBlurQuality 0"));
  GEngine->Exec(world,TEXT("r.PostProcessAAQuality 0"));
  GEngine->Exec(world,TEXT("r.BloomQuality 1"));
  GEngine->Exec(world,TEXT("r.SSR.Quality 0"));
  GEngine->Exec(world,TEXT("r.DepthOfFieldQuality 0"));
  GEngine->Exec(world,TEXT("r.SceneColorFormat 2"));
  GEngine->Exec(world,TEXT("r.TranslucencyVolumeBlur 0"));
  GEngine->Exec(world,TEXT("r.TranslucencyLightingVolumeDim 4"));
  GEngine->Exec(world,TEXT("r.MaxAnisotropy 8"));
  GEngine->Exec(world,TEXT("r.LensFlareQuality 0"));
  GEngine->Exec(world,TEXT("r.SceneColorFringeQuality 0"));
  GEngine->Exec(world,TEXT("r.FastBlurThreshold 0"));
  GEngine->Exec(world,TEXT("r.SSR.MaxRoughness 0.1"));
  GEngine->Exec(world,TEXT("r.AllowOcclusionQueries 1"));
  GEngine->Exec(world,TEXT("r.SSR 0"));
  GEngine->Exec(world,TEXT("r.StencilForLODDither 1"));
  GEngine->Exec(world,TEXT("r.EarlyZPass 2")); //transparent before opaque
  GEngine->Exec(world,TEXT("r.EarlyZPassMovable 1"));
  GEngine->Exec(world,TEXT("r.Foliage.DitheredLOD 0"));
  GEngine->Exec(world,TEXT("r.ForwardShading 0"));
  GEngine->Exec(world,TEXT("sg.PostProcessQuality 0"));
  //GEngine->Exec(world,TEXT("r.ViewDistanceScale 0.1")); //--> too extreme (far clip too short)
  GEngine->Exec(world,TEXT("sg.ShadowQuality 0"));
  GEngine->Exec(world,TEXT("sg.TextureQuality 0"));
  GEngine->Exec(world,TEXT("sg.EffectsQuality 0"));
  GEngine->Exec(world,TEXT("FoliageQuality 0"));
  GEngine->Exec(world,TEXT("foliage.DensityScale 0"));
  GEngine->Exec(world,TEXT("grass.DensityScale 0"));
  GEngine->Exec(world,TEXT("r.TranslucentLightingVolume 0"));
  GEngine->Exec(world,TEXT("r.LightShaftDownSampleFactor 4"));
  GEngine->Exec(world,TEXT("r.OcclusionQueryLocation 1"));
  //GEngine->Exec(world,TEXT("r.BasePassOutputsVelocity 0")); //--> readonly
  //world->Exec(world,TEXT("r.DetailMode 0")); //-->will change to lods 0
}

void UCarlaSettingsDelegate::SetAllLightsLowQuality(UWorld* world) const
{
  TArray<AActor*> actors;
  UGameplayStatics::GetAllActorsOfClass(world, ALight::StaticClass(), actors);
  for(int32 i=0;i<actors.Num();i++)
  {
	if(!IsValid(actors[i]) || actors[i]->IsPendingKillPending()) continue;
    //tweak directional lights
    ADirectionalLight* directionallight = Cast<ADirectionalLight>(actors[i]);
    if(directionallight)
    {
      directionallight->SetCastShadows(false);
      directionallight->SetLightFunctionFadeDistance(CarlaSettings->LowLightFadeDistance);
      continue;
    }
    //disable any other type of light
    actors[i]->SetActorHiddenInGame(true);
  }
}

void UCarlaSettingsDelegate::SetAllRoadsLowQuality(UWorld* world) const
{
  if(CarlaSettings->LowRoadMaterials.Num()==0) return; //no materials configured for low quality
  TArray<AActor*> actors;
  UGameplayStatics::GetAllActorsWithTag(world, UCarlaSettings::CARLA_ROAD_TAG,actors);
  for(int32 i=0; i<actors.Num(); i++)
  {
	if(!IsValid(actors[i]) || actors[i]->IsPendingKillPending()) continue;
  	TArray<UActorComponent*> components = actors[i]->GetComponentsByClass(UStaticMeshComponent::StaticClass());
  	for(int32 j=0; j<components.Num(); j++)
  	{
  	  UStaticMeshComponent* staticmesh = Cast<UStaticMeshComponent>(components[j]);
  	  if(staticmesh)
  	  {
        TArray<FName> slotsnames = staticmesh->GetMaterialSlotNames();
        for(int32 k=0; k<slotsnames.Num(); k++)
        {
          const FName &slotname = slotsnames[k];
          bool found = CarlaSettings->LowRoadMaterials.ContainsByPredicate([staticmesh,slotname](const FStaticMaterial& material)
          {
            if(material.MaterialSlotName.IsEqual(slotname))
            {
              staticmesh->SetMaterial(
                staticmesh->GetMaterialIndex(slotname), 
                material.MaterialInterface
              );
              return true;
            } else return false;
          });
        }
  	  }
  	}
  }
}

void UCarlaSettingsDelegate::SetAllActorsDrawDistance(UWorld* world, const float max_draw_distance) const
{
  ///@TODO: use semantics to grab all actors by type (vehicles,ground,people,props) and set different distances configured in the global properties
  TArray<AActor*> actors;
  #define _MAX_SCALE_SIZE 50.0f
  //set the lower quality - max draw distance
  UGameplayStatics::GetAllActorsOfClass(world, AActor::StaticClass(),actors);
  for(int32 i=0; i<actors.Num(); i++)
  {
	 if(!IsValid(actors[i]) || actors[i]->IsPendingKillPending() || actors[i]->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG)) continue;
     TArray<UActorComponent*> components = actors[i]->GetComponentsByClass(UPrimitiveComponent::StaticClass());
	   float dist = max_draw_distance;
     const float maxscale = actors[i]->GetActorScale().GetMax();
	   if(maxscale>_MAX_SCALE_SIZE)  dist *= 100.0f;
	   for(int32 j=0; j<components.Num(); j++)
  	 {
		  UPrimitiveComponent* primitivecomponent = Cast<UPrimitiveComponent>(components[j]);
		  if(IsValid(primitivecomponent))
		  {
			 primitivecomponent->SetCullDistance(dist);
		  }
	   }
     
  }
}


void UCarlaSettingsDelegate::SetPostProcessEffectsEnabled(UWorld* world, bool enabled) const
{
  TArray<AActor*> actors;
  UGameplayStatics::GetAllActorsOfClass(world, APostProcessVolume::StaticClass(), actors);
  for(int32 i=0; i<actors.Num(); i++)
  {
	if(!IsValid(actors[i]) || actors[i]->IsPendingKillPending()) continue;
    APostProcessVolume* postprocessvolume = Cast<APostProcessVolume>(actors[i]);
	if(postprocessvolume)
	{
		postprocessvolume->bEnabled = enabled;
	}
  }
}

void UCarlaSettingsDelegate::ApplyLowQualitySettings() const
{
  UWorld *world = GetWorld();
  if(!world) return ;
  LaunchLowQualityCommands(world);
	
  //iterate all directional lights, deactivate shadows
  SetAllLightsLowQuality(world);
  
  //Set all the roads the low quality material
  SetAllRoadsLowQuality(world);

  //Set all actors with static meshes a max disntace configured in the global settings for the low quality
  SetAllActorsDrawDistance(world, CarlaSettings->LowStaticMeshMaxDrawDistance);

  //Disable all post process volumes
  SetPostProcessEffectsEnabled(world, false);
}