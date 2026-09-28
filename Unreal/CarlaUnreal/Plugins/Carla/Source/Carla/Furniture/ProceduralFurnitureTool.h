#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGSettings.h"
#include "ProceduralFurnitureTool.generated.h"

class UBoxComponent;
class UPCGComponent;
class UPCGGraphInterface;
class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FCarlaFurniturePoint
{
  GENERATED_BODY()
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Id;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Provenance;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FTransform Transform;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TSoftObjectPtr<UStaticMesh> Mesh;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Seed = 0;
};

/** Editor preview and persistent bake for validated TwinModel furniture plans. */
UCLASS(BlueprintType)
class CARLA_API AProceduralFurnitureTool : public AActor
{
  GENERATED_BODY()
public:
  AProceduralFurnitureTool();

  UPROPERTY(VisibleAnywhere, Category="Furniture") TObjectPtr<UBoxComponent> GenerationBounds;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Furniture") FFilePath PlanFile;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Furniture") TSoftObjectPtr<UPCGGraphInterface> Graph;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Furniture") TObjectPtr<UPCGComponent> PCG;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Furniture") FString Status;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Furniture") TArray<FCarlaFurniturePoint> Points;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Furniture") int32 BakedInstanceCount = 0;

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Furniture") bool ImportPlan();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Furniture") void Preview();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Furniture") bool Bake();
  UFUNCTION(BlueprintCallable, CallInEditor, Category="Furniture") void ClearGenerated();
  UFUNCTION(BlueprintPure, Category="Furniture") int32 PreviewInstanceCount() const;
  UFUNCTION(BlueprintPure, Category="Furniture") bool IsGenerating() const;
  UFUNCTION(BlueprintCallable, Category="Furniture") void TickCommandletGeneration();

private:
  UFUNCTION() void OnGenerated(UPCGComponent* Component);
  UPROPERTY(VisibleAnywhere, Instanced, Category="Furniture") TArray<TObjectPtr<UInstancedStaticMeshComponent>> BakedComponents;
  UPROPERTY() bool bPreviewReady = false;
};

/** Emits validated plan points with Mesh, SourceId, Provenance and stable seeds. */
UCLASS(BlueprintType, ClassGroup=(Procedural))
class CARLA_API UPCGFurniturePlanSettings : public UPCGSettings
{
  GENERATED_BODY()
public:
#if WITH_EDITOR
  virtual FName GetDefaultNodeName() const override { return TEXT("CarlaFurniturePlan"); }
  virtual FText GetDefaultNodeTitle() const override { return FText::FromString(TEXT("CARLA Furniture Plan")); }
  virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::InputOutput; }
#endif
protected:
  virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
  virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return DefaultPointOutputPinProperties(); }
  virtual FPCGElementPtr CreateElement() const override;
};
