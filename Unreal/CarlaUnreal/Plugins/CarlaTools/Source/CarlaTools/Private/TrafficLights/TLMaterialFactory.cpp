#include "TrafficLights/TLMaterialFactory.h"

#include "CarlaTools.h"
#include "UObject/Object.h"

UMaterialInterface* FMaterialFactory::GetBaseLightMaterial()
{
	static UMaterialInterface* BaseMat{Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
		TEXT("/CarlaDigitalTwinsTool/Carla/Static/TrafficLight/TrafficLights2025/TrafficLights/"
			 "M_TrafficLights_Inst.M_TrafficLights_Inst")))};
	if (!IsValid(BaseMat))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("MaterialFactory: failed to load base material at '%s'"),
			TEXT("/CarlaDigitalTwinsTool/Carla/Static/TrafficLight/TrafficLights2025/TrafficLights/"
				 "M_TrafficLights_Inst.M_TrafficLights_Inst"));
		return nullptr;
	}

	return BaseMat;
}

UMaterialInstanceDynamic* FMaterialFactory::CreateLightMaterialInstanceDynamic(UStaticMeshComponent* Comp, const FName& SlotName)
{
	if (!IsValid(Comp))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("CreateLightMaterialInstanceDynamic: Invalid StaticMeshComponent"));
		return nullptr;
	}
	const int32 MatIndex{Comp->GetMaterialIndex(SlotName)};
	if (MatIndex == INDEX_NONE)
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("CreateLightMID: slot '%s' not found on component '%s'"),
			*SlotName.ToString(), *Comp->GetName());
		return nullptr;
	}

	UMaterialInterface* BaseMat{GetBaseLightMaterial()};
	if (!IsValid(BaseMat))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("CreateLightMID: base material invalid"));
		return nullptr;
	}

	UMaterialInstanceDynamic* MID{Comp->CreateDynamicMaterialInstance(MatIndex, BaseMat)};
	if (!IsValid(MID))
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("CreateLightMID: failed to create MID for slot '%s'"), *SlotName.ToString());
		return nullptr;
	}
	return MID;
}
