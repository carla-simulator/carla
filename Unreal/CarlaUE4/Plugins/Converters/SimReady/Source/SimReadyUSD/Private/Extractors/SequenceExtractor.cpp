// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SequenceExtractor.h"
#include "LevelSequenceActor.h"
#include "TrackExtractor.h"
#include "TrackExtractorFactory.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSequence.h"
#include "MovieSceneCommonHelpers.h"
#include "Sections/MovieSceneCameraCutSection.h"

FSequenceExtractor& FSequenceExtractor::Get()
{
    static FSequenceExtractor Singleton;
    return Singleton;
}

bool FSequenceExtractor::SetData(UWorld* InWorldContext, const TArray<ALevelSequenceActor*>& LevelSequenceActors)
{
    ClearData();
    WorldContext = InWorldContext == nullptr ? GWorld : InWorldContext;

    // Get global TimeCodesPerSecond
    TArray<UMovieSceneSequence*> ValidSequences;
    for (auto LevelSequenceActor : LevelSequenceActors)
    {
        // if there're more than one sequence to export, checking bAutoPlay flag to decide which to export
        if (LevelSequenceActors.Num() > 1 && !LevelSequenceActor->PlaybackSettings.bAutoPlay)
        {
            continue;
        }

        ULevelSequence* LevelSequence = LevelSequenceActor->GetSequence();
        if (!LevelSequence)
        {
            LevelSequence = LevelSequenceActor->LoadSequence();
            if (!LevelSequence)
            {
                continue;
            }
        }

        UMovieScene* MovieScene = LevelSequence->GetMovieScene();
        if (!MovieScene)
        {
            continue;
        }

        FFrameRate DisplayRate = MovieScene->GetDisplayRate();

        if (DisplayRate.AsDecimal() > MasterTimeCodesPerSecond)
        {
            MasterTimeCodesPerSecond = DisplayRate.AsDecimal();
        }

        ValidSequences.Add(LevelSequence);
    }

    SetDataInternal(ValidSequences);

    return ValidSequences.Num() > 0;
}

bool FSequenceExtractor::SetData(UWorld* InWorldContext, const TArray<UMovieSceneSequence*>& Sequences)
{
    ClearData();
    WorldContext = InWorldContext == nullptr ? GWorld : InWorldContext;

    for (auto Sequence : Sequences)
    {
        UMovieScene* MovieScene = Sequence->GetMovieScene();
        if (!MovieScene)
        {
            continue;
        }

        FFrameRate DisplayRate = MovieScene->GetDisplayRate();

        if (DisplayRate.AsDecimal() > MasterTimeCodesPerSecond)
        {
            MasterTimeCodesPerSecond = DisplayRate.AsDecimal();
        }
    }

    SetDataInternal(Sequences);
    
    return Sequences.Num() > 0;
}

void FSequenceExtractor::SetDataInternal(const TArray<UMovieSceneSequence*>& Sequences)
{
    for (auto Sequence : Sequences)
    {
        auto MovieScene = Sequence->GetMovieScene();
        if (!MovieScene)
        {
            continue;
        }

        TRange<FFrameNumber> TimeRange = MovieScene->GetPlaybackRange();
        FFrameRate FrameRate = MovieScene->GetTickResolution();

        if (TimeRange.GetUpperBound().IsClosed() && TimeRange.GetLowerBound().IsClosed())
        {
            int32 UpperValue = TimeRange.GetUpperBoundValue().Value;
            int32 LowerValue = TimeRange.GetLowerBoundValue().Value;
            double CurrentStartTimeCode = FMath::RoundToDouble(LowerValue * FrameRate.AsInterval() * MasterTimeCodesPerSecond);
            if (TimeRange.GetLowerBound().IsExclusive())
            {
                CurrentStartTimeCode += 1;
            }
            double CurrentEndTimeCode = FMath::RoundToDouble(UpperValue * FrameRate.AsInterval() * MasterTimeCodesPerSecond);
            if (TimeRange.GetUpperBound().IsExclusive())
            {
                CurrentEndTimeCode -= 1;
            }

            if (!MasterStartTimeCode.IsSet() || !MasterEndTimeCode.IsSet())
            {
                MasterStartTimeCode = FMath::Max(CurrentStartTimeCode, 0.0);
                MasterEndTimeCode = FMath::Max(CurrentEndTimeCode, 0.0);
            }

            if (CurrentStartTimeCode >= 0.0 && CurrentStartTimeCode < MasterStartTimeCode.GetValue())
            {
                MasterStartTimeCode = CurrentStartTimeCode;
            }

            if (CurrentEndTimeCode >= 0.0 && CurrentEndTimeCode > MasterEndTimeCode.GetValue())
            {
                MasterEndTimeCode = CurrentEndTimeCode;
            }

            Extract(Sequence, CurrentStartTimeCode, CurrentEndTimeCode, 0.0, 1.0f, ETrackBlendMode::Keep);

            PostExtract();
        }
    }
}

void FSequenceExtractor::ClearData()
{
    ObjectsTimeSamplesData.Empty();
    SpawnedObjectsLUT.Empty();
    if (WorldContext)
    {
        for (auto Obj : SpawnedActors)
        {
            if (Obj->IsA<AActor>())
            {
                WorldContext->DestroyActor(Cast<AActor>(Obj));
            }
        }
    }
    SpawnedActors.Empty();
    CurrentSequenceSpawnedActors.Empty();
    MasterTimeCodesPerSecond = 0.0;
    MasterStartTimeCode.Reset();
    MasterEndTimeCode.Reset();

    PriorityTimeline.Empty();
}

void FSequenceExtractor::PostExtract()
{
    for (auto& ObjectSet : ObjectsTimeSamplesData)
    {
        if (ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Num() > 0)
        {
            ObjectSet.Value.VisibilityTimeSamples.TimeSamples.KeySort([](int32 A, int32 B) {
                return A < B; // sort keys in order
            });
        }
    }
}

void FSequenceExtractor::GetSequenceObjects(TArray<UObject*>& OutObjects)
{
    ObjectsTimeSamplesData.GenerateKeyArray(OutObjects);
}

void FSequenceExtractor::GetSpawnableObjects(TArray<class UObject*>& OutObjects)
{
    OutObjects = SpawnedActors;
}

UObject* FSequenceExtractor::SpawnObject(FMovieSceneSpawnable& Spawnable)
{
    AActor* ObjectTemplate = Cast<AActor>(Spawnable.GetObjectTemplate());
    if (!ObjectTemplate)
    {
        return nullptr;
    }

    const EObjectFlags ObjectFlags = RF_Transient | RF_Transactional;
    FName SpawnName = MakeUniqueObjectName(WorldContext->PersistentLevel, ObjectTemplate->GetClass(), *Spawnable.GetName());

    FActorSpawnParameters SpawnInfo;
    {
        SpawnInfo.Name = SpawnName;
        SpawnInfo.ObjectFlags = ObjectFlags;
        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnInfo.bDeferConstruction = true;
        SpawnInfo.Template = ObjectTemplate;
        SpawnInfo.OverrideLevel = WorldContext->PersistentLevel;
    }

    ObjectTemplate->DestroyConstructedComponents();

    FTransform SpawnTransform;

    if (USceneComponent* RootComponent = ObjectTemplate->GetRootComponent())
    {
        SpawnTransform.SetTranslation(RootComponent->GetRelativeLocation());
        SpawnTransform.SetRotation(RootComponent->GetRelativeRotation().Quaternion());
    }
    else
    {
        SpawnTransform = Spawnable.SpawnTransform;
    }

    AActor* SpawnedActor = WorldContext->SpawnActorAbsolute(ObjectTemplate->GetClass(), SpawnTransform, SpawnInfo);
    if (!SpawnedActor)
    {
        return nullptr;
    }

    const bool bIsDefaultTransform = true;
    SpawnedActor->FinishSpawning(SpawnTransform, bIsDefaultTransform);
    SpawnedActor->SetActorLabel(Spawnable.GetName());

    return SpawnedActor;
}

void FSequenceExtractor::Extract(UMovieSceneSequence* Sequence, const double InStartTimeCode, const double InEndTimeCode, const double StartOffset, const float TimeScale, ETrackBlendMode::Type BlendMode)
{
    CurrentSequenceSpawnedActors.Empty();
    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return;
    }

    TArray<FString>& MuteNodes = MovieScene->GetMuteNodes();

    TRange<FFrameNumber> TimeRange = MovieScene->GetPlaybackRange();
    int32 UpperValue = TimeRange.GetUpperBoundValue().Value;
    int32 LowerValue = TimeRange.GetLowerBoundValue().Value;
    FFrameRate FrameRate = MovieScene->GetTickResolution();
    double InternalStartTimeCode = FMath::RoundToDouble(LowerValue * FrameRate.AsInterval() * MasterTimeCodesPerSecond);
    if (TimeRange.GetLowerBound().IsExclusive())
    {
        InternalStartTimeCode += 1;
    }

    if (InStartTimeCode > MasterEndTimeCode.GetValue() || InEndTimeCode < MasterStartTimeCode.GetValue())
    {
        return;
    }

    // Clip to the positivie start
    double NewStartTimeCode = InStartTimeCode < 0.0 ? 0.0 : InStartTimeCode;
    InternalStartTimeCode = InStartTimeCode < 0.0 ? InternalStartTimeCode + FMath::Abs(InStartTimeCode) : InternalStartTimeCode;

    FExtractorInput Input = { MasterTimeCodesPerSecond, NewStartTimeCode, InEndTimeCode, StartOffset, InternalStartTimeCode, TimeScale };

    for (UMovieSceneTrack* SceneTrack : MovieScene->GetMasterTracks())
    {
        // Track is muted.
        if (MuteNodes.Find(SceneTrack->GetName()) != INDEX_NONE)
        {
            continue;
        }

        TSharedPtr<FTrackExtractor> Extractor = MakeShareable<FTrackExtractor>(FTrackExtractorFactory::Get().CreateExtractor(SceneTrack));
        if (Extractor)
        {
            Extractor->Extract(Input);
        }
    }

    auto GetActorByPossessable = [&](const FGuid& Guid)
    {
        TArray<UObject*, TInlineAllocator<1>> OutObjects;

        Sequence->LocateBoundObjects(Guid, nullptr, OutObjects);

        // Get Object
        AActor* SequenceActor = nullptr;
        for (auto Obj : OutObjects)
        {
            SequenceActor = Cast<AActor>(Obj);
            if (SequenceActor)
            {
                break;
            }
        }

        return SequenceActor;
    };

    auto GetComponentByPossessable = [&](const FGuid& ParentGuid, const FGuid& Guid)
    {
        UActorComponent* SequenceComponent = nullptr;

        // Get Parent
        AActor* SequenceActor = GetActorByPossessable(ParentGuid);

        if (SequenceActor)
        {
            TArray<UObject*, TInlineAllocator<1>> OutObjects;
            Sequence->LocateBoundObjects(Guid, SequenceActor, OutObjects);

            for (auto Obj : OutObjects)
            {
                SequenceComponent = Cast<UActorComponent>(Obj);
                if (SequenceComponent)
                {
                    break;
                }
            }
        }

        return SequenceComponent;
    };

    auto ExtractTracks = [&](const FGuid& Guid, const FString& NodeNameHeader, UObject* Object)
    {
        auto ObjectBinding = MovieScene->FindBinding(Guid);
        if (ObjectBinding == nullptr)
        {
            return;
        }

        if (ObjectBinding->GetTracks().Num() > 0)
        {
            FTimeSamplesData TimeSamplesData;
            for (const UMovieSceneTrack* Track : ObjectBinding->GetTracks())
            {
                FString NodeName = NodeNameHeader + Track->GetName();

                // Track is muted.
                if (MuteNodes.Find(NodeName) != INDEX_NONE)
                {
                    continue;
                }

                TSharedPtr<FTrackExtractor> Extractor = MakeShareable<FTrackExtractor>(FTrackExtractorFactory::Get().CreateExtractor(Track));
                if (Extractor)
                {
                    Extractor->Extract(Input, &TimeSamplesData);
                }
            }

            if (TimeSamplesData.ObjectBindingTimeSamples.Num() > 0)
            {
                FObjectBindingTimeSamples& ObjectBindingTimeSamples = TimeSamplesData.ObjectBindingTimeSamples[0];
                auto BindingGuid = ObjectBindingTimeSamples.BindingObjectGuid;
                if (BindingGuid.IsValid())
                {
                    AActor* BindingActor = GetActorByPossessable(BindingGuid);
                    if (BindingActor == nullptr)
                    {
                        UObject** SpawnedObject = SpawnedObjectsLUT.Find(BindingGuid);
                        if (SpawnedObject)
                        {
                            BindingActor = Cast<AActor>(*SpawnedObject);
                        }
                    }
                    ObjectBindingTimeSamples.BindingObject = BindingActor;
                }
            }

            // USD doesn't have spawn behavior, using visible instead.
            if (TimeSamplesData.SpawnTimeSamples.TimeSamples.Num() > 0 && TimeSamplesData.VisibilityTimeSamples.TimeSamples.Num() == 0)
            {
                TimeSamplesData.VisibilityTimeSamples = TimeSamplesData.SpawnTimeSamples;
            }

            EmplaceTimeSamples(Object, TimeSamplesData, BlendMode);
        }
    };

    for (int32 SpawnableIndex = 0; SpawnableIndex < MovieScene->GetSpawnableCount(); ++SpawnableIndex)
    {
        auto Spawnable = MovieScene->GetSpawnable(SpawnableIndex);

        // Try spawning actor
        AActor* ObjectTemplate = Cast<AActor>(Spawnable.GetObjectTemplate());
        if (!ObjectTemplate)
        {
            continue;
        }

        AActor* SpawnedActor = Cast<AActor>(SpawnObject(Spawnable));
        if (!SpawnedActor)
        {
            continue;
        }

        SpawnedObjectsLUT.Add(Spawnable.GetGuid(), SpawnedActor); //NOTE: Object could be overwritten
        SpawnedActors.AddUnique(SpawnedActor);
        CurrentSequenceSpawnedActors.AddUnique(SpawnedActor);
        for( auto ChildGuid : Spawnable.GetChildPossessables())
        {
            auto Possessable = MovieScene->FindPossessable(ChildGuid);
            if (Possessable)
            {
                UObject* PossessableObject = SpawnedActor->GetDefaultSubobjectByName(*Possessable->GetName());
                if (PossessableObject)
                {
                    SpawnedObjectsLUT.Add(ChildGuid, PossessableObject);
                }
            }
        }

        FString SpawnableNodeName;
        if (Spawnable.GetGuid().IsValid())
        {
            // Full Track is muted.
            if (MuteNodes.Find(Spawnable.GetGuid().ToString()) != INDEX_NONE)
            {
                continue;
            }

            SpawnableNodeName += Spawnable.GetGuid().ToString() + TEXT(".");
        }

        ExtractTracks(Spawnable.GetGuid(), SpawnableNodeName, Cast<UObject>(SpawnedActor));
    }

    // Get Actor and components animation
    for (int32 PossessableIndex = 0; PossessableIndex < MovieScene->GetPossessableCount(); ++PossessableIndex)
    {
        auto Possessable = MovieScene->GetPossessable(PossessableIndex);

        // Get Object
        AActor* SequenceActor = GetActorByPossessable(Possessable.GetGuid());
        UActorComponent* SequenceComp = nullptr;
        if (SequenceActor == nullptr)
        {
            SequenceComp = GetComponentByPossessable(Possessable.GetParent(), Possessable.GetGuid());
        }

        // Check from spawnable
        if (SequenceComp == nullptr)
        {
            UObject** Object = SpawnedObjectsLUT.Find(Possessable.GetGuid());
            if (Object)
            {
                SequenceActor = Cast<AActor>(*Object);
                SequenceComp = Cast<UActorComponent>(*Object);
            }
        }

        if (SequenceActor == nullptr && SequenceComp == nullptr)
        {
            continue;
        }


        FString PossessableNodeName;
        if (Possessable.GetParent().IsValid())
        {
            PossessableNodeName += Possessable.GetParent().ToString() + TEXT(".");
        }
        if (Possessable.GetGuid().IsValid())
        {
            // Full Track is muted.
            if (MuteNodes.Find(Possessable.GetGuid().ToString()) != INDEX_NONE)
            {
                continue;
            }

            PossessableNodeName += Possessable.GetGuid().ToString() + TEXT(".");
        }

        ExtractTracks(Possessable.GetGuid(), PossessableNodeName, SequenceActor != nullptr ? Cast<UObject>(SequenceActor) : Cast<UObject>(SequenceComp));
    }

    UMovieSceneTrack* CameraCutTrack = MovieScene->GetCameraCutTrack();
    if (CameraCutTrack)
    {
        const TArray<UMovieSceneSection*>& Sections = CameraCutTrack->GetAllSections();
        for (auto Section : Sections)
        {
            if (Section->IsA<UMovieSceneCameraCutSection>())
            {
                UMovieSceneCameraCutSection* CameraCutSection = Cast<UMovieSceneCameraCutSection>(Section);
                FGuid CameraGuid = CameraCutSection->GetCameraBindingID().GetGuid();
                AActor* CameraActor = GetActorByPossessable(CameraGuid);
                if (CameraActor == nullptr)
                {
                    UObject** SpawnedObject = SpawnedObjectsLUT.Find(CameraGuid);
                    if (SpawnedObject)
                    {
                        CameraActor = Cast<AActor>(*SpawnedObject);
                    }
                }

                if (CameraActor)
                {
                    TRange<FFrameNumber> Range = CameraCutSection->GetTrueRange();
                    auto ObjectTimeSamplesData = ObjectsTimeSamplesData.Find(CameraActor);
                    FTimeSampleRange CameraShotRange(
                        FMath::CeilToDouble(Range.GetLowerBoundValue().Value * (FrameRate.AsInterval() * Input.TimeCodesPerSecond)) - (InternalStartTimeCode + StartOffset) + NewStartTimeCode,
                        FMath::FloorToDouble((Range.GetUpperBoundValue().Value - 1) * (FrameRate.AsInterval() * Input.TimeCodesPerSecond)) - (InternalStartTimeCode + StartOffset) + NewStartTimeCode);
                    if (!CameraShotRange.IsInteractWithRange(FTimeSampleRange(NewStartTimeCode, InEndTimeCode)))
                    {
                        continue;
                    }
                    CameraShotRange.ClampToRange(FTimeSampleRange(NewStartTimeCode, InEndTimeCode));

                    UCameraComponent* CameraComponent = MovieSceneHelpers::CameraComponentFromRuntimeObject(CameraActor);
                    // Check the existed Shots, they have the highest priority
                    TArray<FTimeSampleRange> ClippedCameraRanges;
                    ClippedCameraRanges.Add(CameraShotRange);
                    for (auto Pair : ObjectsTimeSamplesData)
                    {
                        TArray<FTimeSampleRange> ClippedResults;
                        if (CameraComponent)
                        {				
                            for (auto PriorRange : Pair.Value.CameraShotsTimeSamples)
                            {
                                for (auto CameraRange : ClippedCameraRanges)
                                {
                                    ClippedResults.Append(CameraRange.ClipRange(PriorRange));
                                }
                            }
                        }
                        else
                        {
                            for (auto PriorShot : Pair.Value.ViewportShotsTimeSamples)
                            {
                                for (auto CameraRange : ClippedCameraRanges)
                                {
                                    ClippedResults.Append(CameraRange.ClipRange(PriorShot));
                                }
                            }
                        }

                        if (ClippedResults.Num() > 0)
                        {
                            ClippedCameraRanges = ClippedResults;
                        }
                    }

                    for (auto ClippedRange : ClippedCameraRanges)
                    {
                        if (ObjectTimeSamplesData != nullptr)
                        {
                            if (CameraComponent)
                            {
                                const uint32 Index = ObjectTimeSamplesData->CameraShotsTimeSamples.AddUninitialized();
                                ObjectTimeSamplesData->CameraShotsTimeSamples[Index] = ClippedRange;
                            }
                            else
                            {
                                const uint32 Index = ObjectTimeSamplesData->ViewportShotsTimeSamples.AddUninitialized();
                                ObjectTimeSamplesData->ViewportShotsTimeSamples[Index] = ClippedRange;
                            }
                        }
                        else
                        {
                            FTimeSamplesData TimeSamplesData;
                            if (CameraComponent)
                            {
                                const uint32 Index = TimeSamplesData.CameraShotsTimeSamples.AddUninitialized();
                                TimeSamplesData.CameraShotsTimeSamples[Index] = ClippedRange;
                            }
                            else
                            {
                                const uint32 Index = TimeSamplesData.ViewportShotsTimeSamples.AddUninitialized();
                                TimeSamplesData.ViewportShotsTimeSamples[Index] = ClippedRange;
                            }
                            ObjectsTimeSamplesData.Add(CameraActor, TimeSamplesData);
                        }
                    }
                }
            }
        }
    }
}

void FSequenceExtractor::RestoreState(const double StartTimeCode, const double EndTimeCode)
{
    for (auto& ObjectSet : ObjectsTimeSamplesData)
    {
        if (ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Num() > 0)
        {
            // Check if it's spawned component
            bool IsSpawnedComponent = false;
            if (ObjectSet.Key->IsA<UActorComponent>())
            {
                UActorComponent* Component = Cast<UActorComponent>(ObjectSet.Key);
                IsSpawnedComponent = (SpawnedActors.Find(Component->GetOwner()) != INDEX_NONE);
            }

            if (!IsSpawnedComponent && SpawnedActors.Find(ObjectSet.Key) == INDEX_NONE) // only expand visibility for non-spawned objects
            {
                if (StartTimeCode > MasterStartTimeCode.GetValue())
                {
                    if (ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Find(StartTimeCode - 1) == nullptr)
                    {
                        ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Add(StartTimeCode - 1, true);
                    }
                }

                if (EndTimeCode < MasterEndTimeCode.GetValue())
                {
                    if (ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Find(EndTimeCode + 1) == nullptr)
                    {
                        ObjectSet.Value.VisibilityTimeSamples.TimeSamples.Add(EndTimeCode + 1, true);
                    }
                }
            }
        }
    }

    // Clear spawned actors for next sequence
    for (auto SpawnedActor : CurrentSequenceSpawnedActors)
    {
        auto TimeSamplesData = ObjectsTimeSamplesData.Find(SpawnedActor);

        if (TimeSamplesData)
        {
            if (StartTimeCode > MasterStartTimeCode.GetValue())
            {
                auto FoundStart = TimeSamplesData->VisibilityTimeSamples.TimeSamples.Find(StartTimeCode - 1);
                if (FoundStart)
                {
                    *FoundStart = false;
                }
                else
                {
                    TimeSamplesData->VisibilityTimeSamples.TimeSamples.Add(StartTimeCode - 1, false);
                }
            }

            if (EndTimeCode < MasterEndTimeCode.GetValue())
            {
                auto FoundEnd = TimeSamplesData->VisibilityTimeSamples.TimeSamples.Find(EndTimeCode + 1);
                if (FoundEnd)
                {
                    *FoundEnd = false;
                }
                else
                {
                    TimeSamplesData->VisibilityTimeSamples.TimeSamples.Add(EndTimeCode + 1, false);
                }
            }
        }
    }
}

const FTranslateRotateScaleTimeSamples* FSequenceExtractor::GetTransformTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }
    
    if (!TimeSamplesData->TransformTimeSamples.HasAnyTimeSamples())
    {
        return nullptr;
    }

    return &TimeSamplesData->TransformTimeSamples;
}

const FBooleanTimeSamples* FSequenceExtractor::GetVisibilityTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (TimeSamplesData->VisibilityTimeSamples.TimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return &TimeSamplesData->VisibilityTimeSamples;
}

const FNamedParameterTimeSamples* FSequenceExtractor::GetParameterTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (!TimeSamplesData->ParametersTimeSamples.HasAnyParameter())
    {
        return nullptr;
    }

    return &TimeSamplesData->ParametersTimeSamples;
}

const FMaterialTimeSamples* FSequenceExtractor::GetMaterialTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (TimeSamplesData->MaterialsTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return &TimeSamplesData->MaterialsTimeSamples;
}

const TArray<FObjectBindingTimeSamples>* FSequenceExtractor::GetObjectBindingTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (TimeSamplesData->ObjectBindingTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return &TimeSamplesData->ObjectBindingTimeSamples;
}

const TArray<FTimeSampleRange>* FSequenceExtractor::GetCameraShotsTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (TimeSamplesData->CameraShotsTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return &TimeSamplesData->CameraShotsTimeSamples;
}

const FTransformTimeSamples* FSequenceExtractor::GetSocketTimeSamples(const UObject* InObject, const FName& SocketName)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        return nullptr;
    }

    if (TimeSamplesData->SocketTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return TimeSamplesData->SocketTimeSamples.Find(SocketName);
}

const TArray<FSkeletalAnimationTimeSamples>* FSequenceExtractor::GetSkeletalAnimationTimeSamples(const UObject* InObject)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr || TimeSamplesData->SkeletalAnimationTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    if (TimeSamplesData->SkeletalAnimationTimeSamples.Num() == 0)
    {
        return nullptr;
    }

    return &TimeSamplesData->SkeletalAnimationTimeSamples;
}


void FSequenceExtractor::SetTransformTimeSamples(UObject* InObject, const FTranslateRotateScaleTimeSamples& TransformTimeSamples)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        FTimeSamplesData NewTimeSamplesData;
        NewTimeSamplesData.TransformTimeSamples = TransformTimeSamples;
        ObjectsTimeSamplesData.Add(InObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->TransformTimeSamples = TransformTimeSamples;
    }
}

void FSequenceExtractor::SetSocketTimeSamples(UObject* InObject, const FName& SocketName, const FTransformTimeSamples& TransformTimeSamples)
{
    auto TimeSamplesData = ObjectsTimeSamplesData.Find(InObject);

    if (TimeSamplesData == nullptr)
    {
        FTimeSamplesData NewTimeSamplesData;
        NewTimeSamplesData.SocketTimeSamples.Add(SocketName, TransformTimeSamples);
        ObjectsTimeSamplesData.Add(InObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->SocketTimeSamples.FindOrAdd(SocketName, TransformTimeSamples);
    }
}

void FSequenceExtractor::SetViewportShotsTimeSamples(FViewportShotCallback InCallback)
{
    if (InCallback)
    {
        for (auto TimeSamplesData : ObjectsTimeSamplesData)
        {
            if (TimeSamplesData.Value.ViewportShotsTimeSamples.Num() > 0)
            {
                InCallback(TimeSamplesData.Key, TimeSamplesData.Value.ViewportShotsTimeSamples);
            }
        }
    }
}

void FSequenceExtractor::EmplaceTimeSamples(UObject* SequenceObject, const FTimeSamplesData& TimeSamplesData, ETrackBlendMode::Type BlendMode)
{
    auto InsertData = [](TMap<int32, FVector>& Dest, const TMap<int32, FVector>& Src, int32 StartTimeCode, int32 EndTimeCode)
    {
        for (auto Pair : Src)
        {
            if (Pair.Key > EndTimeCode || Pair.Key < StartTimeCode)
            {
                Dest.Add(Pair);
            }
        }
    };

    auto BlendData = [](TMap<int32, FVector>& Dest, const TMap<int32, FVector>& Src, float Weight, bool bRotator = false)
    {
        TMap<int32, FVector> Final;
        TArray<int32> DestKeys;
        Dest.GenerateKeyArray(DestKeys);
        DestKeys.Sort();

        TArray<int32> SrcKeys;
        Src.GenerateKeyArray(SrcKeys);
        SrcKeys.Sort();

        int32 SrcIndex = 0;
        int32 DestIndex = 0;
        while (SrcIndex < SrcKeys.Num() && DestIndex < DestKeys.Num())
        {
            if (SrcKeys[SrcIndex] < DestKeys[DestIndex])
            {
                Final.Add(SrcKeys[SrcIndex], *Src.Find(SrcKeys[SrcIndex]));
                ++SrcIndex;
            }
            else if (SrcKeys[SrcIndex] > DestKeys[DestIndex])
            {
                Final.Add(DestKeys[DestIndex], *Dest.Find(DestKeys[DestIndex]));
                ++DestIndex;
            }
            else // equal
            {
                FVector SrcValue = *Src.Find(SrcKeys[SrcIndex]);
                FVector DestValue = *Dest.Find(DestKeys[DestIndex]);

                if (bRotator)
                {
                    FRotator LerpRotate = FMath::Lerp(FRotator(DestValue.Y, DestValue.Z, DestValue.X), FRotator(SrcValue.Y, SrcValue.Z, SrcValue.X), Weight);
                    Final.Add(SrcKeys[SrcIndex], FVector(LerpRotate.Roll, LerpRotate.Pitch, LerpRotate.Yaw));
                }
                else
                {
                    Final.Add(SrcKeys[SrcIndex], FMath::Lerp(DestValue, SrcValue, Weight));
                }
                ++SrcIndex;
                ++DestIndex;
            }
        }

        while (SrcIndex < SrcKeys.Num())
        {
            Final.Add(SrcKeys[SrcIndex], *Src.Find(SrcKeys[SrcIndex]));
            ++SrcIndex;
        }

        while (DestIndex < DestKeys.Num())
        {
            Final.Add(DestKeys[DestIndex], *Dest.Find(DestKeys[DestIndex]));
            ++DestIndex;
        }

        Dest = Final;
    };

    auto GetStartAndEndTime = [](const TMap<int32, FVector>& Src, TOptional<int32>& StartTime, TOptional<int32>& EndTime)
    {
        TArray<int32> Keys;
        Src.GenerateKeyArray(Keys);
        Keys.Sort();

        if (Keys.Num() > 0)
        {
            if(StartTime.IsSet())
            {
                StartTime = FMath::Min(StartTime.GetValue(), Keys[0]);
            }
            else
            {
                StartTime = Keys[0];
            }
            
            int32 Last = Keys.Num() - 1;
            if (EndTime.IsSet())
            {
                EndTime = FMath::Max(EndTime.GetValue(), Keys[Last]);
            }
            else
            {
                EndTime = Keys[Last];
            }
        }
    };

    auto OverwriteData = [](TMap<int32, FVector>& Dest, const TMap<int32, FVector>& Src)
    {
        Dest.Append(Src);
    };

    auto ObjectTimeSamplesData = ObjectsTimeSamplesData.Find(SequenceObject);
    if (ObjectTimeSamplesData != nullptr)
    {
        switch (BlendMode)
        {
        case ETrackBlendMode::Accumulate:
            {
                float Weight = 1.0f / (1.0f / ObjectTimeSamplesData->Weight + 1.0f);
                BlendData(ObjectTimeSamplesData->TransformTimeSamples.Translate, TimeSamplesData.TransformTimeSamples.Translate, Weight);
                BlendData(ObjectTimeSamplesData->TransformTimeSamples.Rotate, TimeSamplesData.TransformTimeSamples.Rotate, Weight, true);
                BlendData(ObjectTimeSamplesData->TransformTimeSamples.Scale, TimeSamplesData.TransformTimeSamples.Scale, Weight);
                ObjectTimeSamplesData->Weight = Weight;
                break;
            }
        case ETrackBlendMode::Keep:
            {
                TOptional<int32> StartTime;
                TOptional<int32> EndTime;
                GetStartAndEndTime(ObjectTimeSamplesData->TransformTimeSamples.Translate, StartTime, EndTime);
                GetStartAndEndTime(ObjectTimeSamplesData->TransformTimeSamples.Rotate, StartTime, EndTime);
                GetStartAndEndTime(ObjectTimeSamplesData->TransformTimeSamples.Scale, StartTime, EndTime);
                if (StartTime.IsSet() && EndTime.IsSet())
                {
                    InsertData(ObjectTimeSamplesData->TransformTimeSamples.Translate, TimeSamplesData.TransformTimeSamples.Translate, StartTime.GetValue(), EndTime.GetValue());
                    InsertData(ObjectTimeSamplesData->TransformTimeSamples.Rotate, TimeSamplesData.TransformTimeSamples.Rotate, StartTime.GetValue(), EndTime.GetValue());
                    InsertData(ObjectTimeSamplesData->TransformTimeSamples.Scale, TimeSamplesData.TransformTimeSamples.Scale, StartTime.GetValue(), EndTime.GetValue());
                }
                break;
            }
        case ETrackBlendMode::Overwrite:
            {
                OverwriteData(ObjectTimeSamplesData->TransformTimeSamples.Translate, TimeSamplesData.TransformTimeSamples.Translate);
                OverwriteData(ObjectTimeSamplesData->TransformTimeSamples.Rotate, TimeSamplesData.TransformTimeSamples.Rotate);
                OverwriteData(ObjectTimeSamplesData->TransformTimeSamples.Scale, TimeSamplesData.TransformTimeSamples.Scale);
                break;
            }
        }

        //Others are just appended now
        ObjectTimeSamplesData->VisibilityTimeSamples.TimeSamples.Append(TimeSamplesData.VisibilityTimeSamples.TimeSamples);

        EmplaceTimeSamples(ObjectTimeSamplesData->ParametersTimeSamples.BooleanTimeSamples, TimeSamplesData.ParametersTimeSamples.BooleanTimeSamples);
        EmplaceTimeSamples(ObjectTimeSamplesData->ParametersTimeSamples.ScalarTimeSamples, TimeSamplesData.ParametersTimeSamples.ScalarTimeSamples);
        EmplaceTimeSamples(ObjectTimeSamplesData->ParametersTimeSamples.VectorTimeSamples, TimeSamplesData.ParametersTimeSamples.VectorTimeSamples);
        EmplaceTimeSamples(ObjectTimeSamplesData->ParametersTimeSamples.ColorTimeSamples, TimeSamplesData.ParametersTimeSamples.ColorTimeSamples);
        
        for (auto Pair : TimeSamplesData.MaterialsTimeSamples)
        {
            auto MaterialsTimeSamples = ObjectTimeSamplesData->MaterialsTimeSamples.Find(Pair.Key);
            if (MaterialsTimeSamples)
            {
                EmplaceTimeSamples(MaterialsTimeSamples->BooleanTimeSamples, Pair.Value.BooleanTimeSamples);
                EmplaceTimeSamples(MaterialsTimeSamples->ScalarTimeSamples, Pair.Value.ScalarTimeSamples);
                EmplaceTimeSamples(MaterialsTimeSamples->VectorTimeSamples, Pair.Value.VectorTimeSamples);
                EmplaceTimeSamples(MaterialsTimeSamples->ColorTimeSamples, Pair.Value.ColorTimeSamples);
            }
            else
            {
                ObjectTimeSamplesData->MaterialsTimeSamples.Add(Pair);
            }
        }

        ObjectTimeSamplesData->ObjectBindingTimeSamples.Append(TimeSamplesData.ObjectBindingTimeSamples);
        ObjectTimeSamplesData->SkeletalAnimationTimeSamples.Append(TimeSamplesData.SkeletalAnimationTimeSamples);
    }
    else
    {
        ObjectsTimeSamplesData.Add(SequenceObject, TimeSamplesData);
    }
}

FTimeSampleRange FSequenceExtractor::GetRangeByPriority(const FTimeSampleRange& InRange, int32 RowIndex, int32 OverlapPriority)
{
    TOptional<int32> EditStartTimeCode;
    TOptional<int32> EditEndTimeCode;

    for (int32 Time = InRange.StartTimeCode; Time <= InRange.EndTimeCode; ++Time)
    {
        auto Priority = PriorityTimeline.Find(Time);
        if (Priority)
        {
            bool bSectionWins =
                (RowIndex < Priority->RowIndex) ||
                (RowIndex == Priority->RowIndex && OverlapPriority > Priority->OverlapPriority);

            if (bSectionWins)
            {
                Priority->RowIndex = RowIndex;
                Priority->OverlapPriority = OverlapPriority;
                if (!EditStartTimeCode.IsSet())
                {
                    EditStartTimeCode = Time;
                    
                }
                EditEndTimeCode = Time;
            }
        }
        else
        {
            PriorityTimeline.Add(Time, {RowIndex, OverlapPriority });

            if (!EditStartTimeCode.IsSet())
            {
                EditStartTimeCode = Time;
            }
            EditEndTimeCode = Time;
        }
    }

    return { EditStartTimeCode.IsSet() ? EditStartTimeCode.GetValue() : 0, EditEndTimeCode.IsSet() ? EditEndTimeCode.GetValue() : 0 };
}

void FSequenceExtractor::ClipCameraShots(const FTimeSampleRange& InRange)
{
    for (auto Pair : ObjectsTimeSamplesData)
    {
        TArray<FTimeSampleRange> NewRanges;
        for (auto CameraShotsRange : Pair.Value.CameraShotsTimeSamples)
        {
            NewRanges.Append(CameraShotsRange.ClipRange(InRange));
        }

        if (NewRanges.Num() > 0)
        {
            ObjectsTimeSamplesData[Pair.Key].CameraShotsTimeSamples = NewRanges;
        }
    }
}