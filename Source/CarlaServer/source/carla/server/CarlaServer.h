// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <memory>
#include <string>

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
  class CARLA_API CarlaServer : private NonCopyable {
  public:

    /// Starts two threads for writing and reading.
    explicit CarlaServer(int writePort, int readPort);

    ~CarlaServer();

    /// Push a string to the sending queue.
    void writeString(const std::string &message);

    /// Try to read a string from the receiving queue. Return false if the queue
    /// is empty.
    bool tryReadString(std::string &message);

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace server
} // namespace carla
