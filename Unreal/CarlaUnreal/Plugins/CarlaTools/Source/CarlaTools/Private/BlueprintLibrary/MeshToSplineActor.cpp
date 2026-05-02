#include "BlueprintLibrary/MeshToSplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"


// ========================= Actor boilerplate =========================

AMeshToSplineActor::AMeshToSplineActor()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
#if WITH_EDITOR
	SetFlags(RF_Transactional);
#endif
}

void AMeshToSplineActor::OnConstruction(const FTransform& Transform)
{
	// Optional: auto-regenerate when changed in editor.
	// GenerateSpline();
}

USplineComponent* AMeshToSplineActor::CreateEditorVisibleSpline(const FString& Name)
{
	USplineComponent* Spline = NewObject<USplineComponent>(this, USplineComponent::StaticClass(), *Name);
#if WITH_EDITOR
	Spline->SetFlags(RF_Transactional);
	Spline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // show in Details
#endif
	Spline->SetupAttachment(RootComponent);

	AddInstanceComponent(Spline);   // make it an instanced component (visible & saved)
	Spline->RegisterComponent();

#if WITH_EDITOR
	Spline->Modify();
	Modify();
#endif

	return Spline;
}

void AMeshToSplineActor::ClearGeneratedSplines()
{
	for (USplineComponent* S : GeneratedSplines)
	{
		if (S)
		{
#if WITH_EDITOR
			S->Modify();
#endif
			S->DestroyComponent();
		}
	}
	GeneratedSplines.Empty();
}

// ========================= Helpers =========================

// Split triangles into connected components via shared edges
void AMeshToSplineActor::BuildIslands(const TArray<int32>& Tris, TArray<TArray<int32>>& OutIslands)
{
	const int32 TriCount = Tris.Num() / 3;
	if (TriCount <= 0) return;

	auto EdgeKey = [](int32 a, int32 b){ return FIntPoint(FMath::Min(a,b), FMath::Max(a,b)); };

	// edge -> triangles touching it
	TMap<FIntPoint, TArray<int32>> EdgeToTris;
	EdgeToTris.Reserve(TriCount * 3);

	for (int32 t = 0; t < TriCount; ++t)
	{
		const int32 a = Tris[3*t + 0];
		const int32 b = Tris[3*t + 1];
		const int32 c = Tris[3*t + 2];
		for (auto [u,v] : { TPair<int32,int32>(a,b), {b,c}, {c,a} })
		{
			EdgeToTris.FindOrAdd(EdgeKey(u,v)).Add(t);
		}
	}

	// adjacency
	TArray<TArray<int32>> Adj; Adj.SetNum(TriCount);
	for (const auto& KV : EdgeToTris)
	{
		const TArray<int32>& Ts = KV.Value;
		for (int32 i = 0; i < Ts.Num(); ++i)
			for (int32 j = i+1; j < Ts.Num(); ++j)
			{
				Adj[Ts[i]].Add(Ts[j]);
				Adj[Ts[j]].Add(Ts[i]);
			}
	}

	// BFS to form islands
	TArray<uint8> Visited; Visited.SetNumZeroed(TriCount);
	for (int32 t = 0; t < TriCount; ++t)
	{
		if (Visited[t]) continue;
		TArray<int32> Comp;
		TArray<int32> Q; Q.Add(t); Visited[t] = 1;

		while (Q.Num())
		{
			const int32 cur = Q.Pop(false);
			Comp.Add(cur);
			for (int32 nb : Adj[cur])
			{
				if (!Visited[nb]) { Visited[nb] = 1; Q.Add(nb); }
			}
		}
		OutIslands.Add(MoveTemp(Comp));
	}
}

// Build boundary loops (vertex indices) for a specific island (subset of triangles)
void AMeshToSplineActor::BuildBoundaryLoopsForIsland(const TArray<FVector>& /*Verts*/,
                                                     const TArray<int32>& Tris,
                                                     const TArray<int32>& IslandTris,
                                                     TArray<TArray<int32>>& OutLoopsIdx)
{
	auto Key = [](int32 A, int32 B){ return FIntPoint(FMath::Min(A,B), FMath::Max(A,B)); };

	TMap<FIntPoint, int32> EdgeUse;
	TMap<int32, TArray<int32>> Nbrs;

	for (int32 t : IslandTris)
	{
		const int32 a = Tris[3*t + 0];
		const int32 b = Tris[3*t + 1];
		const int32 c = Tris[3*t + 2];

		for (auto [u,v] : { TPair<int32,int32>(a,b), {b,c}, {c,a} })
		{
			EdgeUse.FindOrAdd(Key(u,v))++;
		}
	}

	// boundary edges are used exactly once in this island
	for (const auto& E : EdgeUse)
	{
		if (E.Value == 1)
		{
			const int32 u = E.Key.X, v = E.Key.Y;
			Nbrs.FindOrAdd(u).Add(v);
			Nbrs.FindOrAdd(v).Add(u);
		}
	}

	// walk loops
	TSet<int32> Visited;
	for (const auto& It : Nbrs)
	{
		int32 start = It.Key;
		if (Visited.Contains(start)) continue;

		TArray<int32> Loop;
		int32 prev = INDEX_NONE, cur = start;

		while (cur != INDEX_NONE && !Visited.Contains(cur))
		{
			Visited.Add(cur);
			Loop.Add(cur);

			const TArray<int32>* Ns = Nbrs.Find(cur);
			int32 nxt = INDEX_NONE;
			if (Ns && Ns->Num() > 0)
			{
				if (Ns->Num() == 1) nxt = (*Ns)[0];
				else                 nxt = ((*Ns)[0] == prev) ? (*Ns)[1] : (*Ns)[0];
			}
			prev = cur; cur = nxt;
		}

		if (Loop.Num() >= 3) OutLoopsIdx.Add(MoveTemp(Loop));
	}
}

void AMeshToSplineActor::ChooseProjectionAxisZ(const TArray<FVector>& Pts, int32& AxisZ)
{
	FVector N(0,0,0);
	for (int32 i = 1; i + 1 < Pts.Num(); ++i)
		N += FVector::CrossProduct(Pts[i] - Pts[0], Pts[i+1] - Pts[0]);
	N = N.GetSafeNormal();

	if (FMath::Abs(N.X) >= FMath::Abs(N.Y) && FMath::Abs(N.X) >= FMath::Abs(N.Z)) AxisZ = 0;
	else if (FMath::Abs(N.Y) >= FMath::Abs(N.X) && FMath::Abs(N.Y) >= FMath::Abs(N.Z)) AxisZ = 1;
	else AxisZ = 2;
}

double AMeshToSplineActor::SignedArea2D(const TArray<FVector>& Poly, int32 AxisZ)
{
	auto P2 = [&](const FVector& P)->FVector2D {
		switch (AxisZ)
		{
			case 0: return FVector2D(P.Y, P.Z);
			case 1: return FVector2D(P.X, P.Z);
			default: return FVector2D(P.X, P.Y);
		}
	};

	double A = 0.0;
	const int32 n = Poly.Num();
	for (int32 i = 0; i < n; ++i)
	{
		const FVector2D p = P2(Poly[i]);
		const FVector2D q = P2(Poly[(i+1)%n]);
		A += (p.X * q.Y - q.X * p.Y);
	}
	return 0.5 * A;
}

// Quantized weld to close tiny cracks and reduce micro-islands
void AMeshToSplineActor::WeldVertices(const TArray<FVector>& InVerts, const TArray<int32>& InTris,
                                      float Tol, TArray<FVector>& OutVerts, TArray<int32>& OutTris)
{
	OutVerts.Reset(); OutTris.Reset();
	TMap<FIntVector,int32> Map;

	auto Q = [Tol](const FVector& P)
	{
		return FIntVector(
			FMath::RoundToInt(P.X / Tol),
			FMath::RoundToInt(P.Y / Tol),
			FMath::RoundToInt(P.Z / Tol));
	};

	TArray<int32> Remap; Remap.SetNum(InVerts.Num());

	for (int32 i=0;i<InVerts.Num();++i)
	{
		const FIntVector Key = Q(InVerts[i]);
		if (int32* Found = Map.Find(Key))
		{
			Remap[i] = *Found;
		}
		else
		{
			const int32 NewId = OutVerts.Add(InVerts[i]);
			Map.Add(Key, NewId);
			Remap[i] = NewId;
		}
	}

	for (int32 i=0;i+2<InTris.Num(); i+=3)
	{
		int32 a = Remap[InTris[i+0]];
		int32 b = Remap[InTris[i+1]];
		int32 c = Remap[InTris[i+2]];
		if (a!=b && b!=c && c!=a)
		{
			OutTris.Add(a); OutTris.Add(b); OutTris.Add(c);
		}
	}
}

void AMeshToSplineActor::ProjectPoly2D(const TArray<FVector>& Poly, int32 AxisZ, TArray<FVector2D>& Out)
{
	Out.SetNumUninitialized(Poly.Num());
	for (int32 i=0;i<Poly.Num();++i)
	{
		const FVector& P = Poly[i];
		switch (AxisZ)
		{
			case 0: Out[i] = FVector2D(P.Y, P.Z); break;
			case 1: Out[i] = FVector2D(P.X, P.Z); break;
			default:Out[i] = FVector2D(P.X, P.Y); break;
		}
	}
}

bool AMeshToSplineActor::PointInPoly2D(const FVector2D& P, const TArray<FVector2D>& Poly)
{
	bool c = false;
	const int32 n = Poly.Num();
	for (int32 i=0,j=n-1; i<n; j=i++)
	{
		const FVector2D& A = Poly[i];
		const FVector2D& B = Poly[j];
		const bool Intersect = ((A.Y > P.Y) != (B.Y > P.Y)) &&
			(P.X < (B.X-A.X) * (P.Y-A.Y) / (B.Y-A.Y + 1e-9f) + A.X);
		if (Intersect) c = !c;
	}
	return c;
}

// Quick proximity test between two closed polylines (actor-local)
bool AMeshToSplineActor::LoopsAreWithin(const TArray<FVector>& A, const TArray<FVector>& B, float Dist)
{
	const float Dist2 = Dist*Dist;

	auto SegDist2 = [](const FVector& p, const FVector& a, const FVector& b)
	{
		const FVector ab = b-a;
		const float denom = FVector::DotProduct(ab,ab);
		if (denom <= KINDA_SMALL_NUMBER) return (p-a).SizeSquared();
		const float t = FMath::Clamp(FVector::DotProduct(p-a, ab)/denom, 0.f, 1.f);
		return (a + t*ab - p).SizeSquared();
	};

	for (int32 i=0;i<A.Num();++i)
	{
		const FVector a0 = A[i], a1 = A[(i+1)%A.Num()];
		for (int32 j=0;j<B.Num();++j)
		{
			const FVector b0 = B[j], b1 = B[(j+1)%B.Num()];
			// endpoints vs opposite segment
			if (SegDist2(a0,b0,b1) < Dist2 || SegDist2(a1,b0,b1) < Dist2 ||
				SegDist2(b0,a0,a1) < Dist2 || SegDist2(b1,a0,a1) < Dist2)
				return true;
		}
	}
	return false;
}

// ========================= Main =========================

void AMeshToSplineActor::GenerateSpline()
{
	if (!IsValid(SourceActor)) return;

	UStaticMeshComponent* MeshComp = SourceActor->FindComponentByClass<UStaticMeshComponent>();
	if (!MeshComp) return;

	UStaticMesh* Mesh = MeshComp->GetStaticMesh();
	if (!Mesh) return;

	// Align this actor with source so actor-local matches visually
	SetActorTransform(SourceActor->GetActorTransform());

	// Clean previous components
	ClearGeneratedSplines();

	// -------- Gather ALL sections of LOD0 --------
	const int32 LOD = 0;
	const int32 NumSections = Mesh->GetNumSections(LOD);

	TArray<FVector> Vertices;   // mesh-local
	TArray<int32>   Triangles;

	int32 Base = 0;
	for (int32 s = 0; s < NumSections; ++s)
	{
		TArray<FVector> V; TArray<int32> T;
		TArray<FVector> N; TArray<FVector2D> UV; TArray<FProcMeshTangent> Tan;

		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(Mesh, LOD, s, V, T, N, UV, Tan);

		for (int32& idx : T) idx += Base;

		Vertices.Append(V);
		Triangles.Append(T);
		Base = Vertices.Num();
	}

	if (Vertices.Num() == 0 || Triangles.Num() == 0) return;

	// -------- Weld (close tiny gaps, reduce micro-islands) --------
	// Configurable in header:
	//   WeldVertexTolerance (cm)
	TArray<FVector> WeldVerts;
	TArray<int32>   WeldTris;
	WeldVertices(Vertices, Triangles, WeldVertexTolerance, WeldVerts, WeldTris);

	// -------- Split into triangle islands --------
	TArray<TArray<int32>> Islands;
	BuildIslands(WeldTris, Islands);

	const FTransform CompXf     = MeshComp->GetComponentTransform(); // mesh-local -> world
	const FTransform InvActorXf = GetActorTransform().Inverse();     // world -> actor-local

	// Per-island outer loops we’ll post-process
	struct FOuterLoop { TArray<FVector> Points; int32 AxisZ; double AbsArea; };
	TArray<FOuterLoop> Outer;

	for (int32 islandIdx = 0; islandIdx < Islands.Num(); ++islandIdx)
	{
		// boundary loops for this island
		TArray<TArray<int32>> LoopsIdx;
		BuildBoundaryLoopsForIsland(WeldVerts, WeldTris, Islands[islandIdx], LoopsIdx);
		if (LoopsIdx.Num() == 0) continue;

		// index loops -> actor-local points
		TArray<TArray<FVector>> Loops;
		Loops.Reserve(LoopsIdx.Num());
		for (const TArray<int32>& L : LoopsIdx)
		{
			TArray<FVector> P; P.Reserve(L.Num());
			for (int32 vi : L)
			{
				const FVector World = CompXf.TransformPosition(WeldVerts[vi]);
				P.Add(InvActorXf.TransformPosition(World));
			}
			Loops.Add(MoveTemp(P));
		}

		// choose largest |area| as the outer perimeter (don’t rely on orientation)
		int32 Best = INDEX_NONE;
		double BestAbsArea = -1.0;
		for (int32 i = 0; i < Loops.Num(); ++i)
		{
			int32 AxisZ = 2; ChooseProjectionAxisZ(Loops[i], AxisZ);
			const double A = FMath::Abs(SignedArea2D(Loops[i], AxisZ));
			if (A > BestAbsArea) { BestAbsArea = A; Best = i; }
		}
		if (Best == INDEX_NONE) continue;

		FOuterLoop Out;
		Out.Points  = Loops[Best];
		ChooseProjectionAxisZ(Out.Points, Out.AxisZ);
		Out.AbsArea = FMath::Abs(SignedArea2D(Out.Points, Out.AxisZ));
		Outer.Add(MoveTemp(Out));
	}

	if (Outer.Num() == 0) return;

	// -------- Remove inner islands (containment test in 2D) --------
	TArray<TArray<FVector2D>> Outer2D; Outer2D.SetNum(Outer.Num());
	for (int32 i=0;i<Outer.Num();++i)
		ProjectPoly2D(Outer[i].Points, Outer[i].AxisZ, Outer2D[i]);

	TArray<int32> KeepIdx;
	for (int32 i=0;i<Outer.Num();++i)
	{
		// centroid test
		FVector Centroid(0);
		for (const FVector& p : Outer[i].Points) Centroid += p;
		Centroid /= FMath::Max(1, Outer[i].Points.Num());

		FVector2D C2;
		switch (Outer[i].AxisZ)
		{
			case 0: C2 = FVector2D(Centroid.Y, Centroid.Z); break;
			case 1: C2 = FVector2D(Centroid.X, Centroid.Z); break;
			default:C2 = FVector2D(Centroid.X, Centroid.Y); break;
		}

		bool bContained = false;
		for (int32 j=0;j<Outer.Num() && !bContained;++j)
		{
			if (j==i) continue;
			// only consider potential containers with >= area
			if (Outer[j].AbsArea + 1.0 < Outer[i].AbsArea) continue;

			if (PointInPoly2D(C2, Outer2D[j]))
				bContained = true;
		}

		if (!bContained) KeepIdx.Add(i);
	}

	if (KeepIdx.Num() == 0) return;

	// -------- Proximity merge (cluster outlines by distance) --------
	// Configurable in header:
	//   MergeIslandsDistance (cm)
	TArray<int32> Parent; Parent.SetNum(KeepIdx.Num());
	for (int32 k=0;k<Parent.Num();++k) Parent[k]=k;

	auto Find=[&](int32 x){ while (Parent[x]!=x) x=Parent[x]=Parent[Parent[x]]; return x; };
	auto Union=[&](int32 a,int32 b){ a=Find(a); b=Find(b); if(a!=b) Parent[b]=a; };

	for (int32 a=0; a<KeepIdx.Num(); ++a)
	for (int32 b=a+1; b<KeepIdx.Num(); ++b)
	{
		const FOuterLoop& A = Outer[KeepIdx[a]];
		const FOuterLoop& B = Outer[KeepIdx[b]];
		if (LoopsAreWithin(A.Points, B.Points, MergeIslandsDistance))
			Union(a,b);
	}

	// choose the largest loop in each cluster as representative
	TMap<int32,int32> Rep; // root -> index in KeepIdx
	for (int32 i=0;i<KeepIdx.Num();++i)
	{
		const int32 r  = Find(i);
		const int32 oi = KeepIdx[i];
		if (!Rep.Contains(r) || Outer[oi].AbsArea > Outer[Rep[r]].AbsArea)
			Rep.Add(r, oi);
	}

	// -------- Emit splines --------
	int32 clusterIdx = 0;
	for (auto& Pair : Rep)
	{
		const FOuterLoop& L = Outer[Pair.Value];

		USplineComponent* Spline = CreateEditorVisibleSpline(
			FString::Printf(TEXT("MergedSpline_%03d"), clusterIdx++));
		GeneratedSplines.Add(Spline);

		// Clear the two default points before adding real ones
		Spline->ClearSplinePoints(false);

		for (const FVector& P : L.Points)
			Spline->AddSplinePoint(P, ESplineCoordinateSpace::Local, false);

		// Linear points (no curvature)
		for (int32 i = 0; i < Spline->GetNumberOfSplinePoints(); ++i)
		{
			Spline->SetSplinePointType(i, ESplinePointType::Linear, false);
			Spline->SetTangentsAtSplinePoint(i, FVector::ZeroVector, FVector::ZeroVector,
				ESplineCoordinateSpace::Local, false);
		}

		Spline->SetClosedLoop(true);
		Spline->UpdateSpline();
	}

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		MarkPackageDirty();
	}
#endif
}
