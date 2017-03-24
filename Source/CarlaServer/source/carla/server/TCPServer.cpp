// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "TCPServer.h"

#include <fstream>
#include <iostream>

namespace carla {
    namespace server {

        using boost::asio::ip::tcp;

        std::string GetBytes(int n) {
            std::string bytes;

            bytes = (n >> 24) & 0xFF;
            bytes += (n >> 16) & 0xFF;
            bytes += (n >> 8) & 0xFF;
            bytes += n & 0xFF;

            return bytes;
        }

        int GetInt(char b1, char b2, char b3, char b4) {
            int result = 0;
            result = (result << 8) + b1;
            result = (result << 8) + b2;
            result = (result << 8) + b3;
            result = (result << 8) + b4;

            return result;
        }


        TCPServer::TCPServer(int port) :
            _service(),
            port(port),
            _socket(_service),
            _connected(false){}

        TCPServer::~TCPServer() {}

        void TCPServer::AcceptSocket() {

            try {
              boost::asio::ip::tcp::acceptor acceptor(_service, tcp::endpoint(tcp::v4(), port));
              acceptor.accept(_socket);
              _service.run();
              std::cout << "Connected port " << port << std::endl;
              _connected = true;
            }

            catch (boost::system::system_error) {std::cerr<<"Socket System error"<<std::endl;};
        }

        bool TCPServer::Connected() {
            return _connected;
        }

        void TCPServer::writeString(const std::string &message, error_code &error) {

            std::string outMessage(GetBytes(message.length()) + message);

            boost::asio::write(_socket, boost::asio::buffer(outMessage), error);

            if (error)
            {
              std::cout << "DESCONECTED port " << port << std::endl;
              _connected = false;
            }
        }

        bool TCPServer::readString(std::string &message, error_code &error) {

            bool end = false, readedBytes = false;
            int readedSize = 0, sizeToRead = -1;

            if (_socket.available() > 0){
              do {

                  std::array<char, 128> buf;

                  size_t len = _socket.read_some(boost::asio::buffer(buf), error);


                  if (error)
                  {
                    std::cout << "DESCONECTED port " << port << std::endl;
                    _connected = false;
                  }
                  else if (!error){
                       // @todo find a better way.
                      for (size_t i = 0; i < len && !end; ++i) {
                          if (!readedBytes) {
                              sizeToRead = GetInt(buf[0], buf[1], buf[2], buf[3]);
                              i = 3;
                              readedBytes = true;
                          }
                          else {
                              message += buf[i];
                              ++readedSize;
                          }

                      }
                  }

              } while ((!readedBytes || sizeToRead > readedSize) && _connected);
              return true;
            }
            else return false;

        }

        void TCPServer::close(){

          _connected = false;
          // flush the socket buffer
          std::string message;
          TCPServer::error_code error;
          readString(message, error);
          _service.stop();
          _socket.close();
          /*_socket.cancel();
          _socket.close();
          _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
          _acceptor.cancel();*/
        
        }

    } // namespace server
} // namespace carla





