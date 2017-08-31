// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "AICarlaVehicleController.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "CityMapGenerator.h"
#include "Tagger.h"
#include "TrafficLight.h"
#include "math.h"
#include <DrawDebugHelpers.h>

#include "MapGen/RoadMap.h"

AAICarlaVehicleController::AAICarlaVehicleController() :
  Super(),
  MovementComponent(nullptr),
  TrafficLightStop(false) {
  bAutoManageActiveCameraTarget = false;
  //MAX_SPEED = ((rand() * 10) - 5) + MAX_SPEED;
}

AAICarlaVehicleController::~AAICarlaVehicleController() {}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void AAICarlaVehicleController::SetupInputComponent() {
  Super::SetupInputComponent();
}

void AAICarlaVehicleController::Possess(APawn *aPawn) {
  Super::Possess(aPawn);

  if (IsPossessingAVehicle()) {
    UE_LOG(LogCarla, Error, TEXT("Controller already possessing a pawn!"));
    return;
  }

  auto *WheeledVehicle = Cast<AWheeledVehicle>(aPawn);
  if (WheeledVehicle != nullptr) {

    // Bind hit events.
    // aPawn->OnActorHit.AddDynamic(this, &AAICarlaVehicleController::OnCollisionEvent);
    // Get vehicle movement component.
    MovementComponent = WheeledVehicle->GetVehicleMovementComponent();
    check(MovementComponent != nullptr);
    MovementComponent->bReverseAsBrake = true;

    // Get vehicle box component.
    TArray<UBoxComponent *> BoundingBoxes;
    WheeledVehicle->GetComponents<UBoxComponent>(BoundingBoxes);

    if (BoundingBoxes.Num() > 0) {
      VehicleBounds = BoundingBoxes[0];
    } else {
      UE_LOG(LogCarla, Error, TEXT("Pawn is missing the bounding box!"));
    }

    steerAngle = 70.0f;

  }

}

void AAICarlaVehicleController::BeginPlay() {
  TActorIterator<ACityMapGenerator> It(GetWorld());
  if (It) {
    RoadMap = It->GetRoadMap();
  }
}

bool AAICarlaVehicleController::RayTrace(
    UWorld *World,
    const FVector &Start,
    const FVector &End) {
  FHitResult OutHit;
  static FName TraceTag = FName(TEXT("VehicleTrace"));
  FCollisionQueryParams collisionParams(TraceTag, true);
  collisionParams.AddIgnoredActor(this);

  const bool Success = World->LineTraceSingleByObjectType(
        OutHit,
        Start,
        End,
        FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllDynamicObjects),
        collisionParams);

  return Success && OutHit.bBlockingHit;
}

void AAICarlaVehicleController::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  check(MovementComponent != nullptr);

  if (RoadMap == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Controller doesn't have a road map"));
    return;
  }

  FRoadMapPixelData roadData = RoadMap->GetDataAt(GetPawn()->GetActorLocation());

  float steering = 2.0, throttle = 1.0f;//, brake = 0.0f;

  FVector direction;

  if (route.Num() > 0) {
    steering = GoTo(route[route_it], direction);
  } else {
    steering = CalcStreeringValue(direction);
  }

  auto speed = MovementComponent->GetForwardSpeed() * 0.036f;

  FVector forwardVector = GetPawn()->GetActorForwardVector().GetSafeNormal();

  float distance;
  if (speed < 10.0f) {distance = 50.0f;} else {
    distance = pow(speed / 10.0f, 2) * 100.0f;
  }

  const FVector StartCenter = GetPawn()->GetActorLocation() + (forwardVector * (250.0f + VehicleBounds->GetScaledBoxExtent().X / 2.0f)) + FVector(0.0f, 0.0f, 50.0f);
  const FVector EndCenter = StartCenter + direction * (distance + VehicleBounds->GetScaledBoxExtent().X / 2.0f);

  const FVector StartRight = StartCenter + (FVector(forwardVector.Y, -forwardVector.X, forwardVector.Z) * 100.0f);
  const FVector EndRight = StartRight + direction * (distance + VehicleBounds->GetScaledBoxExtent().X / 2.0f);

  const FVector StartLeft = StartCenter + (FVector(-forwardVector.Y, forwardVector.X, forwardVector.Z) * 100.0f);
  const FVector EndLeft = StartLeft + direction * (distance + VehicleBounds->GetScaledBoxExtent().X / 2.0f);

  //const FVector RightEnd = Start + GetPawn()->GetActorForwardVector().GetSafeNormal() * (200 + VehicleBounds->GetScaledBoxExtent().X/2.0f);

  //RayTrace to detect trafficLights or Vehicles
  auto World = GetWorld();
  if (TrafficLightStop ||
      RayTrace(World, StartCenter, EndCenter) ||
      RayTrace(World, StartRight, EndRight) ||
      RayTrace(World, StartLeft, EndLeft)) {
    throttle = Stop(speed);
  } else {
    throttle = Move(speed);
  }

  MovementComponent->SetSteeringInput(steering);
  MovementComponent->SetThrottleInput(throttle);

  // if (throttle == 0.0f) brake = Stop(speed);
  // MovementComponent->SetBrakeInput(brake);

}

float AAICarlaVehicleController::GoTo(FVector objective, FVector &direction) {

#ifdef CARLA_AI_VEHICLES_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("it: %i"), route_it);
#endif // CARLA_AI_VEHICLES_EXTRA_LOG

  float steering = 0;
  if (objective.Equals(GetPawn()->GetActorLocation(), 80.0f)) {
    ++route_it;
    if (route_it == route.Num()) {
      route.Empty();
      route_it = 0;
      return CalcStreeringValue(direction);
    }
  }

  objective.Z = GetPawn()->GetActorLocation().Z;

  direction = objective - GetPawn()->GetActorLocation();
  direction = direction.GetSafeNormal();

  FVector forward = GetPawn()->GetActorForwardVector();

  float dirAngle = direction.UnitCartesianToSpherical().Y;
  float actorAngle = forward.UnitCartesianToSpherical().Y;

  dirAngle *= (180.0f / PI);
  actorAngle *= (180.0 / PI);

  float angle = dirAngle - actorAngle;

  if (angle > 180.0f) { angle -= 360.0f;} else if (angle < -180.0f) {
    angle += 360.0f;
  }

  if (angle < -steerAngle) { steering = -1.0f;} else if (angle > steerAngle) {
    steering = 1.0f;
  } else { steering += angle / steerAngle;}

  return steering;
}

float AAICarlaVehicleController::CalcStreeringValue(FVector &direction) {

  float steering = 0;
/*
    if (VehicleRightControl == nullptr || VehicleLeftControl == nullptr){
      UE_LOG(LogCarla, Error, TEXT("Vehicle control point not found!"));
      return 0;
    }
*/
  FVector BoxExtent = VehicleBounds->GetScaledBoxExtent();
  FVector forward = GetPawn()->GetActorForwardVector();

  FVector rightSensorPosition(BoxExtent.X / 2.0f, (BoxExtent.Y / 2.0f) + 100.0f, 0.0f);
  FVector leftSensorPosition(BoxExtent.X / 2.0f, -(BoxExtent.Y / 2.0f) - 100.0f, 0.0f);

  float forwardMagnitude = BoxExtent.X / 2.0f;

  float Magnitude = (float) sqrt(pow((double) leftSensorPosition.X, 2.0) + pow((double) leftSensorPosition.Y, 2.0));

  //same for the right and left
  float offset = FGenericPlatformMath::Acos(forwardMagnitude / Magnitude);

  float actorAngle = forward.UnitCartesianToSpherical().Y;

  float sinR = FGenericPlatformMath::Sin(actorAngle + offset);
  float cosR = FGenericPlatformMath::Cos(actorAngle + offset);

  float sinL = FGenericPlatformMath::Sin(actorAngle - offset);
  float cosL = FGenericPlatformMath::Cos(actorAngle - offset);

  rightSensorPosition.Y = sinR * Magnitude;
  rightSensorPosition.X = cosR * Magnitude;

  leftSensorPosition.Y = sinL * Magnitude;
  leftSensorPosition.X = cosL * Magnitude;

  FVector rightPositon = GetPawn()->GetActorLocation() + FVector(rightSensorPosition.X, rightSensorPosition.Y, 0.0f);
  FVector leftPosition = GetPawn()->GetActorLocation() + FVector(leftSensorPosition.X, leftSensorPosition.Y, 0.0f);

  FRoadMapPixelData rightRoadData = RoadMap->GetDataAt(rightPositon);
  if (!rightRoadData.IsRoad()) { steering -= 0.2f;}

  FRoadMapPixelData leftRoadData = RoadMap->GetDataAt(leftPosition);
  if (!leftRoadData.IsRoad()) { steering += 0.2f;}

  FRoadMapPixelData roadData = RoadMap->GetDataAt(GetPawn()->GetActorLocation());
  if (!roadData.IsRoad()) {
    steering = -1;
  } else if (roadData.HasDirection()) {

    direction = roadData.GetDirection();
    FVector right = rightRoadData.GetDirection();
    FVector left = leftRoadData.GetDirection();

    forward.Z = 0.0f;

    float dirAngle = direction.UnitCartesianToSpherical().Y;
    float rightAngle = right.UnitCartesianToSpherical().Y;
    float leftAngle = left.UnitCartesianToSpherical().Y;

    dirAngle *= (180.0f / PI);
    rightAngle *= (180.0 / PI);
    leftAngle *= (180.0 / PI);
    actorAngle *= (180.0 / PI);

    float min = dirAngle - 90.0f;
    if (min < -180.0f) { min = 180.0f + (min + 180.0f);}

    float max = dirAngle + 90.0f;
    if (max > 180.0f) { max = -180.0f + (max - 180.0f);}

    if (dirAngle < -90.0 || dirAngle > 90.0) {
      if (rightAngle < min && rightAngle > max) { steering -= 0.2f;}
      if (leftAngle < min && leftAngle > max) { steering += 0.2f;}
    } else {
      if (rightAngle < min || rightAngle > max) { steering -= 0.2f;}
      if (leftAngle < min || leftAngle > max) { steering += 0.2f;}
    }

    float angle = dirAngle - actorAngle;

    if (angle > 180.0f) { angle -= 360.0f;} else if (angle < -180.0f) {
      angle += 360.0f;
    }

    if (angle < -steerAngle) { steering = -1.0f;} else if (angle > steerAngle) {
      steering = 1.0f;
    } else { steering += angle / steerAngle;}

  }

  return steering;
}

// return throttle value
float AAICarlaVehicleController::Stop(const float speed) {
  // const float brake = 1.0f - (speed/MAX_SPEED);
  // return FMath::Max(0.2f, brake);
  if (speed >= 1.0) {return -speed / MAX_SPEED;}
  return 0.0f;
}

// return throttle value
float AAICarlaVehicleController::Move(const float speed) {
  /// UE_LOG(LogCarla, Log,
  //       TEXT("MAX_SPEED: %f   SPEED: %f"),
  //       MAX_SPEED,
  //       speed
  //       );

  if (speed >= MAX_SPEED) {
    return Stop(speed);
  } else if (speed >= MAX_SPEED - 10.0f) {
    return 0.5;
  }
  return 1.0f;
}

void AAICarlaVehicleController::RedTrafficLight(bool state) {
  if (state) {TrafficLightStop = true;} else {TrafficLightStop = false;}
}

void AAICarlaVehicleController::NewSpeedLimit(float speed) {

  MAX_SPEED = speed;

#ifdef CARLA_AI_VEHICLES_EXTRA_LOG
  UE_LOG(LogCarla, Log, TEXT("New Speed: %f"), MAX_SPEED);
#endif // CARLA_AI_VEHICLES_EXTRA_LOG
}

void AAICarlaVehicleController::NewRoute(TArray<FVector> positions) {
  this->route = positions;
  route_it = 0;
}
