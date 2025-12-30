#include "Carla/Game/TaggedMaterials.h"
#include "Carla/Game/CarlaEpisode.h"

#include "ConstructorHelpers.h"
#include "AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"

#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionSetMaterialAttributes.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"

const FString UTaggedMaterialsRegistry::TaggedMaterialsRootDir = TEXT("/Carla/PostProcessingMaterials/TaggedMaterials");
UTaggedMaterialsRegistry* UTaggedMaterialsRegistry::Registry = nullptr;

UTaggedMaterialsRegistry::UTaggedMaterialsRegistry() {
  FString MaterialPath = TEXT("Material'/Carla/PostProcessingMaterials/AnnotationColor.AnnotationColor'");
  static ConstructorHelpers::FObjectFinder<UMaterial> TaggedOpaqueMaterialObject(*MaterialPath);
  // TODO: Replace with VertexColorViewModeMaterial_ColorOnly?
  TaggedOpaqueMaterial = TaggedOpaqueMaterialObject.Object;

  FString TaggingMPCPath = TEXT("Material'/Carla/PostProcessingMaterials/MPC/MPC_Tagging.MPC_Tagging'");
  static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> TaggingMPCObject(*TaggingMPCPath);
  TaggingParamerCollection = TaggingMPCObject.Object;
}

UTaggedMaterialsRegistry* UTaggedMaterialsRegistry::Create(const FString& RegistryName) {
  UTaggedMaterialsRegistry* NewRegistry = nullptr;
  if (RegistryName.Len() > 0) {
    FString PackageName = TEXT("TaggedMaterials_") + RegistryName;
    FString PackagePath = TaggedMaterialsRootDir / PackageName;
    UPackage* Package = CreatePackage(*PackagePath);

    NewRegistry = NewObject<UTaggedMaterialsRegistry>(Package, *PackageName, RF_Public | RF_Standalone);
  } else {
    NewRegistry = NewObject<UTaggedMaterialsRegistry>();
  }

  return NewRegistry;
}

UTaggedMaterialsRegistry* UTaggedMaterialsRegistry::Get() {
  if (!Registry) {
#if WITH_EDITOR
    // Try loading if editor-registry already exists, else instantiate new one
    FString RegistryNameEditor = TEXT("Editor");
    Registry = Load(RegistryNameEditor);
    if (!Registry) {
      Registry = Create(RegistryNameEditor);
    }
#else
    // Try loading (multiple) precomputed UTaggedMaterialsRegistry (each map package ships its own registry)
    FString PluginPath = IPluginManager::Get().FindPlugin(TEXT("Carla"))->GetBaseDir();
    FString RegistriesDirPath = PluginPath / TEXT("Content/PostProcessingMaterials/TaggedMaterials");
    TArray<FString> RegistryFiles;
    IFileManager::Get().FindFiles(RegistryFiles, *(RegistriesDirPath / TEXT("*.uasset")), true, false);

    Registry = Create(TEXT(""));
    for (const FString& RegistryFile : RegistryFiles) {
      FString RegistryName = FPaths::GetBaseFilename(RegistryFile, true);
      UTaggedMaterialsRegistry* LoadedRegistry = Load(RegistryName, false);
      Registry->TaggedMaskedMaterials.Append(LoadedRegistry->TaggedMaskedMaterials);
    }
#endif // WITH_EDITOR
    Registry->AddToRoot(); // Prevent GC deleting the materials of this registry
  }
  return Registry;
}

void UTaggedMaterialsRegistry::SetTaggingTraverseTranslucency(UCarlaEpisode* Episode, bool bTaggingTraverseTranslucency) {
  UMaterialParameterCollectionInstance* MPCInstance = Episode->GetWorld()->GetParameterCollectionInstance(TaggingParamerCollection);
  MPCInstance->SetScalarParameterValue("TraverseTranslucency", bTaggingTraverseTranslucency);
}

UMaterialInstanceDynamic* UTaggedMaterialsRegistry::GetTaggedMaterial() {
  return UMaterialInstanceDynamic::Create(TaggedOpaqueMaterial, this);
}

UMaterialInstanceDynamic* UTaggedMaterialsRegistry::GetTaggedMaterial(UMaterialInterface* UsedMaterial) {
  // If UsedMaterial is a translucent material, we use the default tagged material but enable IsTranslucent.
  // Using SetTaggingTraverseTranslucency, it can be globally enabled/disabled that these materials appear invisible or not.
  if (UsedMaterial && UsedMaterial->GetBlendMode() == EBlendMode::BLEND_Translucent) {
    UMaterialInstanceDynamic* TranslucentMaterial = GetTaggedMaterial();
    TranslucentMaterial->SetScalarParameterValue("IsTranslucent", true);
    return TranslucentMaterial;
  }

  // If UsedMaterial is null OR neither masked nor using WorldPositionOffset, we return NULL to indicate,
  // that the requested material does not require a fine-grained tag-injected correspondence.
  if (!UsedMaterial || !(UsedMaterial->IsMasked() || UsedMaterial->GetMaterial()->WorldPositionOffset.IsConnected())) {
    return NULL;
  }

#if WITH_EDITOR
  // Create tag-injected correspondence of UsedMaterial, if it does not exist already
  if (!TaggedMaskedMaterials.Contains(UsedMaterial->GetPathName())) {
    if (GIsEditor || IsRunningCommandlet()) {
      UTaggedMaterialsRegistry::InjectTag(UsedMaterial);
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Can only create new materials in editor (PIE) or during cooking. %s will render coarsely in instance segmentation."),
             *UsedMaterial->GetPathName());
    }
  }
#endif // WITH_EDITOR

  // Create an instance-specific MID for this UsedMaterial
  UMaterialInstanceDynamic* UsedMaterialMID = Cast<UMaterialInstanceDynamic>(UsedMaterial);
  if (UsedMaterialMID) {
    // UsedMaterial is a MID, so we create a tagged MID based on the parent and copy the original parameters
    UMaterialInterface** TagInjectedParent = TaggedMaskedMaterials.Find(UsedMaterialMID->Parent->GetPathName());
    if (TagInjectedParent) {
      UMaterialInstanceDynamic* TagInjectedMID = UMaterialInstanceDynamic::Create(*TagInjectedParent, this);
      TagInjectedMID->CopyParameterOverrides(UsedMaterialMID);
      return TagInjectedMID;
    }
  } else {
    // We can simply wrap the UMaterial or UMaterialInstanceConstant in a UMaterialInstanceDynamic
    UMaterialInterface** TagInjectedMaterial = TaggedMaskedMaterials.Find(UsedMaterial->GetPathName());
    if (TagInjectedMaterial) {
      return UMaterialInstanceDynamic::Create(*TagInjectedMaterial, this);
    }
  }
  // Reaching this could happen in a cooked build, if UsedMaterial is missing in the cooked TaggedMaterialsRegistries
  return NULL;
}

#if WITH_EDITOR
void UTaggedMaterialsRegistry::InjectTag(UMaterialInterface* MaterialInterface) {
  UMaterial* Material = Cast<UMaterial>(MaterialInterface);
  UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface);
  if (Material) {
    InjectTagIntoMaterial(Material);
  } else if (MaterialInstance) {
    InjectTagIntoMaterialInstance(MaterialInstance);
  } else {
    UE_LOG(LogCarla, Error, TEXT("MaterialInterface '%s' is neither UMaterial nor UMaterialInstance."), *MaterialInterface->GetPathName());
  }
}

const static FGuid EmissiveColorGuid = FMaterialAttributeDefinitionMap::GetID(EMaterialProperty::MP_EmissiveColor);
const static FGuid OpacityMaskGuid = FMaterialAttributeDefinitionMap::GetID(EMaterialProperty::MP_OpacityMask);
const static FGuid WorldPositionOffsetGuid = FMaterialAttributeDefinitionMap::GetID(EMaterialProperty::MP_WorldPositionOffset);

void UTaggedMaterialsRegistry::InjectTagIntoMaterial(UMaterial* Material) {
  if (TaggedMaskedMaterials.Contains(Material->GetPathName())) {
    // Tag-injected material already exists -> do nothing
    return;
  }

  FName TaggedMaterialName(GetTaggedName(Material->GetName()));
  UMaterial* TagInjectedMaterial = NewObject<UMaterial>(this, TaggedMaterialName, RF_Public | RF_Standalone);

  UMaterialExpressionVectorParameter* ExpressionInstSegColor = NewObject<UMaterialExpressionVectorParameter>(TagInjectedMaterial);
  ExpressionInstSegColor->SetParameterName(FName("AnnotationColor"));
  ExpressionInstSegColor->DefaultValue = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
  ExpressionInstSegColor->Material = TagInjectedMaterial;
  TagInjectedMaterial->Expressions.Add(ExpressionInstSegColor);

  if (Material->bUseMaterialAttributes) {
    TagInjectedMaterial->bUseMaterialAttributes = 1;
    // Create a SetAttributeExpression which will be injected between the existing input and the material node
    // and additionally has a emissive color parameter.
    UMaterialExpressionSetMaterialAttributes* SetAttributeExpression = NewObject<UMaterialExpressionSetMaterialAttributes>(TagInjectedMaterial);
    SetAttributeExpression->Material = TagInjectedMaterial;

    UMaterialExpression* MaterialAttributesExpression = Material->MaterialAttributes.GetTracedInput().Expression;
    UMaterialExpressionMakeMaterialAttributes* MakeMaterialAttributesExpression = Cast<UMaterialExpressionMakeMaterialAttributes>(MaterialAttributesExpression);
    UMaterialExpressionSetMaterialAttributes* SetMaterialAttributesExpression = Cast<UMaterialExpressionSetMaterialAttributes>(MaterialAttributesExpression);
    if (MakeMaterialAttributesExpression) {
      // Only copy OpacityMask and WorldPositionOffset from the MakeMaterialAttributes node.
      UMaterialExpression* OpacityMaskExpression = MakeMaterialAttributesExpression->OpacityMask.GetTracedInput().Expression;
      if (OpacityMaskExpression) {
        UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, OpacityMaskExpression);
        FMaterialAttributesInput SetAttributeOpacityMaskInput = FMaterialAttributesInput();
        SetAttributeOpacityMaskInput.Connect(MakeMaterialAttributesExpression->OpacityMask.GetTracedInput().OutputIndex, CopiedRootExpression);
        SetAttributeExpression->Inputs.Add(SetAttributeOpacityMaskInput);
        SetAttributeExpression->AttributeSetTypes.Add(OpacityMaskGuid);
      }
      UMaterialExpression* WorldPositionOffsetExpression = Material->WorldPositionOffset.GetTracedInput().Expression;
      if (WorldPositionOffsetExpression) {
        UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, WorldPositionOffsetExpression);
        FMaterialAttributesInput SetAttributeWorldPositionOffsetInput = FMaterialAttributesInput();
        SetAttributeWorldPositionOffsetInput.Connect(MakeMaterialAttributesExpression->WorldPositionOffset.GetTracedInput().OutputIndex, CopiedRootExpression);
        SetAttributeExpression->Inputs.Add(SetAttributeWorldPositionOffsetInput);
        SetAttributeExpression->AttributeSetTypes.Add(WorldPositionOffsetGuid);
      }
      if (!OpacityMaskExpression && !WorldPositionOffsetExpression) {
        return; // neither OpacityMask nor WorldPositionOffset are actually used
      }
    } else if (SetMaterialAttributesExpression) {
      // Copy MaterialAttributes, OpacityMask and WorldPositionOffset from the SetMaterialAttributes node.
      // Note: This could be done recursively on the MaterialAttributes, but there are currently no materials that do this.
      //       In the "worst" case, we just copy too much.
      UMaterialExpression* PreMaterialAttributesExpression = SetMaterialAttributesExpression->Inputs[0].GetTracedInput().Expression;
      if (PreMaterialAttributesExpression) {
        UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, PreMaterialAttributesExpression);
        SetAttributeExpression->Inputs[0].Connect(SetMaterialAttributesExpression->Inputs[0].GetTracedInput().OutputIndex, CopiedRootExpression);
      }

      bool bHasExpression = false;
      for (int32 i = 0; i < SetMaterialAttributesExpression->AttributeSetTypes.Num(); ++i) {
        FGuid CurrentInputGuid = SetMaterialAttributesExpression->AttributeSetTypes[i];
        if (CurrentInputGuid == OpacityMaskGuid || CurrentInputGuid == WorldPositionOffsetGuid) {
          UMaterialExpression* InputExpression = SetMaterialAttributesExpression->Inputs[i+1].GetTracedInput().Expression;
          UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, InputExpression);
          FMaterialAttributesInput SetAttributeInput = FMaterialAttributesInput();
          SetAttributeInput.Connect(SetMaterialAttributesExpression->Inputs[i+1].GetTracedInput().OutputIndex, CopiedRootExpression);
          SetAttributeExpression->Inputs.Add(SetAttributeInput);
          SetAttributeExpression->AttributeSetTypes.Add(CurrentInputGuid);
          bHasExpression = true;
        }
      }
      if (!PreMaterialAttributesExpression && !bHasExpression) {
        return; // neither OpacityMask nor WorldPositionOffset are actually used
      }
    } else if (MaterialAttributesExpression) {
      // If none of the above special cases triggered (this is rare), we simply copy everything.
      UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, MaterialAttributesExpression);
      SetAttributeExpression->Inputs[0].Connect(Material->MaterialAttributes.GetTracedInput().OutputIndex, CopiedRootExpression);
    } else {
      return; // no MaterialAttributes are connected at all
    }

    // Finally add the AnnotationColor node as emissive input
    FMaterialAttributesInput SetAttributeEmissiveInput = FMaterialAttributesInput();
    SetAttributeEmissiveInput.Connect(0, ExpressionInstSegColor);
    SetAttributeExpression->Inputs.Add(SetAttributeEmissiveInput);
    SetAttributeExpression->AttributeSetTypes.Add(EmissiveColorGuid);

    TagInjectedMaterial->MaterialAttributes.Connect(0, SetAttributeExpression);
    TagInjectedMaterial->Expressions.Add(SetAttributeExpression);
  } else {
    // Simply plug the tag node into emissive color.
    TagInjectedMaterial->EmissiveColor.Connect(0, ExpressionInstSegColor);

    // Copy the OpacityMask and WorldPositionOffset (if set) so that the new material mirrors these aspects from the original material.
    UMaterialExpression* OpacityMaskExpression = Material->OpacityMask.GetTracedInput().Expression;
    if (OpacityMaskExpression) {
      UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, OpacityMaskExpression);
      TagInjectedMaterial->OpacityMask.Connect(Material->OpacityMask.GetTracedInput().OutputIndex, CopiedRootExpression);
    }
    UMaterialExpression* WorldPositionOffsetExpression = Material->WorldPositionOffset.GetTracedInput().Expression;
    if (WorldPositionOffsetExpression) {
      UMaterialExpression* CopiedRootExpression = CopyMaterialExpressions(TagInjectedMaterial, WorldPositionOffsetExpression);
      TagInjectedMaterial->WorldPositionOffset.Connect(Material->WorldPositionOffset.GetTracedInput().OutputIndex, CopiedRootExpression);
    }
    if (!OpacityMaskExpression && !WorldPositionOffsetExpression) {
      return; // neither OpacityMask nor WorldPositionOffset are actually used
    }
  }

  // Copy other relevant properties
  TagInjectedMaterial->OpacityMaskClipValue = Material->OpacityMaskClipValue;
  TagInjectedMaterial->TwoSided = Material->TwoSided;
  TagInjectedMaterial->DitherOpacityMask = Material->DitherOpacityMask;
  TagInjectedMaterial->bFullyRough = 1;  // might save some shaders
  TagInjectedMaterial->bUseFullPrecision = Material->bUseFullPrecision;
  TagInjectedMaterial->bUseAlphaToCoverage = Material->bUseAlphaToCoverage;
  TagInjectedMaterial->ShadingRate = Material->ShadingRate;

  // Copy the "Used With" settings, enabling the required shaders
  TagInjectedMaterial->bUsedWithSkeletalMesh = Material->bUsedWithSkeletalMesh;
  TagInjectedMaterial->bUsedWithEditorCompositing = Material->bUsedWithEditorCompositing;
  TagInjectedMaterial->bUsedWithParticleSprites = Material->bUsedWithParticleSprites;
  TagInjectedMaterial->bUsedWithBeamTrails = Material->bUsedWithBeamTrails;
  TagInjectedMaterial->bUsedWithMeshParticles = Material->bUsedWithMeshParticles;
  TagInjectedMaterial->bUsedWithNiagaraSprites = Material->bUsedWithNiagaraSprites;
  TagInjectedMaterial->bUsedWithNiagaraRibbons = Material->bUsedWithNiagaraRibbons;
  TagInjectedMaterial->bUsedWithNiagaraMeshParticles = Material->bUsedWithNiagaraMeshParticles;
  TagInjectedMaterial->bUsedWithGeometryCache = Material->bUsedWithGeometryCache;
  TagInjectedMaterial->bUsedWithStaticLighting = Material->bUsedWithStaticLighting;
  TagInjectedMaterial->bUsedWithMorphTargets = Material->bUsedWithMorphTargets;
  TagInjectedMaterial->bUsedWithSplineMeshes = Material->bUsedWithSplineMeshes;
  TagInjectedMaterial->bUsedWithInstancedStaticMeshes = Material->bUsedWithInstancedStaticMeshes;
  TagInjectedMaterial->bUsedWithGeometryCollections = Material->bUsedWithGeometryCollections;
  TagInjectedMaterial->bUsesDistortion = Material->bUsesDistortion;
  TagInjectedMaterial->bUsedWithClothing = Material->bUsedWithClothing;
  TagInjectedMaterial->bUsedWithWater = Material->bUsedWithWater;
  TagInjectedMaterial->bUsedWithHairStrands = Material->bUsedWithHairStrands;
  TagInjectedMaterial->bUsedWithLidarPointCloud = Material->bUsedWithLidarPointCloud;
  TagInjectedMaterial->bUsedWithVirtualHeightfieldMesh = Material->bUsedWithVirtualHeightfieldMesh;

  // Copy BlendMode, set ShadingModel to unlit and recompile
  TagInjectedMaterial->BlendMode = Material->BlendMode;
  TagInjectedMaterial->SetShadingModel(MSM_Unlit);
  TagInjectedMaterial->PreEditChange(nullptr);
  TagInjectedMaterial->PostEditChange();
  TaggedMaskedMaterials.Add(Material->GetPathName(), TagInjectedMaterial);
  bPendingChanges = true;
}

void UTaggedMaterialsRegistry::InjectTagIntoMaterialInstance(UMaterialInstance* MaterialInstance) {
  if (TaggedMaskedMaterials.Contains(MaterialInstance->GetPathName())) {
    // Tag-injected material instance already exists -> do nothing
    return;
  }

  UMaterial* MaterialParent = Cast<UMaterial>(MaterialInstance->Parent);
  UMaterialInstance* MaterialInstanceParent = Cast<UMaterialInstance>(MaterialInstance->Parent);
  if (MaterialInstanceParent) {
    // Recursively inject tag into chains of material instances
    InjectTagIntoMaterialInstance(MaterialInstanceParent);
  } else if (MaterialParent) {
    // Found base material -> inject tag
    InjectTagIntoMaterial(MaterialParent);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Parent of '%s' is neither UMaterialInstance nor UMaterial."), *MaterialInstance->Parent->GetPathName());
    return;
  }

  UMaterialInterface** TagInjectedParent = TaggedMaskedMaterials.Find(MaterialInstance->Parent->GetPathName());

  if (TagInjectedParent) {
    // Now, we definitively have a tag-injected version of the parent.
    // Create a copy of the current material instance and change its parent to the tag-injected one.
    // We can ignore UMaterialInstanceDynamic here, since it cannot do any modifications to the shader.
    UMaterialInstanceConstant* MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstance);
    if (MaterialInstanceConstant) {
      UMaterialInstanceConstant* TagInjectedMIC = DuplicateObject<UMaterialInstanceConstant>(MaterialInstanceConstant, this);
      TagInjectedMIC->Rename(*GetTaggedName(TagInjectedMIC->GetName()));
      TagInjectedMIC->SetFlags(RF_Public | RF_Standalone);
      TagInjectedMIC->SetParentEditorOnly(*TagInjectedParent);
      if (TagInjectedMIC->BasePropertyOverrides.bOverride_ShadingModel) {
        TagInjectedMIC->BasePropertyOverrides.ShadingModel = MSM_Unlit;
      }
      FMaterialUpdateContext LocalMaterialUpdateContext(FMaterialUpdateContext::EOptions::RecreateRenderStates);
      LocalMaterialUpdateContext.AddMaterialInstance(TagInjectedMIC);
      TaggedMaskedMaterials.Add(MaterialInstance->GetPathName(), TagInjectedMIC);
      bPendingChanges = true;
    }
  }
}

FString UTaggedMaterialsRegistry::GetTaggedName(const FString& OriginalName) {
  FString TaggedName = OriginalName + TEXT("_Tagged");
  // Check if the name is already in use by any other object in this TaggedMaterialsRegistry.
  // Append growing index, if this is actually the case (rare).
  int32 i = 0;
  while (StaticFindObject(NULL, this, *TaggedName, true)) {
    i++;
    TaggedName = FString(OriginalName + FString::Printf(TEXT("_Tagged%d"), i));
  }
  return TaggedName;
}

UMaterialExpression* UTaggedMaterialsRegistry::CopyMaterialExpressions(UMaterial* TargetMaterial, UMaterialExpression* RootExpression) {
  TArray<UMaterialExpression*> InputExpressions;
  RootExpression->GetAllInputExpressions(InputExpressions);
  TArray<UMaterialExpression*> CopiedExpressions;
  TArray<UMaterialExpressionComment*> EmptyComments;
  TArray<UMaterialExpression*> CopiedEmptyComments;
  UMaterialExpression::CopyMaterialExpressions(InputExpressions, EmptyComments, TargetMaterial, NULL, CopiedExpressions, CopiedEmptyComments);
  return CopiedExpressions[0];
}

void UTaggedMaterialsRegistry::PostInitProperties() {
  Super::PostInitProperties();

  if (GIsEditor) {
    FWorldDelegates::OnWorldCleanup.AddUObject(this, &UTaggedMaterialsRegistry::OnWorldCleanup);
  }
}

void UTaggedMaterialsRegistry::BeginDestroy() {
  if (GIsEditor) {
    FWorldDelegates::OnWorldCleanup.RemoveAll(this);
  }

  Super::BeginDestroy();
}

void UTaggedMaterialsRegistry::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources) {
  if (bPendingChanges) {
    Save();
    bPendingChanges = false;
  }
}

void UTaggedMaterialsRegistry::Save() {
  UPackage* Package = GetPackage();
  FString PackagePath = Package->GetName();

  MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(this);
  UPackage::SavePackage(Package, this, RF_Public | RF_Standalone,
                        *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()));
}
#endif // WITH_EDITOR

UTaggedMaterialsRegistry* UTaggedMaterialsRegistry::Load(const FString& RegistryName, bool bIsSuffix /* = true */) {
  FString FullRegistryName = RegistryName;
  if (bIsSuffix) {
    FullRegistryName = TEXT("TaggedMaterials_") + RegistryName;
  }
  FSoftObjectPath RegistryPath = FSoftObjectPath(*(TaggedMaterialsRootDir / FullRegistryName + TEXT(".") + FullRegistryName));

  UTaggedMaterialsRegistry* LoadedRegistry = nullptr;
  UObject* RegistryObject = RegistryPath.TryLoad();
  if (RegistryObject) {
    LoadedRegistry = Cast<UTaggedMaterialsRegistry>(RegistryObject);
  }

  if (!LoadedRegistry) {
    UE_LOG(LogCarla, Warning, TEXT("Failed to load UTaggedMaterialsRegistry '%s' in '%s'."), *RegistryName, *TaggedMaterialsRootDir);
  }
  return LoadedRegistry;
}
