// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TCPServer.h"

#include <fstream>

namespace carla {
namespace server {

  using boost::asio::ip::tcp;

  std::ofstream myfile;


  std::string GetBytes(int n) {
	  std::string bytes;

	  bytes = (n >> 24) & 0xFF;
	  bytes += (n >> 16) & 0xFF;
	  bytes += (n >> 8) & 0xFF;
	  bytes += n & 0xFF;

	  return bytes;
  }


  TCPServer::TCPServer(int port) :
    _service(),
    _acceptor(_service, tcp::endpoint(tcp::v4(), port)),
	_port(port){

	  myfile.open("TCP" + std::to_string(_port) + ".txt");
	  myfile << " INIT TCP_SERVER " << std::to_string(_port) << std::endl;
	  myfile.close();

  }

  TCPServer::~TCPServer() {}

  void TCPServer::writeString(const std::string &message, error_code &error) {

	  myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
	  myfile << "--> WRITE <--" << std::endl;
	  myfile << " Create socket " << std::endl;
	  myfile.close();

	  //Sleep(500);

    tcp::socket socket(_service);
    _acceptor.accept(socket);

	myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
	myfile << " Message: " << message << " // length: " << message.length() << " // byte: "<< GetBytes(message.length()) <<std::endl;
	myfile.close();

	//Sleep(500);
	std::string outMessage (GetBytes(message.length()) + message);

    boost::asio::write(socket, boost::asio::buffer(outMessage), error);

	myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
	myfile << "------- DONE ------" << std::endl;
	myfile.close();

	//Sleep(500);
	//std::cout <<  _port << ": DONE " << std::endl;
  }

  void TCPServer::readString(std::string &message, error_code &error) {
    tcp::socket socket(_service);
    _acceptor.accept(socket);

	myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
	myfile << "--> READ <--" << std::endl;
	myfile << " Create socket " << std::endl;
	myfile.close();

    for (;; ) {
      std::array<char, 128> buf;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof) {
        break; // Connection closed cleanly by peer.
      } else if (error) {
        return;
      }

      // @todo find a better way.
      for (size_t i = 0u; i < len; ++i) {
        message += buf[i];
      }
    }

	myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
	myfile << "Receive Message: " << message << std::endl;
	myfile << "------ DONE ------" << message << std::endl;
	myfile.close();
  }

} // namespace server
} // namespace carla
