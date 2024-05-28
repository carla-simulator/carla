#include "Vehicle.h"

#include "Vehicle_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationData.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/FastReferenceCollector.h"
#include "Components/SphereComponent.h"

#include "Game_GI_DARPA.h"

AVehicle::AVehicle()
{
	RootSphere = CreateDefaultSubobject<USphereComponent>(FName(TEXT("RootComponent")));
	RootComponent = RootSphere;
	RootSphere->SetCollisionResponseToAllChannels(ECR_Block);
	RootComponent->SetCanEverAffectNavigation(false);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("SpringArmComponent")));
	SpringArm->TargetArmLength = 100;
	SpringArm->SocketOffset = FVector(150, 0, 75);
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetupAttachment(RootSphere);

	Camera = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("MainCamera")));
	Camera->SetupAttachment(SpringArm);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(FName(TEXT("FloatingPawnMovement")));

	Altitude = 20;
	AltitudeAcceptableRange = 10;
	WaypointIndex = -1;
	AtAltitude = false;
	UseNavMesh = true;

	AtLocationWaitDuration = 5;
	CameraLookAtSpeed = 3;

	
	FindNavmeshExtentRange = FVector(200, 200, 200);
	TraceStartOffset = 1000;
	TraceEndOffset = -1000;
	TraceCapsuleRadius = 50;
	TraceCapsuleHalfHeight = 100;

	YawDiffAcceptableRange = 1;
	YawDiffDivisor = 200;
}

void AVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!AtAltitude)
	{
		AdjustToTerrain();
	}
}


void AVehicle::BasicMovementWithRotation(FVector2d Move)
{

	//UE_LOG(LogTemp, Warning, TEXT("Move: %f, %f"), Move.X, Move.Y);
	FRotator BaseDirection = GetActorRotation();
	BaseDirection.Pitch = 0;

	const FVector m = FVector(Move.X, Move.Y, 0);
	FVector target = BaseDirection.RotateVector(m);
	target.Normalize();

	FloatingPawnMovement->AddInputVector(target);
}

void AVehicle::AdjustToTerrain()
{
	AtAltitude = false;
	float groundHeight = 0;

	if(UseNavMesh)
	{
		const FVector currentLocation = GetActorLocation();
		FNavLocation NavLocation;
		
		if (GetWorld()->GetNavigationSystem()->GetMainNavData()->ProjectPoint(currentLocation, NavLocation, FindNavmeshExtentRange))
		{
			groundHeight = NavLocation.Location.Z;
		}
		else
		{
			//TODO: add error here
		}
	}
	else
	{
		FHitResult hit;
	
		FVector traceStart = GetActorLocation();
		FVector traceEnd = GetActorLocation();
		traceStart.Z += TraceStartOffset;
		traceEnd.Z += TraceEndOffset;

		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
	
		bool blockingHit = GetWorld()->SweepSingleByChannel(hit, traceStart, traceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(TraceCapsuleRadius, TraceCapsuleHalfHeight), params);
		if (!blockingHit)
		{
			//TODO: Add error here
			return;
		}

		groundHeight = hit.Location.Z - TraceCapsuleRadius;
	}
	
	

	double heightDiff = (GetActorLocation().Z - Altitude) - groundHeight;

	//TODO: Replace hard coded numbers
	if (heightDiff < -AltitudeAcceptableRange)
	{
		FloatingPawnMovement->AddInputVector(FVector(0, 0, 0.25));
	}
	else if (heightDiff > AltitudeAcceptableRange)
	{
		FloatingPawnMovement->AddInputVector(FVector(0, 0, -.25));
	}
	else
	{
		AtAltitude = true;
	}
}

void AVehicle::LookRotation(FVector2D Delta)
{
	AddControllerYawInput(Delta.X);
	AddControllerPitchInput(Delta.Y);
}

void AVehicle::UnboundMovement(FVector2D Move)
{
	FRotator BaseDirection = GetActorRotation();

	const FVector m = FVector(Move.X, Move.Y, 0);
	FVector target = BaseDirection.RotateVector(m);
	target.Normalize();

	FloatingPawnMovement->AddInputVector(target);
}

void AVehicle::BasicMovement(FVector2D Move)
{
	FVector target = FVector(Move.X, Move.Y, 0);
	target.Normalize();

	FloatingPawnMovement->AddInputVector(target);
}

void AVehicle::RotateVehicleForLook(FVector target)
{
	//FRotator CurrentRot = Camera->GetComponentRotation();
	FRotator TargetRotCam = (target - Camera->GetComponentLocation()).Rotation();
	FRotator TargetRotVehicle = (target - GetActorLocation()).Rotation();
	TargetRotCam.Roll = 0;
	TargetRotVehicle.Roll = 0;
	
	FRotator NewRotCam = FMath::RInterpTo(Camera->GetComponentRotation(), TargetRotCam, GetWorld()->GetDeltaSeconds(), CameraLookAtSpeed);
	Camera->SetWorldRotation(NewRotCam);

	FRotator NewRotVehicle = FMath::RInterpTo(GetActorRotation(), TargetRotVehicle, GetWorld()->GetDeltaSeconds(), CameraLookAtSpeed);
	SetActorRotation(NewRotVehicle);
}

// bool AVehicle::GetCoordinates(FGeographicCoordinates &Coords)
// {
// 	// if (AGeoReferencingSystem* geoReferencingSystem = AGeoReferencingSystem::GetGeoReferencingSystem(GetWorld()))
// 	// {
// 	// 	geoReferencingSystem->EngineToGeographic(GetActorLocation(), Coords);
// 	// 	return true;
// 	// }
// 	// else
// 	// {
// 	// 	return false;
// 	// }
	
// }

void AVehicle::ExecuteNextWaypoint_Implementation()
{
	if (WaypointIndex >= MoveToLocations.Num() - 1)
	{
		// Inform the game instance that a vehicle with valid waypoint arrays has finished pathing
		if (MoveToLocations.Num() > 0)
		{
			UGame_GI_DARPA* gameInstance = UGame_GI_DARPA::Get(this);
			if (gameInstance != nullptr)
			{
				gameInstance->HandleVehiclePathComplete(VehicleID);
			}
		}

		return;
	}

	WaypointIndex += 1;

	if (AVehicle_AIController* controller = Cast<AVehicle_AIController>(GetController()))
	{
		if (!controller->GetBlackboardComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("NO BLACKBOARD COMPONENT ON VEHICLE"));
			return;
		}
		controller->SetNewLocation(MoveToLocations[WaypointIndex]);
		controller->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("NewTargetLocation")), true);
	}
}

void AVehicle::PauseVehicle()
{
	if (AVehicle_AIController* controller = Cast<AVehicle_AIController>(GetController()))
	{
		if (!controller->GetBlackboardComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("NO BLACKBOARD COMPONENT ON VEHICLE"));
			return;
		}
		controller->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("IsActive")), false);
	}
}

void AVehicle::ResumeVehicle()
{
	if (WaypointIndex >= MoveToLocations.Num() - 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("No more Waypoints Available for VehicleID: %s"), *VehicleID.ToString());
		return;
	} 
	
	if (AVehicle_AIController* controller = Cast<AVehicle_AIController>(GetController()))
	{
		if (!controller->GetBlackboardComponent())
		{
			UE_LOG(LogTemp, Warning, TEXT("NO BLACKBOARD COMPONENT ON VEHICLE"));
			return;
		}
		controller->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("IsActive")), true);
	}
}
