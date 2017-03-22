#pragma once

#include <carla/NonCopyable.h>

#include "carla/server/Protocol.h"
#include "carla/server/TCPServer.h"

#include "carla/thread/AsyncReadWriteJobQueue.h"
#include "carla/thread/AsyncReaderJobQueue.h"
#include "carla/thread/AsyncWriterJobQueue.h"

#include "carla_protocol.pb.h"

class EpisodeReady;

namespace carla {

    struct Scene_Values;
    struct Reward_Values;
    enum class Mode : int8_t;

namespace server {

  class CarlaCommunication : private NonCopyable {
  public:

    explicit CarlaCommunication(int worldPort, int writePort, int readPort);

    ~CarlaCommunication();

    void sendReward(const Reward_Values &values);

    bool tryReadControl(float &steer, float &gas);

    void sendScene(const Scene_Values &scene);

    void sendReset();

    //void sendWorld(const World &world);
    void sendWorld(const uint32_t modes, const uint32_t scenes);

    bool tryReadSceneInit(Mode &mode, uint32_t &scene);

    bool tryReadEpisodeStart(uint32_t &start_index, uint32_t &end_index);

    void restartServer();

    void restartWorld();

    void restartClient();

    thread::AsyncReaderJobQueue<Reward_Values>& getServerThread();
    thread::AsyncWriterJobQueue<std::string>& getClientThread();
    thread::AsyncReadWriteJobQueue<std::string, std::string>& getWorldThread();

    bool worldConnected();
    bool clientConnected();
    bool serverConnected();

    std::mutex getGeneralMutex();

    Mode GetMode();

    bool NeedRestart();
    void Restart();

  private:


    TCPServer _server;

    TCPServer _client;

    TCPServer _world;

    int _worldPort, _clientPort, _serverPort;

	thread::AsyncReaderJobQueue<Reward_Values> _serverThread;

    thread::AsyncWriterJobQueue<std::string> _clientThread;

    thread::AsyncReadWriteJobQueue<std::string, std::string> _worldThread;

    std::atomic_bool _needRestart;

    std::atomic<Mode> _mode;

    const std::unique_ptr<Protocol> _proto;
  };

}
}
