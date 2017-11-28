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

float LidarLaser::Measure(ALidar* Lidar, float HorizontalAngle, bool Debug)
{
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, Lidar);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	FHitResult HitInfo(ForceInit);

	FVector LidarBodyLoc = Lidar->GetActorLocation();
  FRotator LidarBodyRot = Lidar->GetActorRotation();
	FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);
	FVector EndTrace = Lidar->Range * UKismetMathLibrary::GetForwardVector(LidarBodyRot + LaserRot) + LidarBodyLoc;

	Lidar->GetWorld()->LineTraceSingleByChannel(
		HitInfo,
		LidarBodyLoc,
		EndTrace,
		ECC_Visibility,
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

		return (LidarBodyLoc - HitInfo.ImpactPoint).Size();
	} else {
		return -1;
	}
}
