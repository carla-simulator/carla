// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "IntersectionEntrance.h"


// Sets default values
AIntersectionEntrance::AIntersectionEntrance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AIntersectionEntrance::BeginPlay()
{
  Super::BeginPlay();

}

// Called every frame
void AIntersectionEntrance::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

}

TArray<FVector> AIntersectionEntrance::GetRoute(int it)
{
  TArray<AActor*> points = Routes[it].points;
  TArray<FVector> route;

  for (int i = 0; i < points.Num(); ++i){
    route.Add(points[i]->GetActorLocation());
  }

  return route;
}

float AIntersectionEntrance::GetProbability(int it)
{
  return Routes[it].probability;
}

/*
#if WITH_EDITOR

void AIntersectionEntrance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bCreateRoutes && (GetWorld() != nullptr)) {
      //ClearRoutes();
      for (int i = 0; i < Routes.Num(); ++i){
          for(int e = 0; e < Routes[i].points.Num(); ++e){
            AActor* actor= GetWorld()->SpawnActor<AActor>();//USphereComponent* createdComp = NewObject<USphereComponent>(this);//CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
        USceneComponent* SphereMesh = NewObject<USceneComponent>(actor);
          SphereMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepWorldTransform);
          if(actor)
          {
             actor->RegisterAllComponents();
             Routes[i].points[e] = actor;
             //Routes[i].points[e].position = createdComp->GetRelativeTransform().GetLocation();
          }
          }
      }
    }
  }
  bCreateRoutes = false;
}
#endif // WITH_EDITOR
*/

