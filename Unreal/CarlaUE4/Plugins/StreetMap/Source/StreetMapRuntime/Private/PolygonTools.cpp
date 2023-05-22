// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "PolygonTools.h"
#include "StreetMapRuntime.h"


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

	// Remove NumVertices-2 vertices, creating one triangle every time 
	int32 ErrorDetectionCounter = 2 * NumVertices;
	for( int32 V = NumVertices - 1; NumVertices > 2; )
	{
		// If we loop, it is probably a non-simple polygon
		if( --ErrorDetectionCounter <= 0 )
		{
			// Bad polygon!  This can happen if there are adjacent points that are exactly overlapping
			return false;
		}

		// Three consecutive vertices in current polygon, <u,v,w>
		const int32 U = ( V < NumVertices ) ? V : 0;
		V = ( ( U + 1 ) < NumVertices ) ? U + 1 : 0;
		const int32 W = ( ( V + 1 ) < NumVertices ) ? V + 1 : 0;

		if( Snip( Polygon, U, V, W, NumVertices, VertexIndices ) )
		{
			// Output triangle as indices into the original polygon array
			TriangulatedIndices.Add( VertexIndices[ U ] );
			TriangulatedIndices.Add( VertexIndices[ V ] );
			TriangulatedIndices.Add( VertexIndices[ W ] );

			/* Remove V from remaining polygon */
			for( int32 S = V, T = V + 1; T < NumVertices; S++, T++ )
			{
				VertexIndices[ S ] = VertexIndices[ T ]; 
			}
			NumVertices--;

			// Reset error detection counter
			ErrorDetectionCounter = 2 * NumVertices;
		}
	}

	return true;
}

