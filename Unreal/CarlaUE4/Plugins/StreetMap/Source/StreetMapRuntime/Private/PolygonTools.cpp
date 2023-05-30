// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "PolygonTools.h"
#include "StreetMapRuntime.h"
#include "CompGeom/PolygonTriangulation.h"


// Based off "Efficient Polygon Triangulation" algorithm by John W. Ratcliff (http://flipcode.net/archives/Efficient_Polygon_Triangulation.shtml)
bool FPolygonTools::TriangulatePolygon( const TArray<FVector2D>& Polygon, TArray<int32>& TempIndices, TArray<int32>& TriangulatedIndices, bool& OutWindsClockwise )
{
  checkSlow( &TempIndices != &TriangulatedIndices );
  TriangulatedIndices.Reset();
  OutWindsClockwise = false;

  int32 NumVertices = Polygon.Num();
  if( NumVertices < 3 )
  {
    return false;
  }

  // Allocate and initialize a list of vertex indices for the new polygon
  TempIndices.SetNumUninitialized( NumVertices );
  int32* VertexIndices = TempIndices.GetData();

  // We want a counter-clockwise polygon
  OutWindsClockwise = Area( Polygon ) < 0.0f;
  if( !OutWindsClockwise )
  {
    for( int32 PointIndex = 0; PointIndex < NumVertices; PointIndex++ )
    {
      VertexIndices[ PointIndex ] = PointIndex;
    }
  }
  else
  {
    for( int32 PointIndex = 0; PointIndex < NumVertices; PointIndex++ )
    {
      VertexIndices[ PointIndex ] = ( NumVertices - 1 ) - PointIndex;
    }
  }
  TArray<FVector2<float>> InputVertices;
  TArray<FIndex3i> OutTriangles;
  if(OutWindsClockwise){
    for( int32 PointIndex = 0; PointIndex < NumVertices; PointIndex++ )
    {
      InputVertices.Add(FVector2<float>(Polygon[PointIndex].X, Polygon[PointIndex].Y));
    }
  }
  else
  {
    for( int32 PointIndex = 0; PointIndex < NumVertices; PointIndex++ )
    {
      int RealIndex = ( NumVertices - 1 ) - PointIndex;
      InputVertices.Add(FVector2<float>(Polygon[RealIndex].X, Polygon[RealIndex].Y));
    }
  }

  PolygonTriangulation::TriangulateSimplePolygon<float>(InputVertices, OutTriangles);

  for( FIndex3i Triangle : OutTriangles)
  {
    TriangulatedIndices.Add(Triangle.A);
    TriangulatedIndices.Add(Triangle.B);
    TriangulatedIndices.Add(Triangle.C);
  }
  return true;
}

