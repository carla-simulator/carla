// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/NonCopyable.h>
#include <carla/CarlaServer.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace carla {
namespace server {

  class CarlaCommunication;

  /// Asynchronous TCP server. Uses two ports, one for sending messages (write)
  /// and one for receiving messages (read).
  ///
  /// Writing and reading are executed in two different threads. Each thread has
  /// its own queue of messages.
  ///
  /// Note that a new socket is created for every connection (every write and
  /// read).
  class Server : private NonCopyable {
  public:

    /// Starts two threads for writing and reading.
    explicit Server(uint32_t worldPort, uint32_t writePort, uint32_t readPort);

    ~Server();

    ///// Send values of the current player status
    void sendReward(const Reward_Values &values);

    //// Send the values of the generated scene
    void sendSceneValues(const Scene_Values &values);

    //// Send a signal to the client to notify that the car is ready
    void sendEndReset();

    void sendWorld(uint32_t modes, uint32_t scenes);

    ///// Try to read the response of the client. Return false if the queue
    ///// is empty.
    bool tryReadControl(float &steer, float &gas);

    ////Try to read if the client has selected an scene and mode. Return false if the queue is empty
    bool tryReadSceneInit(Mode &mode, uint32_t &scene);

    ////Try to read if the client has selected an end & start point. Return false if the queue is empty
    bool tryReadEpisodeStart(uint32_t &start_index, uint32_t &end_index);

    void setMode(Mode mode);

    Mode GetMode() const;

    void SetScene(int scene);

    int GetScene() const;

    bool worldConnected() const;

    bool clientConnected() const;

    bool serverConnected() const;

    bool needRestart() const;

  private:

    //std::mutex _mutex;

    
    std::atomic_int _scene;

    const std::unique_ptr<CarlaCommunication> _communication;

    //const std::unique_ptr<Protocol> _proto;
  };

} // namespace server
} // namespace carla
