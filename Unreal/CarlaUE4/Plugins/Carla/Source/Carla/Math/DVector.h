

struct FDVector
{

  double X = 0.0;
  double Y = 0.0;
  double Z = 0.0;

  FDVector() : X(0.0), Y(0.0), Z(0.0) {}

  FDVector(const FVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}

  FDVector(const FIntVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}

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

};