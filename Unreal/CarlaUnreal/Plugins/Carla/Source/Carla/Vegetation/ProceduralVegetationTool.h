#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGSettings.h"
#include "ProceduralVegetationTool.generated.h"

class UBoxComponent;
class UPCGComponent;
class UPCGGraphInterface;
class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FCarlaVegetationPoint
{
  GENERATED_BODY()
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Id;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Provenance;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FTransform Transform;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TSoftObjectPtr<UStaticMesh> Mesh;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Seed = 0;
};

/** Editor preview and persistent bake for validated TwinModel vegetation plans. */
UCLASS(BlueprintType)
class CARLA_API AProceduralVegetationTool : public AActor
{
  GENERATED_BODY()
public:
  AProceduralVegetationTool();

  UPROPERTY(VisibleAnywhere, Category="Vegetation") TObjectPtr<UBoxComponent> GenerationBounds;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vegetation") FFilePath PlanFile;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vegetation") TSoftObjectPtr<UPCGGraphInterface> Graph;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vegetation") TObjectPtr<UPCGComponent> PCG;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vegetation") FString Status;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vegetation") TArray<FCarlaVegetationPoint> Points;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vegetation") int32 BakedInstanceCount = 0;

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Vegetation") bool ImportPlan();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Vegetation") void Preview();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Vegetation") bool Bake();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Vegetation") void ClearGenerated();
  UFUNCTION(BlueprintPure, Category="Vegetation") int32 PreviewInstanceCount() const;
  UFUNCTION(BlueprintPure, Category="Vegetation") bool IsGenerating() const;
  UFUNCTION(BlueprintCallable, Category="Vegetation") void TickCommandletGeneration();

private:
  UFUNCTION() void OnGenerated(UPCGComponent* Component);
  UPROPERTY(VisibleAnywhere, Instanced, Category="Vegetation") TArray<TObjectPtr<UInstancedStaticMeshComponent>> BakedComponents;
  UPROPERTY() bool bPreviewReady = false;
};

/** Emits validated plan points with Mesh, SourceId, Provenance and stable seeds. */
UCLASS(BlueprintType, ClassGroup=(Procedural))
class CARLA_API UPCGVegetationPlanSettings : public UPCGSettings
{
  GENERATED_BODY()
public:
#if WITH_EDITOR
  virtual FName GetDefaultNodeName() const override { return TEXT("CarlaVegetationPlan"); }
  virtual FText GetDefaultNodeTitle() const override { return FText::FromString(TEXT("CARLA Vegetation Plan")); }
  virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::InputOutput; }
#endif
protected:
  virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
  virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return DefaultPointOutputPinProperties(); }
  virtual FPCGElementPtr CreateElement() const override;
};
