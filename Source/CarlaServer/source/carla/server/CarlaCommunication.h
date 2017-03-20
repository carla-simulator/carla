#pragma once

#include <carla/NonCopyable.h>

#include "carla/server/Protocol.h"
#include "carla/server/TCPServer.h"

#include "carla/thread/AsyncReadWriteJobQueue.h"
#include "carla/thread/AsyncReaderJobQueue.h"
#include "carla/thread/AsyncWriterJobQueue.h"

class EpisodeReady;

namespace carla {
namespace server {

  class CarlaCommunication : private NonCopyable {
  public:

    explicit CarlaCommunication(int worldPort, int writePort, int readPort);

    void sendReward(const Reward &values);

    bool tryReadControl(std::string &control);

    void sendScene(const Scene &scene);

    void sendReset(const EpisodeReady &ready);

    void sendWorld(const World &world);

    bool tryReadWorldInfo(std::string &info);

    void restartServer();

    void restartWorld();

    void restartClient();

    thread::AsyncReaderJobQueue<Reward>& getServerThread();
    thread::AsyncWriterJobQueue<std::string>& getClientThread();
    thread::AsyncReadWriteJobQueue<std::string, std::string>& getWorldThread();

    bool worldConnected();
    bool clientConnected();
    bool serverConnected();

    std::mutex getGeneralMutex();

    bool NeedRestart();
    void Restart();

  private:


    TCPServer _server;

    TCPServer _client;

    TCPServer _world;

    int _worldPort, _clientPort, _serverPort;

	thread::AsyncReaderJobQueue<Reward> _serverThread;

    thread::AsyncWriterJobQueue<std::string> _clientThread;

    thread::AsyncReadWriteJobQueue<std::string, std::string> _worldThread;

    std::atomic_bool _needRestart;

  };

}
}
