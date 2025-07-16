// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "Misc/FrameRate.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "MovieScene.h"
#include "Extractors/TimeSamplesData.h"


class FSimReadyUSDSequenceImporter
{
public:
    FSimReadyUSDSequenceImporter(TWeakObjectPtr<class ASimReadyStageActor> InStageActor);
    ~FSimReadyUSDSequenceImporter();

public:
    void CreateLevelSequence(const TOptional<double>& StartTimeCode, const TOptional<double>& EndTimeCode, const TOptional<double>& TimeCodesPerSecond);
    void CreateMaterialTrack(class UObject* InObject, int32 MaterialIndex, const struct FNamedParameterTimeSamples& ParameterTimeSamples);
    void CreateBoolTrack(class UObject* InObject, const FName& Name, const struct FBooleanTimeSamples& BooleanTimeSamples);
    void CreateFloatTrack(class UObject* InObject, const FName& Name, const struct FFloatTimeSamples& FloatTimeSamples);
    void CreateVectorTrack(class UObject* InObject, const FName& Name, const struct FVectorTimeSamples& VectorTimeSamples);
    void CreateColorTrack(class UObject* InObject, const FName& Name, const struct FColorTimeSamples& ColorTimeSamples);
    void CreateObjectTrack(class UObject* InObject, const struct FNamedParameterTimeSamples& ParameterTimeSamples);
    void CreateVisibilityTrack(class UObject* InObject, const struct FBooleanTimeSamples& VisibilityTimeSamples);
    void CreateSkeletalAnimationTrack(class UObject* InObject, class UAnimSequence* AnimSequence, int32 StartTimeCode);
    void RemoveEmptyLevelSequence();
    void ResetLevelSequence();
    void SetActorSequenceMode();
    void BuildActorSequence(class UBlueprint* Blueprint, const TArray<class AActor*>& Actors, const TOptional<double>& StartTimeCode, const TOptional<double>& EndTimeCode, const TOptional<double>& TimeCodesPerSecond);

    void CacheObjectTrack(class UObject* InObject, const struct FNamedParameterTimeSamples& ParameterTimeSamples);
    void CacheSkeletalAnimationTrack(class UObject* InObject, class UAnimSequence* AnimSequence, int32 StartTimeCode);
    void CacheVisibilityTrack(class UObject* InObject, const struct FBooleanTimeSamples& VisibilityTimeSamples);
    void CacheMaterialTrack(class UObject* InObject, int32 MaterialIndex, const struct FNamedParameterTimeSamples& ParameterTimeSamples);
    template <typename T>
    void CacheTransformTrack(class UObject* InObject, const T& TransformSamples)
    {
        if (ImporterMode != IM_ActorSequence)
        {
            return;
        }

        if (ActorSequence.IsValid())
        {
            return;
        }

        UObject* AnimedObject = InObject;
        if (InObject->IsA<AActor>())
        {
            AnimedObject = Cast<AActor>(InObject)->GetRootComponent();
        }
        auto TimeSamplesData = ActorSequenceTimeSamples.Find(AnimedObject);
        if (TimeSamplesData == nullptr)
        {
            FTimeSamplesData NewTimeSamplesData;
            NewTimeSamplesData.TransformTimeSamples = TransformSamples;
            ActorSequenceTimeSamples.Add(AnimedObject, NewTimeSamplesData);
        }
        else
        {
            TimeSamplesData->TransformTimeSamples = TransformSamples;
        }
    }

    template <typename T>
    void CreateTransformTrack(class UObject* InObject, const T& TransformSamples)
    {
        CacheTransformTrack(InObject, TransformSamples);

        if (!CheckValid())
        {
            return;
        }

        if (!TransformSamples.HasAnyTimeSamples())
        {
            return;
        }

        UMovieScene3DTransformTrack* TransformTrack = CreateTrack<UMovieScene3DTransformTrack>(InObject);

        if (!TransformTrack)
        {
            return;
        }

        // Transform section
        auto TransformSection = Cast<UMovieScene3DTransformSection>(TransformTrack->CreateNewSection());
        TransformTrack->AddSection(*TransformSection);
        TransformSection->SetRange(TRange<FFrameNumber>::All());

        TArrayView<FMovieSceneFloatChannel*> FloatChannels = TransformSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();

        FFrameRate FrameRate = TransformSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
        FFrameRate DisplayRate = TransformSection->GetTypedOuter<UMovieScene>()->GetDisplayRate();

        auto TraverseTranslateCallback = [&](int32 TimeCode, const FVector& TimeSample)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
            FloatChannels[0]->AddLinearKey(FrameNumber, TimeSample.X);
            FloatChannels[1]->AddLinearKey(FrameNumber, TimeSample.Y);
            FloatChannels[2]->AddLinearKey(FrameNumber, TimeSample.Z);
        };
        TransformSamples.TraverseTranslate(TraverseTranslateCallback);

        auto TraverseRotateCallback = [&](int32 TimeCode, const FVector& TimeSample)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
            FloatChannels[3]->AddLinearKey(FrameNumber, TimeSample.X);
            FloatChannels[4]->AddLinearKey(FrameNumber, TimeSample.Y);
            FloatChannels[5]->AddLinearKey(FrameNumber, TimeSample.Z);
        };
        TransformSamples.TraverseRotate(TraverseRotateCallback);

        auto TraverseScaleCallback = [&](int32 TimeCode, const FVector& TimeSample)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
            FloatChannels[6]->AddLinearKey(FrameNumber, TimeSample.X);
            FloatChannels[7]->AddLinearKey(FrameNumber, TimeSample.Y);
            FloatChannels[8]->AddLinearKey(FrameNumber, TimeSample.Z);
        };
        TransformSamples.TraverseScale(TraverseScaleCallback);
    }

    bool IsPlayingSequence();

private:
    bool CheckValid(); 
    FGuid FindOrBindPossessable(class UObject* InObject, class UObject* BindingContext);
    FGuid FindOrBindObject(class UObject* InObject);
    class UMovieSceneSequence* GetActiveSequence();

    template <typename T>
    T* CreateTrack(class UObject* InObject, const FName& TrackName = NAME_None)
    {
        T* Track = nullptr;

        FGuid Binding = FindOrBindObject(InObject);

        if (Binding.IsValid())
        {
            UMovieScene* MovieScene = GetActiveSequence()->GetMovieScene();

            // Create Track
            Track = MovieScene->FindTrack<T>(Binding, TrackName);
            if (Track)
            {
                Track->RemoveAllAnimationData();
            }
            else
            {
                Track = MovieScene->AddTrack<T>(Binding);
            }
        }

        return Track;
    }

    void OnSequencerCreated(TSharedRef<class ISequencer> InSequencer);
    void OnSequencerClosed(TSharedRef<class ISequencer> InSequencer);

    FORCEINLINE FFrameNumber RoundToFrameNumber(const FFrameRate& FrameRate, double TimeSeconds)
    {
        return FFrameNumber(static_cast<int32>(FMath::RoundToDouble((TimeSeconds * FrameRate.Numerator) / FrameRate.Denominator)));
    }

    enum EImporterMode
    {
        IM_LevelSequence,
        IM_ActorSequence,
    };

    TWeakObjectPtr<class ASimReadyStageActor> StageActor;
    TWeakPtr<ISequencer> StageSequencer;
    FDelegateHandle OnSequencerCreatedHandle;
    FDelegateHandle OnCloseEventHandle;
    TWeakObjectPtr<class ULevelSequence> LevelSequence;
    TWeakObjectPtr<class UActorSequence> ActorSequence;
    TMap<class UObject*, struct FTimeSamplesData> ActorSequenceTimeSamples;
    EImporterMode ImporterMode = EImporterMode::IM_LevelSequence;
};