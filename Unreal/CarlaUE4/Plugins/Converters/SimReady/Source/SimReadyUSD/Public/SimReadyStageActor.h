// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyPxr.h"
#include "UObject/StrongObjectPtr.h"
#include "GameFramework/Info.h"
#include "Components/LightComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/AssetUserData.h"
#include "Components/MeshComponent.h"
#include "SimReadyPathHelper.h"
#include "SimReadyImportSettings.h"
#include "SimReadyStageActor.generated.h"


UCLASS(ClassGroup = SimReady, NotPlaceable, HideCategories = (Transform, Actor, LOD, Cooking), config=Engine, defaultconfig)
class SIMREADYUSD_API ASimReadyStageActor : public AInfo
{
    GENERATED_BODY()

protected: // Blueprint-accessible functions & properties
    /** Default prim of USD stage. */
    UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "SimReady", meta = (DisplayName = "Default Prim"))
    AActor* USDDefaultPrim;
public:
    UFUNCTION(BlueprintCallable, Category = "SimReady")
    FString ExportUSDToString();

public: // Public non-blueprint-accessible functions & properties

    enum ImportType
    {
        Mesh,
        Material,
        Texture,
        Animation,
        Root,
    };

    void SetTimeCode(double NewStageTime)
    {
        StageTimeCode = NewStageTime;
    }

    double GetTimeCode()
    {
        return StageTimeCode.GetValue();
    }

    ASimReadyStageActor();
    ~ASimReadyStageActor();

    void LoadChangedUSDPaths();

    static ASimReadyStageActor& Get(class UWorld& World);
    static ASimReadyStageActor* Find(class UWorld& World);

    template<typename T>
    T* FindObjectFromPath(const pxr::SdfPath& Path, bool IncludeParent = false)const
    {
        auto ObjectPath = Path;
        auto ObjectPtr = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(ObjectPath.GetText()));
        if (IncludeParent)
        {
            while (!ObjectPtr && !ObjectPath.IsEmpty() && ObjectPath != pxr::SdfPath::AbsoluteRootPath())
            {
                ObjectPath = ObjectPath.GetParentPath();
                ObjectPtr = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(ObjectPath.GetText()));
            }
        }

        if (!ObjectPtr || !ObjectPtr->IsValid())
            return nullptr;

        return Cast<T>(ObjectPtr->Get());
    }

    bool IsValidPrimPath(const pxr::SdfPath& Path);

    UPROPERTY()
    TWeakObjectPtr<class ALevelSequenceActor> LevelSequenceActor;

    bool HasValidImportStage() const;

    FString GetUSDPath();
    FString GetUSDName();
    FName GetAssetName(const FString& Name);
    FString GetUniqueImportName(const FString& KeyPath, const FString& Name);
    class UPackage* GetAssetPackage(ImportType Type, const FString& AssetName);
    enum EObjectFlags GetAssetFlag();
    class UTexture* GetTexture(const FString& OmniPath);

    void CreateTextureFromBuffer(const uint8* Content, uint64 Size, const FString& FileName, class UTexture*& OutTexture);

    template<class T>
    T* LoadImportObject(ImportType Type, const FString& AssetName)
    {
        if (HasValidImportStage())
        {
            FString SubPath;
            switch(Type)
            {
            case ImportType::Mesh:
                SubPath = TEXT("Meshes");
                break;
            case ImportType::Material:
                SubPath = TEXT("Materials");
                break;
            case ImportType::Texture:
                SubPath = TEXT("Textures");
                break;
            case ImportType::Animation:
                SubPath = TEXT("Animations");
                break;
            }
            FString PkgPath = FPaths::Combine(SavePackagePath, SubPath);
            PkgPath = FPaths::Combine(PkgPath, AssetName);

            auto ParentPackage = CreatePackage(*PkgPath);
            ParentPackage->FullyLoad();
            T* Obj = FindObject<T>(ParentPackage, *AssetName);
            if (Obj && GetUserResponse(PkgPath))
            {
                return Obj;
            }
        }

        return nullptr;
    }

    void ImportUSD(const FString& UsdPath, const FString& PackagePath, const FSimReadyImportSettings& ImportSettings);
    void InitializePreviewEnvironment();

protected:
    enum class EState
    {
        None,
        Loading,
        Saving,
    };

    const pxr::UsdStageRefPtr& GetUSDStage();
    //void ExportCineCameraActor(class ACineCameraActor*, pxr::UsdStageRefPtr, const pxr::SdfPath&);
    bool ToggleCineCameraVisibility(USceneComponent*);
    // Check if this component or it's attached child includes any supported component type
    bool IsComponentSupported(USceneComponent& Component);
    bool IsInPIEOrStandaloneMode();

    TSharedPtr<void> SetStateScoped(EState State);

    void OnUSDStageNotice(const pxr::SdfPath& InPath, bool bResync);
    void LoadUSD(const pxr::SdfPath& Path, bool bLoadChildren = false);
    void ResyncUSD(const pxr::SdfPath& Path);
    void LoadStaticMeshFromGeomGprim(const pxr::SdfPath& Path, UStaticMeshComponent& MeshComponent, bool bLoadChildren, bool bIgnoreVisibility = false);
    bool LoadStaticMesh(const pxr::UsdGeomMesh& USDMesh, UStaticMeshComponent& MeshComponent, bool bIgnoreVisibility = false);
    void GetInstancedStaticMeshComponents(const pxr::UsdPrim& Prim, bool bLoadChildren, TArray<class UHierarchicalInstancedStaticMeshComponent*>& OutInstancedStaticMeshComponents);
    bool LoadLight(const pxr::SdfPath& Path, bool bLoadChildren);
    void LoadMaterial(class UStaticMeshComponent& Component, const pxr::UsdGeomGprim& USDGprim, pxr::UsdGeomSubset GeomSubset = pxr::UsdGeomSubset());
    class UStaticMeshComponent* LoadBasicShape(const pxr::SdfPath& Path, bool bLoadChildren);
    class USceneComponent* LoadSceneComponent(const pxr::SdfPath& Path, bool bLoadChildren = true, bool bUpdateBound = true, bool bIgnoreVisibility = false);
    void LoadDefaultPrim();
    template<typename TSceneComp, typename TActor = AActor>
    TSceneComp* CreateActorAndComponent(const pxr::UsdPrim& Prim, bool bForceCreateActor = false);
    class USceneComponent* CreateActorAndComponent(UClass& SceneCompClass, UClass& ActorClass, const pxr::UsdPrim& Prim, bool bForceCreateActor);
    void ReserveObject(class UObject& Object, const FString& USDPath);
    bool LoadMdlSchema(class UMeshComponent* Component, int32 SlotIndex, const pxr::UsdShadeShader& ShadeShader);
    void OverrideTextureSRGB(class UTexture* Texture, bool SRGB);
    void UpdateGraphNodePrim(const pxr::SdfPath& Path);
    bool IsValidNodeFromGraph(const pxr::UsdPrim& TargetPrim, const pxr::UsdShadeMaterial& ShadeMaterial);
    bool GetGraphNodeInputName(const pxr::UsdShadeInput& TargetInput, const pxr::UsdPrim& Prim, const pxr::TfToken& SourceName, const FString& ParentInputName, FString& OutInputName);
    void SetMaterial(class UMeshComponent& Component, int32 SlotIndex, class UMaterialInterface* Instance);
    class UMaterialInstanceConstant* CreateDynamicInstanceFromMdl(class UMeshComponent* Component, int32 SlotIndex, const FString& MdlPath, bool bRelativePath, const FString& MdlMaterialName, const FString& InstanceName);
    void LoadPointInstancer(const pxr::SdfPath& Path, bool bLoadChildren);
    void LoadPointInstancerMesh(const pxr::SdfPath& MeshPath, bool bLoadChildren);
    void InitializePrototypes(const pxr::UsdPrim& Prim, const pxr::SdfPathVector& ProtoPaths);
    bool LoadPreviewSurface(class UMeshComponent* Component, int32 SlotIndex, const pxr::UsdGeomMesh& USDMesh, const pxr::UsdShadeShader& ShadeShader);
    void UpdateShadeInputs(class UMaterialInstanceConstant* Instance, const pxr::UsdShadeShader& ShadeShader, struct FNamedParameterTimeSamples* ParameterTimeSamples = nullptr);
    void UpdateShadeInputs(class UMaterialInstanceConstant* Instance, const pxr::UsdShadeShader& ShadeShader, const FString& ParentInputName);
    bool IsPreviewSurface(class UMaterialInstanceConstant* MaterialInst);
    bool LoadSkeletalMeshComponent(const pxr::SdfPath& Path);
    void LoadAnimationSequence(class USkeletalMeshComponent* SkeletalMeshComponent, const pxr::UsdSkelRoot& Root, const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery);
    void LoadMaterial(class UMeshComponent* Component, int32 SlotIndex, const pxr::UsdGeomMesh& USDMesh, const pxr::UsdShadeShader& Shader, const pxr::UsdShadeShader& MdlShader);
    void LoadDomeLight(const pxr::UsdLuxDomeLight& DomeLight);
    void UpdateDomeLightSphere();

    static bool ShouldLoadProperty(const pxr::SdfPath& Path, const pxr::TfTokenVector& PropNames, const pxr::TfTokenVector& ExcludedPropNames = {});

    void LoadCustomLayerData(bool& bCameraSet);
    void LoadLayerTimeInfo();
    bool GetUserResponse(const FString& PkgPath);
    void ComputeEditorCameraPosition();
    bool SetEditorCamera(const FVector& CamPosition, const FRotator& CamRotation);
    void PostLoadUSDStage(bool UpdateViewport);

    template<typename TMeshComponent, typename TMeshActor>
    TMeshComponent* LoadStaticMeshComponent(const pxr::SdfPath& Path, bool bLoadChildren, bool bIgnoreVisibility = false)
    {
        pxr::UsdGeomMesh USDMesh(GetUSDStage()->GetPrimAtPath(Path.GetPrimPath()));
        if (!USDMesh)
        {
            return nullptr;
        }

        TMeshComponent* MeshComponent = CreateActorAndComponent<TMeshComponent, TMeshActor>(USDMesh.GetPrim());
        if (!MeshComponent)
        {
            return nullptr;
        }

        // Load geometry
        bool bPrimvarsAttribute = false;
        if (Path.IsPrimPropertyPath())
        {
            if (Path.GetName().find("primvars:", 0) == 0)
            {
                bPrimvarsAttribute = true;
            }
        }

        if (ShouldLoadProperty(Path, pxr::UsdGeomMesh::GetSchemaAttributeNames(), pxr::UsdGeomBoundable::GetSchemaAttributeNames()) || bPrimvarsAttribute)
        {
            if (!LoadStaticMesh(USDMesh, *MeshComponent, bIgnoreVisibility))
            {
                return nullptr;
            }
        }

        LoadStaticMeshFromGeomGprim(Path, *MeshComponent, bLoadChildren, bIgnoreVisibility);

        return MeshComponent;
    }

#if WITH_EDITOR
    void SpawnGlobalPostProcessVolume();
    TArray<UMeshComponent*> FindMeshComponents(const FString& PrimPath);
    TSet<FString> GetSelectedPrimPaths();
#endif

    static pxr::SdfPath ToUSDPath(const TCHAR* Str)
    {
        return pxr::SdfPath(TCHAR_TO_UTF8(Str));
    }

    EState State = EState::None;

    UPROPERTY(NonTransactional)
    TMap<FString, TWeakObjectPtr<UObject>> USDPathToObject;	// Actors are duplicated during save as. So we need to declare it as UProperty to remember generated objects. So that we can remove them after duplicating.

    UPROPERTY(Transient)
    TMap<FString, UObject*> ReservedUE4Objects;	// Removed objects are reserved here

    UPROPERTY(Transient, Instanced)
    UObject* ReservedUE4ObjectOuter;

    struct
    {
        TArray<pxr::SdfPath>	Paths;
        TArray<bool>			ResyncFlag;
        FDelegateHandle DelegateHandle;
    }ChangedUSDPaths;

    static const EObjectFlags DefaultObjFlag = EObjectFlags::RF_Transactional;

private:
    pxr::UsdTimeCode StageTimeCode = pxr::UsdTimeCode::Default();

    UPROPERTY(Config)
    bool bExportTangentX = true;

    pxr::UsdStageRefPtr ImportUSDStage;

    struct FTimeCodeInfo
    {		
        TOptional<double> StartTimeCode;
        TOptional<double> EndTimeCode;
        TOptional<double> TimeCodesPerSecond;

        void Reset()
        {
            StartTimeCode.Reset();
            EndTimeCode.Reset();
            TimeCodesPerSecond.Reset();
        }
    };
    FTimeCodeInfo TimeCodeInfo;
    TSharedPtr<class FSimReadyUSDSequenceImporter> USDSequenceImporter;
    TArray<TWeakObjectPtr<AActor>> PreviewEnvironmentActors;
    FString ImportUSDSourceFile;
    FString SavePackagePath;
    TMap<FString, FString> ImportedNames;
    FSimReadyImportSettings ImportSettings;
    int32 ImportedSuffixIndex = 0;
    int32 ImportOverwriteStatus = 0;
    FBox StageBounds;
    TWeakObjectPtr<UStaticMeshComponent> SkyMeshComponent;
};

template<typename TSceneComp, typename TActor>
inline TSceneComp* ASimReadyStageActor::CreateActorAndComponent(const pxr::UsdPrim& Prim, bool bForceCreateActor)
{
    return Cast<TSceneComp>(CreateActorAndComponent(*TSceneComp::StaticClass(), *TActor::StaticClass(), Prim, bForceCreateActor));
}

struct FStaticComponentMask
{
    bool R;
    bool G;
    bool B;
    bool A;

    FStaticComponentMask(bool InR, bool InG, bool InB, bool InA)
        : R(InR)
        , G(InG)
        , B(InB)
        , A(InA)
    {

    }
};

extern void UpdateStaticParameters(class UMaterialInstanceConstant* MaterialInst, const TMap<FName, bool>* SwitchParameters = nullptr, const TMap<FName, FStaticComponentMask>* ComponentMaskParameters = nullptr);
