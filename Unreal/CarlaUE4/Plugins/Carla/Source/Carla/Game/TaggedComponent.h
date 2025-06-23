#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/InstancedStaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "SplineMeshSceneProxy.h"
#include "Landscape.h"
#include "LandscapeRender.h"
#include "LandscapeMaterialInstanceConstant.h"

#include "TaggedComponent.generated.h"

UCLASS( meta=(BlueprintSpawnableComponent) )
class CARLA_API UTaggedComponent : public UPrimitiveComponent
{
  GENERATED_BODY()

public:
  UTaggedComponent(const FObjectInitializer& ObjectInitializer);

  virtual FPrimitiveSceneProxy * CreateSceneProxy() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;
  virtual void OnRegister() override;
  virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const ;

  // FIXME: Should take an int32 and convert to FVector4
  void SetColor(FLinearColor color);
  FLinearColor GetColor();

  TArray<UMaterialInstanceDynamic*> GetTaggedMaterials();

private:
  FLinearColor Color;

  UPROPERTY()
  UMaterialInstanceDynamic * TaggedMID;

  UPROPERTY()
  TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials;

  bool bSkeletalMesh = false;

  FPrimitiveSceneProxy * CreateSceneProxy(UStaticMeshComponent * StaticMeshComponent);
  FPrimitiveSceneProxy * CreateSceneProxy(USkeletalMeshComponent * SkeletalMeshComponent);
  FPrimitiveSceneProxy * CreateSceneProxy(UHierarchicalInstancedStaticMeshComponent * MeshComponent);
  FPrimitiveSceneProxy * CreateSceneProxy(UInstancedStaticMeshComponent * MeshComponent);

  // small hack to allow unreal to initialize the base component in skeletal meshes
  bool bShouldWaitFrame = true;
  int NumFramesToWait = 2;
};

class FTaggedStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:

  FTaggedStaticMeshSceneProxy(UStaticMeshComponent * Component, bool bForceLODsShareStaticLighting, UMaterialInstance * MaterialInstance, TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  UMaterialInstance * TaggedMaterialInstance;
};

class FTaggedSplineMeshSceneProxy : public FSplineMeshSceneProxy
{
public:

  FTaggedSplineMeshSceneProxy(USplineMeshComponent * Component, UMaterialInstance * MaterialInstance, TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  UMaterialInstance * TaggedMaterialInstance;
};

class FTaggedSkeletalMeshSceneProxy : public FSkeletalMeshSceneProxy
{
public:
  FTaggedSkeletalMeshSceneProxy(const USkinnedMeshComponent * Component, FSkeletalMeshRenderData * InSkeletalMeshRenderData, UMaterialInstance * MaterialInstance, TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  UMaterialInstance * TaggedMaterialInstance;
};

class FTaggedInstancedStaticMeshSceneProxy : public FInstancedStaticMeshSceneProxy
{
public:
  FTaggedInstancedStaticMeshSceneProxy(UInstancedStaticMeshComponent * Component, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance, TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  UMaterialInstance * TaggedMaterialInstance;
};


class FTaggedHierarchicalStaticMeshSceneProxy : public FHierarchicalStaticMeshSceneProxy
{
public:
  FTaggedHierarchicalStaticMeshSceneProxy(UHierarchicalInstancedStaticMeshComponent * Component, bool bInIsGrass, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInstance * MaterialInstance, TMap<UMaterialInterface*, UMaterialInstanceDynamic*> TaggedMaterials);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  UMaterialInstance * TaggedMaterialInstance;
};

// Specific code for tagging landscapes (terrain)

UCLASS( meta=(BlueprintSpawnableComponent) )
class CARLA_API UTaggedLandscapeComponent : public UPrimitiveComponent
{
  GENERATED_BODY()

public:
  virtual FPrimitiveSceneProxy * CreateSceneProxy() override;
  virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const;

private:
  UPROPERTY()
  ULandscapeMaterialInstanceConstant * TaggedLMIC;
};

class FTaggedLandscapeComponentSceneProxy : public FLandscapeComponentSceneProxy
{
public:
  ULandscapeMaterialInstanceConstant * TaggedLandscapeMaterialInstance;
  
  FTaggedLandscapeComponentSceneProxy(ULandscapeComponent * Component);

  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;

private:
  FSoftObjectPath LandscapeAnnotationWorldPath = FSoftObjectPath("/Carla/PostProcessingMaterials/AnnotationColorLandscape.AnnotationColorLandscape");
};
