#include "Vehicle_AIController.h"

//#include "InterchangeResult.h"
#include "IntVectorTypes.h"
#include "MathUtil.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

void AVehicle_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		MaxSpeedBase = Vehicle->FloatingPawnMovement->MaxSpeed;
	}
}

void AVehicle_AIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!FacingCenter && !CameraBusy) FacingCenter = LookForward();
}

void AVehicle_AIController::MoveToTargetLocation(FVector Target)
{
	if (AVehicle* Vehicle = Cast<AVehicle>(GetPawn()))
	{
		float sideMovement = 0;
		float slowDownMove = 0;
		float YawDiffRange = Vehicle->YawDiffAcceptableRange;
		
		FVector CurrentLocation = Vehicle->GetActorLocation();
		FRotator CurrentRotation = Vehicle->GetActorRotation();

		FVector CorrectedTarget = FVector(Target.X, Target.Y, CurrentLocation.Z);
		FRotator LookAtRotation =  UKismetMathLibrary::FindLookAtRotation(CurrentLocation, CorrectedTarget);
		
		FRotator RotDiff = (LookAtRotation - CurrentRotation).GetNormalized();
		float YawDiff = RotDiff.Yaw;

		//This is been changed a lot in order to avoid the vehicle from going in circles
		if ((YawDiff > YawDiffRange) || (YawDiff < -YawDiffRange))
		{
			slowDownMove = FVector::DotProduct(Vehicle->GetActorRotation().Vector().GetSafeNormal(), LookAtRotation.Vector().GetSafeNormal());
			FMath::Clamp(slowDownMove, 0, 1);
			Vehicle->FloatingPawnMovement->MaxSpeed = MaxSpeedBase * slowDownMove;
			FRotator NewRotation = FMath::RInterpTo(Vehicle->GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 3 * slowDownMove + 2);
			Vehicle->SetActorRotation(NewRotation);
			//Vehicle->AddActorWorldRotation(FRotator( 0, (YawDiff / 10), 0));
			//Vehicle->FloatingPawnMovement->MaxSpeed = FMathf::Clamp(FMathf::Abs(YawDiff) * 20, 200, 1200);
			//sideMovement = FMathf::Clamp(YawDiff / Vehicle->YawDiffDivisor, -1, 1);
		}
		else
		{
			Vehicle->FloatingPawnMovement->MaxSpeed = MaxSpeedBase;
		}
		
		Vehicle->BasicMovementWithRotation(FVector2d(1, sideMovement));
		Vehicle->AdjustToTerrain();
	}
}

void AVehicle_AIController::SetNewLocation(FVector InLocation)
{
	if (!GetBlackboardComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("NO BLACKBOARD COMPONENT ON VEHICLE"))
		return;
	}
	GetBlackboardComponent()->SetValueAsVector(FName(TEXT("PatrolLocation")), InLocation);
	GetBlackboardComponent()->SetValueAsBool(FName(TEXT("NewTargetLocation")), true);
	CameraBusy = false;
}

void AVehicle_AIController::SetNewLocationCoord(FGeographicCoordinates Coords)
{
	if (AGeoReferencingSystem* GeoReferencingSystem = AGeoReferencingSystem::GetGeoReferencingSystem(GetWorld()))
	{
		FVector TargetLocation;
		GeoReferencingSystem->GeographicToEngine(Coords, TargetLocation);
		SetNewLocation(TargetLocation);
	}
	else
	{
		//TODO: Add error for having no georeference system in world
	}
}

void AVehicle_AIController::LookAtComponent_Implementation(USkeletalMeshComponent* component)
{
	CameraBusy = true;
	FacingCenter = false;
	AVehicle* vehicle = Cast<AVehicle>(GetPawn());
	if (!component || !vehicle) return;

	vehicle->RotateVehicleForLook(component->GetSocketLocation(TEXT("neck_02")));
}

void AVehicle_AIController::LookAtTarget_Implementation(AActor* target)
{
	CameraBusy = true;
	FacingCenter = false;
	AVehicle* vehicle = Cast<AVehicle>(GetPawn());
	if (!target || !vehicle) return;

	vehicle->RotateVehicleForLook(target->GetActorLocation());
}

bool AVehicle_AIController::LookForward_Implementation()
{
	AVehicle* vehicle = Cast<AVehicle>(GetPawn());
	if (!vehicle) return false;

	FRotator CurrentRot = vehicle->Camera->GetRelativeRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, FRotator::ZeroRotator, GetWorld()->GetDeltaSeconds(), 3);

	
	bool CloseEnough = (CurrentRot.GetManhattanDistance(FRotator::ZeroRotator) < 1);

	if (CloseEnough)
	{
		vehicle->Camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
	else
	{
		vehicle->Camera->SetRelativeRotation(NewRot);
	}

	return CloseEnough;
}
