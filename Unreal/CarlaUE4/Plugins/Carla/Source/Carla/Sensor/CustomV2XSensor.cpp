// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the 
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <string>
#include <cstring>
#include <algorithm>
#include "CustomV2XSensor.h"
#include "V2X/PathLossModel.h"

std::mutex ACustomV2XSensor::v2xDataLock;
ACustomV2XSensor::ActorV2XDataMap ACustomV2XSensor::gActorV2XDataMap;
ACustomV2XSensor::ActorV2XDataMap ACustomV2XSensor::gActorV2XDataMapNextFrame;

ACustomV2XSensor::ACustomV2XSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

    // Init path loss model
    PathLossModelObj = new PathLossModel(RandomEngine, this);
}

void ACustomV2XSensor::SetOwner(AActor *Owner)
{
    Super::SetOwner(Owner);

    if(Owner != nullptr) {
        UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(GetWorld());
        FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(Owner);
        if (CarlaActor != nullptr) {
            mStationId = static_cast<long>(CarlaActor->GetActorId());
        }
    }
}

void ACustomV2XSensor::UpdateStationId()
{
    if ( mStationId == 0) {
        UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(GetWorld());
        FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(this);
        if (CarlaActor != nullptr) {
            mStationId = static_cast<long>(CarlaActor->GetActorId());
        }
    }
}

FActorDefinition ACustomV2XSensor::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeCustomV2XDefinition();
}

/* Function to add configurable parameters*/
void ACustomV2XSensor::Set(const FActorDescription &ActorDescription)
{
    Super::Set(ActorDescription);
    UActorBlueprintFunctionLibrary::SetCustomV2X(ActorDescription, this);

    auto Channel = ActorDescription.Variations.Find("channel_id");
    if (Channel != nullptr) 
    {
        mChannelId = TCHAR_TO_UTF8(*Channel->Value);
    }
}

void ACustomV2XSensor::SetPropagationParams(const float TransmitPower,
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

void ACustomV2XSensor::SetPathLossModel(const EPathLossModel path_loss_model){
    PathLossModelObj->SetPathLossModel(path_loss_model);
}

void ACustomV2XSensor::SetScenario(EScenario scenario)
{
    PathLossModelObj->SetScenario(scenario);
}

/*
 * Function stores the actor details in to the static list.
 * Calls the CaService object to generate CAM message
 * Stores the message in static map
 */
void ACustomV2XSensor::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);
    if (GetOwner())
    {
        // Swap the messages with the ones sent out since last sim cycle
        std::lock_guard<std::mutex> lock(v2xDataLock);
        auto LastMessagesResult = ACustomV2XSensor::gActorV2XDataMap.find(GetSenderId());
        auto NextMessagesResult = ACustomV2XSensor::gActorV2XDataMapNextFrame.find(GetSenderId());
        if ( LastMessagesResult != ACustomV2XSensor::gActorV2XDataMap.end() ) {
            if ( NextMessagesResult != ACustomV2XSensor::gActorV2XDataMapNextFrame.end() ) {
                // swap the next frame messages to current frame
                LastMessagesResult->second.swap(NextMessagesResult->second);
                ACustomV2XSensor::gActorV2XDataMapNextFrame.erase(NextMessagesResult);
            } else {
                // no new messages, just clear
                ACustomV2XSensor::gActorV2XDataMap.erase(LastMessagesResult);
            }
        }
        else if ( NextMessagesResult != ACustomV2XSensor::gActorV2XDataMapNextFrame.end() ) {
            // first time, just move next to current
            ACustomV2XSensor::gActorV2XDataMap.insert({GetSenderId(), V2XDataList()}).first->second.swap(NextMessagesResult->second);
            ACustomV2XSensor::gActorV2XDataMapNextFrame.erase(NextMessagesResult);
        }
        else {
            // no messages at all
        }
    }
}


/*
 * Function takes care of sending messages to the current actor.
 * First simulates the communication by calling LOSComm object.
 * If there is a list present then messages from those list are sent to the current actor
 */
void ACustomV2XSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    // tracing after we get the lock
    TRACE_CPUPROFILER_EVENT_SCOPE(ACustomV2XSensor::PostPhysTick);
    // Step 1: Create an actor list which has messages to send targeting this v2x sensor instance
    std::vector<ActorPowerPair> ActorPowerList;
    {
       std::lock_guard<std::mutex> lock(v2xDataLock);
        for (const auto &pair : gActorV2XDataMap)
        {
            // only different sensors with the same ChannelId talk to each other
            if ((pair.first.Actor != this) && (mChannelId == pair.first.ChannelId))
            {
                ActorPowerPair actor_power_pair;
                actor_power_pair.first = pair.first.Actor;
                // actor sending with transmit power (identical for all messages of this tick)
                actor_power_pair.second = pair.second.front().Power;
                ActorPowerList.push_back(actor_power_pair);
            }
        }
    }
 
    // Step 2: Simulate the communication for the actors in actor list to current actor.
    FV2XData v2XData;
    if (!ActorPowerList.empty())
    {
        UCarlaEpisode *carla_episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
        PathLossModelObj->Simulate(ActorPowerList, carla_episode, GetWorld());
        // Step 3: Get the list of actors who can send message to current actor, and the receive power of their messages.
        ActorPowerMap actor_receivepower_map = PathLossModelObj->GetReceiveActorPowerList();
        // Step 4: Retrieve the messages of the actors that are received
        {
            std::lock_guard<std::mutex> lock(v2xDataLock);
            for (const auto &pair : actor_receivepower_map)
            {
                for ( const auto & send_msg_and_pw: gActorV2XDataMap.at({pair.first, mChannelId}))
                {
                    carla::sensor::data::CustomV2XData received_msg_and_pw;
                    // sent CAM
                    received_msg_and_pw.Message = send_msg_and_pw.Message;
                    // receive power
                    received_msg_and_pw.Power = pair.second;

                    v2XData.WriteMessage(received_msg_and_pw);
                }
            }
        }
    }
    // Step 5: Send message
    if (v2XData.GetMessageCount() > 0)
    {
        auto DataStream = GetDataStream(*this);
        DataStream.SerializeAndSend(*this, v2XData, DataStream.PopBufferFromPool());
    }
}

void ACustomV2XSensor::Send(const carla::rpc::CustomV2XBytes &data)
{
    // We have to queue the data immediately otherwhise only one single message can be processed per frame!
    std::lock_guard<std::mutex> lock(v2xDataLock);

    UpdateStationId();

    // make a pair of message and sending power
    // if different v2x sensors send with different power, we need to store that
    carla::sensor::data::CustomV2XData message_pw;
    message_pw.Message.header.protocolVersion = mProtocolVersion;
    message_pw.Message.header.messageID = mMessageId;
    message_pw.Message.header.stationID = mStationId;
    message_pw.Message.data = data;
    message_pw.Power = PathLossModelObj->GetTransmitPower();
    gActorV2XDataMapNextFrame.insert({GetSenderId(), V2XDataList()}).first->second.push_back(message_pw);
}

