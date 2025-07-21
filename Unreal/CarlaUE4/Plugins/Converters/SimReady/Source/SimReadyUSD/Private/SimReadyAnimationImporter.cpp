// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAnimationImporter.h"
#include "USDConverter.h"
#include "SimReadyUSDHelper.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequence.h"
#include "AssetRegistryModule.h"
#include "Extractors/TimeSamplesData.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"
#include "UObject/NameTypes.h"


DECLARE_LOG_CATEGORY_EXTERN(LogSimReadyImporter, Log, All);
#define LOCTEXT_NAMESPACE "SimReadyEditor"

FString SanitizeObjectName(const FString& InObjectName)
{
    FString SanitizedText = InObjectName;
    const TCHAR* InvalidChar = INVALID_OBJECTNAME_CHARACTERS;
    while (*InvalidChar)
    {
        SanitizedText.ReplaceCharInline(*InvalidChar, TCHAR('_'), ESearchCase::CaseSensitive);
        ++InvalidChar;
    }

    return SanitizedText;
}

namespace SimReadyAnimationImporterImpl
{
    void FindUsdSkelAnimation(const pxr::UsdPrim& Prim, TArray<pxr::UsdPrim>& AnimationPrims)
    {
        if (Prim.IsA<pxr::UsdSkelAnimation>())
        {
            AnimationPrims.Add(Prim);
        }
        else
        {
            auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
            for (auto ChildPrim : ChildPrims)
            {
                FindUsdSkelAnimation(ChildPrim, AnimationPrims);
            }
        }
    }

    void TryGetFacialCurves(const pxr::UsdSkelAnimation& SkelAnimation, TArray<FString>& OutCurveNames, TMap<double, pxr::VtArray<float>>& OutCurveValuesTimeSamples)
    {
        // Try to get facial curve
        if (pxr::UsdAttribute CurveNamesAttr = SkelAnimation.GetPrim().GetAttribute(pxr::TfToken("custom:mh_curveNames")))
        {
            auto USDCurveNames = GetUSDValue<pxr::VtArray<pxr::TfToken>>(CurveNamesAttr);
            OutCurveNames.Reserve(USDCurveNames.size());
            for (auto CurveName : USDCurveNames)
            {
                OutCurveNames.Add(CurveName.GetText());
            }
        }

        if (pxr::UsdAttribute CurveValuesAttr = SkelAnimation.GetPrim().GetAttribute(pxr::TfToken("custom:mh_curveValues")))
        {
            std::vector<double> TimeSamples;
            CurveValuesAttr.GetTimeSamples(&TimeSamples);

            for (auto TimeCode : TimeSamples)
            {
                pxr::VtArray<float> CurveValues;
                CurveValuesAttr.Get(&CurveValues, pxr::UsdTimeCode(TimeCode));

                if (CurveValues.size() == OutCurveNames.Num())
                {
                    OutCurveValuesTimeSamples.Add(TimeCode, CurveValues);
                }
            }
        }
    }

    bool ImportSkelAnimation(const pxr::UsdSkelAnimation& SkelAnimation, const FString& ParentPath, USkeleton* Skeleton, int32& OverwriteStatus, const FName Name, const EAnimationSource AnimationSource)
    {
        if (SkelAnimation && Skeleton)
        {
            auto Stage = SkelAnimation.GetPrim().GetStage();
            FString RootLayerFile = Stage->GetRootLayer()->GetIdentifier().c_str();

            TArray<FString> CurveNames;
            TMap<double, pxr::VtArray<float>> CurveValuesTimeSamples;
            if (AnimationSource == EAnimationSource::ERC_A2F)
            {
                TryGetFacialCurves(SkelAnimation, CurveNames, CurveValuesTimeSamples);
            }

            // If there's nothing about A2F curves, we'll try importing usd animation
            bool bFacial = CurveValuesTimeSamples.Num() > 0;

            pxr::UsdSkelCache USDSkelCache;
            auto AnimQuery = USDSkelCache.GetAnimQuery(SkelAnimation);
            std::vector<double> TimeSamples;
            TArray<FName> BoneNames;

            bool bSuccess = false;
            if (bFacial)
            {
                // Verify Curve Name
                for (int32 CurveIndex = 0; CurveIndex < CurveNames.Num(); ++CurveIndex)
                {
                    FSmartName NewName;
                    bSuccess |= Skeleton->GetSmartNameByName(USkeleton::AnimCurveMappingName, *CurveNames[CurveIndex], NewName);
                }
            }
            else
            {
                // Verify Bone Name
                if (AnimQuery.IsValid() && AnimQuery.GetJointTransformTimeSamples(&TimeSamples))
                {
                    auto JointOrderArray = AnimQuery.GetJointOrder();
                    for (auto JointOrder : JointOrderArray)
                    {
                        pxr::SdfPath JointPath(JointOrder);
                        FName BoneName = JointPath.GetName().c_str();
                        int32 BoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
                        bSuccess |= (BoneIndex != INDEX_NONE);
                        BoneNames.Add(BoneName);
                    }
                }
            }

            if (!bSuccess)
            {
                UE_LOG(LogSimReadyImporter, Error, TEXT("Skeleton %s isn't compatible with %s"), *Skeleton->GetName(), *RootLayerFile);
                return bSuccess;
            }
            else
            {
                FString AnimationName = Name.ToString();
                FString PkgPath = FPaths::Combine(ParentPath, AnimationName);
                auto ParentPackage = CreatePackage(*PkgPath);
                check(ParentPackage);
                ParentPackage->FullyLoad();

                // Check for an existing object
                UObject* ExistingObject = StaticFindObject(UObject::StaticClass(), ParentPackage, *AnimationName);

                if (ExistingObject != nullptr)
                {
                    EAppReturnType::Type UserResponse;
                    if (OverwriteStatus == 1)
                    {
                        UserResponse = EAppReturnType::YesAll;
                    }
                    else if (OverwriteStatus == 2)
                    {
                        UserResponse = EAppReturnType::NoAll;
                    }
                    else
                    {
                        UserResponse = FMessageDialog::Open(
                            EAppMsgType::YesNoYesAllNoAll,
                            FText::Format(LOCTEXT("ImportAssetAlreadyExists", "Do you want to overwrite the existing asset?\n\nAn asset already exists at the import location: {0}"), FText::FromString(PkgPath)));

                        OverwriteStatus = UserResponse == EAppReturnType::YesAll ? 1 : (UserResponse == EAppReturnType::NoAll ? 2 : 0);
                    }

                    if (UserResponse == EAppReturnType::No || UserResponse == EAppReturnType::NoAll)
                    {
                        return true;
                    }
                }

                UAnimSequence* Sequence = NewObject<UAnimSequence>(ParentPackage, *AnimationName, EObjectFlags::RF_Standalone | EObjectFlags::RF_Public);
                check(Sequence);

                double TimeCodesPerSecond = Stage->GetRootLayer()->HasTimeCodesPerSecond() ? Stage->GetRootLayer()->GetTimeCodesPerSecond() : DEFAULT_TIMECODESPERSECOND;
                double StartTimeCode = Stage->GetRootLayer()->HasStartTimeCode() ? Stage->GetRootLayer()->GetStartTimeCode() : DEFAULT_STARTTIMECODE;
                double EndTimeCode = Stage->GetRootLayer()->HasEndTimeCode() ? Stage->GetRootLayer()->GetEndTimeCode() : 0.0;

                Sequence->SetSkeleton(Skeleton);
                Sequence->ImportFileFramerate = TimeCodesPerSecond;
                Sequence->ImportResampleFramerate = TimeCodesPerSecond;
                double TotalTimeSample = EndTimeCode - StartTimeCode;
                int32 NumberOfFrame = FMath::CeilToInt(FMath::Max(TotalTimeSample + 1.0, 1.0));
                Sequence->SequenceLength = TotalTimeSample / TimeCodesPerSecond;
                Sequence->SetRawNumberOfFrame(NumberOfFrame);

                if (bFacial)
                {
                    const FSmartNameMapping* NameMapping = Skeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
                    if (!NameMapping)
                    {
                        return false;
                    }

                    for (int32 CurveIndex = 0; CurveIndex < CurveNames.Num(); ++CurveIndex)
                    {
                        FSmartName NewName;
                        Skeleton->AddSmartNameAndModify(USkeleton::AnimCurveMappingName, *CurveNames[CurveIndex], NewName);

                        check(Sequence->RawCurveData.AddCurveData(NewName));
                        FFloatCurve* NewCurve = static_cast<FFloatCurve*> (Sequence->RawCurveData.GetCurveData(NewName.UID, ERawCurveTrackTypes::RCT_Float));

                        for (auto CurveTimeSample : CurveValuesTimeSamples)
                        {
                            FKeyHandle NewKeyHandle = NewCurve->FloatCurve.AddKey(CurveTimeSample.Key / TimeCodesPerSecond, CurveTimeSample.Value[CurveIndex], false);

                            ERichCurveInterpMode NewInterpMode = RCIM_Cubic;
                            ERichCurveTangentMode NewTangentMode = RCTM_Auto;
                            ERichCurveTangentWeightMode NewTangentWeightMode = RCTWM_WeightedNone;

                            float LeaveTangent = 0.f;
                            float ArriveTangent = 0.f;
                            float LeaveTangentWeight = 0.f;
                            float ArriveTangentWeight = 0.f;

                            NewCurve->FloatCurve.SetKeyInterpMode(NewKeyHandle, NewInterpMode);
                            NewCurve->FloatCurve.SetKeyTangentMode(NewKeyHandle, NewTangentMode);
                            NewCurve->FloatCurve.SetKeyTangentWeightMode(NewKeyHandle, NewTangentWeightMode);
                        }
                    }

                    Sequence->RawCurveData.RemoveRedundantKeys();
                }
                else
                {
                    TArray<FRawAnimSequenceTrack> RawTracks;
                    RawTracks.SetNum(BoneNames.Num());

                    auto AddKeyToSequence = [&](pxr::UsdTimeCode TimeCode)
                    {
                        pxr::VtArray<pxr::GfMatrix4d> USDJointLocalTransforms;
                        AnimQuery.ComputeJointLocalTransforms(&USDJointLocalTransforms, TimeCode);

                        for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
                        {
                            FTransform LocalTransform = USDConvert(Stage, USDJointLocalTransforms[BoneIndex]);

                            RawTracks[BoneIndex].PosKeys.Add(LocalTransform.GetTranslation());
                            RawTracks[BoneIndex].RotKeys.Add(LocalTransform.GetRotation());
                            RawTracks[BoneIndex].ScaleKeys.Add(LocalTransform.GetScale3D());
                        }
                    };

                    if (TimeSamples.size() == 0)
                    {
                        AddKeyToSequence(pxr::UsdTimeCode::Default());
                    }
                    else
                    {
                        for (double TimeSample = 0.0; TimeSample <= TotalTimeSample; TimeSample += 1.0)
                        {
                            AddKeyToSequence(StartTimeCode + TimeSample);
                        }
                    }

                    for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
                    {
                        // add tracks for the bone existing
                        Sequence->AddNewRawTrack(BoneNames[BoneIndex], &RawTracks[BoneIndex]);
                    }
                }

                Sequence->MarkRawDataAsModified();
                Sequence->PostProcessSequence();
                Sequence->PostEditChange();
                Sequence->MarkPackageDirty();
                FAssetRegistryModule::AssetCreated(Sequence);
                UE_LOG(LogSimReadyImporter, Log, TEXT("Animation Sequence %s is imported."), *Sequence->GetName());
                return bSuccess;
            }
        }

        return false;
    }

    bool LoadUSDAnimationInternal(const FString& Path, const FString& DestinationPath, USkeleton* Skeleton, const FName Name, const EAnimationSource AnimationSource, TSet<FName>& ImportedAnimations, int32& Postfix)
    {
        pxr::UsdStageRefPtr USDStage = FSimReadyUSDHelper::LoadUSDStageFromPath(Path);
        if (USDStage)
        {
            //auto DefaultPrim = USDStage->GetDefaultPrim();
            //if (!DefaultPrim)
            //{
            //	DefaultPrim = USDStage->GetPseudoRoot();
            //}

            auto DefaultPrim = USDStage->GetPseudoRoot();

            TArray<pxr::UsdPrim> AnimationPrims;
            FindUsdSkelAnimation(DefaultPrim, AnimationPrims);

            if (AnimationPrims.Num() == 0)
            {
                UE_LOG(LogSimReadyImporter, Error, TEXT("Cannot find skeleton animation in %s"), *Path);
                return false;
            }

            bool bSuccess = false;
            int32 OverwriteStatus = 0;
            for (int32 AnimationPrimIndex = 0; AnimationPrimIndex < AnimationPrims.Num(); ++AnimationPrimIndex)
            {
                FName SeqName = Name;
                if (SeqName.IsNone())
                {
                    SeqName = AnimationPrims[AnimationPrimIndex].GetName().GetText();
                }
                else if (SeqName.IsEqual(TEXT("__FILENAME__")))
                {
                    SeqName = *FPaths::GetBaseFilename(Path);
                }

                while (ImportedAnimations.Find(SeqName) != nullptr)
                {
                    SeqName = *(SeqName.ToString() + FString::Printf(TEXT("_%d"), Postfix++));
                }

                if (ImportSkelAnimation(pxr::UsdSkelAnimation(AnimationPrims[AnimationPrimIndex]), DestinationPath, Skeleton, OverwriteStatus, *SanitizeObjectName(SeqName.ToString()), AnimationSource))
                {
                    ImportedAnimations.Add(SeqName);
                    bSuccess |= true;
                }
            }

            return bSuccess;
        }

        return false;
    }
}

TArray<FString> FSimReadyAnimationImporter::LoadUSDAnimation(const TArray<FString>& Paths, const FString& Dest, USkeleton* Skeleton, const FName Name, const EAnimationSource AnimationSource)
{
    TArray<FString> FailedFiles;
    TSet<FName> ImportedAnimations;
    int32 Postfix = 0;
    for (auto Path : Paths)
    {
        if (!SimReadyAnimationImporterImpl::LoadUSDAnimationInternal(Path, Dest, Skeleton, Name, AnimationSource, ImportedAnimations, Postfix))
        {
            FailedFiles.Add(Path);
        }
    }
    return FailedFiles;
}

bool FSimReadyAnimationImporter::LoadUSDAnimation(const FString& Path, const FString& DestinationPath, USkeleton* Skeleton, const FName Name, const EAnimationSource AnimationSource)
{
    TSet<FName> ImportedAnimations;
    int32 Postfix = 0;
    return SimReadyAnimationImporterImpl::LoadUSDAnimationInternal(Path, DestinationPath, Skeleton, Name, AnimationSource, ImportedAnimations, Postfix);
}

#undef LOCTEXT_NAMESPACE
