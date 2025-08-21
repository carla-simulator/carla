// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// MeshToSplineActor.cpp
#include "BlueprintLibrary/MeshToSplineActor.h"
#include "Components/SplineComponent.h"
#include "KismetProceduralMeshLibrary.h"

AMeshToSplineActor::AMeshToSplineActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SetRootComponent(SplineComponent);
}

void AMeshToSplineActor::GenerateSpline()
{
    if (!IsValid(SourceActor))
    {
        return;
    }

    UStaticMeshComponent* MeshComp = SourceActor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }

    UStaticMesh* SourceMesh = MeshComp->GetStaticMesh();
    if (!SourceMesh)
    {
        return;
    }

    // Align this actor to the source (optional; harmless even if you add world points)
    SetActorTransform(SourceActor->GetActorTransform());

    // Arrays donde guardaremos geometría
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FProcMeshTangent> Tangents;

    // Extraer info del LOD0 sección 0
    UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(SourceMesh, 0, 0,
        Vertices, Triangles, Normals, UVs, Tangents);

    // Mapa de edges (cuenta cuántas veces aparece cada edge)
    TMap<FIntPoint, int32> EdgeCount;
    TMultiMap<int32, int32> EdgeConnections;

    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        int32 A = Triangles[i];
        int32 B = Triangles[i+1];
        int32 C = Triangles[i+2];

        auto AddEdge = [&](int32 V1, int32 V2)
        {
            FIntPoint Edge(FMath::Min(V1, V2), FMath::Max(V1, V2));
            EdgeCount.FindOrAdd(Edge)++;

            // Guardar conexiones (para reconstruir bucle)
            EdgeConnections.Add(V1, V2);
            EdgeConnections.Add(V2, V1);
        };

        AddEdge(A, B);
        AddEdge(B, C);
        AddEdge(C, A);
    }

    // Encontrar un vértice frontera para empezar
    int32 StartVertex = INDEX_NONE;
    for (auto& Pair : EdgeCount)
    {
        if (Pair.Value == 1) // edge frontera
        {
            StartVertex = Pair.Key.X;
            break;
        }
    }
    if (StartVertex == INDEX_NONE) return; // no hay borde

    // Reconstruir el loop ordenado
    TArray<FVector> LoopPoints;
    TSet<int32> Visited;
    int32 Current = StartVertex;

    while (!Visited.Contains(Current))
    {
        Visited.Add(Current);
        LoopPoints.Add(Vertices[Current]);

        TArray<int32> Connected;
        EdgeConnections.MultiFind(Current, Connected);

        int32 Next = INDEX_NONE;
        for (int32 C : Connected)
        {
            FIntPoint Edge(FMath::Min(Current, C), FMath::Max(Current, C));
            if (EdgeCount.Contains(Edge) && EdgeCount[Edge] == 1 && !Visited.Contains(C))
            {
                Next = C;
                break;
            }
        }

        if (Next == INDEX_NONE) break;
        Current = Next;
    }

    SplineComponent->ClearSplinePoints();


    for (const FVector& P : LoopPoints)
    {
        SplineComponent->AddSplinePoint(P, ESplineCoordinateSpace::Local, false);
    }

    SplineComponent->SetClosedLoop(true);
    SplineComponent->UpdateSpline();
}
