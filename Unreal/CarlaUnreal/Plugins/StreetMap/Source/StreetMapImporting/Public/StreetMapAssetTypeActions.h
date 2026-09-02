// Copyright 2017 Mike Fricker. All Rights Reserved.
#pragma once

#include "AssetTypeActions_Base.h"

class FStreetMapAssetTypeActions : public FAssetTypeActions_Base
{
public:
	
	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual bool IsImportedAsset() const override;
	virtual void GetResolvedSourceFilePaths( const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths ) const override;
};
