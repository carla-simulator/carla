// Copyright (c) 2025 Computer Vision Center (CVC) at
// the Universitat Autonoma de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// MeshToSplineActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshToSplineActor.generated.h"

class USplineComponent;

/**
 * Actor that extracts a boundary loop from a StaticMesh in another Actor
 * and builds a closed Spline from those points.
 */
UCLASS()
class CARLATOOLS_API AMeshToSplineActor : public AActor
{
    GENERATED_BODY()

public:
    AMeshToSplineActor();

    /** Generate the spline from the mesh found in SourceActor, applying the given transform to the mesh vertices. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mesh To Spline")
    void GenerateSpline();

protected:
    /** Spline component used to store the generated path. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USplineComponent* SplineComponent;

public:
    /** Actor that holds the StaticMeshComponent to sample. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh To Spline")
    AActor* SourceActor = nullptr;
};
