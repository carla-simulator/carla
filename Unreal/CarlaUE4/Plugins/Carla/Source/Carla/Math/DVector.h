// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

// Ideally, this could be included in unreal directly but it would be slow to work with
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

  double Size() const
  {
    return std::sqrt(X*X + Y*Y + Z*Z);
  }

  double SizeSquared() const
  {
    return X*X + Y*Y + Z*Z;
  }

  static double Dist(const FDVector &V1, const FDVector &V2)
  {
    return std::sqrt(FDVector::DistSquared(V1, V2));
  }

  static double DistSquared(const FDVector &V1, const FDVector &V2)
  {
    return FMath::Square(V2.X-V1.X) + FMath::Square(V2.Y-V1.Y) + FMath::Square(V2.Z-V1.Z);
  }

  FVector ToFVector() const
  {
    return FVector(X, Y, Z);
  }

  FString ToString() const
  {
    return FString::Printf(TEXT("X=%.2lf Y=%.2lf Z=%.2lf"), X, Y, Z);
  }

  FIntVector ToFIntVector() const
  {
    return FIntVector((int32)X, (int32)Y, (int32)Z);
  }

  FDVector& operator=(const FDVector& Other)
  {
    this->X = Other.X;
    this->Y = Other.Y;
    this->Z = Other.Z;
    return *this;
  }

  bool operator==(const FDVector& Other)
  {
    return X == Other.X &&
           Y == Other.Y &&
           Z == Other.Z;
  }

  FDVector operator+(const FDVector& V) const
  {
    return FDVector(X + V.X, Y + V.Y, Z + V.Z);
  }

  FDVector operator-(const FDVector& V) const
  {
    return FDVector(X - V.X, Y - V.Y, Z - V.Z);
  }

  FDVector operator-=(const FIntVector& V) const
  {
    return FDVector(X - V.X, Y - V.Y, Z - V.Z);
  }

  FDVector& operator-=(const FIntVector& V)
  {
    this->X -= V.X;
    this->Y -= V.Y;
    this->Z -= V.Z;
    return *this;
  }

  FDVector operator/(float Scale) const
  {
    const float RScale = 1.f/Scale;
    return FDVector(X * RScale, Y * RScale, Z * RScale);
  }

  FDVector operator*(float Scale) const
  {
    return FDVector(X * Scale, Y * Scale, Z * Scale);
  }

};