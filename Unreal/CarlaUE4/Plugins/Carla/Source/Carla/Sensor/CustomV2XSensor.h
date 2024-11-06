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
#include "V2X/PathLossModel.h"
#include <list>
#include <map>
#include "CustomV2XSensor.generated.h"


UCLASS()
class CARLA_API ACustomV2XSensor : public ASensor
{
    GENERATED_BODY()

    using FV2XData = carla::sensor::data::CustomV2XDataS;
    using V2XDataList = std::vector<carla::sensor::data::CustomV2XData>;
    using ActorV2XDataMap = std::map<AActor *, carla::sensor::data::CustomV2XData>;

public:
    ACustomV2XSensor(const FObjectInitializer &ObjectInitializer);

    static FActorDefinition GetSensorDefinition();

    void Set(const FActorDescription &ActorDescription) override;
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
    void SetPathLossModel(const EPathLossModel path_loss_model);
    
    virtual void PrePhysTick(float DeltaSeconds) override;
    virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
    void SetOwner(AActor *Owner) override;

    void Send(const FString message);

private:
    static std::list<AActor *> mV2XActorContainer;
    PathLossModel *PathLossModelObj;

    //store data
    static ACustomV2XSensor::ActorV2XDataMap mActorV2XDataMap;
    FV2XData mV2XData;

    //write
    void WriteMessageToV2XData(const ACustomV2XSensor::V2XDataList &msg_received_power_list);

    //msg gen
    void CreateITSPduHeader(CustomV2XM_t &message);
    CustomV2XM_t CreateCustomV2XMessage();
    const long mProtocolVersion = 2;
    const long mMessageId = ITSContainer::messageID_custom;
    long mStationId;
    std::string mMessageData;
    bool mMessageDataChanged = false;
    constexpr static uint16_t data_size = sizeof(CustomV2XM_t::message);

};
