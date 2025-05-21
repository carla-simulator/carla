// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/V2XSensor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <string.h>
#include <algorithm>
#include "V2X/CaService.h"
#include "V2XSensor.h"
#include "V2X/PathLossModel.h"
std::list<AActor *> AV2XSensor::mV2XActorContainer;
AV2XSensor::ActorV2XDataMap AV2XSensor::mActorV2XDataMap;

AV2XSensor::AV2XSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

    // Init path loss model
    PathLossModelObj = new PathLossModel(RandomEngine);
    CaServiceObj = new CaService(RandomEngine);
}

void AV2XSensor::SetOwner(AActor *Owner)
{
    UE_LOG(LogCarla, Warning, TEXT("V2XSensor: called setowner with %p"), Owner);
    if (GetOwner() != nullptr)
    {
        AV2XSensor::mV2XActorContainer.remove(GetOwner());
        UE_LOG(LogCarla, Warning, TEXT("V2XSensor: removed old owner %p"), GetOwner());
    }

    Super::SetOwner(Owner);

    // Store the actor into the static list if the actor details are not available
    if (Owner != nullptr)
    {
        if (std::find(AV2XSensor::mV2XActorContainer.begin(), AV2XSensor::mV2XActorContainer.end(), Owner) == AV2XSensor::mV2XActorContainer.end())
        {
            AV2XSensor::mV2XActorContainer.push_back(Owner);
            UE_LOG(LogCarla, Warning, TEXT("V2XSensor: added owner, length now %d"), AV2XSensor::mV2XActorContainer.size());
        }
        UWorld *world = GetWorld();
        CaServiceObj->SetOwner(world, Owner);
        PathLossModelObj->SetOwner(Owner);
    }
}

FActorDefinition AV2XSensor::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeV2XDefinition();
}

/* Function to add configurable parameters*/
void AV2XSensor::Set(const FActorDescription &ActorDescription)
{
    UE_LOG(LogCarla, Warning, TEXT("V2XSensor: Set function called"));
    Super::Set(ActorDescription);
    UActorBlueprintFunctionLibrary::SetV2X(ActorDescription, this);
}

void AV2XSensor::SetCaServiceParams(const float GenCamMin, const float GenCamMax, const bool FixedRate)
{
    // forward parameters to CaService Obj
    CaServiceObj->SetParams(GenCamMin, GenCamMax, FixedRate);
}

void AV2XSensor::SetPropagationParams(const float TransmitPower,
                                      const float ReceiverSensitivity,
                                      const float Frequency,
                                      const float combined_antenna_gain,
                                      const float path_loss_exponent,
                                      const float reference_distance_fspl,
                                      const float filter_distance,
                                      const bool use_etsi_fading,
                                      const float custom_fading_stddev)
{
    // forward parameters to PathLossModel Obj
    PathLossModelObj->SetParams(TransmitPower, ReceiverSensitivity, Frequency, combined_antenna_gain, path_loss_exponent, reference_distance_fspl, filter_distance, use_etsi_fading, custom_fading_stddev);
}

void AV2XSensor::SetPathLossModel(const EPathLossModel path_loss_model)
{
    PathLossModelObj->SetPathLossModel(path_loss_model);
}

void AV2XSensor::SetScenario(EScenario scenario)
{
    PathLossModelObj->SetScenario(scenario);
}

/*
 * Function stores the actor details in to the static list.
 * Calls the CaService object to generate CAM message
 * Stores the message in static map
 */
void AV2XSensor::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);
    // Clear the message created during the last sim cycle
    if (GetOwner())
    {
        AV2XSensor::mActorV2XDataMap.erase(GetOwner());

        // Step 0: Create message to send, if triggering conditions fulfilled
        // this needs to be done in pre phys tick to enable synchronous reception in all other v2x sensors
        // Check whether the message is generated
        if (CaServiceObj->Trigger(DeltaSeconds))
        {
            // If message is generated store it
            // make a pair of message and sending power
            // if different v2x sensors send with different power, we need to store that
            carla::sensor::data::CAMData cam_pw;
            cam_pw.Message = CaServiceObj->GetCamMessage();
            cam_pw.Power = PathLossModelObj->GetTransmitPower();
            AV2XSensor::mActorV2XDataMap.insert({GetOwner(), cam_pw});
        }
    }
}

void AV2XSensor::SetAccelerationStandardDeviation(const FVector &Vec)
{
    CaServiceObj->SetAccelerationStandardDeviation(Vec);
}

void AV2XSensor::SetGNSSDeviation(const float noise_lat_stddev,
                                  const float noise_lon_stddev,
                                  const float noise_alt_stddev,
                                  const float noise_head_stddev,
                                  const float noise_lat_bias,
                                  const float noise_lon_bias,
                                  const float noise_alt_bias,
                                  const float noise_head_bias)
{
    CaServiceObj->SetGNSSDeviation(noise_lat_stddev,
                                   noise_lon_stddev,
                                   noise_alt_stddev,
                                   noise_head_stddev,
                                   noise_lat_bias,
                                   noise_lon_bias,
                                   noise_alt_bias,
                                   noise_head_bias);
}

void AV2XSensor::SetVelDeviation(const float noise_vel_stddev)
{
    CaServiceObj->SetVelDeviation(noise_vel_stddev);
}

void AV2XSensor::SetYawrateDeviation(const float noise_yawrate_stddev, const float noise_yawrate_bias)
{
    CaServiceObj->SetYawrateDeviation(noise_yawrate_stddev, noise_yawrate_bias);
}

/*
 * Function takes care of sending messages to the current actor.
 * First simulates the communication by calling LOSComm object.
 * If there is a list present then messages from those list are sent to the current actor
 */
void AV2XSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(AV2XSensor::PostPhysTick);
    if (GetOwner())
    {
        // Step 1: Create an actor list which has messages to send targeting this v2x sensor instance
        std::vector<ActorPowerPair> ActorPowerList;
        for (const auto &pair : AV2XSensor::mActorV2XDataMap)
        {
            if (pair.first != GetOwner())
            {
                ActorPowerPair actor_power_pair;
                actor_power_pair.first = pair.first;
                // actor sending with transmit power
                actor_power_pair.second = pair.second.Power;
                ActorPowerList.push_back(actor_power_pair);
            }
        }

        // Step 2: Simulate the communication for the actors in actor list to current actor.
        if (!ActorPowerList.empty())
        {
            UCarlaEpisode *carla_episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
            PathLossModelObj->Simulate(ActorPowerList, carla_episode, GetWorld());
            // Step 3: Get the list of actors who can send message to current actor, and the receive power of their messages.
            ActorPowerMap actor_receivepower_map = PathLossModelObj->GetReceiveActorPowerList();
            // Step 4: Retrieve the messages of the actors that are received

            // get registry to retrieve carla actor IDs
            const FActorRegistry &Registry = carla_episode->GetActorRegistry();

            AV2XSensor::V2XDataList msg_received_power_list;
            for (const auto &pair : actor_receivepower_map)
            {
                // Note: AActor* sender_actor = pair.first;
                carla::sensor::data::CAMData send_msg_and_pw = AV2XSensor::mActorV2XDataMap.at(pair.first);
                carla::sensor::data::CAMData received_msg_and_pw;
                // sent CAM
                received_msg_and_pw.Message = send_msg_and_pw.Message;
                // receive power
                received_msg_and_pw.Power = pair.second;

                msg_received_power_list.push_back(received_msg_and_pw);
            }

            WriteMessageToV2XData(msg_received_power_list);
        }
        // Step 5: Send message

        if (mV2XData.GetMessageCount() > 0)
        {
            auto DataStream = GetDataStream(*this);
            DataStream.SerializeAndSend(*this, mV2XData, DataStream.PopBufferFromPool());
        }
        mV2XData.Reset();
    }
}

/*
 * Function the store the message into the structure so it can be sent to python client
 */
void AV2XSensor::WriteMessageToV2XData(const AV2XSensor::V2XDataList &msg_received_power_list)
{
    for (const auto &elem : msg_received_power_list)
    {
        mV2XData.WriteMessage(elem);
    }
}
