// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"

#include <carla/server/CarlaServer.h>

#include <ctime>
#include <iostream>
#include <string>
#include <thread>

enum ErrorCodes {
  InvalidArguments,
  STLException,
  UnknownException,
  ErrorSending,
  ErrorReading
};

static int toInt(const std::string &str) {
  return std::stoi(str);
}

static std::string daytimeString() {
  using namespace std;
  time_t now = time(0);
  std::string str = ctime(&now);
  return str;
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: server <send-port> <read-port>" << std::endl;
      return InvalidArguments;
    }

    // This already starts the two threads.
    carla::server::CarlaServer server(toInt(argv[1u]), toInt(argv[2u]));

    // Let's simulate the game loop.
    for (;;) {
      std::cout << "Sending..." << std::endl;
      auto time = daytimeString();
      server.writeString(time);

      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1s);

      std::cout << "Listening..." << std::endl;
      std::string message;
      if (server.tryReadString(message)) {
        std::cout << "Received: " << message << std::endl;
        if ((message == "q") || (message == "quit"))
          break;
      }
    }

  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return STLException;
  } catch (...) {
    std::cerr << "Unknown exception thrown" << std::endl;
    return UnknownException;
  }
}
