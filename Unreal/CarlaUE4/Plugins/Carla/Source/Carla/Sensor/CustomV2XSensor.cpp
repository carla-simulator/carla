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

std::list<AActor *> ACustomV2XSensor::mV2XActorContainer;
ACustomV2XSensor::ActorV2XDataMap ACustomV2XSensor::mActorV2XDataMap;

ACustomV2XSensor::ACustomV2XSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));

    // Init path loss model
    PathLossModelObj = new PathLossModel(RandomEngine);
}

void ACustomV2XSensor::SetOwner(AActor *Owner)
{
    UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: called setowner with %p"), Owner);
    if (GetOwner() != nullptr)
    {
        ACustomV2XSensor::mV2XActorContainer.remove(GetOwner());
        UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: removed old owner %p"), GetOwner());
    }

    Super::SetOwner(Owner);

    // Store the actor into the static list if the actor details are not available
    if(Owner != nullptr)
    {
        if (std::find(ACustomV2XSensor::mV2XActorContainer.begin(), ACustomV2XSensor::mV2XActorContainer.end(), Owner) == ACustomV2XSensor::mV2XActorContainer.end())
        {
            ACustomV2XSensor::mV2XActorContainer.push_back(Owner);
            UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: added owner, length now %d"), ACustomV2XSensor::mV2XActorContainer.size());
        }

    }

    PathLossModelObj->SetOwner(Owner);
    
    UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(GetWorld());
    FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(Owner);
    if (CarlaActor != nullptr)
    {
        mStationId = static_cast<long>(CarlaActor->GetActorId());
    }
}

FActorDefinition ACustomV2XSensor::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeCustomV2XDefinition();
}

/* Function to add configurable parameters*/
void ACustomV2XSensor::Set(const FActorDescription &ActorDescription)
{
    UE_LOG(LogCarla, Warning, TEXT("CustomV2XSensor: Set function called"));
    Super::Set(ActorDescription);
    UActorBlueprintFunctionLibrary::SetCustomV2X(ActorDescription, this);
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
    // Clear the message created during the last sim cycle
    if (GetOwner())
    {
        ACustomV2XSensor::mActorV2XDataMap.erase(GetOwner());

        // Step 0: Create message to send, if triggering conditions fulfilled
        // this needs to be done in pre phys tick to enable synchronous reception in all other v2x sensors
        // Check whether the message is generated
        if (mMessageDataChanged)
        {
            // If message is generated store it
            // make a pair of message and sending power
            // if different v2x sensors send with different power, we need to store that
            carla::sensor::data::CustomV2XData message_pw;
            message_pw.Message = CreateCustomV2XMessage();
            
            message_pw.Power = PathLossModelObj->GetTransmitPower();
            ACustomV2XSensor::mActorV2XDataMap.insert({GetOwner(), message_pw});
        }
    }
}

CustomV2XM_t ACustomV2XSensor::CreateCustomV2XMessage()
{
    CustomV2XM_t message = CustomV2XM_t();

    CreateITSPduHeader(message);
    std::strcpy(message.message,mMessageData.c_str());
    mMessageDataChanged = false;
    return message;
}

void ACustomV2XSensor::CreateITSPduHeader(CustomV2XM_t &message)
{
    ITSContainer::ItsPduHeader_t& header = message.header;
    header.protocolVersion = mProtocolVersion;
    header.messageID = mMessageId;
    header.stationID = mStationId;
}

/*
 * Function takes care of sending messages to the current actor.
 * First simulates the communication by calling LOSComm object.
 * If there is a list present then messages from those list are sent to the current actor
 */
void ACustomV2XSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ACustomV2XSensor::PostPhysTick);

    // Step 1: Create an actor list which has messages to send targeting this v2x sensor instance
    std::vector<ActorPowerPair> ActorPowerList;
    for (const auto &pair : ACustomV2XSensor::mActorV2XDataMap)
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

        ACustomV2XSensor::V2XDataList msg_received_power_list;
        for (const auto &pair : actor_receivepower_map)
        {
            carla::sensor::data::CustomV2XData send_msg_and_pw = ACustomV2XSensor::mActorV2XDataMap.at(pair.first);
            carla::sensor::data::CustomV2XData received_msg_and_pw;
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

/*
 * Function the store the message into the structure so it can be sent to python client
 */
void ACustomV2XSensor::WriteMessageToV2XData(const ACustomV2XSensor::V2XDataList &msg_received_power_list)
{
    for (const auto &elem : msg_received_power_list)
    {
        mV2XData.WriteMessage(elem);
    }
}


void ACustomV2XSensor::Send(const FString message)
{
    //note: this is unsafe! 
    //should be fixed to limit length somewhere
    mMessageData = TCHAR_TO_UTF8(*message);
    mMessageDataChanged = true;
}

