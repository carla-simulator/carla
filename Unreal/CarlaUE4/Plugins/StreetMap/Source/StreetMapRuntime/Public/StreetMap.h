// Copyright 2017 Mike Fricker. All Rights Reserved.

#pragma once

#include "StreetMap.generated.h"


USTRUCT(BlueprintType)
struct STREETMAPRUNTIME_API FStreetMapCollisionSettings
{
	GENERATED_USTRUCT_BODY()

public:


	/** Uses triangle mesh data for collision data. (Cannot be used for physics simulation). */
	UPROPERTY(EditAnywhere, Category = "StreetMap")
		uint32 bGenerateCollision : 1;

	/**
	*	If true, the physics triangle mesh will use double sided faces when doing scene queries.
	*	This is useful for planes and single sided meshes that need traces to work on both sides.
	*/
	UPROPERTY(EditAnywhere, Category = "StreetMap", meta = (editcondition = "bGenerateCollision"))
		uint32 bAllowDoubleSidedGeometry : 1;


	FStreetMapCollisionSettings() :
		bGenerateCollision(false),
		bAllowDoubleSidedGeometry(false)
	{

	}

};

/** Mesh generation settings */
USTRUCT(BlueprintType)
struct STREETMAPRUNTIME_API FStreetMapMeshBuildSettings
{
	GENERATED_USTRUCT_BODY()

public:

	/** Roads base vertical offset */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"), DisplayName = "Road Vertical Offset")
		float RoadOffesetZ;

	/** if true buildings mesh will be 3D instead of flat representation. */
	UPROPERTY(Category = StreetMap, EditAnywhere, DisplayName = "Create 3D Buildings")
		uint32 bWant3DBuildings : 1;

	/** building level floor conversion factor in centimeters
		@todo: harmonize with OSMToCentimetersScaleFactor refactoring
	*/
	UPROPERTY(Category = StreetMap, EditAnywhere, DisplayName = "Building Level Floor Factor")
		float BuildingLevelFloorFactor = 300.0f;

	/**
	* If true, buildings mesh will receive light information.
	* Lit buildings can't share vertices beyond quads (all quads have their own face normals), so this uses a lot more geometry.
	*/
	UPROPERTY(Category = StreetMap, EditAnywhere, DisplayName = "Lit buildings")
	uint32 bWantLitBuildings : 1;

	/** Streets thickness */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
		float StreetThickness;

	/** Street vertex color */
	UPROPERTY(Category = StreetMap, EditAnywhere)
		FLinearColor StreetColor;

	/** Major road thickness */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
		float MajorRoadThickness;

	/** Major road vertex color */
	UPROPERTY(Category = StreetMap, EditAnywhere)
		FLinearColor MajorRoadColor;

	/** Highway thickness */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
		float HighwayThickness;

	/** Highway vertex color */
	UPROPERTY(Category = StreetMap, EditAnywhere)
		FLinearColor HighwayColor;

	/** Streets Thickness */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
		float BuildingBorderThickness;

	/** Building border vertex color */
	UPROPERTY(Category = StreetMap, EditAnywhere)
		FLinearColor BuildingBorderLinearColor;

	/** Buildings border vertical offset */
	UPROPERTY(Category = StreetMap, EditAnywhere, meta = (ClampMin = "0", UIMin = "0"))
		float BuildingBorderZ;

	FStreetMapMeshBuildSettings() :
		RoadOffesetZ(0.0f),
		bWant3DBuildings(true),
		bWantLitBuildings(true),
		StreetThickness(800.0f),
		StreetColor(0.05f, 0.75f, 0.05f),
		MajorRoadThickness(1000.0f),
		MajorRoadColor(0.15f, 0.85f, 0.15f),
		HighwayThickness(1400.0f),
		HighwayColor(FLinearColor(0.25f, 0.95f, 0.25f)),
		BuildingBorderThickness(20.0f),
		BuildingBorderLinearColor(0.85f, 0.85f, 0.85f),
		BuildingBorderZ(10.0f)
	{
	}

};



/** Types of roads */
UENUM( BlueprintType )
enum EStreetMapRoadType
{
	/** Small road or residential street */
	Street,
	
	/** Major road or minor state highway */
	MajorRoad,
	
	/** Highway */
	Highway,
	
	/** Other (path, bus route, etc) */
	Other,
};


/** A road */
USTRUCT( BlueprintType )
struct STREETMAPRUNTIME_API FStreetMapRoad
{
	GENERATED_USTRUCT_BODY()

	/** Name of the road */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FString RoadName;
	
	/** Type of road */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	TEnumAsByte<EStreetMapRoadType> RoadType;
	
	/** Nodes along this road, one at each point in the RoadPoints list */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	TArray<int32> NodeIndices;

	/** List of all of the points on this road, one for each node in the NodeIndices list */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	TArray<FVector2D> RoadPoints;
	
	// @todo: Performance: Bounding information could be computed at load time if we want to avoid the memory cost of storing it

	/** 2D bounds (min) of this road's points */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FVector2D BoundsMin;
	
	/** 2D bounds (max) of this road's points */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FVector2D BoundsMax;

	/** True if this node is a one way.  One way nodes are only traversable in the order the nodes are listed in the above array. */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	uint8 bIsOneWay : 1;


	/** Returns this node's index */
	inline int32 GetRoadIndex( const class UStreetMap& StreetMap ) const;

	/** Gets the node for the specified point, or the node that came before that if the specified point doesn't have a node */
	inline const struct FStreetMapNode& GetNodeAtPointIndexOrEarlier( const class UStreetMap& StreetMap, const int32 PointIndex, int32& OutNodeAtPointIndex ) const;

	/** Gets the node for the specified point, or the node that comes next after that if the specified point doesn't have a node */
	inline const struct FStreetMapNode& GetNodeAtPointIndexOrLater( const class UStreetMap& StreetMap, const int32 PointIndex, int32& OutNodeAtPointIndex ) const;

	/** Computes the total length of this road by following along all of it's points */
	float ComputeLengthOfRoad( const class UStreetMap& StreetMap ) const;

	/** Computes the distance along the road between two points on the road.  Be careful!  The same node can appear on a road twice. */
	float ComputeDistanceBetweenNodesOnRoad( const class UStreetMap& StreetMap, const int32 NodePointIndexA, const int32 NodePointIndexB ) const;

	/** Given a position along the road, finds the nodes that come earlier and later on that road */
	void FindEarlierAndLaterNodesForPositionAlongRoad( const class UStreetMap& StreetMap, const float PositionAlongRoad, const FStreetMapNode*& OutEarlierNode, float& OutEarlierNodePositionAlongRoad, const FStreetMapNode*& OutLaterNode, float& OutLaterNodePositionAlongRoad ) const;

	/** Given a node that exists at a point index on this road, finds the nodes that are immediately earlier and later to it (adjacent.)  Will set a nullptr if there are no earlier or later nodes */
	void FindEarlierAndLaterNodes( const class UStreetMap& StreetMap, const int32 RoadPointIndex, const FStreetMapNode*& OutEarlierNode, float& OutEarlierNodePositionAlongRoad, const FStreetMapNode*& OutLaterNode, float& OutLaterNodePositionAlongRoad ) const;

	// NOTE: You may notice there is no "FindPointIndexForNode()" method in this class.  This is because
	//       the same node may appear more than once on any single road, so it's never safe to ask for
	//       a single point index on a road.

	/** Given a node that exists on this road, computes the position along this road of that node */
	float FindPositionAlongRoadForNode( const class UStreetMap& StreetMap, const int32 PointIndexForNode ) const;

	/** Computes the location of a point along this road, given a distance along this road from the road's beginning */
	FVector2D MakeLocationAlongRoad( const class UStreetMap& StreetMap, const float PositionAlongRoad ) const;

	/** @return True if this is a one way road */
	inline bool IsOneWay() const
	{
		return bIsOneWay == 1 ? true : false;
	}
};


/** Nodes have a list of road refs, one for each road that intersects this node.  Each road ref references a road and also the 
    point along that road where this node exists. */
USTRUCT( BlueprintType )
struct STREETMAPRUNTIME_API FStreetMapRoadRef
{
	GENERATED_USTRUCT_BODY()

	/** Index of road in the list of all roads in this street map */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	int32 RoadIndex;
	
	/** Index of the point along road where this node exists */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	int32 RoadPointIndex;
};


/** Describes a node on a road.  Nodes usually connect at least two roads together, but they might also exist at the end of a dead-end street.  They are sort of like an "intersection". */
USTRUCT( BlueprintType )
struct STREETMAPRUNTIME_API FStreetMapNode
{
	GENERATED_USTRUCT_BODY()
	
	/** All of the roads that intersect this node.  We have references to each of these roads, as well as the point along each
	    road where this node exists */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	TArray<FStreetMapRoadRef> RoadRefs;

	/** Returns this node's index */
	inline int32 GetNodeIndex( const UStreetMap& StreetMap ) const;

	/** Gets the location of this node */
	inline FVector2D GetLocation( const UStreetMap& StreetMap ) const;


	///
	/// Utility functions which may be useful for pathfinding algorithms (not used internally.)
	///

	/** Pathfinding: Given a node that is known to connect to this node via some road, searches for the road and returns it */
	inline const FStreetMapRoad& GetShortestCostRoadToNode( UStreetMap& StreetMap, const FStreetMapNode& OtherNode, const bool bIsTravelingForward, int32& OutPointIndexOnRoad ) const;

	/** Pathfinding: Returns true if this node is the end point on a road with no connections */
	inline bool IsDeadEnd( const UStreetMap& StreetMap ) const;

	/** Pathfinding: Returns the number of connections between this node and other roads, taking into account the direction of travel */
	inline int32 GetConnectionCount( const class UStreetMap& StreetMap, const bool bIsTravelingForward ) const;

	/** Pathfinding: Returns a connected node by index (between 0 and GetConnectionCount() - 1 ), taking into account the direction of travel.  Also returns the connecting road and wherabouts on the road the connection occurs */
	inline const FStreetMapNode* GetConnection( const class UStreetMap& StreetMap, const int32 ConnectionIndex, const bool bIsTravelingForward, const struct FStreetMapRoad** OutConnectingRoad = nullptr, int32* OutPointIndexOnRoad = nullptr, int32* OutConnectedNodePointIndexOnRoad = nullptr ) const;

	/** Pathfinding: Estimates the 'cost' of the specified connected by index (between 0 and GetConnectionCount() - 1) */
	inline float GetConnectionCost( const class UStreetMap& StreetMap, const int32 ConnectionIndex, const bool bIsTravelingForward ) const;
};


/** A building */
USTRUCT( BlueprintType )
struct STREETMAPRUNTIME_API FStreetMapBuilding
{
	GENERATED_USTRUCT_BODY()

	/** Name of the building */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FString BuildingName;

	/** Polygon points that define the perimeter of the building */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	TArray<FVector2D> BuildingPoints;

	/** Height of the building in meters (if known, otherwise zero) */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	float Height;

	/** Levels of the building (if known, otherwise zero) */
	UPROPERTY(Category = StreetMap, EditAnywhere)
	int BuildingLevels;

	// @todo: Performance: Bounding information could be computed at load time if we want to avoid the memory cost of storing it

	/** 2D bounds (min) of this building's points */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FVector2D BoundsMin;
	
	/** 2D bounds (max) of this building's points */
	UPROPERTY( Category=StreetMap, EditAnywhere )
	FVector2D BoundsMax;
};


/** A loaded street map */
UCLASS()
class STREETMAPRUNTIME_API UStreetMap : public UObject
{
	GENERATED_BODY()
	
public:

	/** Default constructor for UStreetMap */
	UStreetMap();

	// UObject overrides
	virtual void GetAssetRegistryTags( TArray<FAssetRegistryTag>& OutTags ) const override;
	
	/** Gets the roads in this street map (read only) */
	const TArray<FStreetMapRoad>& GetRoads() const
	{
		return Roads;
	}

	/** Gets the roads in this street map */
	TArray<FStreetMapRoad>& GetRoads()
	{
		return Roads;
	}
	
	/** Gets the nodes on the map (read only.)  Nodes describe intersections between roads */
	const TArray<FStreetMapNode>& GetNodes() const
	{
		return Nodes;
	}

	/** Gets the nodes on the map.  Nodes describe intersections between roads */
	TArray<FStreetMapNode>& GetNodes()
	{
		return Nodes;
	}
	
	/** Gets all of the buildings (read only) */
	const TArray<FStreetMapBuilding>& GetBuildings() const
	{
		return Buildings;
	}

	/** Gets all of the buildings */
	TArray<FStreetMapBuilding>& GetBuildings()
	{
		return Buildings;
	}

	/** Gets the bounding box of the map */
	FVector2D GetBoundsMin() const
	{
		return BoundsMin;
	}
	FVector2D GetBoundsMax() const
	{
		return BoundsMax;
	}


protected:
	
	/** List of roads */
	UPROPERTY( Category=StreetMap, VisibleAnywhere )
	TArray<FStreetMapRoad> Roads;
	
	/** List of nodes on this map.  Nodes describe interesting points along roads, usually where roads intersect or at the end of a dead-end street */
	UPROPERTY( Category=StreetMap, VisibleAnywhere )
	TArray<FStreetMapNode> Nodes;

	/** List of all buildings on the street map */
	UPROPERTY( Category=StreetMap, VisibleAnywhere)
	TArray<FStreetMapBuilding> Buildings;

	/** 2D bounds (min) of this map's roads and buildings */
	UPROPERTY( Category=StreetMap, VisibleAnywhere)
	FVector2D BoundsMin;
	
	/** 2D bounds (max) of this map's roads and buildings */
	UPROPERTY( Category=StreetMap, VisibleAnywhere)
	FVector2D BoundsMax;

#if WITH_EDITORONLY_DATA
	/** Importing data and options used for this mesh */
	UPROPERTY( VisibleAnywhere, Instanced, Category=ImportSettings )
	class UAssetImportData* AssetImportData;

	friend class UStreetMapFactory;
	friend class UStreetMapReimportFactory;
	friend class FStreetMapAssetTypeActions;
#endif	// WITH_EDITORONLY_DATA

};


inline int32 FStreetMapRoad::GetRoadIndex( const UStreetMap& StreetMap ) const
{
	// Pointer arithmetic based on array start
	const int32 RoadIndex = this - StreetMap.GetRoads().GetData();
	return RoadIndex;
}


inline const FStreetMapNode& FStreetMapRoad::GetNodeAtPointIndexOrEarlier( const UStreetMap& StreetMap, const int32 PointIndex, int32& OutNodeAtPointIndex ) const
{
	const FStreetMapNode* CurrentOrEarlierPointNode = nullptr;
	for( int32 NodePointIndex = PointIndex; NodePointIndex >= 0; --NodePointIndex )
	{
		if( NodeIndices[ NodePointIndex ] != INDEX_NONE )
		{
			CurrentOrEarlierPointNode = &StreetMap.GetNodes()[ NodeIndices[ NodePointIndex ] ];
			OutNodeAtPointIndex = NodePointIndex;
			break;
		}
	}
	return *CurrentOrEarlierPointNode;
}


inline const FStreetMapNode& FStreetMapRoad::GetNodeAtPointIndexOrLater( const UStreetMap& StreetMap, const int32 PointIndex, int32& OutNodeAtPointIndex ) const
{
	const FStreetMapNode* NextOrUpcomingNode = nullptr;
	for( int32 NodePointIndex = PointIndex; NodePointIndex < RoadPoints.Num(); ++NodePointIndex )
	{
		if( NodeIndices[ NodePointIndex ] != INDEX_NONE )
		{
			NextOrUpcomingNode = &StreetMap.GetNodes()[ NodeIndices[ NodePointIndex ] ];
			OutNodeAtPointIndex = NodePointIndex;
			break;
		}
	}

	return *NextOrUpcomingNode;
}


inline float FStreetMapRoad::ComputeLengthOfRoad( const class UStreetMap& StreetMap ) const
{
	// @todo: Performance: We could cache the road's total length at load time to avoid having to compute it,
	//        or we could save it right into the asset file

	return ComputeDistanceBetweenNodesOnRoad( StreetMap, 0, this->NodeIndices.Num() - 1 );
}


inline float FStreetMapRoad::ComputeDistanceBetweenNodesOnRoad( const class UStreetMap& StreetMap, const int32 NodePointIndexA, const int32 NodePointIndexB ) const
{
	float TotalDistanceSoFar = 0.0f;

	// NOTE: It is very important that we use the actual road point indices here and not nodes directly, because the same node can appear
	// more than once on a single road!

	// @todo: Performance: We can cache distances between connected nodes rather than computing them every time.  This
	//        can be computed at load time or at import time (and stored in the asset).  Most of the other functions
	//        in this class that perform Size() computations could be changed to use cached distances also!

	const int32 SmallerPointIndex = FMath::Max( 0, FMath::Min( NodePointIndexA, NodePointIndexB ) );
	const int32 LargerPointIndex = FMath::Min( RoadPoints.Num() - 1, FMath::Max( NodePointIndexA, NodePointIndexB ) );

	for( int32 PointIndex = SmallerPointIndex; PointIndex < LargerPointIndex; ++PointIndex )
	{
		const FVector2D PointLocation = RoadPoints[ PointIndex ];
		const FVector2D NextPointLocation = RoadPoints[ PointIndex + 1 ];

		const float DistanceBetweenPoints = ( NextPointLocation - PointLocation ).Size();
			
		TotalDistanceSoFar += DistanceBetweenPoints;
	}
	
	// @todo: Malformed data can cause this assertion to trigger.  This could be a single road with at least two adjacent nodes
	//        at the exact same location.  We need to filter this out at load time probably.
	// check( TotalDistanceSoFar > 0.0f );

	return TotalDistanceSoFar;
}


inline void FStreetMapRoad::FindEarlierAndLaterNodesForPositionAlongRoad( const class UStreetMap& StreetMap, const float PositionAlongRoad, const FStreetMapNode*& OutEarlierNode, float& OutEarlierNodePositionAlongRoad, const FStreetMapNode*& OutLaterNode, float& OutLaterNodePositionAlongRoad ) const
{
	float CurrentPointPositionAlongRoad = 0.0f;

	const FStreetMapNode* EarlierStreetMapNode = nullptr;
	const FStreetMapNode* LaterStreetMapNode = nullptr;

	const int32 NumPoints = RoadPoints.Num();
	for( int32 CurrentPointIndex = 0; CurrentPointIndex < NumPoints - 1; ++CurrentPointIndex )
	{
		if( this->NodeIndices[ CurrentPointIndex ] != INDEX_NONE )
		{
			EarlierStreetMapNode = &StreetMap.GetNodes()[ this->NodeIndices[ CurrentPointIndex ] ];
			OutEarlierNodePositionAlongRoad = CurrentPointPositionAlongRoad;
		}

		const int32 NextPointIndex = CurrentPointIndex + 1;
		const FVector2D CurrentPointLocation = RoadPoints[ CurrentPointIndex ];
		const FVector2D NextPointLocation = RoadPoints[ NextPointIndex ];

		const float DistanceBetweenPoints = ( NextPointLocation - CurrentPointLocation ).Size();
		const float NextPointPositionAlongRoad = CurrentPointPositionAlongRoad + DistanceBetweenPoints;

		if( NextPointPositionAlongRoad >= PositionAlongRoad )
		{
			if( NodeIndices[ NextPointIndex ] != INDEX_NONE )
			{
				LaterStreetMapNode = &StreetMap.GetNodes()[ this->NodeIndices[ NextPointIndex ] ];
				OutLaterNodePositionAlongRoad = NextPointPositionAlongRoad;
				break;
			}
		}
			
		CurrentPointPositionAlongRoad = NextPointPositionAlongRoad;
	}

	check( EarlierStreetMapNode != nullptr && LaterStreetMapNode != nullptr );
	OutEarlierNode = EarlierStreetMapNode;
	OutLaterNode = LaterStreetMapNode;
}


inline void FStreetMapRoad::FindEarlierAndLaterNodes( const class UStreetMap& StreetMap, const int32 RoadPointIndex, const FStreetMapNode*& OutEarlierNode, float& OutEarlierNodePositionAlongRoad, const FStreetMapNode*& OutLaterNode, float& OutLaterNodePositionAlongRoad ) const
{
	OutEarlierNode = nullptr;
	OutEarlierNodePositionAlongRoad = -1.0f;
	OutLaterNode = nullptr;
	OutLaterNodePositionAlongRoad = -1.0f;

	for( int32 EarlierPointIndex = RoadPointIndex - 1; EarlierPointIndex >= 0; --EarlierPointIndex )
	{
		if( this->NodeIndices[ EarlierPointIndex ] != INDEX_NONE )
		{
			OutEarlierNode = &StreetMap.GetNodes()[ this->NodeIndices[ EarlierPointIndex ] ];
			OutEarlierNodePositionAlongRoad = FindPositionAlongRoadForNode( StreetMap, EarlierPointIndex );
			break;
		}
	}

	for( int32 LaterPointIndex = RoadPointIndex + 1; LaterPointIndex < this->RoadPoints.Num(); ++LaterPointIndex )
	{
		if( this->NodeIndices[ LaterPointIndex ] != INDEX_NONE )
		{
			OutLaterNode = &StreetMap.GetNodes()[ this->NodeIndices[ LaterPointIndex ] ];
			OutLaterNodePositionAlongRoad = FindPositionAlongRoadForNode( StreetMap, LaterPointIndex );
			break;
		}
	}
}


inline float FStreetMapRoad::FindPositionAlongRoadForNode( const class UStreetMap& StreetMap, const int32 PointIndexForNode ) const
{
	float CurrentPointPositionAlongRoad = 0.0f;

	bool bFoundLocation = false;
	const int32 NumPoints = RoadPoints.Num();
	for( int32 CurrentPointIndex = 0; CurrentPointIndex < PointIndexForNode; ++CurrentPointIndex )
	{
		const FVector2D CurrentPointLocation = RoadPoints[ CurrentPointIndex ];
		const FVector2D NextPointLocation = RoadPoints[ CurrentPointIndex + 1 ];

		const float DistanceBetweenPoints = ( NextPointLocation - CurrentPointLocation ).Size();
		const float NextPointPositionAlongRoad = CurrentPointPositionAlongRoad + DistanceBetweenPoints;

		CurrentPointPositionAlongRoad = NextPointPositionAlongRoad;
	}

	return CurrentPointPositionAlongRoad;
}


inline FVector2D FStreetMapRoad::MakeLocationAlongRoad( const class UStreetMap& StreetMap, const float PositionAlongRoad ) const
{
	FVector2D LocationAlongRoad = FVector2D::ZeroVector;
	float CurrentPointPositionAlongRoad = 0.0f;

	bool bFoundLocation = false;
	const int32 NumPoints = RoadPoints.Num();
	for( int32 CurrentPointIndex = 0; CurrentPointIndex < NumPoints - 1; ++CurrentPointIndex )
	{
		const FVector2D CurrentPointLocation = RoadPoints[ CurrentPointIndex ];
		const FVector2D NextPointLocation = RoadPoints[ CurrentPointIndex + 1 ];

		const float DistanceBetweenPoints = ( NextPointLocation - CurrentPointLocation ).Size();
		const float NextPointPositionAlongRoad = CurrentPointPositionAlongRoad + DistanceBetweenPoints;

		if( NextPointPositionAlongRoad >= PositionAlongRoad )
		{
			const float LerpAlpha = ( PositionAlongRoad - CurrentPointPositionAlongRoad ) / DistanceBetweenPoints;
			LocationAlongRoad = FMath::Lerp( CurrentPointLocation, NextPointLocation, LerpAlpha );
			bFoundLocation = true;
			break;
		}
			
		CurrentPointPositionAlongRoad = NextPointPositionAlongRoad;
	}

	check( bFoundLocation == true );

	return LocationAlongRoad;
}


inline int32 FStreetMapNode::GetNodeIndex( const UStreetMap& StreetMap ) const
{
	// Pointer arithmetic based on array start
	const int32 NodeIndex = this - StreetMap.GetNodes().GetData();
	return NodeIndex;
}


inline bool FStreetMapNode::IsDeadEnd( const UStreetMap& StreetMap ) const
{
	if( RoadRefs.Num() == 1 )
	{
		// @todo: If this road only connects to dead end roads that oppose the direction, we need to treat this road
		//        as a dead end.  This case should be extremely uncommon, though!

		const FStreetMapRoadRef& SoleRoadRef = RoadRefs[ 0 ];
		const FStreetMapRoad& SoleRoad = StreetMap.GetRoads()[ SoleRoadRef.RoadIndex ];
		if( SoleRoadRef.RoadPointIndex == 0 || SoleRoadRef.RoadPointIndex == ( SoleRoad.NodeIndices.Num() - 1 ) )
		{
			// The node is attached to only one road, and the node is at the very end of one of the ends of the road
			return true;
		}
	}

	return false;
}


inline FVector2D FStreetMapNode::GetLocation( const UStreetMap& StreetMap ) const
{
	const FStreetMapRoadRef& MyFirstRoadRef = RoadRefs[ 0 ];
	const FVector2D Location = StreetMap.GetRoads()[ MyFirstRoadRef.RoadIndex ].RoadPoints[ MyFirstRoadRef.RoadPointIndex ];
	return Location;
}


inline const FStreetMapRoad& FStreetMapNode::GetShortestCostRoadToNode( UStreetMap& StreetMap, const FStreetMapNode& OtherNode, const bool bIsTravelingForward, int32& OutPointIndexOnRoad ) const
{
	const FStreetMapRoad* ConnectingRoad = nullptr;

	float BestConnectionCost = TNumericLimits<float>::Max();
	int32 BestConnectionIndex = INDEX_NONE;
	int32 BestConnectionPointIndex = INDEX_NONE;

	const int32 ConnectionCount = GetConnectionCount( StreetMap, bIsTravelingForward );
	for( int32 ConnectionIndex = 0; ConnectionIndex < ConnectionCount; ++ConnectionIndex )
	{
		int32 MyPointIndexOnRoad;
		int32 ConnectedNodePointIndexOnRoad;

		const FStreetMapRoad* CurrentConnectingRoad;
		const FStreetMapNode* ConnectedNode = GetConnection( StreetMap, ConnectionIndex, bIsTravelingForward, /* Out */ &CurrentConnectingRoad, /* Out */ &MyPointIndexOnRoad, /* Out */ &ConnectedNodePointIndexOnRoad );
		if( ConnectedNode == &OtherNode )
		{
			if( BestConnectionIndex != INDEX_NONE )
			{
				// We evaluate cost lazily, only in the unusual case of the two nodes being connected by multiple roads
				BestConnectionCost = GetConnectionCost( StreetMap, BestConnectionIndex, bIsTravelingForward );
				if( GetConnectionCost( StreetMap, ConnectionIndex, bIsTravelingForward ) < BestConnectionCost )
				{
					ConnectingRoad = CurrentConnectingRoad;
					BestConnectionIndex = ConnectionIndex;
					BestConnectionPointIndex = MyPointIndexOnRoad;
				}
			}
			else
			{
				ConnectingRoad = CurrentConnectingRoad;
				BestConnectionIndex = ConnectionIndex;
				BestConnectionPointIndex = MyPointIndexOnRoad;
			}
		}
	}

	check( ConnectingRoad != nullptr );
	OutPointIndexOnRoad = BestConnectionPointIndex;
	return *ConnectingRoad;
}

inline int32 FStreetMapNode::GetConnectionCount( const UStreetMap& StreetMap, const bool bIsTravelingForward ) const
{
	// NOTE: We're iterating here in the exact same order as in the GetConnection() function below!  That's critically important!
	int32 TotalConnections = 0;
	for( const FStreetMapRoadRef& RoadRef : RoadRefs )
	{
		const FStreetMapRoad& Road = StreetMap.GetRoads()[ RoadRef.RoadIndex ];
		
		if( RoadRef.RoadPointIndex > 0 && ( !bIsTravelingForward || !Road.IsOneWay() ) )
		{
			// We connect to a node earlier up this road
			++TotalConnections;
		}

		if( RoadRef.RoadPointIndex < ( Road.NodeIndices.Num() - 1 ) && ( bIsTravelingForward || !Road.IsOneWay() ) )
		{
			// We connect to a node further down this road
			++TotalConnections;
		}
	}

	return TotalConnections;
}


inline const FStreetMapNode* FStreetMapNode::GetConnection( const UStreetMap& StreetMap, const int32 ConnectionIndex, const bool bIsTravelingForward, const FStreetMapRoad** OutConnectingRoad, int32* OutPointIndexOnRoad, int32* OutConnectedNodePointIndexOnRoad ) const
{
	if( OutConnectingRoad != nullptr )
	{
		*OutConnectingRoad = nullptr;
	}
	const FStreetMapNode* ConnectedNode = nullptr;

	// @todo: Performance: We can improve performance by caching additional connectivity information right on
	//        the node itself.  This function would be a hot spot for any sort of pathfinding computation

	// NOTE: We're iterating here in the exact same order as in the GetConnectionCount() function above!  That's critically important!
	int32 CurrentConnectionIndex = 0;
	for( const FStreetMapRoadRef& RoadRef : RoadRefs )
	{
		const FStreetMapRoad& Road = StreetMap.GetRoads()[ RoadRef.RoadIndex ];
		
		// @todo: Performance: We could avoid the "while" loops below by not storing INDEX_NONEs in the NodeIndices array,
		//        but instead mapping them to points by going through the node itself, then back to a road

		if( RoadRef.RoadPointIndex > 0 && ( !bIsTravelingForward || !Road.IsOneWay() ) )
		{
			// We connect to an earlier node up this road
			if( CurrentConnectionIndex == ConnectionIndex )
			{
				int32 EarlierNodeRoadPointIndex = RoadRef.RoadPointIndex - 1;
				while( Road.NodeIndices[ EarlierNodeRoadPointIndex ] == INDEX_NONE )
				{
					--EarlierNodeRoadPointIndex;
				}
				const int32 EarlierNodeIndex = Road.NodeIndices[ EarlierNodeRoadPointIndex ];

				const FStreetMapNode& EarlierNode = StreetMap.GetNodes()[ EarlierNodeIndex ];
				ConnectedNode = &EarlierNode;
				if( OutConnectingRoad != nullptr )
				{
					*OutConnectingRoad = &Road;
				}
				if( OutPointIndexOnRoad != nullptr )
				{
					*OutPointIndexOnRoad = RoadRef.RoadPointIndex;
				}
				if( OutConnectedNodePointIndexOnRoad != nullptr )
				{
					*OutConnectedNodePointIndexOnRoad = EarlierNodeRoadPointIndex;
				}

				// Got it!
				break;
			}

			++CurrentConnectionIndex;
		}

		if( RoadRef.RoadPointIndex < ( Road.NodeIndices.Num() - 1 ) && ( bIsTravelingForward || !Road.IsOneWay() ) )
		{
			// We connect to node further down this road
			if( CurrentConnectionIndex == ConnectionIndex )
			{
				int32 LaterNodeRoadPointIndex = RoadRef.RoadPointIndex + 1;
				while( Road.NodeIndices[ LaterNodeRoadPointIndex ] == INDEX_NONE )
				{
					++LaterNodeRoadPointIndex;
				}
				const int32 LaterNodeIndex = Road.NodeIndices[ LaterNodeRoadPointIndex ];

				const FStreetMapNode& LaterNode = StreetMap.GetNodes()[ LaterNodeIndex ];
				ConnectedNode = &LaterNode;
				if( OutConnectingRoad != nullptr )
				{
					*OutConnectingRoad = &Road;
				}
				if( OutPointIndexOnRoad != nullptr )
				{
					*OutPointIndexOnRoad = RoadRef.RoadPointIndex;
				}
				if( OutConnectedNodePointIndexOnRoad != nullptr )
				{
					*OutConnectedNodePointIndexOnRoad = LaterNodeRoadPointIndex;
				}

				// Got it!
				break;
			}

			++CurrentConnectionIndex;
		}
	}

	check( ConnectedNode != nullptr );
	return ConnectedNode;
}


inline float FStreetMapNode::GetConnectionCost( const UStreetMap& StreetMap, const int32 ConnectionIndex, const bool bIsTravelingForward ) const
{
	/////////////////////////////////////////////////////////
	// Tweakables for connection cost estimation
	//
	const float MaxSpeedLimit = 120.0f;	// 120 Km/hr
	const float HighwaySpeed = 110.0f;
	const float HighwayTrafficFactor = 0.0;
	const float MajorRoadSpeed = 70.0f;
	const float MajorRoadTrafficFactor = 0.2f;
	const float StreetSpeed = 40.0f;
	const float StreetTrafficFactor = 1.0f;
	/////////////////////////////////////////////////////////

	// @todo: Street map pathfinding is a grand art in itself, and estimating cost of connections is
	//        a very complicated problem.  We're only doing some basic estimates for now, but in the
	//        future we could consider taking into account the cost of different types of turns and
	//        intersections, lane counts, actual speed limits, etc.

	int32 MyPointIndexOnRoad;
	int32 ConnectedNodePointIndexOnRoad;

	const FStreetMapRoad* ConnectingRoad = nullptr;
	const FStreetMapNode& ConnectedNode = *GetConnection( StreetMap, ConnectionIndex, bIsTravelingForward, /* Out */ &ConnectingRoad, /* Out */ &MyPointIndexOnRoad, /* Out */ &ConnectedNodePointIndexOnRoad );

	const float DistanceBetweenNodes = ConnectingRoad->ComputeDistanceBetweenNodesOnRoad( StreetMap, MyPointIndexOnRoad, ConnectedNodePointIndexOnRoad );
	
	float TotalCost = DistanceBetweenNodes;

	// Apply some scaling to the cost of traveling between these nodes
	{
		float SpeedLimit = 0.0f;
		float TrafficFactor = 0.0f;
		switch( ConnectingRoad->RoadType )
		{
			case EStreetMapRoadType::Highway:
				SpeedLimit = HighwaySpeed;
				TrafficFactor = HighwayTrafficFactor;
				break;

			case EStreetMapRoadType::MajorRoad:
				SpeedLimit = MajorRoadSpeed;
				TrafficFactor = MajorRoadTrafficFactor;
				break;

			case EStreetMapRoadType::Street:
			case EStreetMapRoadType::Other:
				SpeedLimit = StreetSpeed;
				TrafficFactor = StreetTrafficFactor;
				break;

			default:
				check( 0 );
				break;
		}

		const float RoadSpeedCostScale = ( 1.0f - ( SpeedLimit / MaxSpeedLimit ) );
		TotalCost *= 1.0f + RoadSpeedCostScale * 15.0f * ( 0.5f + TrafficFactor * 0.5f );
	}

	return TotalCost;
}


