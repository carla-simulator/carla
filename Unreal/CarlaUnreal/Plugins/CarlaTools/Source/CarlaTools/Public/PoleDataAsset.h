// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TreeTableRow.h"
#include "PoleDataAsset.generated.h"


/**
 * 
 */
UCLASS()
class CARLATOOLS_API UPoleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* PoleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESignStyle SignStyle;
};
