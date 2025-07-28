#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RoadSpline.generated.h"

UENUM(BlueprintType)
enum class ERoadSplineBoundaryType : uint8
{
  LaneBoundary     UMETA(DisplayName = "Lane Boundary"),
  RoadBoundary     UMETA(DisplayName = "Road Boundary")
};

UCLASS()
class CARLA_API ARoadSpline : public AActor
{
  GENERATED_BODY()

public:
  ARoadSpline();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
  USplineComponent* SplineComponent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boundary")
  ERoadSplineBoundaryType BoundaryType;

  void SetSplinePoints(const TArray<FVector>& Points, bool bClosedLoop = false);

protected:

};

