// Copyright Epic Games, Inc. All Rights Reserved.

#include "CarlaTools.h"
#include "VehicleImporter.h"

#include <util/ue-header-guard-begin.h>
#include "Components/DecalComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include <util/ue-header-guard-end.h>

#define LOCTEXT_NAMESPACE "FCarlaToolsModule"

DEFINE_LOG_CATEGORY(LogCarlaTools);

void FCarlaToolsModule::StartupModule()
{
	// Module startup only ensures the class is available, doesn't auto-start server
	PreSavePackageHandle = UPackage::PreSavePackageWithContextEvent.AddStatic(
		&FCarlaToolsModule::AdoptForeignMaterialInstanceDynamics);
}

void FCarlaToolsModule::ShutdownModule()
{
	UPackage::PreSavePackageWithContextEvent.Remove(PreSavePackageHandle);
	PreSavePackageHandle.Reset();
}

namespace
{
	/// A MID is only saveable next to the object that references it when both live
	/// in the same package. Transient MIDs (runtime ones) are dropped by the save as
	/// null and are not a problem; MIDs in the transient package neither. Everything
	/// else is "foreign": it would be written as an import that can never resolve.
	bool IsForeignMID(const UMaterialInterface* Material, const UPackage* Package)
	{
		const UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Material);
		return MID != nullptr
			&& MID->GetPackage() != Package
			&& !MID->HasAnyFlags(RF_Transient)
			&& MID->GetPackage() != GetTransientPackage();
	}

	/// Copy every parameter override of Source onto Target, leaving the parameters
	/// Target already carries in place unless Source overrides the same one. Mirrors
	/// UMaterialInstanceDynamic::CopyParameterOverrides (MaterialInstanceDynamic.cpp)
	/// but merges instead of clearing, so a MID-of-MID chain can be flattened by
	/// applying the chain from the root down to the leaf.
	void MergeParameterOverrides(UMaterialInstanceDynamic* Target, const UMaterialInstance* Source)
	{
		for (const FScalarParameterValue& Value : Source->ScalarParameterValues)
		{
			Target->SetScalarParameterValueByInfo(Value.ParameterInfo, Value.ParameterValue);
		}
		for (const FVectorParameterValue& Value : Source->VectorParameterValues)
		{
			Target->SetVectorParameterValueByInfo(Value.ParameterInfo, Value.ParameterValue);
		}
		for (const FDoubleVectorParameterValue& Value : Source->DoubleVectorParameterValues)
		{
			Target->SetDoubleVectorParameterValue(Value.ParameterInfo.Name, Value.ParameterValue);
		}
		for (const FTextureParameterValue& Value : Source->TextureParameterValues)
		{
			Target->SetTextureParameterValueByInfo(Value.ParameterInfo, Value.ParameterValue);
		}
		for (const FRuntimeVirtualTextureParameterValue& Value : Source->RuntimeVirtualTextureParameterValues)
		{
			Target->SetRuntimeVirtualTextureParameterValueByInfo(Value.ParameterInfo, Value.ParameterValue);
		}
		for (const FFontParameterValue& Value : Source->FontParameterValues)
		{
			Target->SetFontParameterValue(Value.ParameterInfo, Value.FontValue, Value.FontPage);
		}
	}

	/// Number of parameter overrides carried by a material instance, for the log line.
	int32 CountOverrides(const UMaterialInstance* Instance)
	{
		return Instance->ScalarParameterValues.Num() + Instance->VectorParameterValues.Num()
			+ Instance->DoubleVectorParameterValues.Num() + Instance->TextureParameterValues.Num()
			+ Instance->RuntimeVirtualTextureParameterValues.Num() + Instance->FontParameterValues.Num();
	}

	/// Adopt a foreign MID into Package by creating an equivalent MID outered to Owner
	/// (the component holding the slot), so the save writes it as an export of Package.
	/// Returns null when there is nothing to parent the copy to; the caller then falls
	/// back to the parent material.
	UMaterialInstanceDynamic* AdoptMID(UMaterialInstanceDynamic* MID, UObject* Owner, const UPackage* Package)
	{
		// Walk up through any foreign MID parents (MID of MID): those would be dangling
		// imports too, so the copy is parented to the first material that can be saved,
		// and their overrides are flattened into the copy below.
		TArray<UMaterialInstance*, TInlineAllocator<4>> Chain;
		Chain.Add(MID);
		UMaterialInterface* Parent = MID->Parent;
		while (IsForeignMID(Parent, Package))
		{
			UMaterialInstanceDynamic* ParentMID = CastChecked<UMaterialInstanceDynamic>(Parent);
			Chain.Add(ParentMID);
			Parent = ParentMID->Parent;
		}
		if (Parent == nullptr)
		{
			return nullptr;
		}

		const FName Name = MakeUniqueObjectName(Owner, UMaterialInstanceDynamic::StaticClass(), MID->GetFName());
		UMaterialInstanceDynamic* Copy = UMaterialInstanceDynamic::Create(Parent, Owner, Name);
		if (Copy == nullptr)
		{
			return nullptr;
		}
		// Created with NewObject's default flags, but be explicit: a transient copy
		// would be skipped by the save and we would be back to an empty slot.
		Copy->ClearFlags(RF_Transient);
		// Root-most first so the leaf MID's values win on collision.
		for (int32 Index = Chain.Num() - 1; Index >= 0; --Index)
		{
			MergeParameterOverrides(Copy, Chain[Index]);
		}
		return Copy;
	}

	/// The material to fall back to when adoption fails: the first ancestor that is not
	/// itself a foreign MID (may be null, in which case the slot is emptied).
	UMaterialInterface* SafeParentOf(const UMaterialInstanceDynamic* MID, const UPackage* Package)
	{
		UMaterialInterface* Parent = MID->Parent;
		while (IsForeignMID(Parent, Package))
		{
			Parent = CastChecked<UMaterialInstanceDynamic>(Parent)->Parent;
		}
		return Parent;
	}

	void ReportAdopted(const UActorComponent* Component, const FString& Slot,
		const UMaterialInstanceDynamic* Old, const UMaterialInstanceDynamic* New)
	{
		const AActor* Owner = Component->GetOwner();
		UE_LOG(LogCarlaTools, Warning,
			TEXT("Save guard: %s%s%s %s referenced MaterialInstanceDynamic %s from package %s, "
			     "which would not survive a save (dangling import on cook); adopted it as %s "
			     "(parent %s, %d parameter override(s))."),
			Owner ? *Owner->GetActorLabel() : TEXT(""), Owner ? TEXT(".") : TEXT(""),
			*Component->GetName(), *Slot, *Old->GetPathName(), *Old->GetPackage()->GetName(),
			*New->GetPathName(), New->Parent ? *New->Parent->GetPathName() : TEXT("None"),
			CountOverrides(New));
	}

	void ReportFallback(const UActorComponent* Component, const FString& Slot,
		const UMaterialInstanceDynamic* Old, const UMaterialInterface* New)
	{
		const AActor* Owner = Component->GetOwner();
		UE_LOG(LogCarlaTools, Warning,
			TEXT("Save guard: %s%s%s %s referenced MaterialInstanceDynamic %s from package %s, "
			     "which would not survive a save (dangling import on cook); could not adopt it, "
			     "replaced with %s (per-instance parameters lost)."),
			Owner ? *Owner->GetActorLabel() : TEXT(""), Owner ? TEXT(".") : TEXT(""),
			*Component->GetName(), *Slot, *Old->GetPathName(), *Old->GetPackage()->GetName(),
			New ? *New->GetPathName() : TEXT("None"));
	}

	/// One slot that has to be repointed. Collected first, mutated afterwards: adoption
	/// creates new objects inside Package, which must not happen while
	/// ForEachObjectWithPackage walks the object hash of that same package.
	struct FPendingSlot
	{
		UActorComponent* Component = nullptr;
		UMaterialInstanceDynamic* Foreign = nullptr;
		int32 MaterialIndex = INDEX_NONE;   // INDEX_NONE: the decal component's DecalMaterial
	};
}

void FCarlaToolsModule::AdoptForeignMaterialInstanceDynamics(UPackage* Package, FObjectPreSaveContext SaveContext)
{
	// Cook saves are read-only snapshots of already loaded content; only guard
	// interactive/scripted editor saves (level, external actors, blueprints).
	if (SaveContext.IsCooking() || SaveContext.IsProceduralSave() || Package == nullptr)
	{
		return;
	}

	TArray<FPendingSlot> Pending;
	ForEachObjectWithPackage(Package, [Package, &Pending](UObject* Object)
	{
		if (UDecalComponent* Decal = Cast<UDecalComponent>(Object))
		{
			UMaterialInterface* Material = Decal->GetDecalMaterial();
			if (IsForeignMID(Material, Package))
			{
				Pending.Add({Decal, CastChecked<UMaterialInstanceDynamic>(Material), INDEX_NONE});
			}
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(Object))
		{
			const int32 Num = Mesh->GetNumMaterials();
			for (int32 Index = 0; Index < Num; ++Index)
			{
				UMaterialInterface* Material = Mesh->GetMaterial(Index);
				if (IsForeignMID(Material, Package))
				{
					Pending.Add({Mesh, CastChecked<UMaterialInstanceDynamic>(Material), Index});
				}
			}
		}
		return true;
	}, EGetObjectsFlags::IncludeNestedObjects, RF_ClassDefaultObject | RF_ArchetypeObject);

	for (const FPendingSlot& Slot : Pending)
	{
		const FString SlotName = Slot.MaterialIndex == INDEX_NONE
			? FString(TEXT("DecalMaterial"))
			: FString::Printf(TEXT("Material[%d]"), Slot.MaterialIndex);

		UMaterialInstanceDynamic* Adopted = AdoptMID(Slot.Foreign, Slot.Component, Package);
		UMaterialInterface* Replacement = Adopted;
		if (Adopted != nullptr)
		{
			ReportAdopted(Slot.Component, SlotName, Slot.Foreign, Adopted);
		}
		else
		{
			// No saveable parent to copy from: better an empty (or parent-material)
			// slot than a package that fails to load.
			Replacement = SafeParentOf(Slot.Foreign, Package);
			ReportFallback(Slot.Component, SlotName, Slot.Foreign, Replacement);
		}

		Slot.Component->Modify();
		if (Slot.MaterialIndex == INDEX_NONE)
		{
			CastChecked<UDecalComponent>(Slot.Component)->SetDecalMaterial(Replacement);
		}
		else
		{
			CastChecked<UMeshComponent>(Slot.Component)->SetMaterial(Slot.MaterialIndex, Replacement);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCarlaToolsModule, CarlaTools)

#ifdef LIBCARLA_NO_EXCEPTIONS
#include <util/disable-ue4-macros.h>
#include <carla/Exception.h>
#include <util/enable-ue4-macros.h>

#include <exception>

namespace carla {

  void throw_exception(const std::exception &e) {
    UE_LOG(LogCarlaTools, Fatal, TEXT("Exception thrown: %s"), UTF8_TO_TCHAR(e.what()));
    // It should never reach this part.
    std::terminate();
  }

} // namespace carla 
#endif
