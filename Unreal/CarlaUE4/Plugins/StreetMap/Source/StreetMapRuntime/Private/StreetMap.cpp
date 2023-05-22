// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMap.h"
#include "StreetMapRuntime.h"
#include "EditorFramework/AssetImportData.h"


UStreetMap::UStreetMap()
{
#if WITH_EDITORONLY_DATA
	if( !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		AssetImportData = NewObject<UAssetImportData>( this, TEXT( "AssetImportData" ) );
	}
#endif
}


void UStreetMap::GetAssetRegistryTags( TArray<FAssetRegistryTag>& OutTags ) const
{
#if WITH_EDITORONLY_DATA
	if( AssetImportData )
	{
		OutTags.Add( FAssetRegistryTag( SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden ) );
	}
#endif

	Super::GetAssetRegistryTags( OutTags );
}
