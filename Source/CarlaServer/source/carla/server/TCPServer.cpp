// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TCPServer.h"

#include <fstream>

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
			_acceptor(_service, tcp::endpoint(tcp::v4(), port)),
			_port(port),
			_socket(_service),
			_connected(false){
			/*std::ofstream myfile;
			myfile.open("TCP" + std::to_string(_port) + ".txt");
			myfile << " INIT TCP_SERVER " << std::to_string(_port) << std::endl;
			myfile.close();*/
		}

		TCPServer::~TCPServer() {
		
			/*std::ofstream myfile;
			myfile.open("TCP" + std::to_string(_port) + ".txt");
			myfile << " Y VOLO " << std::to_string(_port) << std::endl;
			myfile.close();*/

		}

		void TCPServer::AcceptSocket() {
			/*std::ofstream myfile;
			myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << " Create socket " << std::to_string(_port)  <<std::endl;
			myfile.close();*/

			try {
				_acceptor.accept(_socket);

				_connected = true;
			}

			catch (boost::system::system_error) {
				/*myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
				myfile << " >>>>> ACCEPT ERROR <<<<<" << std::endl;
				myfile.close();*/
			};


			/*myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << " CONNECTED... " << std::to_string(_port) << std::endl;
			myfile.close();*/
		}

		bool TCPServer::Connected() {
			return _connected;
		}

		void TCPServer::writeString(const std::string &message, error_code &error) {

			/*std::ofstream myfile;
			myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << "--> WRITE <--" << std::endl;
			myfile << " Message: " << message << " // length: " << message.length() << " // byte: " << GetBytes(message.length()) << std::endl;
			myfile.close();*/

			std::string outMessage(GetBytes(message.length()) + message);

			boost::asio::write(_socket, boost::asio::buffer(outMessage), error);

			/*myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << "------- DONE ------" << std::endl;
			myfile.close();*/
		}

		void TCPServer::readString(std::string &message, error_code &error) {

			/*tcp::socket socket(_service);
			_acceptor.accept(socket);*/

			/*std::ofstream myfile;
			myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << "--> READ <--" << std::endl;
			myfile << " Create socket " << std::endl;
			myfile.close();

			myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << "--> READ <--" << std::endl;
			myfile.close();*/

			bool end = false, readedBytes = false;
			int readedSize = 0, sizeToRead = -1;
			do {

				std::array<char, 128> buf;

				size_t len = _socket.read_some(boost::asio::buffer(buf), error);

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

			} while (!readedBytes || sizeToRead > readedSize);

			/*myfile.open("TCP" + std::to_string(_port) + ".txt", std::ios::app);
			myfile << "Receive Message: " << message << std::endl;
			myfile << "------ DONE ------" << message << std::endl;
			myfile.close();*/

		}

	} // namespace server
} // namespace carla
