// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CapturedLidarSegment.generated.h"

///
/// Lidar segment captured by tick
///
USTRUCT()
struct FCapturedLidarSegment
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(VisibleAnywhere)
  float horizontal = 0u;



  UPROPERTY(VisibleAnywhere)
  uint32 SizeX = 0u;

  UPROPERTY(VisibleAnywhere)
  uint32 SizeY = 0u;

  UPROPERTY(VisibleAnywhere)
  EPostProcessEffect PostProcessEffect = EPostProcessEffect::INVALID;

  UPROPERTY(VisibleAnywhere)
  TArray<FColor> BitMap;
};
