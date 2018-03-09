#include <future>

#include <gtest/gtest.h>

#include <carla/Logging.h>
#include <carla/server/Protobuf.h>
#include <carla/server/TCPServer.h>

using namespace carla::server;
using namespace boost::posix_time;

// These tests assume there is an echo client connected to port 4000.
static constexpr uint32_t PORT = 4000u;
static const auto TIMEOUT = seconds(10);

TEST(TCPServer, SayHello) {
  TCPServer server;
  ASSERT_FALSE(server.Connect(PORT, TIMEOUT)) << "missing echo client!";

  const std::string message = Protobuf::Encode("Hello client!");
  const auto length = message.size();

  for (auto i = 0u; i < 20u; ++i) {
    ASSERT_FALSE(server.Write(boost::asio::buffer(message), TIMEOUT));

    auto buffer = std::make_unique<char[]>(length);
    ASSERT_FALSE(server.Read(boost::asio::buffer(buffer.get(), length), TIMEOUT));
    ASSERT_EQ(message, std::string(buffer.get(), length));
  }
}

TEST(TCPServer, ConnectTwice) {
  TCPServer server;
  ASSERT_FALSE(server.Connect(PORT, TIMEOUT)) << "missing echo client!";
  ASSERT_TRUE(server.Connect(PORT, TIMEOUT)) << "we shouldn't be able to connect twice";
  TCPServer server2;
  ASSERT_TRUE(server2.Connect(PORT, TIMEOUT)) << "we shouldn't be able to connect twice";
}

TEST(TCPServer, ReadTimedOut) {
  TCPServer server;
  ASSERT_FALSE(server.Connect(PORT, TIMEOUT)) << "missing echo client!";

  constexpr size_t size = 50u;
  char received[size];
  ASSERT_TRUE(server.Read(boost::asio::buffer(received, size), seconds(1))) << "we received something, and we shouldn't";
}

TEST(TCPServer, ConnectionTimedOut) {
  TCPServer server;
  ASSERT_TRUE(server.Connect(4567, seconds(2))) << "woah! it seems we connected";
}

TEST(TCPServer, AsyncDisconnect) {
  TCPServer server;
  ASSERT_FALSE(server.Connect(PORT, TIMEOUT)) << "missing echo client!";

  auto result = std::async(std::launch::async, [&](){
    std::this_thread::sleep_for(std::chrono::seconds(2u));
    server.Disconnect();
  });

  constexpr size_t size = 50u;
  char received[size];
  ASSERT_TRUE(server.Read(boost::asio::buffer(received, size), TIMEOUT)) << "we received something, and we shouldn't";

  result.get();
}
