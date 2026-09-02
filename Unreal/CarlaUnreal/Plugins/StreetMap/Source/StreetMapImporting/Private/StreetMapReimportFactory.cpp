// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapReimportFactory.h"
#include "StreetMapImporting.h"

#include "StreetMap.h"


UStreetMapReimportFactory::UStreetMapReimportFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UStreetMapReimportFactory::CanReimport( UObject* Obj, TArray<FString>& OutFilenames )
{
	UStreetMap* StreetMap = Cast<UStreetMap>( Obj );
	if( StreetMap != nullptr )
	{
		OutFilenames.Add( StreetMap->AssetImportData->GetFirstFilename() );
	}
	return true;
}


void UStreetMapReimportFactory::SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths )
{
	UStreetMap* StreetMap = CastChecked<UStreetMap>( Obj );
	StreetMap->Modify();
	StreetMap->AssetImportData->Update( NewReimportPaths[0] );
}


EReimportResult::Type UStreetMapReimportFactory::Reimport( UObject* Obj ) 
{ 
	UStreetMap* StreetMap = CastChecked<UStreetMap>( Obj );

	const FString Filename = StreetMap->AssetImportData->GetFirstFilename();
	const FString FileExtension = FPaths::GetExtension(Filename);

	// If there is no file path provided, can't reimport from source
	if ( !Filename.Len() )
	{
		return EReimportResult::Failed;
	}

	if( UFactory::StaticImportObject( StreetMap->GetClass(), StreetMap->GetOuter(), *StreetMap->GetName(), RF_Public|RF_Standalone, *Filename, nullptr, this ) )
	{
		// Mark the package dirty after the successful import
		StreetMap->MarkPackageDirty();
		return EReimportResult::Succeeded;
	}

	return EReimportResult::Failed;
}


int32 UStreetMapReimportFactory::GetPriority() const
{
	return 0;
}
