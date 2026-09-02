// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "TrafficLights/TLHead.h"
#include "TrafficLights/TLLightType.h"
#include "TrafficLights/TLModule.h"
#include "TrafficLights/TLPole.h"
#include "UObject/ObjectMacros.h"
#include "Math/NumericLimits.h"

#include "TrafficLightActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CARLATOOLS_API ATrafficLightActor : public AActor
{
	GENERATED_BODY()

public:
	ATrafficLightActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	void Build();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "TrafficLight")
	void Bake(const FString& MapName, const FString& DesiredLabel);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "TrafficLight|JSON")
	void BuildFromJSON();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "TrafficLight|JSON")
	FString ExportToJSON(bool bUseTransform = true) const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "TrafficLight|Logic")
	FString ExportLogicToJSON() const;

	// Internal version that uses custom actor name for baking
	FString ExportLogicToJSON(const FString& ActorName) const;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "TrafficLight|Demo")
	void PlayDemoSequence();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "TrafficLight|Demo")
	void StopDemoSequence();

	/// Build from JSON text rather than from JSONFile. Exposed so a placer script can
	/// substitute the per-head SignalID of a preset before building it.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "TrafficLight|JSON")
	void BuildFromJSONString(const FString& JSONConfig);

	UPROPERTY(EditAnywhere, Category = "TrafficLight|JSON")
	FFilePath JSONFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic")
    int32 JunctionID {MIN_int32};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic")
    FString TrafficLightGroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic",
        meta = (ToolTip = "Unique Signal ID from OpenDRIVE file"))
    FString SignalID;

	UPROPERTY(EditAnywhere, Category = "TrafficLight")
	TArray<FTLPole> Poles;

	void PopulateDefault();

	// Traffic light timing values for this traffic light
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic",
		meta = (ToolTip = "Red phase duration in seconds"))
	float RedDuration{2.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic",
		meta = (ToolTip = "Green phase duration in seconds"))
	float GreenDuration{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic",
		meta = (ToolTip = "Amber phase duration in seconds"))
	float AmberDuration{3.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Light|Logic",
		meta = (ToolTip = "Amber blink interval in seconds"))
	float AmberBlinkInterval{0.25f};

private:
	USceneComponent* AddRootPole(USceneComponent* Parent, FTLPole& Pole);
	UStaticMeshComponent* AddPoleBase(USceneComponent* Parent, FTLPole& Pole);
	UStaticMeshComponent* AddPoleExtensible(USceneComponent* Parent, FTLPole& Pole);
	UStaticMeshComponent* AddPoleCap(USceneComponent* Parent, FTLPole& Pole);
	USceneComponent* AddHead(USceneComponent* Parent, FTLPole& Pole, FTLHead& Head);
	UStaticMeshComponent* AddModule(USceneComponent* Parent, FTLPole& Pole, FTLHead& Head, FTLModule& ModuleData);
	void AddBackplate(USceneComponent* Parent, FTLPole& Pole, FTLHead& Head);
	FVector2D GetAtlasCoordsForLightType(ETLLightType LightType) const;
	void RebuildModuleChain(FTLHead& Head);
	void Clear();
	USceneComponent* EnsurePolesRoot();

private:
	enum class EDemoPhase : uint8
	{
		Red,
		Green,
		Amber,
	};
	bool bDemoPlaying{false};
	FTimerHandle PhaseTimerHandle;
	FTimerHandle AmberBlinkTimerHandle;

	EDemoPhase CurrentPhase{EDemoPhase::Red};
	bool bAmberVisible{false};

	void AdvanceDemoPhase();
	void ToggleAmberBlink();
	void EndAmberPhase();
	void ResetAllLights();

	// Helper functions to get timing from first head
	float GetRedDuration() const;
	float GetGreenDuration() const;
	float GetAmberDuration() const;
	float GetAmberBlinkInterval() const;

private:
	TArray<UStaticMeshComponent*> ModuleMeshComponents;
	TArray<FTLModuleLight*> DemoLights;
};
