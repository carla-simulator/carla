// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include <carla/sensor/data/V2XData.h>
#include "V2X/CaService.h"
#include "V2X/PathLossModel.h"
#include <list>
#include <map>
#include "V2XSensor.generated.h"

UCLASS()
class CARLA_API AV2XSensor : public ASensor
{
    GENERATED_BODY()

public:
    AV2XSensor(const FObjectInitializer &ObjectInitializer);

    static FActorDefinition GetSensorDefinition();

    void Set(const FActorDescription &ActorDescription) override;

    void SetCaServiceParams(const float GenCamMin, const float GenCamMax, const bool FixedRate);
    void SetPropagationParams(const float TransmitPower,
                              const float ReceiverSensitivity,
                              const float Frequency,
                              const float combined_antenna_gain,
                              const float path_loss_exponent,
                              const float reference_distance_fspl,
                              const float filter_distance,
                              const bool use_etsi_fading,
                              const float custom_fading_stddev);
    void SetScenario(EScenario scenario);

    // CAM params
    void SetAccelerationStandardDeviation(const FVector &Vec);
    void SetGNSSDeviation(const float noise_lat_stddev,
                          const float noise_lon_stddev,
                          const float noise_alt_stddev,
                          const float noise_head_stddev,
                          const float noise_lat_bias,
                          const float noise_lon_bias,
                          const float noise_alt_bias,
                          const float noise_head_bias);
    void SetVelDeviation(const float noise_vel_stddev);
    void SetYawrateDeviation(const float noise_yawrate_stddev, const float noise_yawrate_bias);
    void SetPathLossModel(const EPathLossModel path_loss_model);

    virtual void PrePhysTick(float DeltaSeconds) override;
    virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
    void SetOwner(AActor *Owner) override;

private:
    CaService *CaServiceObj;
    PathLossModel *PathLossModelObj;
    void InitModel(AActor *Owner);

    // store data
    using ActorV2XDataMap = std::map<AActor *, carla::sensor::data::CAMData>;
    static ActorV2XDataMap mActorV2XDataMap;
};
