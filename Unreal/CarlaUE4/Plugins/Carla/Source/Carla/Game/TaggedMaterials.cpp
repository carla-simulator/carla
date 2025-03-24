#include "TaggedMaterials.h"
#include "ConstructorHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"

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

UMaterialInstanceDynamic* UTaggedMaterialsRegistry::GetTaggedMaterial() {
  return UMaterialInstanceDynamic::Create(TaggedOpaqueMaterial, this);
}

UMaterialInstanceDynamic* UTaggedMaterialsRegistry::GetTaggedMaterial(UMaterialInterface* UsedMaterial) {
  // If UsedMaterial is null OR neither masked nor using WorldPositionOffset OR is using ShadingModel from
  // MaterialExpression (which results in an error when changing the ShadingModel), we return NULL to indicate,
  // that the requested material does not require a fine-grained tag-injected correspondence.
  if (!UsedMaterial
      || !(UsedMaterial->IsMasked() || UsedMaterial->GetMaterial()->WorldPositionOffset.IsConnected())
      || UsedMaterial->IsShadingModelFromMaterialExpression()) {
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

void UTaggedMaterialsRegistry::InjectTagIntoMaterial(UMaterial* Material) {
  if (TaggedMaskedMaterials.Contains(Material->GetPathName())) {
    // Tag-injected material already exists -> do nothing
    return;
  }

  UMaterial* TagInjectedMaterial = DuplicateObject<UMaterial>(Material, this);
  TagInjectedMaterial->SetFlags(RF_Public | RF_Standalone);

  UMaterialExpressionVectorParameter* ExpressionInstSegColor = NewObject<UMaterialExpressionVectorParameter>(TagInjectedMaterial);
  ExpressionInstSegColor->SetParameterName(FName("AnnotationColor"));
  ExpressionInstSegColor->DefaultValue = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
  ExpressionInstSegColor->Material = TagInjectedMaterial;
  TagInjectedMaterial->Expressions.Add(ExpressionInstSegColor);

  if (TagInjectedMaterial->bUseMaterialAttributes) {
    // Create a SetAttributeExpression which will be injected between the existing input and the material node
    // and additionally has a emissive color parameter.
    UMaterialExpressionSetMaterialAttributes* SetAttributeExpression = NewObject<UMaterialExpressionSetMaterialAttributes>(TagInjectedMaterial);
    SetAttributeExpression->Material = TagInjectedMaterial;

    SetAttributeExpression->Inputs[0].Connect(0, TagInjectedMaterial->MaterialAttributes.GetTracedInput().Expression);

    FMaterialAttributesInput SetAttributeEmissiveInput = FMaterialAttributesInput();
    SetAttributeEmissiveInput.Expression = ExpressionInstSegColor;
    SetAttributeExpression->Inputs.Add(SetAttributeEmissiveInput);
    SetAttributeExpression->AttributeSetTypes.Add(EmissiveColorGuid);

    TagInjectedMaterial->MaterialAttributes.Connect(0, SetAttributeExpression);
    TagInjectedMaterial->Expressions.Add(SetAttributeExpression);
  } else {
    // Simply plug the tag node into emissive color. Existing values are simply replaced.
    TagInjectedMaterial->EmissiveColor.Connect(0, ExpressionInstSegColor);
  }
  // Set ShadingModel to unlit and recompile
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

  UMaterialInterface* TagInjectedParent = TaggedMaskedMaterials.FindChecked(MaterialInstance->Parent->GetPathName());

  // Now, we definitively have a tag-injected version of the parent.
  // Create a copy of the current material instance and change its parent to the tag-injected one.
  // We can ignore UMaterialInstanceDynamic here, since it cannot do any modifications to the shader.
  UMaterialInstanceConstant* MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstance);
  if (MaterialInstanceConstant) {
    UMaterialInstanceConstant* TagInjectedMIC = DuplicateObject<UMaterialInstanceConstant>(MaterialInstanceConstant, this);
    TagInjectedMIC->SetFlags(RF_Public | RF_Standalone);
    TagInjectedMIC->SetParentEditorOnly(TagInjectedParent);
    if (TagInjectedMIC->BasePropertyOverrides.bOverride_ShadingModel) {
      TagInjectedMIC->BasePropertyOverrides.ShadingModel = MSM_Unlit;
    }
    FMaterialUpdateContext LocalMaterialUpdateContext(FMaterialUpdateContext::EOptions::RecreateRenderStates);
    LocalMaterialUpdateContext.AddMaterialInstance(TagInjectedMIC);
    TaggedMaskedMaterials.Add(MaterialInstance->GetPathName(), TagInjectedMIC);
    bPendingChanges = true;
  }
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
