#pragma once

#include "carla/server/Protocol.h"
#include "carla/server/TCPServer.h"

#include "carla/thread/AsyncReadWriteJobQueue.h"
#include "carla/thread/AsyncReaderJobQueue.h"
#include "carla/thread/AsyncWriterJobQueue.h"

namespace carla {
namespace server {

  class CarlaCommunication : private NonCopyable {
  public:

    explicit CarlaCommunication(int worldPort, int writePort, int readPort);

    void sendReward(const Reward &reward);

    bool tryReadControl(std::string &control);

    void sendScene(const Scene &scene);

    void sendReset(const EpisodeReady &ready);

    void sendWorld(const World &world);

    bool tryReadWorldInfo(std::string &info);

  private:

    TCPServer _server;

    TCPServer _client;

    TCPServer _world;

    thread::AsyncReaderJobQueue<std::string> _serverThread;

    thread::AsyncWriterJobQueue<std::string> _clientThread;

    thread::AsyncReadWriteJobQueue<std::string, std::string> _worldThread;
  };

}
}
