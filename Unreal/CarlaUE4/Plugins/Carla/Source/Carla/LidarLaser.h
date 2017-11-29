// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ALidar;

/**
 *
 */
class CARLA_API LidarLaser
{
public:

	LidarLaser(int Id, float VerticalAngle) :
		Id(Id),
		VerticalAngle(VerticalAngle)
	{}

	int GetId();
	bool Measure(ALidar* Lidar, float HorizontalAngle, FVector& XYZ, bool Debug = false);

private:

	int Id;
	float VerticalAngle;

};
