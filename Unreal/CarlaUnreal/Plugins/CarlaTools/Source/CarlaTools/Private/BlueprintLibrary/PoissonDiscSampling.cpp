// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibrary/PoissonDiscSampling.h"

#include "PCGContext.h"
#include "PCGComponent.h"
#include "PCGParamData.h"
#include "Metadata/PCGMetadata.h"
#include "Metadata/PCGMetadataAttributeTpl.h" 

#include "PCGCustomVersion.h"
#include "PCGPin.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSplineData.h"

#include <unordered_map>
#include <algorithm>
#include <optional>
#include <atomic>
#include <vector>
#include <random>
#include <array>
#include <span>
#include <bit>

using RealT = float;
using IntT = int32;
using V2 = UE::Math::TVector2<RealT>;
using I2 = FIntPoint;
using Edge = std::pair<V2, V2>;

UPCGPoissonDiscSamplingSettings::UPCGPoissonDiscSamplingSettings()
{
}

TArray<FPCGPinProperties> UPCGPoissonDiscSamplingSettings::InputPinProperties() const
{
  TArray<FPCGPinProperties> Properties;
  FPCGPinProperties& SplineInputPinProperty = Properties.Emplace_GetRef(PCGPinConstants::DefaultInputLabel, EPCGDataType::Spline);
  SplineInputPinProperty.SetRequiredPin();

  return Properties;
}

float GetZFromAlphaTable(const TArray<TPair<float, float>>& Table, float Alpha)
{
    for (int32 i = 1; i < Table.Num(); ++i)
    {
        if (Alpha <= Table[i].Key)
        {
            float Alpha1 = Table[i - 1].Key;
            float Alpha2 = Table[i].Key;
            float Z1 = Table[i - 1].Value;
            float Z2 = Table[i].Value;

            float T = FMath::Clamp((Alpha - Alpha1) / (Alpha2 - Alpha1), 0.0f, 1.0f);
            return FMath::Lerp(Z1, Z2, T);
        }
    }
    return Table.Last().Value;
}

TArray<FPCGPinProperties> UPCGPoissonDiscSamplingSettings::OutputPinProperties() const
{
  return Super::DefaultPointOutputPinProperties();
}

FPCGElementPtr UPCGPoissonDiscSamplingSettings::CreateElement() const
{
  return MakeShared<FPCGPoissonDiscSampling>();
}

static RealT KahanDeterminant(
  V2 u, V2 v)
{
  RealT
    a = u.X, b = v.X,
    c = u.Y, d = v.Y;
  RealT w = b * c;
  return std::fma(-b, c, w) + std::fma(a, d, -w);
}

static bool IsInsideSpline(
  std::span<Edge> Edges,
  V2 point)
{
  double TotalAngle = 0.0;

  for (const Edge& e : Edges)
  {
    V2 u = e.first - point;
    V2 v = e.second - point;

    double det = u.X * v.Y - u.Y * v.X;
    double dot = u.X * v.X + u.Y * v.Y;
    double angle = std::atan2(det, dot);

    TotalAngle += angle;
  }

  // Winding number check: if abs(total angle) ~ 2*PI, we're inside
  return FMath::Abs(TotalAngle) > PI;
}

static FBox ComputeSplineBoundingBox(
  std::span<V2> Points)
{
  V2 Min, Max;
  if (Points.empty())
    return FBox();
  Min = Points[0];
  Max = Points[0];
  for (std::size_t i = 1; i != Points.size(); ++i)
  {
    Min = V2::Min(Min, Points[i]);
    Max = V2::Max(Max, Points[i]);
  }
  return FBox(FVector(Min.X, Min.Y, 0.0), FVector(Max.X, Max.Y, 0.0));
}

static std::vector<V2> GeneratePoissonDiscPoints(
  FPCGContext* Context,
  FBox SplineBB,
  std::span<Edge> Edges,
  float MinDistance,
  const UPCGPoissonDiscSamplingSettings& Settings)
{
  std::random_device RD;
  std::ranlux48 PRNG(RD());
  std::uniform_real_distribution<RealT> URD(0, 1);

  const RealT Sqrt2 = FMath::Sqrt((RealT)2);
  const RealT Tau = (RealT)(2.0 * PI);

  const V2 Min(SplineBB.Min.X, SplineBB.Min.Y);
  const V2 Max(SplineBB.Max.X, SplineBB.Max.Y);
  const V2 Extent = Max - Min;

  const RealT R = (RealT)MinDistance;
  const RealT R2 = R * R;
  const IntT MaxRetries = Settings.MaxRetries;

  const RealT CellSize = R / Sqrt2;
  const V2 GridFloat = Extent / CellSize;
  const FIntPoint GridSize(FMath::CeilToInt(GridFloat.X), FMath::CeilToInt(GridFloat.Y));
  const int32 CellCount = GridSize.X * GridSize.Y;
  const bool UseBitMask = (4096 >= (CellCount / 8));

  std::unordered_map<IntT, V2> Grid;
  std::vector<bool> Occupancy;
  if (UseBitMask)
    Occupancy.resize(CellCount);

  auto GetRandomScalar = [&](RealT MinVal, RealT MaxVal)
    {
      return std::fma(URD(PRNG), MaxVal - MinVal, MinVal);
    };

  auto GetRandomPoint = [&](V2 A, V2 B)
    {
      return V2(
        GetRandomScalar(A.X, B.X),
        GetRandomScalar(A.Y, B.Y));
    };

  auto GridCoordToFlatIndex = [&](I2 Key)
    {
      return Key.X + Key.Y * GridSize.X;
    };

  auto GridQuery = [&](I2 Key) -> std::optional<V2>
    {
      const int32 Flat = GridCoordToFlatIndex(Key);
      if (UseBitMask && !Occupancy[Flat])
        return std::nullopt;
      auto it = Grid.find(Flat);
      if (it == Grid.end())
        return std::nullopt;
      return it->second;
    };

  auto GridAdd = [&](I2 Key, V2 Value)
    {
      const int32 Flat = GridCoordToFlatIndex(Key);
      if (UseBitMask)
        Occupancy[Flat] = true;
      Grid[Flat] = Value;
    };

  std::vector<V2> Results2D;
  std::vector<V2> Pending;
  Results2D.reserve(CellCount);
  Pending.reserve(CellCount);
  Grid.reserve(CellCount);

  V2 First = GetRandomPoint(Min, Max);
  Results2D.push_back(First);
  Pending.push_back(First);

  while (!Pending.empty())
  {
    std::uniform_int_distribution<IntT> UID(0, (IntT)Pending.size() - 1);
    IntT Index = UID(PRNG);
    V2 Point = Pending[Index];
    bool Found = false;

    for (IntT i = 0; i < MaxRetries; ++i)
    {
      const RealT Theta = GetRandomScalar(0, Tau);
      const RealT Rho = GetRandomScalar(R, 2 * R);
      V2 SinCos;
      FMath::SinCos(&SinCos.Y, &SinCos.X, Theta);
      V2 NewPoint = Point + Rho * SinCos;

      if (NewPoint.X < Min.X || NewPoint.X >= Max.X ||
        NewPoint.Y < Min.Y || NewPoint.Y >= Max.Y)
        continue;

      V2 Tmp = (NewPoint - Min) / CellSize;
      I2 GridCoord((IntT)Tmp.X, (IntT)Tmp.Y);

      const I2 Offsets[] =
      {
        I2(-2,-2), I2(-1,-2), I2(0,-2), I2(1,-2), I2(2,-2),
        I2(-2,-1), I2(-1,-1), I2(0,-1), I2(1,-1), I2(2,-1),
        I2(-2, 0), I2(-1, 0), I2(0, 0), I2(1, 0), I2(2, 0),
        I2(-2, 1), I2(-1, 1), I2(0, 1), I2(1, 1), I2(2, 1),
        I2(-2, 2), I2(-1, 2), I2(0, 2), I2(1, 2), I2(2, 2)
      };

      std::vector<I2> TestPositions;
      TestPositions.reserve(25);
      for (I2 Offset : Offsets)
      {
        I2 Test = GridCoord + Offset;
        if (Test.X >= 0 && Test.Y >= 0 && Test.X < GridSize.X && Test.Y < GridSize.Y)
          TestPositions.push_back(Test);
      }

      bool OK = true;
      for (I2 Test : TestPositions)
      {
        auto Neighbor = GridQuery(Test);
        if (Neighbor && V2::DistSquared(NewPoint, *Neighbor) < R2)
        {
          OK = false;
          break;
        }
      }

      if (OK)
      {
        Results2D.push_back(NewPoint);
        Pending.push_back(NewPoint);
        GridAdd(GridCoord, NewPoint);
        Found = true;
        break;
      }
    }

    if (!Found)
      Pending.erase(Pending.begin() + Index);
  }

  return Results2D;
}

bool FPCGPoissonDiscSampling::ExecuteInternal(
  FPCGContext* Context) const
{ 
  const UPCGPoissonDiscSamplingSettings* SettingsPtr = Context->GetInputSettings<UPCGPoissonDiscSamplingSettings>();
  check(SettingsPtr);

  auto Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
  float MinDistance = SettingsPtr->MinDistance;

  for (FPCGTaggedData& Input : Inputs)
  {
    const UPCGSplineData* InputData = Cast<UPCGSplineData>(Input.Data);
    if (!InputData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid spline input."));
        continue;
    }

    // Transform bounding box to world space
    FTransform SplineTransform = InputData->GetTransform();
    // Sample points along the spline
    int32 SampleCount = SettingsPtr->SplineSampleCount;
    TArray<TPair<float, FVector>> SplineAlphaTable;
    SplineAlphaTable.Reserve(SampleCount);

    std::vector<V2> SplinePoints;
    SplinePoints.reserve(SampleCount);

    FBox SplineBoundingBox(EForceInit::ForceInit);

    for (int32 i = 0; i < SampleCount; ++i)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(SampleCount - 1);

        // Local spline position (used for filtering and bounds)
        FVector LocalPos = InputData->GetLocationAtAlpha(Alpha);
        SplinePoints.emplace_back(LocalPos.X, LocalPos.Y);
        SplineBoundingBox += FVector(LocalPos.X, LocalPos.Y, LocalPos.Z);

        // World position (used for Z height and placing points)
        FVector WorldPos = InputData->GetTransform().TransformPosition(LocalPos);
        SplineAlphaTable.Emplace(Alpha, LocalPos);
    }

    // Build polygon edges
    std::vector<Edge> SplineEdges;
    for (size_t i = 0; i + 1 < SplinePoints.size(); ++i)
        SplineEdges.emplace_back(SplinePoints[i], SplinePoints[i + 1]);
    if (InputData->IsClosed() && SplinePoints.size() > 2)
        SplineEdges.emplace_back(SplinePoints.back(), SplinePoints.front());

    // Generate Poisson points
    std::vector<V2> Results2D = GeneratePoissonDiscPoints(
        Context, SplineBoundingBox, SplineEdges, SettingsPtr->MinDistance, *SettingsPtr);

    // Filter inside spline
    Results2D.erase(std::remove_if(Results2D.begin(), Results2D.end(),
        [&](const V2& Point) { return !IsInsideSpline(SplineEdges, Point); }),
        Results2D.end());

    // Build output data
    UPCGPointData* Output = FPCGContext::NewObject_AnyThread<UPCGPointData>(Context);
    Output->InitializeFromData(InputData);
    auto& OutputPoints = Output->GetMutablePoints();
    OutputPoints.Reserve(Results2D.size());

    
    for (const V2& P : Results2D)
    {
        FVector2D Target2D(P.X, P.Y);

        // Find closest point in the table
        float ClosestDistSqr = FLT_MAX;
        float BestAlpha = 0.0f;
        FVector BestWorldPos = FVector::ZeroVector;

        for (const TPair<float, FVector>& Entry : SplineAlphaTable)
        {
            const FVector2D Table2D(Entry.Value.X, Entry.Value.Y);
            float DistSqr = FVector2D::DistSquared(Target2D, Table2D);

            if (DistSqr < ClosestDistSqr)
            {
                ClosestDistSqr = DistSqr;
                BestAlpha = Entry.Key;
                BestWorldPos = Entry.Value;
            }
        }

        // Build PCG point
        FPCGPoint Point;
        FVector WorldPoint = FVector(Target2D.X, Target2D.Y, BestWorldPos.Z);
        Point.Transform.SetLocation(WorldPoint);

        
      

        OutputPoints.Add(Point);
    }

    // Add to output tagged data (per spline)
    FPCGTaggedData& TaggedOutput = Context->OutputData.TaggedData.Emplace_GetRef();
    TaggedOutput.Pin = PCGPinConstants::DefaultOutputLabel;  // Safe default
    TaggedOutput.Data = Output;
  }


  return true;
}
