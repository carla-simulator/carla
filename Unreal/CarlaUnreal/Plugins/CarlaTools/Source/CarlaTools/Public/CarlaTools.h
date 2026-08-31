// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "EngineMinimal.h"
#include "UObject/ObjectSaveContext.h"
#include <util/ue-header-guard-end.h>


DECLARE_LOG_CATEGORY_EXTERN(LogCarlaTools, Log, All);

class FCarlaToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/// Editor save guard: a component about to be saved must not reference a
	/// MaterialInstanceDynamic that lives in another package. Such a MID is
	/// never written anywhere (nothing inside its own package references it),
	/// so the saved package would carry a dangling import and fail to cook
	/// ("Failed import for MaterialInstanceDynamic").
	///
	/// The classic offender is an editor utility widget calling
	/// KismetMaterialLibrary::CreateDynamicMaterialInstance with the editor
	/// world as world-context: KismetMaterialLibrary.cpp then outers the MID to
	/// the UWorld, and a World Partition external actor referencing it ends up
	/// importing "<Map>.<Map>:MaterialInstanceDynamic_N", an object the map
	/// package never saves.
	///
	/// Rather than dropping the MID (which loses every per-instance parameter
	/// the tool just set), adopt it: create a copy outered to the component
	/// that references it -- so it becomes an export of the package being
	/// saved -- carrying the same parent and the same scalar/vector/texture/
	/// font parameter overrides, and repoint the slot at that copy. Only if
	/// the copy cannot be made do we fall back to the MID's parent material.
	static void AdoptForeignMaterialInstanceDynamics(UPackage* Package, FObjectPreSaveContext SaveContext);

	FDelegateHandle PreSavePackageHandle;
};
