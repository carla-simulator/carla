#include "RoadSpline.h"
#include "DrawDebugHelpers.h"

ARoadSpline::ARoadSpline()
{
  PrimaryActorTick.bCanEverTick = false;
  SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
  RootComponent = SplineComponent;
  BoundaryType = ERoadSplineBoundaryType::Unknown;
  bIsJunction = false;
}

void ARoadSpline::SetSplinePoints(const TArray<FVector>& Points, bool bClosedLoop)
{
  SplineComponent->ClearSplinePoints(false);
  for (const FVector& Point : Points)
  {
    SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
  }
  SplineComponent->SetClosedLoop(bClosedLoop, false);
  SplineComponent->UpdateSpline();
}
