// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TreeTableRow.h"
#include "SignDataAsset.generated.h"
/**
 * 
 */
UCLASS()
class CARLATOOLS_API USignDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SignMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Unique; //Unused for now

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Diffuse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id_X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id_Y;
};
