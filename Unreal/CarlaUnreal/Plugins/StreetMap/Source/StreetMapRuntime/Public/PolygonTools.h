// Copyright 2017 Mike Fricker. All Rights Reserved.
#pragma once

class FPolygonTools
{

public:

	/** Triangulate a polygon given a list of contour points, then places results as indices into the original polygon array.  Does not support polygons with holes. */
	static bool TriangulatePolygon( const TArray<FVector2D>& Polygon, TArray<int32>& TempIndices, TArray<int32>& TriangulatedIndices, bool& OutWindsClockwise );

	/** Compute area of a polygon */
	static inline float Area( const TArray<FVector2D>& Polygon );

	/** Determines if the specified point is inside the triangle defined by the three triangle corners */
	static inline bool IsPointInsideTriangle( const FVector2D TriangleA, const FVector2D TriangleB, const FVector2D TriangleC, const FVector2D Point );

	/** Given a 2D polygon and a point, determines whether the point is inside the polygon.  Supports convex polygons.  If the point is exactly on the polygon boundary, the return value could be either false or true. */
	static inline bool IsPointInsidePolygon( const TArray<FVector2D>& Polygon, const FVector2D Point );


private:

	/** Clips a polygon */
	static inline bool Snip( const TArray<FVector2D>& Polygon, const int32 U, const int32 V, const int32 W, const int32 PointCount, const int32* VertexIndices );
};


float FPolygonTools::Area( const TArray<FVector2D>& Polygon )
{
	const int32 PointCount = Polygon.Num();

	float HalfArea = 0.0f;
	for( int32 P = PointCount - 1, Q = 0; Q < PointCount; P = Q++ )
	{
		HalfArea += Polygon[ P ].X * Polygon[ Q ].Y - Polygon[ Q ].X * Polygon[ P ].Y;
	}
	return HalfArea * 0.5f;
}


bool FPolygonTools::IsPointInsideTriangle( const FVector2D TriangleA, const FVector2D TriangleB, const FVector2D TriangleC, const FVector2D Point )
{
	const FVector2D BToC = TriangleC - TriangleB;
	const FVector2D CToA = TriangleA - TriangleC; 
	const FVector2D AToB = TriangleB - TriangleA;

	const FVector2D AToP = Point - TriangleA;
	const FVector2D BToP = Point - TriangleB;
	const FVector2D CToP = Point - TriangleC;

	const float ACrossBP = BToC ^ BToP;
	const float CCrossAP = AToB ^ AToP;
	const float BCrossCP = CToA ^ CToP;

	return ( ( ACrossBP >= -SMALL_NUMBER ) && ( BCrossCP >= -SMALL_NUMBER ) && ( CCrossAP >= -SMALL_NUMBER ) );
};


bool FPolygonTools::IsPointInsidePolygon( const TArray<FVector2D>& Polygon, const FVector2D Point )
{
	const int NumCorners = Polygon.Num();
	int PreviousCornerIndex = NumCorners - 1;
	bool bIsInside = false;

	for( int CornerIndex = 0; CornerIndex < NumCorners; CornerIndex++ )
	{
		if( ( ( Polygon[ CornerIndex ].Y < Point.Y && Polygon[ PreviousCornerIndex ].Y >= Point.Y ) || ( Polygon[ PreviousCornerIndex ].Y < Point.Y && Polygon[ CornerIndex ].Y >= Point.Y ) ) && 
			( Polygon[ CornerIndex ].X <= Point.X || Polygon[ PreviousCornerIndex ].X <= Point.X ) )
		{
			bIsInside ^= ( Polygon[ CornerIndex ].X + ( Point.Y - Polygon[ CornerIndex ].Y ) / ( Polygon[ PreviousCornerIndex ].Y - Polygon[ CornerIndex ].Y )*( Polygon[ PreviousCornerIndex ].X - Polygon[ CornerIndex ].X ) < Point.X );
		}

		PreviousCornerIndex = CornerIndex;
	}

	return bIsInside;
}


bool FPolygonTools::Snip( const TArray<FVector2D>& Polygon, const int32 U, const int32 V, const int32 W, const int32 PointCount, const int32* VertexIndices )
{
	const FVector2D A = Polygon[ VertexIndices[ U ] ];
	const FVector2D B = Polygon[ VertexIndices[ V ] ];
	const FVector2D C = Polygon[ VertexIndices[ W ] ];

	if( SMALL_NUMBER > ( ( ( B.X - A.X ) * ( C.Y - A.Y ) ) - ( ( B.Y - A.Y ) * ( C.X - A.X ) ) ) &&

		// Don't fail because of degenerate points
		( A - B ).SizeSquared() > SMALL_NUMBER &&
		( A - C ).SizeSquared() > SMALL_NUMBER &&
		( C - A ).SizeSquared() > SMALL_NUMBER )
	{
		return false;
	}

	for( int32 PointIndex = 0; PointIndex < PointCount; ++PointIndex )
	{
		if( ( PointIndex == U ) || ( PointIndex == V ) || ( PointIndex == W ) )
		{
			continue;
		}

		if( IsPointInsideTriangle( A, B, C, Polygon[ VertexIndices[ PointIndex ] ] ) )
		{
			return false;
		}
	}

	return true;
}


