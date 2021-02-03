// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

struct FDVector
{

  double X = 0.0;
  double Y = 0.0;
  double Z = 0.0;

  FDVector() : X(0.0), Y(0.0), Z(0.0) {}

  FDVector(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}

  FDVector(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}

  FDVector(const FVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}

  FDVector(const FIntVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}

  static double Dist(const FDVector &V1, const FDVector &V2)
  {
    return std::sqrt(FDVector::DistSquared(V1, V2));
  }

  static double DistSquared(const FDVector &V1, const FDVector &V2)
  {
    return FMath::Square(V2.X-V1.X) + FMath::Square(V2.Y-V1.Y) + FMath::Square(V2.Z-V1.Z);
  }

  FString ToString() const
  {
    return FString::Printf(TEXT("X=%.2lf Y=%.2lf Z=%.2lf"), X, Y, Z);
  }

  FDVector& operator=(const FDVector& Other)
  {
    this->X = Other.X;
    this->Y = Other.Y;
    this->Z = Other.Z;
    return *this;
  }

  FDVector operator+(const FDVector& V) const
  {
    return FDVector(X + V.X, Y + V.Y, Z + V.Z);
  }


};