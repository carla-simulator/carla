// Copyright (c) 2026 Computer Vision Center (CVC) at
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

	UPROPERTY(EditInstanceOnly, Category="MeshToSpline")
	AActor* SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, Transient, Category="MeshToSpline")
	TArray<USplineComponent*> GeneratedSplines;

	UFUNCTION(CallInEditor, Category="MeshToSpline")
	void GenerateSpline();

    // --- Post processing controls ---
    UPROPERTY(EditAnywhere, Category="MeshToSpline|Merge", meta=(ClampMin="0.0"))
    float WeldVertexTolerance = 5.0f;        // cm: weld vertices before islanding (reduces micro-gaps)

    UPROPERTY(EditAnywhere, Category="MeshToSpline|Merge", meta=(ClampMin="0.0"))
    float MergeIslandsDistance = 100.0f;     // cm: if two outer loops are closer than this, merge them

    UPROPERTY(EditAnywhere, Category="MeshToSpline|Merge", meta=(ClampMin="0.0"))
    float ContainmentEpsilon = 2.0f;         // cm: treat a loop inside another (2D) as inner -> remove


protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	USplineComponent* CreateEditorVisibleSpline(const FString& Name);
	void ClearGeneratedSplines();

	// --- helpers ---
	static void BuildIslands(const TArray<int32>& Triangles, TArray<TArray<int32>>& OutIslands);
	static void BuildBoundaryLoopsForIsland(const TArray<FVector>& Vertices,
	                                        const TArray<int32>& Triangles,
	                                        const TArray<int32>& IslandTriangles,
	                                        TArray<TArray<int32>>& OutLoopsIdx);

	static void ChooseProjectionAxisZ(const TArray<FVector>& Pts, int32& AxisZ /*0=X,1=Y,2=Z*/);
	static double SignedArea2D(const TArray<FVector>& Poly, int32 AxisZ);

    static void WeldVertices(const TArray<FVector>& InVerts, const TArray<int32>& InTris,
                            float Tol, TArray<FVector>& OutVerts, TArray<int32>& OutTris);

    static void ProjectPoly2D(const TArray<FVector>& Poly, int32 AxisZ, TArray<FVector2D>& Out);
    static bool PointInPoly2D(const FVector2D& P, const TArray<FVector2D>& Poly);

    static bool LoopsAreWithin(const TArray<FVector>& A, const TArray<FVector>& B, float Dist);
};
