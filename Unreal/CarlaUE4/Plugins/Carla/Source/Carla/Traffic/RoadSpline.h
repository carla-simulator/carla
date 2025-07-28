#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RoadSpline.generated.h"

UENUM(BlueprintType)
enum class ERoadSplineBoundaryType : uint8
{
  // LaneBoundary     UMETA(DisplayName = "Lane Boundary"),
  // RoadBoundary     UMETA(DisplayName = "Road Boundary")
  None          UMETA(DisplayName = "None"),
  Driving       UMETA(DisplayName = "Driving"),
  Stop          UMETA(DisplayName = "Stop"),
  Shoulder      UMETA(DisplayName = "Shoulder"),
  Biking        UMETA(DisplayName = "Biking"),
  Sidewalk      UMETA(DisplayName = "Sidewalk"),
  Border        UMETA(DisplayName = "Border"),
  Restricted    UMETA(DisplayName = "Restricted"),
  Parking       UMETA(DisplayName = "Parking"),
  Bidirectional UMETA(DisplayName = "Bidirectional"),
  Median        UMETA(DisplayName = "Median"),
  Special1      UMETA(DisplayName = "Special1"),
  Special2      UMETA(DisplayName = "Special2"),
  Special3      UMETA(DisplayName = "Special3"),
  RoadWorks     UMETA(DisplayName = "RoadWorks"),
  Tram          UMETA(DisplayName = "Tram"),
  Rail          UMETA(DisplayName = "Rail"),
  Entry         UMETA(DisplayName = "Entry"),
  Exit          UMETA(DisplayName = "Exit"),
  OffRamp       UMETA(DisplayName = "OffRamp"),
  OnRamp        UMETA(DisplayName = "OnRamp"),
  Unknown       UMETA(DisplayName = "Unknown")
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

