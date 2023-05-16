// Copyright 2017 Mike Fricker. All Rights Reserved.
#pragma once

#include "FastXml.h"


/** OpenStreetMap file loader */
class FOSMFile : public IFastXmlCallback
{
	
public:
	
	/** Default constructor for FOSMFile */
	FOSMFile();

	/** Destructor for FOSMFile */
	virtual ~FOSMFile();

	/** Loads the map from an OpenStreetMap XML file.  Note that in the case of the file path containing the XML data, the string must be mutable for us to parse it quickly. */
	bool LoadOpenStreetMapFile( FString& OSMFilePath, const bool bIsFilePathActuallyTextBuffer, class FFeedbackContext* FeedbackContext );


	struct FOSMWayInfo;
		
	/** Types of ways */
	enum class EOSMWayType
	{
		///
		/// ROADS
		///
		
		/** A restricted access major divided highway, normally with 2 or more running lanes plus emergency hard shoulder. Equivalent to the Freeway, Autobahn, etc. */
		Motorway,

		/** The link roads (sliproads/ramps) leading to/from a motorway from/to a motorway or lower class highway. Normally with the same motorway restrictions. */
		Motorway_Link,

		/** The most important roads in a country's system that aren't motorways. (Need not necessarily be a divided highway.) */
		Trunk,

		/** The link roads (sliproads/ramps) leading to/from a trunk road from/to a trunk road or lower class highway. */
		Trunk_Link,

		/** The next most important roads in a country's system. (Often link larger towns.) */
		Primary,

		/** The link roads (sliproads/ramps) leading to/from a primary road from/to a primary road or lower class highway. */
		Primary_Link,

		/** The next most important roads in a country's system. (Often link smaller towns and villages.) */
		Secondary,

		/** The link roads (sliproads/ramps) leading to/from a secondary road from/to a secondary road or lower class highway. */
		Secondary_Link,

		/** The next most important roads in a country's system. */
		Tertiary,

		/** The link roads (sliproads/ramps) leading to/from a tertiary road from/to a tertiary road or lower class highway. */
		Tertiary_Link,

		/** Roads which are primarily lined with and serve as an access to housing. */
		Residential,

		/** For access roads to, or within an industrial estate, camp site, business park, car park etc. */
		Service,

		/** The least most important through roads in a country's system, i.e. minor roads of a lower classification than tertiary, but which serve a purpose other than access to properties. */
		Unclassified,


		///
		/// NON-ROADS
		///
		
		/** Residential streets where pedestrians have legal priority over cars, speeds are kept very low and where children are allowed to play on the street. */
		Living_Street,

		/** For roads used mainly/exclusively for pedestrians in shopping and some residential areas which may allow access by motorised vehicles only for very limited periods of the day. */
		Pedestrian,

		/** Roads for agricultural or forestry uses etc, often rough with unpaved/unsealed surfaces, that can be used only by off-road vehicles (4WD, tractors, ATVs, etc.) */
		Track,

		/** A busway where the vehicle guided by the way (though not a railway) and is not suitable for other traffic. */
		Bus_Guideway,

		/** A course or track for (motor) racing */
		Raceway,

		/** A road where the mapper is unable to ascertain the classification from the information available. */
		Road,


		///
		/// PATHS
		///
		
		/** For designated footpaths; i.e., mainly/exclusively for pedestrians. This includes walking tracks and gravel paths. */
		Footway,

		/** For designated cycleways. */
		Cycleway,

		/** Paths normally used by horses */
		Bridleway,

		/** For flights of steps (stairs) on footways. */
		Steps,

		/** A non-specific path. */
		Path,


		///
		/// LIFECYCLE
		///
		
		/** For planned roads, use with proposed=* and also proposed=* with a value of the proposed highway value. */
		Proposed,

		/** For roads under construction. */
		Construction,


		///
		/// BUILDINGS
		///

		/** Default type of building.  A general catch-all. */
		Building,


		///
		/// UNSUPPORTED
		/// 
		
		/** Currently unrecognized type */
		Other,
	};


	struct FOSMWayRef
	{
		// Way that we're referencing at this node
		FOSMWayInfo* Way;
			
		// Index of the node in the way's array of nodes
		int32 NodeIndex;
	};
		
		
	struct FOSMNodeInfo
	{
		double Latitude;
		double Longitude;
		TArray<FOSMWayRef> WayRefs;
	};
		
		
	struct FOSMWayInfo
	{
		FString Name;
		FString Ref;
		TArray<FOSMNodeInfo*> Nodes;
		EOSMWayType WayType;
		double Height;
		int32 BuildingLevels;

		// If true, way is only traversable in the order the nodes are listed in the Nodes list
		uint8 bIsOneWay : 1;
	};

	// Minimum latitude/longitude bounds
	double MinLatitude = MAX_dbl;
	double MinLongitude = MAX_dbl;
	double MaxLatitude = -MAX_dbl;
	double MaxLongitude = -MAX_dbl;

	// Average Latitude (roughly the center of the map)
	double AverageLatitude = 0.0;
	double AverageLongitude = 0.0;
		
	// All ways we've parsed
	TArray<FOSMWayInfo*> Ways;
		
	// Maps node IDs to info about each node
	TMap<int64, FOSMNodeInfo*> NodeMap;

protected:

	// IFastXmlCallback overrides
	virtual bool ProcessXmlDeclaration( const TCHAR* ElementData, int32 XmlFileLineNumber ) override;
	virtual bool ProcessComment( const TCHAR* Comment ) override;
	virtual bool ProcessElement( const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber ) override;
	virtual bool ProcessAttribute( const TCHAR* AttributeName, const TCHAR* AttributeValue ) override;
	virtual bool ProcessClose( const TCHAR* Element ) override;

	
protected:
	
	enum class ParsingState
	{
		Root,
		Node,
		Way,
		Way_NodeRef,
		Way_Tag
	};
		
	// Current state of parser
	ParsingState ParsingState;
		
	// ID of node that is currently being parsed
	int64 CurrentNodeID;
		
	// Node that is currently being parsed
	FOSMNodeInfo* CurrentNodeInfo;
		
	// Way that is currently being parsed
	FOSMWayInfo* CurrentWayInfo;
		
	// Current way's tag key string
	const TCHAR* CurrentWayTagKey;
};


