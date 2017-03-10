#include "Carla.h"
#include <boost/asio.hpp>
#include <boost\bind.hpp>

namespace carla {
	namespace server {

		class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
		{

		public:

			typedef boost::shared_ptr<TCPConnection> pointer;

			static pointer create(boost::asio::io_service &io_service) {
				return pointer(new TCPConnection(io_service));
			}

			boost::asio::ip::tcp::socket& socket() {
				return socket_;
			}

			void Send(const std::string &message) {
				message_ = message;
				boost::asio::async_write(socket_, boost::asio::buffer(message_),
					boost::bind(&TCPConnection::handle_write, shared_from_this()));
			}


		private:

			TCPConnection(boost::asio::io_service& io_service)
				: socket_(io_service)
			{
			}

			void handle_write(const boost::system::error_code& /*error*/,
				size_t /*bytes_transferred*/)
			{
			}


			std::string message_;
			boost::asio::ip::tcp::socket socket_;
		};
	}
}