// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Generation/PoissonDiscSampling.h"

#include "CarlaMeshGeneration.h"

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

  UE_LOG(LogCarlaMeshGeneration, Log,
    TEXT("Generating Poisson Disc Sampling points array.\n")
    TEXT(" R = %f | MaxRetries = %d | GridSize = (%d, %d) | CellCount = %d\n")
    TEXT(" AABB Min = (%f, %f), Max = (%f, %f) | CellSize = %f"),
    R, MaxRetries, GridSize.X, GridSize.Y, CellCount,
    Min.X, Min.Y, Max.X, Max.Y, CellSize);

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

  UE_LOG(LogCarlaMeshGeneration, Log,
    TEXT("Poisson Disc Sampling generated %d valid points."),
    Results2D.size());

  return Results2D;
}

bool FPCGPoissonDiscSampling::ExecuteInternal(
  FPCGContext* Context) const
{ 
  auto MakePCGPoint = [&](V2 Point)
  {
      FPCGPoint P;
      P.Transform.SetLocation(FVector(Point.X, Point.Y, 0.0F));
      return P;
  };

  const UPCGPoissonDiscSamplingSettings* SettingsPtr = Context->GetInputSettings<UPCGPoissonDiscSamplingSettings>();
  check(SettingsPtr);

  auto Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
  auto Output = FPCGContext::NewObject_AnyThread<UPCGPointData>(Context);
  float MinDistance = SettingsPtr->MinDistance;

  for (FPCGTaggedData& Input : Inputs)
  {
    const UPCGSplineData* InputData = Cast<UPCGSplineData>(Input.Data);
    check(InputData != nullptr);

    auto LocalToWorld = InputData->GetTransform();
    int32 SplineSampleCount = SettingsPtr->SplineSampleCount;
    std::vector<V2> Results2D;
    FBox SplineBoundingBox = InputData->GetBounds();
    std::vector<V2> SplinePoints;
    
    SplinePoints.reserve(SplineSampleCount);
    for (int32 i = 0; i < SplineSampleCount; ++i)
    {
      float Alpha = (float)i / (float)SplineSampleCount;
      FVector Pos = InputData->GetLocationAtAlpha(Alpha);
      Pos = InputData->GetTransform().TransformPosition(Pos);
      SplinePoints.push_back(V2(Pos.X, Pos.Y));
    }

    // Build edges
    bool bIsClosed = InputData->IsClosed();
    std::vector<Edge> SplineEdges;
    SplineEdges.reserve(SplinePoints.size());
    size_t Count = SplinePoints.size();
    for (size_t i = 0; i + 1 < Count; ++i)
      SplineEdges.emplace_back(SplinePoints[i], SplinePoints[i + 1]);
    if (bIsClosed && Count > 2)
      SplineEdges.emplace_back(SplinePoints.back(), SplinePoints.front());
    if (bIsClosed)
      SplineEdges.emplace_back(SplinePoints.back(), SplinePoints[0]);

    Results2D = GeneratePoissonDiscPoints(
      Context, SplineBoundingBox, SplineEdges, MinDistance, *SettingsPtr);

    Results2D.erase(std::remove_if(Results2D.begin(), Results2D.end(),
      [&](const V2& Point) { return !IsInsideSpline(SplineEdges, Point); }),
      Results2D.end());
    
    Output->InitializeFromData(InputData);
    auto& OutputPoints = Output->GetMutablePoints();
    OutputPoints.Reserve(Results2D.size());
    for (auto& Point2D : Results2D)
      OutputPoints.Add(MakePCGPoint(Point2D));
    Context->OutputData.TaggedData.Add_GetRef(Input).Data = Output;
  }


  return true;
}
