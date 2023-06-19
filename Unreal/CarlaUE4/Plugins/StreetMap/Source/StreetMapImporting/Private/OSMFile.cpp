// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "OSMFile.h"
#include "StreetMapImporting.h"



FOSMFile::FOSMFile()
	: ParsingState( ParsingState::Root )
{
}
		

FOSMFile::~FOSMFile()
{
	// Clean up time
	{
		for( auto* Way : Ways )
		{
			delete Way;
		}
		Ways.Empty();
				
		for( auto HashPair : NodeMap )
		{
			FOSMNodeInfo* NodeInfo = HashPair.Value;
			delete NodeInfo;
		}
		NodeMap.Empty();
	}
}


bool FOSMFile::LoadOpenStreetMapFile( FString& OSMFilePath, const bool bIsFilePathActuallyTextBuffer, FFeedbackContext* FeedbackContext )
{
	const bool bShowSlowTaskDialog = true;
	const bool bShowCancelButton = true;

	FText ErrorMessage;
	int32 ErrorLineNumber;
	if( FFastXml::ParseXmlFile( 
		this, 
		bIsFilePathActuallyTextBuffer ? nullptr : *OSMFilePath, 
		bIsFilePathActuallyTextBuffer ? OSMFilePath.GetCharArray().GetData() : nullptr, 
		FeedbackContext, 
		bShowSlowTaskDialog, 
		bShowCancelButton, 
		/* Out */ ErrorMessage, 
		/* Out */ ErrorLineNumber ) )
	{
		if( NodeMap.Num() > 0 )
		{
			AverageLatitude /= NodeMap.Num();
			AverageLongitude /= NodeMap.Num();
		}

		return true;
	}

	if( FeedbackContext != nullptr )
	{
		FeedbackContext->Logf(
			ELogVerbosity::Error,
			TEXT( "Failed to load OpenStreetMap XML file ('%s', Line %i)" ),
			*ErrorMessage.ToString(),
			ErrorLineNumber );
	}

	return false;
}

		
bool FOSMFile::ProcessXmlDeclaration( const TCHAR* ElementData, int32 XmlFileLineNumber )
{
	// Don't care about XML declaration
	return true;
}


bool FOSMFile::ProcessComment( const TCHAR* Comment )
{
	// Don't care about comments
	return true;
}
	
	
bool FOSMFile::ProcessElement( const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber )
{
	if( ParsingState == ParsingState::Root )
	{
		if( !FCString::Stricmp( ElementName, TEXT( "node" ) ) )
		{
			ParsingState = ParsingState::Node;
			CurrentNodeInfo = new FOSMNodeInfo();
			CurrentNodeInfo->Latitude = 0.0;
			CurrentNodeInfo->Longitude = 0.0;
		}
		else if (!FCString::Stricmp(ElementName, TEXT("way")))
		{
			ParsingState = ParsingState::Way;
			CurrentWayInfo = new FOSMWayInfo();
			CurrentWayInfo->Name.Empty();
			CurrentWayInfo->Ref.Empty();
			CurrentWayInfo->WayType = EOSMWayType::Other;
			CurrentWayInfo->Height = 0.0;
			CurrentWayInfo->bIsOneWay = false;

			// @todo: We're currently ignoring the "visible" tag on ways, which means that roads will always
			//        be included in our data set.  It might be nice to make this an import option.
		}
	}
	else if (ParsingState == ParsingState::Way)
	{
		if( !FCString::Stricmp( ElementName, TEXT( "nd" ) ) )
		{
			ParsingState = ParsingState::Way_NodeRef;
		}
		else if( !FCString::Stricmp( ElementName, TEXT( "tag" ) ) )
		{
			ParsingState = ParsingState::Way_Tag;
		}
	}

	return true;
}


bool FOSMFile::ProcessAttribute( const TCHAR* AttributeName, const TCHAR* AttributeValue )
{
	if( ParsingState == ParsingState::Node )
	{
		if( !FCString::Stricmp( AttributeName, TEXT( "id" ) ) )
		{
			CurrentNodeID = FPlatformString::Atoi64( AttributeValue );
		}
		else if( !FCString::Stricmp( AttributeName, TEXT( "lat" ) ) )
		{
			CurrentNodeInfo->Latitude = FPlatformString::Atod( AttributeValue );

			AverageLatitude += CurrentNodeInfo->Latitude;
					
			// Update minimum and maximum latitude
			// @todo: Performance: Instead of computing our own bounding box, we could parse the "minlat" and
			//        "minlon" tags from the OSM file
			if( CurrentNodeInfo->Latitude < MinLatitude )
			{
				MinLatitude = CurrentNodeInfo->Latitude;
			}
			if( CurrentNodeInfo->Latitude > MaxLatitude )
			{
				MaxLatitude = CurrentNodeInfo->Latitude;
			}
		}
		else if( !FCString::Stricmp( AttributeName, TEXT( "lon" ) ) )
		{
			CurrentNodeInfo->Longitude = FPlatformString::Atod( AttributeValue );

			AverageLongitude += CurrentNodeInfo->Longitude;
					
			// Update minimum and maximum longitude
			if( CurrentNodeInfo->Longitude < MinLongitude )
			{
				MinLongitude = CurrentNodeInfo->Longitude;
			}
			if( CurrentNodeInfo->Longitude > MaxLongitude )
			{
				MaxLongitude = CurrentNodeInfo->Longitude;
			}
		}
	}
	else if( ParsingState == ParsingState::Way )
	{
		// ...
	}
	else if( ParsingState == ParsingState::Way_NodeRef )
	{
		if( !FCString::Stricmp( AttributeName, TEXT( "ref" ) ) )
		{
			FOSMNodeInfo* ReferencedNode = NodeMap.FindRef( FPlatformString::Atoi64( AttributeValue ) );
			const int NewNodeIndex = CurrentWayInfo->Nodes.Num();
			CurrentWayInfo->Nodes.Add( ReferencedNode );
					
			// Update the node with information about the way that is referencing it
			/* {
				FOSMWayRef NewWayRef;
				NewWayRef.Way = CurrentWayInfo;
				NewWayRef.NodeIndex = NewNodeIndex;
				ReferencedNode->WayRefs.Add( NewWayRef );
			}*/
		}
	}
	else if (ParsingState == ParsingState::Way_Tag)
	{
		if( !FCString::Stricmp( AttributeName, TEXT( "k" ) ) )
		{
			CurrentWayTagKey = AttributeValue;
		}
		else if( !FCString::Stricmp( AttributeName, TEXT( "v" ) ) )
		{
			if( !FCString::Stricmp( CurrentWayTagKey, TEXT( "name" ) ) )
			{
				CurrentWayInfo->Name = AttributeValue;
			}
			else if( !FCString::Stricmp( CurrentWayTagKey, TEXT( "ref" ) ) )
			{
				CurrentWayInfo->Ref = AttributeValue;
			}
			/*else if (!FCString::Stricmp(CurrentWayTagKey, TEXT("highway")))
			{
				EOSMWayType WayType = EOSMWayType::Other;
						
				if( !FCString::Stricmp( AttributeValue, TEXT( "motorway" ) ) )
				{
					WayType = EOSMWayType::Motorway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "motorway_link" ) ) )
				{
					WayType = EOSMWayType::Motorway_Link;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "trunk" ) ) )
				{
					WayType = EOSMWayType::Trunk;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "trunk_link" ) ) )
				{
					WayType = EOSMWayType::Trunk_Link;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "primary" ) ) )
				{
					WayType = EOSMWayType::Primary;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "primary_link" ) ) )
				{
					WayType = EOSMWayType::Primary_Link;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "secondary" ) ) )
				{
					WayType = EOSMWayType::Secondary;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "secondary_link" ) ) )
				{
					WayType = EOSMWayType::Secondary_Link;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "tertiary" ) ) )
				{
					WayType = EOSMWayType::Tertiary;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "tertiary_link" ) ) )
				{
					WayType = EOSMWayType::Tertiary_Link;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "residential" ) ) )
				{
					WayType = EOSMWayType::Residential;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "service" ) ) )
				{
					WayType = EOSMWayType::Service;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "unclassified" ) ) )
				{
					WayType = EOSMWayType::Unclassified;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "living_street" ) ) )
				{
					WayType = EOSMWayType::Living_Street;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "pedestrian" ) ) )
				{
					WayType = EOSMWayType::Pedestrian;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "track" ) ) )
				{
					WayType = EOSMWayType::Track;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "bus_guideway" ) ) )
				{
					WayType = EOSMWayType::Bus_Guideway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "raceway" ) ) )
				{
					WayType = EOSMWayType::Raceway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "road" ) ) )
				{
					WayType = EOSMWayType::Road;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "footway" ) ) )
				{
					WayType = EOSMWayType::Footway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "cycleway" ) ) )
				{
					WayType = EOSMWayType::Cycleway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "bridleway" ) ) )
				{
					WayType = EOSMWayType::Bridleway;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "steps" ) ) )
				{
					WayType = EOSMWayType::Steps;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "path" ) ) )
				{
					WayType = EOSMWayType::Path;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "proposed" ) ) )
				{
					WayType = EOSMWayType::Proposed;
				}
				else if( !FCString::Stricmp( AttributeValue, TEXT( "construction" ) ) )
				{
					WayType = EOSMWayType::Construction;
				}
				else
				{
					// Other type that we don't recognize yet.  See http://wiki.openstreetmap.org/wiki/Key:highway
				}
						
						
				CurrentWayInfo->WayType = WayType;
			}*/
			else if( !FCString::Stricmp( CurrentWayTagKey, TEXT( "building" ) ) )
			{
				CurrentWayInfo->WayType = EOSMWayType::Building;
				CurrentWayInfo->Category = AttributeValue;
				if( !FCString::Stricmp( AttributeValue, TEXT( "yes" ) ) )
				{
					CurrentWayInfo->WayType = EOSMWayType::Building;
				}
				else
				{
					// Other type that we don't recognize yet.  See http://wiki.openstreetmap.org/wiki/Key:building
				}
			}
			else if( !FCString::Stricmp( CurrentWayTagKey, TEXT( "height" ) ) )
			{
				// Check to see if there is a space character in the height value.  For now, we're looking
				// for straight-up floating point values.
				if( !FString( AttributeValue ).Contains( TEXT( " " ) ) )
				{
					// Okay, no space character.  So this has got to be a floating point number.  The OSM
					// spec says that the height values are in meters.
					CurrentWayInfo->Height = FPlatformString::Atod( AttributeValue );
				}
				else
				{
					// Looks like the height value contains units of some sort.
					// @todo: Add support for interpreting unit strings and converting the values
				}
			}
			else if (!FCString::Stricmp(CurrentWayTagKey, TEXT("building:levels")))
			{
				CurrentWayInfo->BuildingLevels = FPlatformString::Atoi(AttributeValue);
			}
			else if( !FCString::Stricmp( CurrentWayTagKey, TEXT( "oneway" ) ) )
			{
				if( !FCString::Stricmp( AttributeValue, TEXT( "yes" ) ) )
				{
					CurrentWayInfo->bIsOneWay = true;
				}
				else
				{
					CurrentWayInfo->bIsOneWay = false;
				}
			}
		}
	}

	return true;
}


bool FOSMFile::ProcessClose( const TCHAR* Element )
{
	if( ParsingState == ParsingState::Node )
	{
		NodeMap.Add( CurrentNodeID, CurrentNodeInfo );
		CurrentNodeID = 0;
		CurrentNodeInfo = nullptr;
				
		ParsingState = ParsingState::Root;
	}
	else if( ParsingState == ParsingState::Way )
	{
		Ways.Add( CurrentWayInfo );
		CurrentWayInfo = nullptr;
				
		ParsingState = ParsingState::Root;
	}
	else if( ParsingState == ParsingState::Way_NodeRef )
	{
		ParsingState = ParsingState::Way;
	}
	else if( ParsingState == ParsingState::Way_Tag )
	{
		CurrentWayTagKey = TEXT( "" );
		ParsingState = ParsingState::Way;
	}

	return true;
}
