// Copyright 2017 Mike Fricker. All Rights Reserved.
#pragma once

#include "Factories/Factory.h"
#include "StreetMapFactory.generated.h"


/**
 * Import factory object for OpenStreetMap assets
 */
UCLASS()
class UStreetMapFactory : public UFactory
{
	GENERATED_BODY()

public:

	/** UStreetMapFactory constructor */
	UStreetMapFactory( const class FObjectInitializer& ObjectInitializer );

protected:

	// UFactory overrides
	virtual UObject* FactoryCreateText( UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn ) override;

	/** Loads the street map from an OpenStreetMap XML file.  Note that in the case of the file path containing the XML data, the string must be mutable for us to parse it quickly. */
	bool LoadFromOpenStreetMapXMLFile( class UStreetMap* StreetMap, FString& OSMFilePath, const bool bIsFilePathActuallyTextBuffer, class FFeedbackContext* FeedbackContext );

	/** Static: Latitude/longitude scale factor */
	static const double LatitudeLongitudeScale;
};

