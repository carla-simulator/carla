// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/incomingMessage.h"
#include "carla/multigpu/secondary.h"

#include "carla/BufferPool.h"
#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/Time.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/bind_executor.hpp>

#include <exception>

namespace carla {
namespace multigpu {

  Secondary::Secondary(
    boost::asio::ip::tcp::endpoint ep,
    SecondaryCommands::callback_type callback) :
      _pool(),
      _socket(_pool.io_context()),
      _endpoint(ep),
      _strand(_pool.io_context()),
      _connection_timer(_pool.io_context()),
      _buffer_pool(std::make_shared<BufferPool>()) {
        
      _commander.set_callback(callback);
    }
  

  Secondary::Secondary(
    std::string ip,
    uint16_t port,
    SecondaryCommands::callback_type callback) :
      _pool(),
      _socket(_pool.io_context()),
      _strand(_pool.io_context()),
      _connection_timer(_pool.io_context()),
      _buffer_pool(std::make_shared<BufferPool>()) {

    boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(ip);
    _endpoint = boost::asio::ip::tcp::endpoint(ip_address, port);
    _commander.set_callback(callback);
  }

  Secondary::~Secondary() {
    _pool.Stop();
  }

  void Secondary::Connect() {
    AsyncRun(2u);

    _commander.set_secondary(shared_from_this());

    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;

      if (self->_done) {
        return;
      }

      if (self->_socket.is_open()) {
        self->_socket.close();
      }

      auto handle_connect = [weak](boost::system::error_code ec) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("secondary server: connection failed:", ec.message());
          if (!self->_done)
            self->Reconnect();
          return;
        }

        if (self->_done) {
          return;
        }

        // This forces not using Nagle's algorithm.
        // Improves the sync mode velocity on Linux by a factor of ~3.
        self->_socket.set_option(boost::asio::ip::tcp::no_delay(true));
        
        log_info("secondary server: connected to ", self->_endpoint);

        self->ReadData();
      };

      self->_socket.async_connect(self->_endpoint, boost::asio::bind_executor(self->_strand, handle_connect));
    });
  }

  void Secondary::Stop() {
    _connection_timer.cancel();
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;
      self->_done = true;
      if (self->_socket.is_open()) {
        self->_socket.close();
      }
    });
  }

  void Secondary::Reconnect() {
    std::weak_ptr<Secondary> weak = shared_from_this();
    _connection_timer.expires_from_now(time_duration::seconds(1u));
    _connection_timer.async_wait([weak](boost::system::error_code ec) {
      auto self = weak.lock();
      if (!self) return;
      if (!ec) {
        self->Connect();
      }
    });
  }

  void Secondary::AsyncRun(size_t worker_threads) {
    _pool.AsyncRun(worker_threads);
  }

  void Secondary::Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message) {
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      auto handle_sent = [weak, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("error sending data: ", ec.message());
        }
      };

      // _deadline.expires_from_now(_timeout);
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }
  
  void Secondary::Write(Buffer buffer) {
    auto message = Secondary::MakeMessage(std::move(buffer));

    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      auto handle_sent = [weak, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("error sending data: ", ec.message());
        }
      };

      // _deadline.expires_from_now(_timeout);
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }
  
  void Secondary::Write(std::string text) {
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      auto handle_sent = [weak](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("error sending data: ", ec.message());
        }
      };

      // _deadline.expires_from_now(_timeout);
      // sent first size buffer
      int this_size = text.size();
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(&this_size, sizeof(this_size)),
          boost::asio::bind_executor(self->_strand, handle_sent));
      
      // send characters
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(text.c_str(), text.size()),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }

  void Secondary::ReadData() {
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;
      if (self->_done) {
        return;
      }

      auto message = std::make_shared<IncomingMessage>(self->_buffer_pool->Pop());

      auto handle_read_data = [weak, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // Move the buffer to the callback function and start reading the next
          // piece of data.
          self->GetCommander().process_command(message->pop());
          self->ReadData();
        } else {
          // As usual, if anything fails start over from the very top.
          log_error("secondary server: failed to read data: ", ec.message());
          // Connect();
        }
      };

      auto handle_read_header = [weak, message, handle_read_data](
        boost::system::error_code ec,
        size_t DEBUG_ONLY(bytes)) {
          auto self = weak.lock();
          if (!self) return;
          if (!ec && (message->size() > 0u)) {
            DEBUG_ASSERT_EQ(bytes, sizeof(carla::streaming::detail::message_size_type));
            if (self->_done) {
              return;
            }
            // Now that we know the size of the coming buffer, we can allocate our
            // buffer and start putting data into it.
            boost::asio::async_read(
                self->_socket,
                message->buffer(),
                boost::asio::bind_executor(self->_strand, handle_read_data));
          } else if (!self->_done) {
            log_error("secondary server: failed to read header: ", ec.message());
            // DEBUG_ONLY(printf("size  = ", message->size()));
            // DEBUG_ONLY(printf("bytes = ", bytes));
            // Connect();
          }
        };

      // Read the size of the buffer that is coming.
      boost::asio::async_read(
          self->_socket,
          message->size_as_buffer(),
          boost::asio::bind_executor(self->_strand, handle_read_header));
    });
  }

} // namespace multigpu
} // namespace carla
