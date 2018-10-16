// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RoutePlanner.h"

#include "Util/RandomEngine.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/WheeledVehicleAIController.h"

#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"

static bool IsSplineValid(const USplineComponent *SplineComponent)
{
  return (SplineComponent != nullptr) &&
         (SplineComponent->GetNumberOfSplinePoints() > 1);
}

static AWheeledVehicleAIController *GetVehicleController(AActor *Actor)
{
  auto *Vehicle = (Actor->IsPendingKill() ? nullptr : Cast<ACarlaWheeledVehicle>(Actor));
  return (Vehicle != nullptr ?
      Cast<AWheeledVehicleAIController>(Vehicle->GetController()) :
      nullptr);
}

static const USplineComponent *PickARoute(
    URandomEngine &RandomEngine,
    const TArray<USplineComponent *> &Routes,
    const TArray<float> &Probabilities)
{
  check(Routes.Num() > 0);

  if (Routes.Num() == 1) {
    return Routes[0];
  }

  auto Index = RandomEngine.GetIntWithWeight(Probabilities);
  check((Index >= 0) && (Index < Routes.Num()));
  return Routes[Index];
}

ARoutePlanner::ARoutePlanner(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  RootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);

  TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  TriggerVolume->SetupAttachment(RootComponent);
  TriggerVolume->SetHiddenInGame(true);
  TriggerVolume->SetMobility(EComponentMobility::Static);
  TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
  TriggerVolume->bGenerateOverlapEvents = true;
}

#if WITH_EDITOR
void ARoutePlanner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  const auto Size = Routes.Num();
  if (PropertyChangedEvent.Property && (Size != Probabilities.Num())) {
    Probabilities.Reset(Size);
    for (auto i = 0; i < Size; ++i) {
      Probabilities.Add(1.0f / static_cast<float>(Size));
      if (Routes[i] == nullptr) {
        Routes[i] = NewObject<USplineComponent>(this);
        Routes[i]->SetupAttachment(RootComponent);
        Routes[i]->SetHiddenInGame(true);
        Routes[i]->SetMobility(EComponentMobility::Static);
        Routes[i]->RegisterComponent();
      }
    }
  }
}
#endif // WITH_EDITOR

void ARoutePlanner::AddRoute(float probability, const TArray<FVector> &routePoints)
{
    USplineComponent *NewSpline = NewObject<USplineComponent>(this);

    for (int i = 0; i < routePoints.Num(); ++i)
    {
        NewSpline->AddSplinePoint(routePoints[i], ESplineCoordinateSpace::World, true);
        DrawDebugPoint(GetWorld(), routePoints[i], 3, FColor::Red, true, 999999999999);
    }

    Routes.Add(NewSpline);
    Probabilities.Add(probability);
}

void ARoutePlanner::CleanRoute()
{
    Routes.Empty();
    Probabilities.Empty();
}

void ARoutePlanner::Init()
{
    if (Routes.Num() < 1)
    {
        UE_LOG(LogCarla, Warning, TEXT("ARoutePlanner has no route assigned."));
        return;
    }

    for (auto &&Route : Routes)
    {
        if (!IsSplineValid(Route))
        {
            UE_LOG(LogCarla, Error, TEXT("ARoutePlanner has a route with zero way-points."));
            return;
        }
    }

    // Register delegate on begin overlap.
    if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ARoutePlanner::OnTriggerBeginOverlap))
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ARoutePlanner::OnTriggerBeginOverlap);
    }
}

void ARoutePlanner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UE_LOG(LogCarla, Log, TEXT("%f %f %f\n"), GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
}

void ARoutePlanner::BeginPlay()
{
  Super::BeginPlay();
  Init();
}

void ARoutePlanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Deregister the delegate.
  if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ARoutePlanner::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ARoutePlanner::OnTriggerBeginOverlap);
  }

  Super::EndPlay(EndPlayReason);
}

void ARoutePlanner::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{
  auto *Controller = GetVehicleController(OtherActor);
  auto *RandomEngine = (Controller != nullptr ? Controller->GetRandomEngine() : nullptr);
  if (RandomEngine != nullptr)
  {
    auto *Route = PickARoute(*RandomEngine, Routes, Probabilities);

    TArray<FVector> WayPoints;
    const auto Size = Route->GetNumberOfSplinePoints();
    check(Size > 1);
    WayPoints.Reserve(Size);
    for (auto i = 1; i < Size; ++i)
    {
      WayPoints.Add(Route->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
    }

    Controller->SetFixedRoute(WayPoints);
  }
}
