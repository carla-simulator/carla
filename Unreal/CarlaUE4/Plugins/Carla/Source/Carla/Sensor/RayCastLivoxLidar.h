#pragma once

#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"             
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" 

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/data/LivoxLidarData.h> 
#include <compiler/enable-ue4-macros.h>

#include <vector>
#include <time.h>

#include "RayCastLivoxLidar.generated.h"

// livox lidar sensor
UCLASS()
class CARLA_API ARayCastLivoxLidar : public ARayCastSemanticLidar
{
    GENERATED_BODY()

    using FLivoxLidarData = carla::sensor::data::LivoxLidarData;
    using FLivoxDetection = carla::sensor::data::LivoxLidarDetection;

public:
    static FActorDefinition GetSensorDefinition(); 

    ARayCastLivoxLidar(const FObjectInitializer &ObjectInitializer);

    virtual void Set(const FActorDescription &Description) override; 
    virtual void Set(const FLidarDescription &LidarDescription) override;
    virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime);

private:
    float ComputeIntensity(const FSemanticDetection &RawDetection) const; 

    FLivoxDetection ComputeDetection(const FHitResult &HitInfo, const FTransform &SensorTransf) const;
    void PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) override;

    bool PostprocessDetection(FLivoxDetection &Detection) const; 

    void ComputeAndSaveDetections(const FTransform &SensorTransform); 

    bool DropOffGenActive; 
    float DropOffAlpha;
    float DropOffBeta;


    FLivoxLidarData LivoxLidarData;
};