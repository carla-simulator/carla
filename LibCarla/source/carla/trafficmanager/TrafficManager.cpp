// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//#include "carla/client/Client.h"


#include "carla/Sockets.h"
#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerLocal.h"
#include "carla/trafficmanager/TrafficManagerRemote.h"

#include "carla/Logging.h"

#include <condition_variable>
#include <deque>
#include <exception>
#include <thread>
#include <utility>

#ifndef _WIN32
#include <unistd.h>
#endif

#define DEBUG_PRINT_TM  0
#define IP_DATA_BUFFER_SIZE     80

namespace carla {
namespace traffic_manager {

using namespace constants::SpeedThreshold;
using namespace constants::PID;

std::map<uint16_t, TrafficManagerBase*> TrafficManager::_tm_map;
std::mutex TrafficManager::_mutex;

TrafficManager::TrafficManager(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port)
  : _port(port) {

  if(!GetTM(_port)){
    // Check if a TM server already exists and connect to it
    if(!CreateTrafficManagerClient(episode_proxy, port)) {
      // As TM server not running, create one
      CreateTrafficManagerServer(episode_proxy, port);
    }
  }
}

// Released vehicles and deleted TMs are dropped after unlocking _mutex: they may destroy a client.

static void AppendReleased(std::vector<ActorPtr> &out, TrafficManagerBase &tm) {
  auto released = tm.TakeReleasedActors();
  out.insert(out.end(),
             std::make_move_iterator(released.begin()),
             std::make_move_iterator(released.end()));
}

void TrafficManager::Release() {
  std::vector<ActorPtr> released;
  std::vector<TrafficManagerBase *> to_delete;
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    for(auto& tm : _tm_map) {
      tm.second->Release();
      AppendReleased(released, *tm.second);
      to_delete.push_back(tm.second);
    }
    _tm_map.clear();
  }
  for (auto *tm_ptr : to_delete) {
    delete tm_ptr;
  }
}

void TrafficManager::Reset(uint64_t episode_id) {
  std::vector<ActorPtr> released;
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    for(auto& tm : _tm_map) {
      tm.second->Reset(episode_id);
      AppendReleased(released, *tm.second);
    }
  }
}

void TrafficManager::Tick() {
  std::vector<ActorPtr> released;
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    for(auto& tm : _tm_map) {
      tm.second->SynchronousTick();
      AppendReleased(released, *tm.second);
    }
  }
}

void TrafficManager::ShutDown() {
  TrafficManagerBase* tm_ptr = nullptr;
  std::vector<ActorPtr> released;
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    auto it = _tm_map.find(_port);
    if (it != _tm_map.end()) {
      tm_ptr = it->second;
      _tm_map.erase(it);
    }
    if(tm_ptr != nullptr) {
      tm_ptr->ShutDown();
      AppendReleased(released, *tm_ptr);
    }
  }
  delete tm_ptr;
}

namespace {

  /// Resets the TMs on an episode change; owned by no client, so it never joins itself.
  class EpisodeResetThread {
  public:

    void Post(uint64_t episode_id, std::function<void()> on_done) {
      std::unique_lock<std::mutex> lock(_mutex);
      _queue.emplace_back(episode_id, std::move(on_done));
      // A forked child inherits _running but not the thread.
      if (!_running || _owner != CurrentProcess()) {
        _running = true;
        _owner = CurrentProcess();
        std::thread(&EpisodeResetThread::Run, this).detach();
      }
      _cv.notify_all();
    }

  private:

    static long CurrentProcess() {
#ifdef _WIN32
      return 0;
#else
      return static_cast<long>(::getpid());
#endif
    }

    void Run() {
      std::unique_lock<std::mutex> lock(_mutex);
      while (true) {
        _cv.wait(lock, [this]() { return !_queue.empty(); });
        auto item = std::move(_queue.front());
        _queue.pop_front();
        const bool reset = !_has_last || item.first != _last_episode_id;
        _last_episode_id = item.first;
        _has_last = true;
        lock.unlock();
        if (reset) {
          try {
            TrafficManager::Reset(item.first);
          } catch (const std::exception &e) {
            log_error("failed to reset the Traffic Manager for the new episode:", e.what());
          }
        }
        if (item.second) {
          item.second();
        }
        // Drop the callback before re-locking.
        item = {};
        lock.lock();
      }
    }

    std::mutex _mutex;
    std::condition_variable _cv;
    std::deque<std::pair<uint64_t, std::function<void()>>> _queue;
    uint64_t _last_episode_id = 0u;
    bool _has_last = false;
    bool _running = false;
    long _owner = 0;
  };

  EpisodeResetThread &GetEpisodeResetThread() {
    // Never destroyed: the detached thread may outlive static destruction at exit.
    static EpisodeResetThread *instance = new EpisodeResetThread();
    return *instance;
  }

} // namespace

void TrafficManager::RequestReset(uint64_t episode_id, std::function<void()> on_done) {
  GetEpisodeResetThread().Post(episode_id, std::move(on_done));
}

void TrafficManager::CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  // Get local IP details.
  auto GetLocalIP = [=](const uint16_t sport)-> std::pair<std::string, uint16_t> {
    std::pair<std::string, uint16_t> localIP;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == SOCK_INVALID_INDEX) {
      #if DEBUG_PRINT_TM
      std::cout << "Error number 1: " << errno << std::endl;
      std::cout << "Error message: " << strerror(errno) << std::endl;
      #endif
    } else {
      int err;
      sockaddr_in loopback;
      std::memset(&loopback, 0, sizeof(loopback));
      loopback.sin_family = AF_INET;
      loopback.sin_addr.s_addr = INADDR_LOOPBACK;
      loopback.sin_port = htons(9);
      err = connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback));
      if(err == SOCK_INVALID_INDEX) {
        #if DEBUG_PRINT_TM
        std::cout << "Error number 2: " << errno << std::endl;
        std::cout << "Error message: " << strerror(errno) << std::endl;
        #endif
      } else {
        socklen_t addrlen = sizeof(loopback);
        err = getsockname(sock, reinterpret_cast<struct sockaddr*> (&loopback), &addrlen);
        if(err == SOCK_INVALID_INDEX) {
          #if DEBUG_PRINT_TM
          std::cout << "Error number 3: " << errno << std::endl;
          std::cout << "Error message: " << strerror(errno) << std::endl;
          #endif
        } else {
          char buffer[IP_DATA_BUFFER_SIZE];
          const char* p = inet_ntop(AF_INET, &loopback.sin_addr, buffer, IP_DATA_BUFFER_SIZE);
          if(p != NULL) {
            localIP = std::pair<std::string, uint16_t>(std::string(buffer), sport);
          } else {
            #if DEBUG_PRINT_TM
            std::cout << "Error number 4: " << errno << std::endl;
            std::cout << "Error message: " << strerror(errno) << std::endl;
            #endif
          }
        }
      }
      #ifdef _WIN32
        closesocket(sock);
      #else
        close(sock);
      #endif
    }
    return localIP;
  };

  /// Define local constants
  const std::vector<float> longitudinal_param = LONGITUDIAL_PARAM;
  const std::vector<float> longitudinal_highway_param = LONGITUDIAL_HIGHWAY_PARAM;
  const std::vector<float> lateral_param = LATERAL_PARAM;
  const std::vector<float> lateral_highway_param = LATERAL_HIGHWAY_PARAM;
  const float perc_difference_from_limit = INITIAL_PERCENTAGE_SPEED_DIFFERENCE;

  std::pair<std::string, uint16_t> serverTM;

  /// Create local instance of TM
  TrafficManagerLocal* tm_ptr = new TrafficManagerLocal(
    longitudinal_param,
    longitudinal_highway_param,
    lateral_param,
    lateral_highway_param,
    perc_difference_from_limit,
    episode_proxy,
    port);

  /// Get TM server info (Local IP & PORT)
  serverTM = GetLocalIP(port);

  /// Set this client as the TM to server
  episode_proxy.Lock()->AddTrafficManagerRunning(serverTM);

  #if DEBUG_PRINT_TM
  /// Print status
  std::cout << "NEW@: Registered TM at "
        << serverTM.first  << ":"
        << serverTM.second << " ..... SUCCESS."
        << std::endl;
  #endif

  /// Set the pointer of the instance
  _tm_map.insert(std::make_pair(port, tm_ptr));

}

bool TrafficManager::CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  bool result = false;

  if(episode_proxy.Lock()->IsTrafficManagerRunning(port)) {

    /// Get TM server info (Remote IP & PORT)
    std::pair<std::string, uint16_t> serverTM =
      episode_proxy.Lock()->GetTrafficManagerRunning(port);

    /// Set remote TM server IP and port
    TrafficManagerRemote* tm_ptr = new(std::nothrow)
      TrafficManagerRemote(serverTM, episode_proxy);

    /// Try to connect to remote TM server
    try {

      /// Check memory allocated or not
      if(tm_ptr != nullptr) {

        #if DEBUG_PRINT_TM
        // Test print
        std::cout << "OLD@: Registered TM at "
              << serverTM.first  << ":"
              << serverTM.second << " ..... TRY "
              << std::endl;
        #endif
        /// Try to reset all traffic lights
        tm_ptr->HealthCheckRemoteTM();

        /// Set the pointer of the instance
        _tm_map.insert(std::make_pair(port, tm_ptr));

        result = true;
      }
    }

    /// If Connection error occurred
    catch (...) {

      /// Clear previously allocated memory
      delete tm_ptr;

      #if DEBUG_PRINT_TM
      /// Test print
      std::cout 	<< "OLD@: Registered TM at "
            << serverTM.first  << ":"
            << serverTM.second << " ..... FAILED "
            << std::endl;
      #endif
    }

  }

  return result;
}

} // namespace traffic_manager
} // namespace carla
