// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"

#include <carla/server/TCPServer.h>

#include <iostream>
#include <string>

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

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: server <port>" << std::endl;
      return InvalidArguments;
    }

    using namespace carla::server;

    TCPServer server(toInt(argv[1u]));

    // Send message.
    {
      const std::string message = "What's up?";
      std::cout << "Sending " << message << "..." << std::endl;
      TCPServer::error_code error;
      server.writeString(message, error);
      if (error) {
        std::cerr << "Error sending: " << error.message() << std::endl;
        return ErrorSending;
      }
    }

    for (;;) {
      // Read message.
      {
        std::cout << "Reading..." << std::endl;
        std::string message;
        TCPServer::error_code error;
        server.readString(message, error);
        if (error && (error != boost::asio::error::eof)) {
          std::cerr << "Error reading: " << error.message() << std::endl;
          return ErrorReading;
        }

        std::cout << "They said " << message << std::endl;
      }

      // Send reply.
      {
        std::cout << "What do I say now?" << std::endl;
        std::string message;
        std::cin >> message;

        if ((message == "q") || (message == "quit"))
          break;

        TCPServer::error_code error;
        server.writeString(message, error);
        if (error) {
          std::cerr << "Error sending: " << error.message() << std::endl;
          return ErrorSending;
        }
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
