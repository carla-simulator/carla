// Fill out your copyright notice in the Description page of Project Settings.

#include "Carla.h"
#include "LidarLaser.h"
#include "Lidar.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

int LidarLaser::GetId()
{
	return Id;
}

bool LidarLaser::Measure(ALidar* Lidar, float HorizontalAngle, FVector& XYZ, bool Debug)
{
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, Lidar);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	FHitResult HitInfo(ForceInit);

	FVector LidarBodyLoc = Lidar->GetActorLocation();
  FRotator LidarBodyRot = Lidar->GetActorRotation();
	FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
    LaserRot,
    LidarBodyRot
  );
	FVector EndTrace = Lidar->Range * UKismetMathLibrary::GetForwardVector(ResultRot) + LidarBodyLoc;

	Lidar->GetWorld()->LineTraceSingleByChannel(
		HitInfo,
		LidarBodyLoc,
		EndTrace,
		ECC_MAX,
		TraceParams,
		FCollisionResponseParams::DefaultResponseParam
	);

	if (HitInfo.bBlockingHit)
	{
    if (Debug)
    {
  		DrawDebugPoint(
  			Lidar->GetWorld(),
  			HitInfo.ImpactPoint,
  			10,  //size
  			FColor(255,0,255),
  			false,  //persistent (never goes away)
  			0.1  //point leaves a trail on moving object
  		);
    }

    XYZ = LidarBodyLoc - HitInfo.ImpactPoint;
    XYZ = UKismetMathLibrary::RotateAngleAxis(
      XYZ,
      - LidarBodyRot.Yaw + 90,
      FVector(0, 0, 1)
    );

		return true;
	} else {
    XYZ = FVector(0, 0, 0);
		return false;
	}
}
