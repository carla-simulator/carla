// Copyright 2017 Mike Fricker. All Rights Reserved.

#include "StreetMapAssetTypeActions.h"
#include "StreetMapImporting.h"

#include "StreetMap.h"
#include "AssetData.h"


#define LOCTEXT_NAMESPACE "StreetMapImporting"


FText FStreetMapAssetTypeActions::GetName() const
{
	return LOCTEXT("StreetMapAssetTypeActionsName", "Street Map");
}


FColor FStreetMapAssetTypeActions::GetTypeColor() const
{
	return FColor(50, 255, 120);
}


UClass* FStreetMapAssetTypeActions::GetSupportedClass() const
{
	return UStreetMap::StaticClass();
}


void FStreetMapAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	FSimpleAssetEditor::CreateEditor( EToolkitMode::Standalone, EditWithinLevelEditor, InObjects );
}


uint32 FStreetMapAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}


FText FStreetMapAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return FText::GetEmpty();
}


bool FStreetMapAssetTypeActions::IsImportedAsset() const
{
	return true;
}


void FStreetMapAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto* Asset : TypeAssets)
	{
		const auto* StreetMap = CastChecked<UStreetMap>(Asset);
		if( !StreetMap->AssetImportData->GetFirstFilename().IsEmpty() )
		{
			OutSourceFilePaths.Add( StreetMap->AssetImportData->GetFirstFilename() );
		}
	}
}

#undef LOCTEXT_NAMESPACE
