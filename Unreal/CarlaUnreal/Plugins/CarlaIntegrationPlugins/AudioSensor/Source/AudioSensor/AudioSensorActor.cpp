#include "AudioSensorActor.h"
#include "AudioMixerDevice.h"
#include "AudioDeviceManager.h"
#include "AudioSensorBlueprintLibrary.h"
#include "AudioSensor/ros2/ROS2Audio.h"

AAudioSensorActor::AAudioSensorActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
    // PrimaryActorTick.bCanEverTick = true;
    this->bIsRecording = false;
    this->Active = false;
    this->ChunkDuration = 0.25;
    this->AudioBuffer = Audio::TSampleBuffer();
}

void AAudioSensorActor::BeginPlay()
{
    Super::BeginPlay();

    // Create the master submix
    this->MasterSubmix = NewObject<USoundSubmix>();
    Activate();
}

void AAudioSensorActor::SetChunkDuration(double Duration){
    this->ChunkDuration = Duration;
}

FActorDefinition AAudioSensorActor::GetSensorDefinition()
{
  return UAudioSensorBlueprintLibrary::MakeAudioSensorDefinition(AAudioSensorActor::StaticClass());
}

void AAudioSensorActor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UAudioSensorBlueprintLibrary::SetAudioSensor(ActorDescription, this);
}

void AAudioSensorActor::SetOwner(AActor* OwningActor)
{
  Super::SetOwner(OwningActor);
}

void AAudioSensorActor::PrePhysTick(float DeltaSeconds){
    if (!Active){
        return;
    }

    StopRecording(); // end the previous recording (if one is in progress)
    StartRecording(); // start recording the next frame

    if (AudioBuffer.GetNumSamples() == 0)
    {
        return;
    }

    auto ROS2Audio = carla::ros2::ROS2Audio::GetInstance();
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    AActor* ParentActor = GetAttachParentActor();

    if (ParentActor)
    {
        FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
        ROS2Audio->ProcessDataFromAudio(StreamId, LocalTransformRelativeToParent, AudioBuffer.GetNumChannels(), AudioBuffer.GetSampleRate(), AudioBuffer.GetNumFrames(), AudioBuffer.GetData(), this);
    }
    else
    {
        ROS2Audio->ProcessDataFromAudio(StreamId, GetActorTransform(), AudioBuffer.GetNumChannels(), AudioBuffer.GetSampleRate(), AudioBuffer.GetNumFrames(), AudioBuffer.GetData(), this);
    }
}

void AAudioSensorActor::StartRecording()
{
    if (MasterSubmix && !bIsRecording)
    {
        if(Audio::FMixerDevice* MixerDevice = FAudioDeviceManager::GetAudioMixerDeviceFromWorldContext(GetWorld()))
        {
            MixerDevice->StartRecording(MasterSubmix, ChunkDuration);
            bIsRecording = true;
        }
    }
}

void AAudioSensorActor::StopRecording()
{
    AudioBuffer.Reset();
    if (MasterSubmix && bIsRecording)
    {
        if(Audio::FMixerDevice* MixerDevice = FAudioDeviceManager::GetAudioMixerDeviceFromWorldContext(GetWorld()))
        {
            float Channels;
            float SampleRate;
            Audio::FAlignedFloatBuffer& Buffer = MixerDevice->StopRecording(MasterSubmix, Channels, SampleRate);
            if (Buffer.Num() > 0)
            {
                Audio::FSampleBuffer SampleBuffer(Buffer, Channels, SampleRate);

                if (SampleBuffer.GetNumChannels() > 2)
                {
                    SampleBuffer.MixBufferToChannels(2);
                }
                // This conveniently converts the data from float to int16
                AudioBuffer = SampleBuffer;
                bIsRecording = false;
            }
        }
    }
}

void AAudioSensorActor::Activate(){
    Active = true;
}

void AAudioSensorActor::Deactivate(){
    Active = false;
    StopRecording();
}