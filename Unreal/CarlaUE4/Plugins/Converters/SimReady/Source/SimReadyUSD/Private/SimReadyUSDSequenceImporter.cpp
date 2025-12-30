// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDSequenceImporter.h"
#include "Animation/AnimSequence.h"
#include "MovieSceneBinding.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneVectorTrack.h"
#include "Tracks/MovieSceneColorTrack.h"
#include "Tracks/MovieSceneVisibilityTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Sections/MovieSceneBoolSection.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneVectorSection.h"
#include "Sections/MovieSceneColorSection.h"
#include "Sections/MovieSceneSkeletalAnimationSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "SimReadyStageActor.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadySettings.h"
#include "EngineUtils.h"
#include "Extractors/TimeSamplesData.h"
#include "Extractors/SequenceExtractor.h"
#include "Engine/Blueprint.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "ActorSequence.h"
#include "ActorSequenceComponent.h"

#include "ILevelSequenceEditorToolkit.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "ISimReadyRuntimeModule.h"

FSimReadyUSDSequenceImporter::FSimReadyUSDSequenceImporter(TWeakObjectPtr<ASimReadyStageActor> InStageActor)
    : StageActor(InStageActor)
{
    ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
    OnSequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(FOnSequencerCreated::FDelegate::CreateRaw(this, &FSimReadyUSDSequenceImporter::OnSequencerCreated));

}

FSimReadyUSDSequenceImporter::~FSimReadyUSDSequenceImporter()
{
    ISequencerModule* SequencerModulePtr = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
    if (SequencerModulePtr)
    {
        SequencerModulePtr->UnregisterOnSequencerCreated(OnSequencerCreatedHandle);
    }

    TSharedPtr<ISequencer> Sequencer = StageSequencer.Pin();
    if (Sequencer.IsValid())
    {
        Sequencer->OnCloseEvent().Remove(OnCloseEventHandle);
    }

    StageActor = nullptr;
    ResetLevelSequence();
}

void FSimReadyUSDSequenceImporter::ResetLevelSequence()
{
    LevelSequence = nullptr;
}

void FSimReadyUSDSequenceImporter::CreateLevelSequence(const TOptional<double>& InStartTimeCode, const TOptional<double>& InEndTimeCode, const TOptional<double>& InTimeCodesPerSecond)
{
    if (!StageActor.IsValid())
    {
        return;
    }
    ImporterMode = IM_LevelSequence;

    if (!LevelSequence.IsValid())
    {
        FString Name = StageActor->GetUSDName() + TEXT("_Sequence");
        //LevelSequence = StageActor->LoadImportObject<ULevelSequence>(ASimReadyStageActor::ImportType::Root, Name);
        //if (!LevelSequence.IsValid())
        {
            LevelSequence = NewObject<ULevelSequence>(StageActor->GetAssetPackage(ASimReadyStageActor::ImportType::Root, Name), *Name, StageActor->GetAssetFlag());
            LevelSequence->MarkPackageDirty();
            FAssetRegistryModule::AssetCreated(LevelSequence.Get());
        }
    }

    if (LevelSequence.IsValid())
    {
        FSequenceExtractor::Get().ClearData();

        LevelSequence->Initialize();

        UMovieScene* MovieScene = LevelSequence->GetMovieScene();
        check(MovieScene);
        double StartTimeCode = InStartTimeCode.IsSet() ? InStartTimeCode.GetValue() : DEFAULT_STARTTIMECODE;
        double TimeCodesPerSecond = InTimeCodesPerSecond.IsSet() ? InTimeCodesPerSecond.GetValue() : DEFAULT_TIMECODESPERSECOND;
        MovieScene->SetDisplayRate(FFrameRate(TimeCodesPerSecond, 1));

        if (InEndTimeCode.IsSet())
        {
            double StartSecond = StartTimeCode / TimeCodesPerSecond;
            double EndSecond = InEndTimeCode.GetValue() / TimeCodesPerSecond;

            FFrameRate FrameRate = MovieScene->GetTickResolution();
            FFrameNumber StartFrame = RoundToFrameNumber(FrameRate, StartSecond);
            FFrameNumber EndFrame = RoundToFrameNumber(FrameRate, EndSecond);

            TRange<FFrameNumber> TimeRange = TRange<FFrameNumber>::Inclusive(StartFrame, EndFrame);

            MovieScene->SetPlaybackRange(TimeRange);
            MovieScene->SetViewRange(StartSecond - 1.0f, 1.0f + EndSecond);
            MovieScene->SetWorkingRange(StartSecond - 1.0f, 1.0f + EndSecond);
            MovieScene->SetEvaluationType(EMovieSceneEvaluationType::FrameLocked);
        
            auto World = StageActor->GetWorld();
            if (!StageActor->LevelSequenceActor.IsValid())
            {
                StageActor->LevelSequenceActor = World->SpawnActor<ALevelSequenceActor>(ALevelSequenceActor::StaticClass());
            }

            if (StageActor->LevelSequenceActor.IsValid())
            {
                StageActor->LevelSequenceActor->SetSequence(LevelSequence.Get());
                // Always initialize the player so that the playback settings/range can be initialized from editor.
                StageActor->LevelSequenceActor->InitializePlayer();
                StageActor->LevelSequenceActor->PlaybackSettings.bAutoPlay = true;
                StageActor->LevelSequenceActor->PlaybackSettings.LoopCount.Value = -1;
            }
        }
    }
}

void FSimReadyUSDSequenceImporter::RemoveEmptyLevelSequence()
{
    if (CheckValid())
    {
        UMovieScene* MovieScene = LevelSequence->GetMovieScene();
        if (MovieScene->GetPossessableCount() == 0 && MovieScene->GetSpawnableCount() == 0)
        {
            if (StageActor->LevelSequenceActor.IsValid())
            {
                auto World = StageActor->GetWorld();
                StageActor->LevelSequenceActor->SetSequence(nullptr);
                World->DestroyActor(StageActor->LevelSequenceActor.Get());
                StageActor->LevelSequenceActor = nullptr;
            }

            if (LevelSequence->GetOutermost() != GetTransientPackage() && !LevelSequence->HasAnyFlags(RF_Transient))
            {
                FAssetRegistryModule::AssetDeleted(LevelSequence.Get());
                LevelSequence->ClearFlags(RF_Standalone | RF_Public);
                LevelSequence->RemoveFromRoot();
                LevelSequence->MarkPendingKill();
                LevelSequence = nullptr;
            }		
        }
    }
}

void FSimReadyUSDSequenceImporter::SetActorSequenceMode()
{
    ImporterMode = EImporterMode::IM_ActorSequence;
}

void FSimReadyUSDSequenceImporter::CacheObjectTrack(UObject* InObject, const FNamedParameterTimeSamples& ParameterTimeSamples)
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
        NewTimeSamplesData.ParametersTimeSamples = ParameterTimeSamples;
        ActorSequenceTimeSamples.Add(AnimedObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->ParametersTimeSamples = ParameterTimeSamples;
    }
}

void FSimReadyUSDSequenceImporter::CacheSkeletalAnimationTrack(UObject* InObject, UAnimSequence* AnimSequence, int32 StartTimeCode)
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

    FSkeletalAnimationTimeSamples AnimeTimeSamples;
    AnimeTimeSamples.Animation = AnimSequence;
    AnimeTimeSamples.Range = {StartTimeCode, StartTimeCode};

    auto TimeSamplesData = ActorSequenceTimeSamples.Find(AnimedObject);
    if (TimeSamplesData == nullptr)
    {
        FTimeSamplesData NewTimeSamplesData;
        
        NewTimeSamplesData.SkeletalAnimationTimeSamples.Add(AnimeTimeSamples);
        ActorSequenceTimeSamples.Add(AnimedObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->SkeletalAnimationTimeSamples.Add(AnimeTimeSamples);
    }
}

void FSimReadyUSDSequenceImporter::CacheVisibilityTrack(UObject* InObject, const FBooleanTimeSamples& VisibilityTimeSamples)
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
        NewTimeSamplesData.VisibilityTimeSamples = VisibilityTimeSamples;
        ActorSequenceTimeSamples.Add(AnimedObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->VisibilityTimeSamples = VisibilityTimeSamples;
    }
}

void FSimReadyUSDSequenceImporter::CacheMaterialTrack(UObject* InObject, int32 MaterialIndex, const FNamedParameterTimeSamples& ParameterTimeSamples)
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
        NewTimeSamplesData.MaterialsTimeSamples.Add(MaterialIndex, ParameterTimeSamples);
        ActorSequenceTimeSamples.Add(AnimedObject, NewTimeSamplesData);
    }
    else
    {
        TimeSamplesData->MaterialsTimeSamples.Add(MaterialIndex, ParameterTimeSamples);
    }
}

void FSimReadyUSDSequenceImporter::BuildActorSequence(UBlueprint* Blueprint, const TArray<AActor*>& Actors, const TOptional<double>& InStartTimeCode, const TOptional<double>& InEndTimeCode, const TOptional<double>& InTimeCodesPerSecond)
{
    TMap<UObject*, USCS_Node*> ObjectToNodeMap;
    TArray<USCS_Node*> AllNodes = Blueprint->SimpleConstructionScript->GetAllNodes();
    int32 NodeIndex = 0;
    for (auto Actor : Actors)
    {
        for (UActorComponent* ComponentToConsider : Actor->GetComponents())
        {
            if (ComponentToConsider && !ComponentToConsider->IsVisualizationComponent())
            {
                ObjectToNodeMap.Add(ComponentToConsider, AllNodes[NodeIndex++]);
            }
        }
    }

    UActorSequenceComponent* ActorSequenceComponent = Cast<UActorSequenceComponent>(AllNodes[AllNodes.Num() - 1]->ComponentTemplate);
    if (ActorSequenceComponent)
    {
        ActorSequence = ActorSequenceComponent->GetSequence();
        UMovieScene* MovieScene = ActorSequence->GetMovieScene();
        double StartTimeCode = InStartTimeCode.IsSet() ? InStartTimeCode.GetValue() : DEFAULT_STARTTIMECODE;
        double TimeCodesPerSecond = InTimeCodesPerSecond.IsSet() ? InTimeCodesPerSecond.GetValue() : DEFAULT_TIMECODESPERSECOND;
        MovieScene->SetDisplayRate(FFrameRate(TimeCodesPerSecond, 1));
        if (InEndTimeCode.IsSet())
        {
            double StartSecond = StartTimeCode / TimeCodesPerSecond;
            double EndSecond = InEndTimeCode.GetValue() / TimeCodesPerSecond;

            FFrameRate FrameRate = MovieScene->GetTickResolution();
            FFrameNumber StartFrame = RoundToFrameNumber(FrameRate, StartSecond);
            FFrameNumber EndFrame = RoundToFrameNumber(FrameRate, EndSecond);

            TRange<FFrameNumber> TimeRange = TRange<FFrameNumber>::Inclusive(StartFrame, EndFrame);

            MovieScene->SetPlaybackRange(TimeRange);
            MovieScene->SetViewRange(StartSecond - 1.0f, 1.0f + EndSecond);
            MovieScene->SetWorkingRange(StartSecond - 1.0f, 1.0f + EndSecond);
            MovieScene->SetEvaluationType(EMovieSceneEvaluationType::FrameLocked);
        }

        AActor* BlueprintActor = nullptr;
        auto World = StageActor->GetWorld();
        for ( TActorIterator<AActor> It(World); It; ++It )
        {
            AActor* Actor = *It;
            if (!Actor->IsPendingKillPending()
                && Actor->GetClass()->ClassGeneratedBy == Blueprint)
            {
                BlueprintActor = Actor;
                break;
            }
        }

        if (BlueprintActor)
        {
            for (auto Pair : ActorSequenceTimeSamples)
            {
                auto Node = ObjectToNodeMap.Find(Pair.Key);
                if (Node)
                {
                    FName VariableName = (*Node)->GetVariableName();
                    if (VariableName != NAME_None)
                    {
                        FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(BlueprintActor->GetClass(), VariableName);
                        if (Property != NULL)
                        {
                            // Return the component instance that's stored in the property with the given variable name
                            auto ComponentInstance = Cast<UActorComponent>(Property->GetObjectPropertyValue_InContainer(BlueprintActor));
                            if (ComponentInstance)
                            {
                                CreateTransformTrack(ComponentInstance, Pair.Value.TransformTimeSamples);
                                CreateObjectTrack(ComponentInstance, Pair.Value.ParametersTimeSamples);
                                CreateVisibilityTrack(ComponentInstance, Pair.Value.VisibilityTimeSamples);
                                for (auto MaterialPair : Pair.Value.MaterialsTimeSamples)
                                {
                                    CreateMaterialTrack(ComponentInstance, MaterialPair.Key, MaterialPair.Value);
                                }
                                for (auto AnimationTimeSamples : Pair.Value.SkeletalAnimationTimeSamples)
                                {
                                    CreateSkeletalAnimationTrack(ComponentInstance, Cast<UAnimSequence>(AnimationTimeSamples.Animation), AnimationTimeSamples.Range.StartTimeCode);
                                }
                            }
                        }
                    }
                }
            }
        }
        ActorSequence = nullptr;
    }
}

bool FSimReadyUSDSequenceImporter::CheckValid()
{
    return (StageActor.IsValid() && (LevelSequence.IsValid() || ActorSequence.IsValid()));
}

UMovieSceneSequence* FSimReadyUSDSequenceImporter::GetActiveSequence()
{
    return LevelSequence.IsValid() ? Cast<UMovieSceneSequence>(LevelSequence.Get()) : Cast<UMovieSceneSequence>(ActorSequence.Get());
}

FGuid FSimReadyUSDSequenceImporter::FindOrBindPossessable(UObject* InObject, UObject* BindingContext)
{
    if (!CheckValid())
    {
        return FGuid();
    }

    UMovieScene* MovieScene = GetActiveSequence()->GetMovieScene();

    AActor* PossessedActor = Cast<AActor>(InObject);

    FGuid ObjectBinding;
    if (PossessedActor)
    {
        ObjectBinding = GetActiveSequence()->FindPossessableObjectId(*InObject, BindingContext);
    }
    else if (InObject->IsA<UActorComponent>() && BindingContext->IsA<AActor>())
    {
        // can't use FindPossessableObjectId to find binding, it might return another scenecomponent with the same name
        AActor* ParentActor = Cast<AActor>(BindingContext);
        auto ParentBinding = GetActiveSequence()->FindPossessableObjectId(*ParentActor, ImporterMode == IM_ActorSequence ? Cast<UObject>(ParentActor) : ParentActor->GetWorld());

        for (int32 PossessableIndex = 0; PossessableIndex < MovieScene->GetPossessableCount(); ++PossessableIndex)
        {
            auto Possessable = MovieScene->GetPossessable(PossessableIndex);

            if (Possessable.GetParent() == ParentBinding)
            {
                TArray<UObject*, TInlineAllocator<1>> OutObjects;
                GetActiveSequence()->LocateBoundObjects(Possessable.GetGuid(), ParentActor, OutObjects);

                for (auto Obj : OutObjects)
                {
                    if (Obj == InObject)
                    {
                        ObjectBinding = Possessable.GetGuid();
                        break;
                    }
                }
            }
        }
    }

    if (ObjectBinding == FGuid())
    {
        ObjectBinding = MovieScene->AddPossessable(PossessedActor ? PossessedActor->GetActorLabel() : InObject->GetName(), InObject->GetClass());
        GetActiveSequence()->BindPossessableObject(ObjectBinding, *InObject, BindingContext);
    }

    return ObjectBinding;
}

FGuid FSimReadyUSDSequenceImporter::FindOrBindObject(UObject* InObject)
{
    if (!CheckValid())
    {
        return FGuid();
    }

    UMovieScene* MovieScene = GetActiveSequence()->GetMovieScene();

    if (InObject->IsA<AActor>())
    {
        auto Actor = Cast<AActor>(InObject);
        return FindOrBindPossessable(Cast<AActor>(InObject), ImporterMode == IM_ActorSequence ? InObject : Actor->GetWorld());
    }
    else if (InObject->IsA<UActorComponent>())
    {
        auto Component = Cast<UActorComponent>(InObject);
        auto ActorBinding = FindOrBindPossessable(Component->GetOwner(), ImporterMode == IM_ActorSequence ? Cast<UObject>(Component->GetOwner()) : Component->GetOwner()->GetWorld());
        auto ComponentBinding = FindOrBindPossessable(Component, Component->GetOwner());

        FMovieScenePossessable* ChildPossessable = MovieScene->FindPossessable(ComponentBinding);
        if (ensure(ChildPossessable))
        {
            ChildPossessable->SetParent(ActorBinding);
        }

        FMovieSceneSpawnable* ParentSpawnable = MovieScene->FindSpawnable(ActorBinding);
        if (ParentSpawnable)
        {
            ParentSpawnable->AddChildPossessable(ComponentBinding);
        }

        return ComponentBinding;
    }

    return FGuid();
}

void FSimReadyUSDSequenceImporter::CreateMaterialTrack(UObject* InObject, int32 MaterialIndex, const FNamedParameterTimeSamples& ParameterTimeSamples)
{
    CacheMaterialTrack(InObject, MaterialIndex, ParameterTimeSamples);

    if (!CheckValid())
    {
        return;
    }

    if (!ParameterTimeSamples.HasAnyParameter())
    {
        return;
    }

    UMovieSceneComponentMaterialTrack* MaterialTrack = CreateTrack<UMovieSceneComponentMaterialTrack>(InObject, *FString::FromInt(MaterialIndex));

    if (!MaterialTrack)
    {
        return;
    }
    
    MaterialTrack->SetMaterialIndex(MaterialIndex);

    FFrameRate FrameRate = MaterialTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = MaterialTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();

    for (auto ScalarParameter : ParameterTimeSamples.ScalarTimeSamples)
    {
        auto TraverseScalarCallback = [&](int32 TimeCode, float TimeSample)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
            MaterialTrack->AddScalarParameterKey(ScalarParameter.Key, FrameNumber, TimeSample);
        };
        ParameterTimeSamples.TraverseScalarTimeSamples(ScalarParameter.Key, TraverseScalarCallback);
    }

    for (auto ColorParameter : ParameterTimeSamples.ColorTimeSamples)
    {
        auto TraverseColorCallback = [&](int32 TimeCode, const FLinearColor& TimeSample)
        {
            FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
            MaterialTrack->AddColorParameterKey(ColorParameter.Key, FrameNumber, TimeSample);
        };
        ParameterTimeSamples.TraverseColorTimeSamples(ColorParameter.Key, TraverseColorCallback);
    }
}

void FSimReadyUSDSequenceImporter::CreateBoolTrack(UObject* InObject, const FName& Name, const FBooleanTimeSamples& BooleanTimeSamples)
{
    if (!CheckValid())
    {
        return;
    }

    if (BooleanTimeSamples.TimeSamples.Num() == 0)
    {
        return;
    }

    UMovieSceneBoolTrack* BoolTrack = CreateTrack<UMovieSceneBoolTrack>(InObject, Name);

    if (!BoolTrack)
    {
        return;
    }
    BoolTrack->UniqueTrackName = Name;
    BoolTrack->SetPropertyNameAndPath(Name, Name.ToString());
    // Bool section
    auto BoolSection = Cast<UMovieSceneBoolSection>(BoolTrack->CreateNewSection());
    BoolTrack->AddSection(*BoolSection);
    BoolSection->SetRange(TRange<FFrameNumber>::All());

    FFrameRate FrameRate = BoolTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = BoolTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();

    FMovieSceneBoolChannel* BoolChannel = BoolSection->GetChannelProxy().GetChannel<FMovieSceneBoolChannel>(0);

    auto TraverseCallback = [&](int32 TimeCode, bool TimeSample)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));

        BoolChannel->GetData().AddKey(FrameNumber, TimeSample);
    };

    BooleanTimeSamples.TraverseTimeSamples(TraverseCallback);
}

void FSimReadyUSDSequenceImporter::CreateFloatTrack(UObject* InObject, const FName& Name, const FFloatTimeSamples& FloatTimeSamples)
{
    if (!CheckValid())
    {
        return;
    }

    if (FloatTimeSamples.TimeSamples.Num() == 0)
    {
        return;
    }

    UMovieSceneFloatTrack* FloatTrack = CreateTrack<UMovieSceneFloatTrack>(InObject, Name);

    if (!FloatTrack)
    {
        return;
    }
    FloatTrack->UniqueTrackName = Name;
    FloatTrack->SetPropertyNameAndPath(Name, Name.ToString());
    // Float section
    auto FloatSection = Cast<UMovieSceneFloatSection>(FloatTrack->CreateNewSection());
    FloatTrack->AddSection(*FloatSection);
    FloatSection->SetRange(TRange<FFrameNumber>::All());

    FFrameRate FrameRate = FloatTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = FloatTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();
    TArrayView<FMovieSceneFloatChannel*> FloatChannels = FloatSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();

    auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));

        FloatChannels[0]->AddLinearKey(FrameNumber, TimeSample);
    };

    FloatTimeSamples.TraverseTimeSamples(TraverseCallback);
}

void FSimReadyUSDSequenceImporter::CreateVectorTrack(UObject* InObject, const FName& Name, const FVectorTimeSamples& VectorTimeSamples)
{
    if (!CheckValid())
    {
        return;
    }

    if (VectorTimeSamples.TimeSamples.Num() == 0)
    {
        return;
    }

    // Vector Track
    UMovieSceneVectorTrack* VectorTrack = CreateTrack<UMovieSceneVectorTrack>(InObject, Name);

    if (!VectorTrack)
    {
        return;
    }

    VectorTrack->UniqueTrackName = Name;
    VectorTrack->SetNumChannelsUsed(VectorTimeSamples.ChannelsUsed);
    VectorTrack->SetPropertyNameAndPath(Name, Name.ToString());
    // Vector section
    auto VectorSection = Cast<UMovieSceneVectorSection>(VectorTrack->CreateNewSection());
    VectorTrack->AddSection(*VectorSection);
    VectorSection->SetRange(TRange<FFrameNumber>::All());

    FFrameRate FrameRate = VectorTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = VectorTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();
    TArrayView<FMovieSceneFloatChannel*> FloatChannels = VectorSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();

    auto TraverseCallback = [&](int32 TimeCode, int32 ChannelsUsed, const FVector4& TimeSample)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));

        FloatChannels[0]->AddLinearKey(FrameNumber, TimeSample.X);
        FloatChannels[1]->AddLinearKey(FrameNumber, TimeSample.Y);
        if (ChannelsUsed > 2)
        {
            FloatChannels[2]->AddLinearKey(FrameNumber, TimeSample.Z);
        }
        if (ChannelsUsed > 3)
        {
            FloatChannels[3]->AddLinearKey(FrameNumber, TimeSample.W);
        }
    };

    VectorTimeSamples.TraverseTimeSamples(TraverseCallback);
}

void FSimReadyUSDSequenceImporter::CreateColorTrack(UObject* InObject, const FName& Name, const FColorTimeSamples& ColorTimeSamples)
{
    if (!CheckValid())
    {
        return;
    }

    if (ColorTimeSamples.TimeSamples.Num() == 0)
    {
        return;
    }

    // Color Track
    UMovieSceneColorTrack* ColorTrack = CreateTrack<UMovieSceneColorTrack>(InObject, Name);

    if (!ColorTrack)
    {
        return;
    }

    ColorTrack->UniqueTrackName = Name;
    ColorTrack->SetPropertyNameAndPath(Name, Name.ToString());
    // Color section
    auto ColorSection = Cast<UMovieSceneColorSection>(ColorTrack->CreateNewSection());
    ColorTrack->AddSection(*ColorSection);
    ColorSection->SetRange(TRange<FFrameNumber>::All());
    FFrameRate FrameRate = ColorTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = ColorTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();
    TArrayView<FMovieSceneFloatChannel*> FloatChannels = ColorSection->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();

    auto TraverseCallback = [&](int32 TimeCode, const FLinearColor& TimeSample)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));

        FloatChannels[0]->AddLinearKey(FrameNumber, TimeSample.R);
        FloatChannels[1]->AddLinearKey(FrameNumber, TimeSample.G);
        FloatChannels[2]->AddLinearKey(FrameNumber, TimeSample.B);
        FloatChannels[3]->AddLinearKey(FrameNumber, TimeSample.A);
    };
    
    ColorTimeSamples.TraverseTimeSamples(TraverseCallback);
}

void FSimReadyUSDSequenceImporter::CreateObjectTrack(UObject* InObject, const FNamedParameterTimeSamples& ParameterTimeSamples)
{
    CacheObjectTrack(InObject, ParameterTimeSamples);

    for (auto BoolParam : ParameterTimeSamples.BooleanTimeSamples)
    {
        CreateBoolTrack(InObject, BoolParam.Key, BoolParam.Value);
    }
    for (auto ScalarParam : ParameterTimeSamples.ScalarTimeSamples)
    {
        CreateFloatTrack(InObject, ScalarParam.Key, ScalarParam.Value);
    }
    for (auto VectorParam : ParameterTimeSamples.VectorTimeSamples)
    {
        CreateVectorTrack(InObject, VectorParam.Key, VectorParam.Value);
    }
    for (auto ColorParam : ParameterTimeSamples.ColorTimeSamples)
    {
        CreateColorTrack(InObject, ColorParam.Key, ColorParam.Value);
    }
}

void FSimReadyUSDSequenceImporter::CreateVisibilityTrack(UObject* InObject, const FBooleanTimeSamples& VisibilityTimeSamples)
{
    CacheVisibilityTrack(InObject, VisibilityTimeSamples);

    if (!CheckValid())
    {
        return;
    }

    if (VisibilityTimeSamples.TimeSamples.Num() == 0)
    {
        return;
    }

    static const FName ActorVisibilityTrackName = TEXT("bHidden");
    static const FName ComponentVisibilityTrackName = TEXT("bHiddenInGame");
    FName TrackName = InObject->IsA<AActor>() ? ActorVisibilityTrackName : ComponentVisibilityTrackName;
    UMovieSceneVisibilityTrack* VisTrack = CreateTrack<UMovieSceneVisibilityTrack>(InObject, TrackName);

    if (!VisTrack)
    {
        return;
    }

    VisTrack->UniqueTrackName = TrackName;
    VisTrack->SetPropertyNameAndPath(TrackName, TrackName.ToString());

    // Bool section
    auto BoolSection = Cast<UMovieSceneBoolSection>(VisTrack->CreateNewSection());
    VisTrack->AddSection(*BoolSection);
    BoolSection->SetRange(TRange<FFrameNumber>::All());

    FFrameRate FrameRate = VisTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
    FFrameRate DisplayRate = VisTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();

    FMovieSceneBoolChannel* BoolChannel = BoolSection->GetChannelProxy().GetChannel<FMovieSceneBoolChannel>(0);

    auto TraverseCallback = [&](int32 TimeCode, bool TimeSample)
    {
        FFrameNumber FrameNumber = FFrameNumber((int32)(TimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));

        BoolChannel->GetData().AddKey(FrameNumber, TimeSample);
    };

    VisibilityTimeSamples.TraverseTimeSamples(TraverseCallback);
}

void FSimReadyUSDSequenceImporter::CreateSkeletalAnimationTrack(UObject* InObject, UAnimSequence* AnimSequence, int32 StartTimeCode)
{
    CacheSkeletalAnimationTrack(InObject, AnimSequence, StartTimeCode);

    if (!CheckValid())
    {
        return;
    }

    UMovieSceneSkeletalAnimationTrack* AnimTrack = CreateTrack<UMovieSceneSkeletalAnimationTrack>(InObject);

    if (AnimTrack)
    {
        FFrameRate FrameRate = AnimTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
        FFrameRate DisplayRate = AnimTrack->GetTypedOuter<UMovieScene>()->GetDisplayRate();
        FFrameNumber StartFrameNumber = FFrameNumber((int32)(StartTimeCode / (FrameRate.AsInterval() * DisplayRate.AsDecimal())));
        AnimTrack->AddNewAnimation(StartFrameNumber, AnimSequence);
    }
}

bool FSimReadyUSDSequenceImporter::IsPlayingSequence()
{
    if (!CheckValid())
    {
        return false;
    }

    if (StageActor->LevelSequenceActor.IsValid() && StageActor->LevelSequenceActor->SequencePlayer)
    {
        if (StageActor->LevelSequenceActor->SequencePlayer->IsPlaying())
        {
            return true;
        }
    }

    IAssetEditorInstance*        AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(LevelSequence.Get(), false);
    ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
    TWeakPtr<ISequencer> WeakSequencer = LevelSequenceEditor ? LevelSequenceEditor->GetSequencer() : nullptr;

    return (WeakSequencer.IsValid());
}

void FSimReadyUSDSequenceImporter::OnSequencerCreated(TSharedRef<ISequencer> InSequencer)
{
    UMovieSceneSequence* Sequence = InSequencer->GetRootMovieSceneSequence();
    if (LevelSequence.IsValid() && LevelSequence.Get() == Sequence)
    {
        StageSequencer = TWeakPtr<ISequencer>(InSequencer);
        OnCloseEventHandle = InSequencer->OnCloseEvent().AddRaw(this, &FSimReadyUSDSequenceImporter::OnSequencerClosed);
    }
}

void FSimReadyUSDSequenceImporter::OnSequencerClosed(TSharedRef<ISequencer> InSequencer)
{
}