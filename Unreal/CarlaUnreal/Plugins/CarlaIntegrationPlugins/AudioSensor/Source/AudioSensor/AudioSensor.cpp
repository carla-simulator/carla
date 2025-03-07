#include "AudioSensor.h"

#include "AudioSensorActor.h"


#define LOCTEXT_NAMESPACE "FAudioSensor"


DEFINE_LOG_CATEGORY(LogAudioSensor)

void FAudioSensor::StartupModule()
{
}

void FAudioSensor::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAudioSensor, AudioSensor)

