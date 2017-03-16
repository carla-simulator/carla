// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaCommunication.h"

#include "../CarlaServer.h"

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace carla {
  namespace server {

    /// Asynchronous TCP server. Uses two ports, one for sending messages (write)
    /// and one for receiving messages (read).
    ///
    /// Writing and reading are executed in two different threads. Each thread has
    /// its own queue of messages.
    ///
    /// Note that a new socket is created for every connection (every write and
    /// read).
    class CARLA_API Server : private NonCopyable {
    public:

      /// Starts two threads for writing and reading.
      explicit Server(int worldPort, int writePort, int readPort);

      ~Server();

      ///// Send values of the current player status
      void sendReward(const Reward_Values &values);

      //// Send the values of the generated scene
      void sendSceneValues(const Scene_Values &values);

      //// Send a signal to the client to notify that the car is ready
      void sendEndReset();

      void sendWorld(const int modes, const int scenes);

      ///// Try to read the response of the client. Return false if the queue
      ///// is empty.
      bool tryReadControl(float &steer, float &gas);

      ////Try to read if the client has selected an scene and mode. Return false if the queue is empty
      bool tryReadSceneInit(int &mode, int &scene);

      ////Try to read if the client has selected an end & start point. Return false if the queue is empty
      bool tryReadEpisodeStart(size_t &start_index, size_t &end_index);

      void setMode(Mode mode);

      Mode GetMode() const;

      void SetScene(int scene);

      int GetScene() const;

      void SetReset(bool reset);


      bool Reset() const;

    private:

      //std::mutex _mutex;

      std::atomic<Mode> _mode { Mode::MONO };
      std::atomic_int _scene;
      std::atomic_bool _reset;

      const std::unique_ptr<CarlaCommunication> _communication;

      const std::unique_ptr<Protocol> _proto;
    };

  } // namespace server
} // namespace carla
