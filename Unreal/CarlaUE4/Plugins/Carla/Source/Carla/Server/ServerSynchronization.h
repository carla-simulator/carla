// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <map>
#include <mutex>
#include "carla/rpc/ServerSynchronizationTypes.h"
#include "carla/rpc/Response.h"
#include "carla/Logging.h"
#include "Carla/Game/CarlaEngine.h"

/// The interface to the CARLA server required from TCP and ROS2 client side.
/// The parts only required from TPC client side are handled by lambdas directly.
class ServerSynchronization {
public:
  ServerSynchronization() = default;
  virtual ~ServerSynchronization() = default;


  /** @brief Register a synchronization participant
   *  
   *  After the first synchronization participant is registered, the server runs in synchronous mode.
   */
  carla::rpc::Response<carla::rpc::synchronization_participant_id_type> RegisterSynchronizationParticipant(
    carla::rpc::synchronization_client_id_type const &ClientId, 
    carla::rpc::synchronization_participant_id_type const &ParticipantIdHint = carla::rpc::ALL_PARTICIPANTS) {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);

    UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%u:%u] hint"), ClientId, ParticipantIdHint);
    auto MaxIdIter = ParticipantIdMaxMap.find(ClientId);
    if ( MaxIdIter==ParticipantIdMaxMap.end()) {
      auto InsertResult = ParticipantIdMaxMap.insert( {ClientId, carla::rpc::ALL_PARTICIPANTS});
      MaxIdIter = InsertResult.first;
    }
    auto ParticipantId = ParticipantIdHint;
    if ( ParticipantId==carla::rpc::ALL_PARTICIPANTS ) {
      ParticipantId = ++(MaxIdIter->second);
    }

    auto InsertResultIter = SynchronizationWindowMap.insert( { ClientId, {ParticipantId, carla::rpc::NO_SYNC_TARGET_GAME_TIME}});
    if ( InsertResultIter == SynchronizationWindowMap.end() ) {
      // collision
      UE_LOG(LogCarla, Error, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%u:%u] failed unexpectedly because of id clash"), ClientId, ParticipantId);
      LogSynchronizationMap("Register failed");
      return carla::rpc::ResponseError("ServerSynchronization::RegisterSynchronizationParticipant failed unexpectedly because of id clash\n");
    }
    if (ParticipantId > MaxIdIter->second) {
      MaxIdIter->second = ParticipantId;
    }
    UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%u:%u]"), ClientId, ParticipantId);
    LogSynchronizationMap("Register end");
    return ParticipantId;
  }

  bool DeregisterSynchronizationParticipant(carla::rpc::synchronization_client_id_type const &ClientId, 
    carla::rpc::synchronization_participant_id_type const &ParticipantId) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::DeregisterSynchronizationParticipant[%u:%u]"), ClientId, ParticipantId);
    LogSynchronizationMap("Deregister start");
    auto const SynchronizationParticipantEqualRange = SynchronizationWindowMap.equal_range(ClientId);
    for (auto SynchronizationWindowIter=SynchronizationParticipantEqualRange.first; 
       SynchronizationWindowIter != SynchronizationParticipantEqualRange.second;
        /* no iterator update here to support erase */) {
        if (SynchronizationWindowIter->second.ParticipantId == ParticipantId ) {
          SynchronizationWindowIter = SynchronizationWindowMap.erase(SynchronizationWindowIter);
        }
        else {
          SynchronizationWindowIter++;
        }
    }
    LogSynchronizationMap("Deregister end");
    return true;
  }

  void DisconnectClient(carla::rpc::synchronization_client_id_type const &ClientId) {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    LogSynchronizationMap("Disconnect client start");
    auto ErasedEntries = SynchronizationWindowMap.erase(ClientId);
    if ( ErasedEntries > 0u ) {
      UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::DisconnectClient[%u:ALL]"), ClientId);
    }
    else {
      // try erase ClientId==0 because RPC client-id from time to time is 0 (BUG?), but not on disconnect
      UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::DisconnectClient[%u:ALL] client id not found"), ClientId);
      LogSynchronizationMap("Disconnect client not found");
      ErasedEntries = SynchronizationWindowMap.erase(0u);
      if ( ErasedEntries > 0u ) {
        UE_LOG(LogCarla, Warning, TEXT("ServerSynchronization::DisconnectClient[0u:ALL] because ClientId=%u not found"), ClientId);
      }
      else {
        UE_LOG(LogCarla, Warning, TEXT("ServerSynchronization::DisconnectClient[] failed because neither ClientId=%u, nor ClientId=0u found"), ClientId);
      }
    }
    LogSynchronizationMap("Disconnect client end");
  }

  void EnableSynchronousMode(carla::rpc::synchronization_client_id_type const &ClientId) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    for(auto &SynchronizationWindow: SynchronizationWindowMap) {
      if ( (ClientId == SynchronizationWindow.first) && (SynchronizationWindow.second.TargetGameTime <= carla::rpc::NO_SYNC_TARGET_GAME_TIME))  {
        SynchronizationWindow.second.TargetGameTime = carla::rpc::BLOCKING_TARGET_GAME_TIME;
      }
    }
    UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::EnableSynchronousMode[%u:ALL]"), ClientId);
  }

  void DisableSynchronousMode(carla::rpc::synchronization_client_id_type const &ClientId) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    for(auto &SynchronizationWindow: SynchronizationWindowMap) {
      if ( (ClientId == SynchronizationWindow.first) && (SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME))  {
        SynchronizationWindow.second.TargetGameTime = carla::rpc::NO_SYNC_TARGET_GAME_TIME;
      }
    }
    UE_LOG(LogCarla, Log, TEXT("ServerSynchronization::DisableSynchronousMode[%u:ALL]"), ClientId);
  }

  bool IsSynchronousModeActive() const {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    for(auto const &SynchronizationWindow: SynchronizationWindowMap) {
      if ( SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME)  {
        return true;
      }
    }
    return false;
  }

  carla::rpc::synchronization_target_game_time GetTargetSynchronizationTime(double const CurrentGameTime, double const RequestedDltaTime) const {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    static int LogOncePerFrameCouter = 0;
    bool LogOutput = false;
    if (LogOncePerFrameCouter < FCarlaEngine::GetFrameCounter()) {
      LogOutput = true;
      LogOncePerFrameCouter = FCarlaEngine::GetFrameCounter();
    }

    carla::rpc::synchronization_target_game_time TargetGameTime = CurrentGameTime+RequestedDltaTime;
    for(auto const &SynchronizationWindow: SynchronizationWindowMap) {
      if ( (SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME) && (SynchronizationWindow.second.TargetGameTime < TargetGameTime) ) {
        if (LogOutput) {
          UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::GetTargetSynchronizationTime[%u:%u] = %f"), SynchronizationWindow.first, SynchronizationWindow.second.ParticipantId, SynchronizationWindow.second.TargetGameTime);
        }
        TargetGameTime = SynchronizationWindow.second.TargetGameTime;
      }
    }
    if (LogOutput) {
      UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::GetTargetSynchronizationTime[ALL:ALL] = %f"), TargetGameTime);
    }
    return TargetGameTime;
  }

  carla::rpc::Response<bool> UpdateSynchronizationWindow(
      carla::rpc::synchronization_client_id_type const &ClientId, 
      carla::rpc::synchronization_participant_id_type const &ParticipantId,
     carla::rpc::synchronization_target_game_time const &TargetGameTime) {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    if ( ClientId != carla::rpc::ALL_CLIENTS ) {
      auto const SynchronizationParticipantEqualRange = SynchronizationWindowMap.equal_range(ClientId);
      bool ParticipantFound = false;
      for (auto SynchronizationWindowIter=SynchronizationParticipantEqualRange.first; 
        SynchronizationWindowIter != SynchronizationParticipantEqualRange.second;
          SynchronizationWindowIter++) {
          if (SynchronizationWindowIter->second.ParticipantId == ParticipantId ) {
            ParticipantFound=true;
            SynchronizationWindowIter->second.TargetGameTime = TargetGameTime;
            UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%u:%u] = %f"), ClientId, ParticipantId, TargetGameTime);
          }
      }
      if ( !ParticipantFound ) {
        UE_LOG(LogCarla, Error, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%u:%u] = %f failed."), ClientId, ParticipantId, TargetGameTime);
        LogSynchronizationMap("Update failed");
        return carla::rpc::ResponseError("ServerSynchronization::UpdateSynchronizationWindow did not find requested SynchronizationParticipant\n");
      }
    }
    else {
      for (auto &SynchronizationWindow: SynchronizationWindowMap) {
        if (SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME) {
          SynchronizationWindow.second.TargetGameTime = TargetGameTime;
          UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%u:%u] = %f FORCE"), SynchronizationWindow.first, SynchronizationWindow.second.ParticipantId, TargetGameTime);
        }
      }
    }
    return true;
  }

  void LogSynchronizationMap(std::string const &Reason) {
    for (auto &SynchronizationWindow: SynchronizationWindowMap) {
      UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::LogSynchronizationMap[%u:%u] = %f (%s)"), SynchronizationWindow.first, SynchronizationWindow.second.ParticipantId, SynchronizationWindow.second.TargetGameTime, *FString(Reason.c_str()));
    }
  }

private:
  mutable std::mutex SynchronizationMutex{};

  struct SynchonizationWindow{
    carla::rpc::synchronization_participant_id_type ParticipantId;
    carla::rpc::synchronization_target_game_time TargetGameTime{carla::rpc::NO_SYNC_TARGET_GAME_TIME};
  };

  std::map<carla::rpc::synchronization_client_id_type, carla::rpc::synchronization_participant_id_type> ParticipantIdMaxMap;
  std::multimap<carla::rpc::synchronization_client_id_type, SynchonizationWindow> SynchronizationWindowMap;

};
