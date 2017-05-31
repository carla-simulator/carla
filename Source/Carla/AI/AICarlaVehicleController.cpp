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


// Find first component of type road.
static bool RayTrace(
    UWorld *World,
    const FVector &Start,
    const FVector &End,
    bool &Stop)
{

  FHitResult Hit;
  TArray <FHitResult> OutHits;
  static FName TraceTag = FName(TEXT("VehicleTrace"));
  
  World->DebugDrawTraceTag = TraceTag;
  
  const bool Success = World->LineTraceMultiByObjectType(
        OutHits,
        Start,
        End,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_Vehicle),
        FCollisionQueryParams(TraceTag, true));


  if (Success) {
    for (FHitResult &Item : OutHits) {
      if (ATagger::MatchComponent(*Item.Component, ECityObjectLabel::Vehicles)) {
        Stop = true;
        return true;
      }
    }
  }
  return false;
}



AAICarlaVehicleController::AAICarlaVehicleController() : 
  Super(),
  MovementComponent(nullptr),
  TrafficLightStop(false)
{
  bAutoManageActiveCameraTarget = false;
  //MAX_SPEED = ((rand() * 10) - 5) + MAX_SPEED; 
}

AAICarlaVehicleController::~AAICarlaVehicleController() {}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void AAICarlaVehicleController::SetupInputComponent(){ Super::SetupInputComponent(); }

void AAICarlaVehicleController::Possess(APawn *aPawn)
{
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

    // Get vehicle box component.
    TArray<UBoxComponent *> BoundingBoxes;
    WheeledVehicle->GetComponents<UBoxComponent>(BoundingBoxes);

    if (BoundingBoxes.Num() > 0) {
      VehicleBounds = BoundingBoxes[0];
    } else {
      UE_LOG(LogCarla, Error, TEXT("Pawn is missing the bounding box!"));
    }

/*
    ////////////////////////////////
    if (VehicleBounds != nullptr){
      FVector BoxExtent = VehicleBounds->GetScaledBoxExtent();


       USphereComponent* rightSphere = NewObject<USphereComponent>(VehicleBounds, TEXT("Right"));
       if(rightSphere)
       {
           rightSphere->RegisterComponent();
           rightSphere->SetupAttachment(RootComponent);
           //rightSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
       }

       USphereComponent* leftSphere = NewObject<USphereComponent>(VehicleBounds, TEXT("Left"));
       if(leftSphere)
       {
           leftSphere->RegisterComponent();
           leftSphere->SetupAttachment(RootComponent);
           //leftSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
       }


        //CompClass can be a BP
        //USphereComponent* SphereVisual = ConstructObject<USphereComponent>(USphereComponent::StaticClass, this, name);
        USphereComponent* rightSphere = GetOwner() -> CreateDefaultSubobject<USphereComponent>(TEXT("Right"));
        RootComponent = rightSphere;

         //could use different than Root Comp
      if (rightSphere)
      {
          //SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
          rightSphere->RegisterComponent();
          rightSphere->SetRelativeLocation(FVector(BoxExtent.X/2.0, 50.0 + BoxExtent.Y/2.0, 0.0));
          rightSphere->SetWorldScale3D(FVector(1.0f));
          rightSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform); 
      }

      USphereComponent* leftSphere = GetOwner() -> CreateDefaultSubobject<USphereComponent>(TEXT("Left"));
        RootComponent = leftSphere;

         //could use different than Root Comp
      if (leftSphere)
      {
          //SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
          leftSphere->RegisterComponent();
          leftSphere->SetRelativeLocation(FVector(BoxExtent.X/2.0, 50.0 + BoxExtent.Y/2.0, 0.0));
          leftSphere->SetWorldScale3D(FVector(1.0f));
          leftSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform); 
      }

    }
///////////////////////////////////
    TArray<USphereComponent *> ControlPoints;
    WheeledVehicle->GetComponents<USphereComponent>(ControlPoints);
    if (ControlPoints.Num() > 0) {
      if (ControlPoints[0]->GetName().Equals("Right")){
        VehicleRightControl = ControlPoints[0];
        VehicleLeftControl = ControlPoints[1];
      }
      else{
        VehicleRightControl = ControlPoints[1];
        VehicleLeftControl = ControlPoints[0];
      }
    }else{
      UE_LOG(LogCarla, Error, TEXT("Vehicle control point not found!"));
    }
*/
  }

}

void AAICarlaVehicleController::BeginPlay()
{

  TActorIterator<ACityMapGenerator> It(GetWorld());
  if (It) {
    RoadMap = It->GetRoadMap();
  }

}


void AAICarlaVehicleController::Tick(float DeltaTime){
	Super::PlayerTick(DeltaTime);

  check(MovementComponent != nullptr);
  
  if (RoadMap == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Controller doesn't have a road map"));
    return;
  }

  FRoadMapPixelData roadData = RoadMap->GetDataAt(GetPawn()->GetActorLocation());


  float steering = 2.0, throttle = 1.0f;

  if (route.Num() > 0){
    steering = GoTo(route[route_it]);
  }
  else{
    steering = CalcStreeringValue();
  }

  const FVector Start = GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector().GetSafeNormal() * (200.0f + VehicleBounds->GetScaledBoxExtent().X/2.0f)) + FVector(0.0f, 0.0f, 50.0f);
  const FVector End = Start + GetPawn()->GetActorForwardVector().GetSafeNormal() * (300.0f + VehicleBounds->GetScaledBoxExtent().X/2.0f);


  auto speed = MovementComponent->GetForwardSpeed() * 0.036f;
  
  //RayTrace to detect trafficLights or Vehicles
  bool stop;
  auto World = GetWorld();
  if (TrafficLightStop) throttle = Stop(speed);
  else {
    if (RayTrace(World, Start, End, stop)) {
      if (stop) throttle = Stop(speed);
      else throttle = Move(speed);
    }
    else{
       throttle = Move(speed);
    }
  }
  

  MovementComponent->SetSteeringInput(steering);
  MovementComponent->SetThrottleInput(throttle);

}

float AAICarlaVehicleController::GoTo(FVector objective){


    UE_LOG(LogCarla, Log,
        TEXT("it: %i"),
        route_it
        );

    float steering = 0;
    if (objective.Equals(GetPawn()->GetActorLocation(), 80.0f)){
        ++route_it;
      if (route_it == route.Num()){
        route.Empty();
        route_it = 0;
        return CalcStreeringValue();
      }
    }


    FVector direction = objective - GetPawn()->GetActorLocation();
    direction = direction.GetSafeNormal();

    FVector forward = GetPawn()->GetActorForwardVector();

    float dirAngle = direction.UnitCartesianToSpherical().Y;
    float actorAngle = forward.UnitCartesianToSpherical().Y;

    dirAngle *= (180.0f/PI);
    actorAngle *= (180.0/PI);

    float angle = dirAngle - actorAngle;

    if (angle > 180.0f) angle -= 360.0f;
    else if (angle < -180.0f) angle += 360.0f;

    if (angle < -70.0f) steering = -1.0f;
    else if (angle > 70.0f) steering = 1.0f;
    else steering += angle/70.0f;

  return steering;
}

float AAICarlaVehicleController::CalcStreeringValue(){

    float steering = 0;
/*
    if (VehicleRightControl == nullptr || VehicleLeftControl == nullptr){
      UE_LOG(LogCarla, Error, TEXT("Vehicle control point not found!"));
      return 0;
    } 
*/
    FVector BoxExtent = VehicleBounds->GetScaledBoxExtent();
    FVector forward = GetPawn()->GetActorForwardVector();

    FVector rightSensorPosition (BoxExtent.X/2.0f, (BoxExtent.Y/2.0f) + 150.0f, 0.0f);
    FVector leftSensorPosition (BoxExtent.X/2.0f, -(BoxExtent.Y/2.0f) - 150.0f, 0.0f);

    


    float forwardMagnitude = BoxExtent.X/2.0f;

    float Magnitude = (float) sqrt(pow((double)leftSensorPosition.X,2.0) + pow((double)leftSensorPosition.Y,2.0));

    //same for the right and left
    float offset = FGenericPlatformMath::Acos(forwardMagnitude/Magnitude);

    float actorAngle = forward.UnitCartesianToSpherical().Y;
    
    float sinR = FGenericPlatformMath::Sin(actorAngle+offset);
    float cosR = FGenericPlatformMath::Cos(actorAngle+offset);
    

    float sinL = FGenericPlatformMath::Sin(actorAngle-offset);
    float cosL = FGenericPlatformMath::Cos(actorAngle-offset);

    rightSensorPosition.Y = sinR * Magnitude;
    rightSensorPosition.X = cosR * Magnitude;

    leftSensorPosition.Y = sinL * Magnitude;
    leftSensorPosition.X = cosL * Magnitude;

    FVector rightPositon = GetPawn()->GetActorLocation() + FVector(rightSensorPosition.X, rightSensorPosition.Y, 0.0f);
    FVector leftPosition = GetPawn()->GetActorLocation() + FVector(leftSensorPosition.X, leftSensorPosition.Y, 0.0f);

    FRoadMapPixelData rightRoadData = RoadMap->GetDataAt(rightPositon);
    if (!rightRoadData.IsRoad()) steering -= 0.2f;

    FRoadMapPixelData leftRoadData = RoadMap->GetDataAt(leftPosition);
    if (!leftRoadData.IsRoad()) steering += 0.2f;

    FRoadMapPixelData roadData = RoadMap->GetDataAt(GetPawn()->GetActorLocation());
    if (!roadData.IsRoad()){
      steering = -1;
    }
    else if (roadData.HasDirection()){

      FVector direction = roadData.GetDirection();
      FVector right = rightRoadData.GetDirection();
      FVector left = leftRoadData.GetDirection();


      forward.Z = 0.0f;

      float dirAngle = direction.UnitCartesianToSpherical().Y;
      float rightAngle = right.UnitCartesianToSpherical().Y;
      float leftAngle = left.UnitCartesianToSpherical().Y;

      dirAngle *= (180.0f/PI);
      rightAngle *= (180.0/PI);
      leftAngle *= (180.0/PI);
      actorAngle *= (180.0/PI);

      float min = dirAngle - 90.0f;
      if (min < -180.0f) min = 180.0f + (min + 180.0f);

      float max = dirAngle + 90.0f;
      if (max > 180.0f) max = -180.0f + (max - 180.0f);
      
      if (dirAngle < -90.0 || dirAngle > 90.0){
        if (rightAngle < min && rightAngle > max) steering -= 0.2f;
        if (leftAngle < min && leftAngle > max) steering += 0.2f;
      }
      else{
        if (rightAngle < min || rightAngle > max) steering -= 0.2f;
        if (leftAngle < min || leftAngle > max) steering += 0.2f;
      }

      float angle = dirAngle - actorAngle;

      if (angle > 180.0f) angle -= 360.0f;
      else if (angle < -180.0f) angle += 360.0f;

      if (angle < -70.0f) steering = -1.0f;
      else if (angle > 70.0f) steering = 1.0f;
      else steering += angle/70.0f;

    }

    return steering;
}

float AAICarlaVehicleController::Stop(float &speed){
  if (speed > 0.0f) return -1.0f;
  else return 0;
}

float AAICarlaVehicleController::Move(float &speed){ 
  return  1.0f - (speed/MAX_SPEED);
}


void AAICarlaVehicleController::RedTrafficLight(bool state){
  if (state) TrafficLightStop = true;
  else TrafficLightStop = false;
}


void AAICarlaVehicleController::NewSpeedLimit(float speed){
  MAX_SPEED = speed;



  UE_LOG(LogCarla, Log,
        TEXT("New Speed: %f"),
        MAX_SPEED
        );
}


void AAICarlaVehicleController::NewRoute(TArray<FVector> positions){
  this->route = positions;
  route_it = 0;
}
/*
 bool AAICarlaVehicleController::DoTrace()
 {
    FHitResult RV_Hit(ForceInit);

 
    FVector Start = GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector() * 250) + FVector(0.0, 0.0, 50.0);
    
    // you need to add a uproperty to the header file for a float PlayerInteractionDistance
    FVector End = Start + (GetPawn()->GetActorForwardVector() * 500);
 
    //If Trace Hits anything
    if(  UMyStaticFunctionLibrary::Trace(GetWorld(),GetPawn(),Start,End,HitData)  )
    {
      //Print out the name of the traced actor
      if(HitData.GetActor())
      {
        ClientMessage(HitData.GetActor()->GetName());
     
              //Print out distance from start of trace to impact point
              ClientMessage("Trace Distance: " + FString::SanitizeFloat(HitData.Distance));
      }
      return true;
    }
 
    return false;
 }
*/