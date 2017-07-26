// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include <array>
#include <iostream>

#include <boost/asio.hpp>

enum ErrorCodes {
  InvalidArguments,
  STLException,
  UnknownException
};

int main(int argc, char* argv[]) {
  try {
    if (argc != 4) {
      std::cerr << "Usage: client <host-ip> <read-port> <send-port>" << std::endl;
      return InvalidArguments;
    }

    using boost::asio::ip::tcp;

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query read_query(argv[1], argv[2]);
    tcp::resolver::iterator read_endpoint_iterator = resolver.resolve(read_query);
    tcp::resolver::query send_query(argv[1], argv[3]);
    tcp::resolver::iterator send_endpoint_iterator = resolver.resolve(send_query);

    for (;;) {
      // Read message.
      {
        tcp::socket socket(io_service);
        boost::asio::connect(socket, read_endpoint_iterator);

        for (;;)
        {
          std::array<char, 128> buf;
          boost::system::error_code error;

          size_t len = socket.read_some(boost::asio::buffer(buf), error);

          if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
          else if (error)
            throw boost::system::system_error(error); // Some other error.

          std::cout.write(buf.data(), len);
        }
      }

      // Send message.
      {
        std::cout << std::endl;
        std::string reply;
        std::cin >> reply;

        tcp::socket socket(io_service);
        boost::asio::connect(socket, send_endpoint_iterator);

        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(reply), error);
        if (error)
          throw boost::system::system_error(error);

        if ((reply == "q") || (reply == "quit"))
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
