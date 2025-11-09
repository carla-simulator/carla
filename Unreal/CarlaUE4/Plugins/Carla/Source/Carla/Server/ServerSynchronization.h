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

    UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%s:%u] hint"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantIdHint);
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
      UE_LOG(LogCarla, Error, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%s:%u] failed unexpectedly because of id clash"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId);
      LogSynchronizationMap("Register failed");
      return carla::rpc::ResponseError("ServerSynchronization::RegisterSynchronizationParticipant failed unexpectedly because of id clash\n");
    }
    if (ParticipantId > MaxIdIter->second) {
      MaxIdIter->second = ParticipantId;
    }
    UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::RegisterSynchronizationParticipant[%s:%u]"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId);
    LogSynchronizationMap("Register end");
    SyncStateChanged=true;
    return ParticipantId;
  }

  bool DeregisterSynchronizationParticipant(carla::rpc::synchronization_client_id_type const &ClientId, 
    carla::rpc::synchronization_participant_id_type const &ParticipantId) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::DeregisterSynchronizationParticipant[%s:%u]"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId);
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
    SyncStateChanged=true;
    return true;
  }

  void DisconnectClient(carla::rpc::synchronization_client_id_type const &ClientId) {
    
    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    LogSynchronizationMap("Disconnect client start");
    auto ErasedEntries = SynchronizationWindowMap.erase(ClientId);
    if ( ErasedEntries > 0u ) {
      UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::DisconnectClient[%s:ALL]"), UTF8_TO_TCHAR(ClientId.c_str()));
    }
    else {
      UE_LOG(LogCarla, Warning, TEXT("ServerSynchronization::DisconnectClient[%s:ALL] client id not found"), UTF8_TO_TCHAR(ClientId.c_str()));
      LogSynchronizationMap("Disconnect client not found");
    }
    SyncStateChanged=true;
    LogSynchronizationMap("Disconnect client end");
  }

  void EnableSynchronousMode(carla::rpc::synchronization_client_id_type const &ClientId, 
                    carla::rpc::synchronization_participant_id_type const &ParticipantId = carla::rpc::ALL_PARTICIPANTS) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    for(auto &SynchronizationWindow: SynchronizationWindowMap) {
      if ( (ClientId == SynchronizationWindow.first) && 
           (( ParticipantId == carla::rpc::ALL_PARTICIPANTS ) || ( SynchronizationWindow.second.ParticipantId == ParticipantId )) &&
          (SynchronizationWindow.second.TargetGameTime <= carla::rpc::NO_SYNC_TARGET_GAME_TIME))  {
        SynchronizationWindow.second.TargetGameTime = carla::rpc::BLOCKING_TARGET_GAME_TIME;
        SyncStateChanged=true;
      }
    }
    UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::EnableSynchronousMode[%s:%d]"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId);
  }

  void DisableSynchronousMode(carla::rpc::synchronization_client_id_type const &ClientId, 
                              carla::rpc::synchronization_participant_id_type const &ParticipantId = carla::rpc::ALL_PARTICIPANTS) {

    std::lock_guard<std::mutex> SyncLock(SynchronizationMutex);
    
    for(auto &SynchronizationWindow: SynchronizationWindowMap) {
      if ( (ClientId == SynchronizationWindow.first) && 
           (( ParticipantId == carla::rpc::ALL_PARTICIPANTS ) || ( SynchronizationWindow.second.ParticipantId == ParticipantId )) &&
           (SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME))  {
        SynchronizationWindow.second.TargetGameTime = carla::rpc::NO_SYNC_TARGET_GAME_TIME;
        SyncStateChanged=true;
      }
    }
    UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::DisableSynchronousMode[%s:%d]"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId);
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
          UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::GetTargetSynchronizationTime[%s:%u] = %f"), UTF8_TO_TCHAR(SynchronizationWindow.first.c_str()), SynchronizationWindow.second.ParticipantId, SynchronizationWindow.second.TargetGameTime);
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
            UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%s:%u] = %f"), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId, TargetGameTime);
          }
      }
      if ( !ParticipantFound ) {
        UE_LOG(LogCarla, Error, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%s:%u] = %f failed."), UTF8_TO_TCHAR(ClientId.c_str()), ParticipantId, TargetGameTime);
        LogSynchronizationMap("Update failed");
        return carla::rpc::ResponseError("ServerSynchronization::UpdateSynchronizationWindow did not find requested SynchronizationParticipant\n");
      }
    }
    else {
      for (auto &SynchronizationWindow: SynchronizationWindowMap) {
        if (SynchronizationWindow.second.TargetGameTime > carla::rpc::NO_SYNC_TARGET_GAME_TIME) {
          SynchronizationWindow.second.TargetGameTime = TargetGameTime;
          UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::UpdateSynchronizationWindow[%s:%u] = %f FORCE"), UTF8_TO_TCHAR(SynchronizationWindow.first.c_str()), SynchronizationWindow.second.ParticipantId, TargetGameTime);
        }
      }
    }
    SyncStateChanged=true;
    return true;
  }

  void LogSynchronizationMap(std::string const &Reason) {
    for (auto &SynchronizationWindow: SynchronizationWindowMap) {
      UE_LOG(LogCarla, Verbose, TEXT("ServerSynchronization::LogSynchronizationMap[%s:%u] = %f (%s)"), UTF8_TO_TCHAR(SynchronizationWindow.first.c_str()), SynchronizationWindow.second.ParticipantId, SynchronizationWindow.second.TargetGameTime, *FString(Reason.c_str()));
    }
  }

  /**
   * @brief Get the synchronization window participant states and a flag if they have changed since last call.
  */
  std::pair<bool, std::vector<carla::rpc::synchronization_window_participant_state> > GetSynchronizationWindowParticipantStates() {
    std::vector<carla::rpc::synchronization_window_participant_state> SynchronizationWindowParticipantStates;
    SynchronizationWindowParticipantStates.reserve(SynchronizationWindowMap.size());
    for (auto &SynchronizationWindow: SynchronizationWindowMap) {
      carla::rpc::synchronization_window_participant_state ParticipantState {
        SynchronizationWindow.first,
        SynchronizationWindow.second.ParticipantId,
        SynchronizationWindow.second.TargetGameTime
      };
      SynchronizationWindowParticipantStates.push_back(ParticipantState);
    }
    auto ResultChanged = SyncStateChanged;
    SyncStateChanged = false;
    return std::make_pair(ResultChanged, SynchronizationWindowParticipantStates);
  }

private:
  mutable std::mutex SynchronizationMutex{};

  struct SynchonizationWindow{
    carla::rpc::synchronization_participant_id_type ParticipantId;
    carla::rpc::synchronization_target_game_time TargetGameTime{carla::rpc::NO_SYNC_TARGET_GAME_TIME};
  };

  std::map<carla::rpc::synchronization_client_id_type, carla::rpc::synchronization_participant_id_type> ParticipantIdMaxMap;
  std::multimap<carla::rpc::synchronization_client_id_type, SynchonizationWindow> SynchronizationWindowMap;
  bool SyncStateChanged {false};

};
