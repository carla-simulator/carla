#include "TrafficLights/TLMeshFactory.h"

#include "CarlaTools.h"
#include "Components/StaticMeshComponent.h"
#include "CoreGlobals.h"
#include "Logging/LogVerbosity.h"
#include "Math/MathFwd.h"
#include "Misc/AssertionMacros.h"
#include "TrafficLights/TLBackplateDataTable.h"
#include "TrafficLights/TLLightTypeDataTable.h"
#include "TrafficLights/TLModuleDataTable.h"
#include "TrafficLights/TLPoleDataTable.h"
#include "TrafficLights/TLPoleType.h"
#include "TrafficLights/TLToolContent.h"
#include "Misc/Paths.h"
#include "UObject/NameTypes.h"

UDataTable* FTLMeshFactory::s_ModuleMeshTable{nullptr};
UDataTable* FTLMeshFactory::s_PoleMeshTable{nullptr};
UDataTable* FTLMeshFactory::s_LightTypeMeshTable{nullptr};
UDataTable* FTLMeshFactory::s_BackplateMeshTable{nullptr};

FString TLToolContent::PresetsDir()
{
	return FPaths::ProjectContentDir() / TEXT("Carla/Static/TrafficLight/TrafficLightTool/Presets");
}

FString FTLMeshFactory::ResolveMeshName(const FString& Name)
{
	// Mesh names written by the tool while its assets lived in the CarlaDigitalTwinsTool
	// plugin, so older JSON files keep loading.
	static const TMap<FString, FString> LegacyNames{
		{TEXT("SM_TrafficLight_Pole01_Base"), TEXT("SM_TLTool_Pole01_NA_Base")},
		{TEXT("SM_TrafficLight_Pole01_01"), TEXT("SM_TLTool_Pole01_NA_Vertical")},
		{TEXT("SM_TrafficLight_Pole01_02"), TEXT("SM_TLTool_Pole01_NA_Horizontal")},
		{TEXT("SM_TrafficLight_Pole01_Connector"), TEXT("SM_TLTool_Pole01_NA_Connector")},
		{TEXT("SM_TrafficLight_Pole01_Black_Base1"), TEXT("SM_TLTool_Pole01_EU_Base")},
		{TEXT("SM_TrafficLight_Pole01_Black_01"), TEXT("SM_TLTool_Pole01_EU_Vertical")},
		{TEXT("SM_TrafficLight_Pole01_Black_02"), TEXT("SM_TLTool_Pole01_EU_Horizontal")},
		{TEXT("SM_TrafficLight_Pole01_Black_Connector1"), TEXT("SM_TLTool_Pole01_EU_Connector")},
		{TEXT("SM_TrafficLight_Pole02_Base"), TEXT("SM_TLTool_Pole02_NA_Base")},
		{TEXT("SM_TrafficLight_Pole02_01"), TEXT("SM_TLTool_Pole02_NA_Vertical")},
		{TEXT("SM_TrafficLight_Pole02_02"), TEXT("SM_TLTool_Pole02_NA_Horizontal")},
		{TEXT("SM_TrafficLight_Pole02_Cap"), TEXT("SM_TLTool_Pole02_NA_Cap")},
		{TEXT("SM_TrafficLight_Pole02_Connector1"), TEXT("SM_TLTool_Pole02_NA_Connector")},
		{TEXT("SM_TrafficLight_Pole02_EndCap1"), TEXT("SM_TLTool_Pole02_NA_EndCap")},
		{TEXT("SM_TrafficLight_Pole02_Black_Base"), TEXT("SM_TLTool_Pole02_EU_Base")},
		{TEXT("SM_TrafficLight_Black_Pole02_01"), TEXT("SM_TLTool_Pole02_EU_Vertical")},
		{TEXT("SM_TrafficLight_Black_Pole02_02"), TEXT("SM_TLTool_Pole02_EU_Horizontal")},
		{TEXT("SM_TrafficLight_Black_Pole02_Cap"), TEXT("SM_TLTool_Pole02_EU_Cap")},
		{TEXT("SM_TrafficLight_Pole02_Black_Connector"), TEXT("SM_TLTool_Pole02_EU_Connector")},
		{TEXT("SM_TrafficLight_Pole02_Black_EndCap"), TEXT("SM_TLTool_Pole02_EU_EndCap")},
		{TEXT("SM_TrafficLight_Pole03_Base"), TEXT("SM_TLTool_Pole03_NA_Base")},
		{TEXT("SM_TrafficLight_Pole03_Base1"), TEXT("SM_TLTool_Pole03_NA_BaseAlt")},
		{TEXT("SM_TrafficLight_Pole03"), TEXT("SM_TLTool_Pole03_NA_Vertical")},
		{TEXT("SM_TrafficLight_Pole03_Cap"), TEXT("SM_TLTool_Pole03_NA_Cap")},
		{TEXT("SM_TrafficLight_Pole03_Cap1"), TEXT("SM_TLTool_Pole03_NA_CapAlt")},
		{TEXT("SM_TrafficLight_Black_Pole01"), TEXT("SM_TLTool_Pole03_EU_Vertical")},
		{TEXT("SM_TrafficLight_Base"), TEXT("SM_TLTool_Pole04_NA_Base")},
		{TEXT("SM_TrafficLight_Pole04"), TEXT("SM_TLTool_Pole04_NA_Vertical")},
		{TEXT("SM_TrafficLight_Pole04_Cap"), TEXT("SM_TLTool_Pole04_NA_Cap")},
		{TEXT("SM_TrafficLight_Black_Base"), TEXT("SM_TLTool_Pole04_EU_Base")},
		{TEXT("SM_TrafficLight_Black_Pole04"), TEXT("SM_TLTool_Pole04_EU_Vertical")},
		{TEXT("SM_TrafficLight_Black_Pole04_Cap"), TEXT("SM_TLTool_Pole04_EU_Cap")},
		{TEXT("SM_TrafficLights_Module_01"), TEXT("SM_TLTool_Module_NA_Visor")},
		{TEXT("SM_TrafficLights_Module_02"), TEXT("SM_TLTool_Module_NA")},
		{TEXT("SM_TrafficLights_Horizontal_Module_01"), TEXT("SM_TLTool_Module_NA_Horizontal_Visor")},
		{TEXT("SM_TrafficLights_Horizontal_Module_02"), TEXT("SM_TLTool_Module_NA_Horizontal")},
		{TEXT("SM_TrafficLights_Black_Module_01"), TEXT("SM_TLTool_Module_EU_Visor")},
		{TEXT("SM_TrafficLights_Black_Module_02"), TEXT("SM_TLTool_Module_EU")},
		{TEXT("SM_TrafficLights_Horizontal_Black_Module_01"), TEXT("SM_TLTool_Module_EU_Horizontal_Visor")},
		{TEXT("SM_TrafficLights_Horizontal_Black_Module_02"), TEXT("SM_TLTool_Module_EU_Horizontal")},
		{TEXT("SM_TrafficLights_Crosswalk_01"), TEXT("SM_TLTool_Crosswalk_NA_01")},
		{TEXT("SM_TrafficLights_Crosswalk_02"), TEXT("SM_TLTool_Crosswalk_NA_02")},
		{TEXT("SM_TrafficLights_Crosswalk_03"), TEXT("SM_TLTool_Crosswalk_NA_03")},
		{TEXT("SM_TrafficLights_Crosswalk_04"), TEXT("SM_TLTool_Crosswalk_NA_04")},
		{TEXT("SM_TrafficLights_Black_Crosswalk_03"), TEXT("SM_TLTool_Crosswalk_EU_03")},
		{TEXT("SM_TrafficLights_Black_Crosswalk_04"), TEXT("SM_TLTool_Crosswalk_EU_04")},
		{TEXT("SM_BackPlate_01"), TEXT("SM_TLTool_Backplate_NA_Vertical")},
		{TEXT("SM_BackPlate_02"), TEXT("SM_TLTool_Backplate_NA_Horizontal")},
		{TEXT("SM_BackPlate_Corner01"), TEXT("SM_TLTool_Backplate_NA_Corner")},
		{TEXT("SM_BackPlate_Middle"), TEXT("SM_TLTool_Backplate_NA_Middle")},
		{TEXT("SM_BackPlate_Black_01"), TEXT("SM_TLTool_Backplate_EU_Vertical")},
		{TEXT("SM_BackPlate_Black_02"), TEXT("SM_TLTool_Backplate_EU_Horizontal")},
		{TEXT("SM_BackPlate_Black_Corner01"), TEXT("SM_TLTool_Backplate_EU_Corner")},
		{TEXT("SM_BackPlate_Black_Middle"), TEXT("SM_TLTool_Backplate_EU_Middle")},
	};
	const FString* const Renamed{LegacyNames.Find(Name)};
	return Renamed ? *Renamed : Name;
}

UDataTable* FTLMeshFactory::GetLightTypeMeshTable()
{
	if (!s_LightTypeMeshTable)
	{
		const FString Path{TLToolContent::DataTable(TEXT("DT_TLTool_LightTypes"))};
		UObject* Loaded = StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path);
		s_LightTypeMeshTable = Cast<UDataTable>(Loaded);
	}
	check(s_LightTypeMeshTable);
	return s_LightTypeMeshTable;
}

UDataTable* FTLMeshFactory::GetModuleMeshTable()
{
	if (!s_ModuleMeshTable)
	{
		const FString Path{TLToolContent::DataTable(TEXT("DT_TLTool_Modules"))};
		UObject* Loaded = StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path);
		s_ModuleMeshTable = Cast<UDataTable>(Loaded);
	}
	check(s_ModuleMeshTable);
	return s_ModuleMeshTable;
}

UDataTable* FTLMeshFactory::GetPoleMeshTable()
{
	if (!s_PoleMeshTable)
	{
		const FString Path{TLToolContent::DataTable(TEXT("DT_TLTool_Poles"))};
		UObject* Loaded = StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path);
		s_PoleMeshTable = Cast<UDataTable>(Loaded);
	}
	check(s_PoleMeshTable);
	return s_PoleMeshTable;
}

UDataTable* FTLMeshFactory::GetBackplateMeshTable()
{
	if (!s_BackplateMeshTable)
	{
		const FString Path{TLToolContent::DataTable(TEXT("DT_TLTool_Backplates"))};
		UObject* Loaded = StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path);
		s_BackplateMeshTable = Cast<UDataTable>(Loaded);
	}
	check(s_BackplateMeshTable);
	return s_BackplateMeshTable;
}

UStaticMesh* FTLMeshFactory::GetBaseMeshByName(const FString& InName)
{
	const FString Name{ResolveMeshName(InName)};
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return nullptr;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetBaseMeshByName"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}
		if (Row->PoleType == ETLPoleType::Base && IsValid(Row->Mesh) && Row->Mesh->GetName() == Name)
		{
			return Row->Mesh;
		}
	}
	return nullptr;
}

UStaticMesh* FTLMeshFactory::GetExtensibleMeshByName(const FString& InName)
{
	const FString Name{ResolveMeshName(InName)};
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return nullptr;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetExtensibleMeshByName"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}
		if (Row->PoleType == ETLPoleType::Extensible && IsValid(Row->Mesh) && Row->Mesh->GetName() == Name)
		{
			return Row->Mesh;
		}
	}
	return nullptr;
}

UStaticMesh* FTLMeshFactory::GetCapMeshByName(const FString& InName)
{
	const FString Name{ResolveMeshName(InName)};
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return nullptr;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetCapMeshByName"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}
		if (Row->PoleType == ETLPoleType::Cap && IsValid(Row->Mesh) && Row->Mesh->GetName() == Name)
		{
			return Row->Mesh;
		}
	}
	return nullptr;
}

UStaticMesh* FTLMeshFactory::GetMeshForModule(const FTLHead& Head, const FTLModule& Module)
{
	UDataTable* ModuleMeshTable{GetModuleMeshTable()};
	if (!ModuleMeshTable)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("ModuleMeshFactory: ModuleMeshTable is null"));
		return nullptr;
	}

	for (const FName& RowName : ModuleMeshTable->GetRowNames())
	{
		const FTLModuleRow* Row{ModuleMeshTable->FindRow<FTLModuleRow>(RowName, TEXT("GetMeshForModule"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("ModuleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}
		if (Row->Style == Head.Style && Row->Orientation == Head.Orientation && Row->bHasVisor == Module.bHasVisor &&
			IsValid(Row->Mesh))
		{
			return Row->Mesh;
		}
	}

	return nullptr;
}

TArray<UStaticMesh*> FTLMeshFactory::GetAllMeshesForModule(const FTLHead& Head, const FTLModule& Module)
{
	TArray<UStaticMesh*> Meshes;
	UDataTable* ModuleMeshTable{GetModuleMeshTable()};
	if (!ModuleMeshTable)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("ModuleMeshFactory: ModuleMeshTable is null"));
		return Meshes;
	}

	for (const FName& RowName : ModuleMeshTable->GetRowNames())
	{
		const FTLModuleRow* Row{ModuleMeshTable->FindRow<FTLModuleRow>(RowName, TEXT("GetMeshForModule"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("ModuleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}
		if (Row->Style == Head.Style && Row->Orientation == Head.Orientation && Row->bHasVisor == Module.bHasVisor &&
			IsValid(Row->Mesh))
		{
			Meshes.Add(Row->Mesh);
		}
	}

	return Meshes;
}

TArray<UStaticMesh*> FTLMeshFactory::GetAllBaseMeshesForPole(const FTLPole& Pole)
{
	TArray<UStaticMesh*> Meshes;
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return Meshes;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetAllBaseMeshesForPole"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}

		if (Row->Style == Pole.Style && Row->Orientation == Pole.Orientation && Row->PoleType == ETLPoleType::Base &&
			IsValid(Row->Mesh))
		{
			Meshes.Add(Row->Mesh);
		}
	}
	return Meshes;
}

TArray<UStaticMesh*> FTLMeshFactory::GetAllExtensibleMeshesForPole(const FTLPole& Pole)
{
	TArray<UStaticMesh*> Meshes;
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return Meshes;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetAllExtensibleMeshesForPole"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}

		if (Row->Style == Pole.Style && Row->Orientation == Pole.Orientation && Row->PoleType == ETLPoleType::Extensible &&
			IsValid(Row->Mesh))
		{
			Meshes.Add(Row->Mesh);
		}
	}
	return Meshes;
}

TArray<UStaticMesh*> FTLMeshFactory::GetAllCapMeshesForPole(const FTLPole& Pole)
{
	TArray<UStaticMesh*> Meshes;
	UDataTable* Table{GetPoleMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: PoleMeshTable is null"));
		return Meshes;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		const FTLPoleRow* Row{Table->FindRow<FTLPoleRow>(RowName, TEXT("GetAllCapMeshesForPole"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}

		if (Row->Style == Pole.Style && Row->Orientation == Pole.Orientation && Row->PoleType == ETLPoleType::Cap &&
			IsValid(Row->Mesh))
		{
			Meshes.Add(Row->Mesh);
		}
	}
	return Meshes;
}

FTLBackplateRow* FTLMeshFactory::GetBackplateRow(ETLStyle Style)
{
	UDataTable* Table{GetBackplateMeshTable()};
	if (!Table)
	{
		UE_LOG(LogCarlaTools, Error, TEXT("PoleMeshFactory: BackplateMeshTable is null"));
		return nullptr;
	}

	for (const FName& RowName : Table->GetRowNames())
	{
		FTLBackplateRow* Row{Table->FindRow<FTLBackplateRow>(RowName, TEXT("GetBackplateRow"))};
		if (!Row)
		{
			UE_LOG(LogCarlaTools, Warning, TEXT("PoleMeshFactory: row '%s' not found"), *RowName.ToString());
			continue;
		}

		if (Row->Style == Style)
		{
			return Row;
		}
	}
	return nullptr;
}

UStaticMesh* FTLMeshFactory::GetBackplateCornerMesh(const FTLHead& Head)
{
	return GetBackplateRow(Head.Style) ? GetBackplateRow(Head.Style)->CornerMesh : nullptr;
}

UStaticMesh* FTLMeshFactory::GetBackplateHorizontalMesh(const FTLHead& Head)
{
	return GetBackplateRow(Head.Style) ? GetBackplateRow(Head.Style)->HorizontalMesh : nullptr;
}

UStaticMesh* FTLMeshFactory::GetBackplateVerticalMesh(const FTLHead& Head)
{
	return GetBackplateRow(Head.Style) ? GetBackplateRow(Head.Style)->VerticalMesh : nullptr;
}

UStaticMesh* FTLMeshFactory::GetBackplateMiddleMesh(const FTLHead& Head)
{
	return GetBackplateRow(Head.Style) ? GetBackplateRow(Head.Style)->MiddleMesh : nullptr;
}

int32 FTLMeshFactory::CountLedMaterials(UStaticMesh* Mesh)
{
	if (!IsValid(Mesh))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("CountLedSockets: Invalid Mesh"));
		return 0;
	}

	int32 Count{0};
	const TArray<FStaticMaterial>& StaticMaterials{Mesh->GetStaticMaterials()};
	for (const FStaticMaterial& Material : StaticMaterials)
	{
		if (Material.MaterialSlotName.ToString().StartsWith(TEXT("led_")))
		{
			++Count;
		}
	}
	return Count;
}

FIntPoint FTLMeshFactory::GetAtlasCoordsForLightType(ETLLightType LightType)
{
	if (s_LightTypeMeshTable == nullptr)
	{
		GetLightTypeMeshTable();
		if (s_LightTypeMeshTable == nullptr)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("LightTypesTable is not set"));
			return FIntPoint::ZeroValue;
		}
	}
	const UEnum* EnumPtr{StaticEnum<ETLLightType>()};
	if (!EnumPtr)
	{
		return FIntPoint::ZeroValue;
	}

	const FString EnumName{EnumPtr->GetNameStringByValue(static_cast<int64>(LightType))};
	const FName RowName(*EnumName);
	if (const FTLLightTypeRow* Row = s_LightTypeMeshTable->FindRow<FTLLightTypeRow>(RowName, TEXT("GetAtlasCoordsForLightType")))
	{
		return Row->AtlasCoords;
	}
	return FIntPoint::ZeroValue;
}
