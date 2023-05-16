// Copyright 2017 Mike Fricker. All Rights Reserved.
#pragma once

#include "StreetMapFactory.h"
#include "EditorReimportHandler.h"
#include "StreetMapReimportFactory.generated.h"


/**
 * Import factory object for OpenStreetMap assets
 */
UCLASS()
class UStreetMapReimportFactory : public UStreetMapFactory, public FReimportHandler
{
	GENERATED_BODY()

public:

	/** UStreetMapReimportFactory constructor */
	UStreetMapReimportFactory( const class FObjectInitializer& ObjectInitializer );

protected:

	// FReimportHandler overrides
	virtual bool CanReimport( UObject* Obj, TArray<FString>& OutFilenames ) override;
	virtual void SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths ) override;
	virtual EReimportResult::Type Reimport( UObject* Obj ) override;
	virtual int32 GetPriority() const override;

};

