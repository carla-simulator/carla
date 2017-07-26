#include <iostream>
#include <random>
#include <string>

#include <gtest/gtest.h>

#include <carla/Logging.h>
#include <carla/server/AsyncServer.h>
#include <carla/server/CarlaEncoder.h>
#include <carla/server/EncoderServer.h>
#include <carla/server/TCPServer.h>

using namespace carla::server;
using namespace boost::posix_time;

// These tests assume there is an echo client connected to port 4000.
static constexpr uint32_t PORT = 4000u;
static const auto TIMEOUT = seconds(10);

using AsyncTCPServer = AsyncServer<EncoderServer<TCPServer>>;

static std::string MakeRandomString(size_t length) {
  constexpr char charset[] =
      " 0123456789"
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static thread_local std::mt19937 rg{std::random_device{}()};
  using distribution_t = std::uniform_int_distribution<decltype(length)>;
  static thread_local distribution_t pick(0, sizeof(charset) - 2);

  std::string result;
  result.reserve(length);
  std::generate_n(std::back_inserter(result), length, [&]() {
    return charset[pick(rg)];
  });

  return result;
}

TEST(AsyncServer, SyncSayHello) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  auto result = server.Connect(PORT, TIMEOUT);
  ASSERT_FALSE(result.get()) << "missing echo client!";

  for (auto i = 0u; i < 1000u; ++i) {
    const std::string message = "Hello client!";

    WriteTask<std::string> say_hello(TIMEOUT);
    server.Execute(say_hello);
    say_hello.set_message(message);
    ASSERT_FALSE(say_hello.get_result());

    ReadTask<std::string> read_answer(TIMEOUT);
    server.Execute(read_answer);
    auto reading = read_answer.get_result();
    ASSERT_FALSE(reading.error_code);
    ASSERT_EQ(message, reading.message);
  }
}

TEST(AsyncServer, ASyncSayHello) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  constexpr auto numberOfIterations = 1000u;

  struct write_and_read {
    write_and_read() : write(TIMEOUT), read(TIMEOUT) {}
    WriteTask<std::string> write;
    ReadTask<std::string> read;
  };

  std::vector<write_and_read> tasks(numberOfIterations);

  auto result = server.Connect(PORT, TIMEOUT);
  ASSERT_FALSE(result.get()) << "missing echo client!";

  // Submit tasks.
  for (auto &task : tasks) {
    server.Execute(task.write);
    server.Execute(task.read);
  }

  std::string message;
  char c = 'a';
  for (auto &task : tasks) {
    message += c++;
    task.write.set_message(message);
    ASSERT_FALSE(task.write.get_result());
    auto reading = task.read.get_result();
    ASSERT_FALSE(reading.error_code);
    ASSERT_EQ(message, reading.message);
  }
}

TEST(AsyncServer, ConnectTwice) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  auto result = server.Connect(PORT, TIMEOUT);
  ASSERT_FALSE(result.get()) << "missing echo client!";

  ASSERT_TRUE(server.Connect(PORT, TIMEOUT).get()) << "we shouldn't be able to connect twice";

  AsyncTCPServer server2(encoder);
  ASSERT_TRUE(server2.Connect(PORT, TIMEOUT).get()) << "we shouldn't be able to connect twice";
}

TEST(AsyncServer, Disconnect) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  auto result = server.Connect(PORT, TIMEOUT);
  ASSERT_FALSE(result.get()) << "missing echo client!";

  ReadTask<std::string> read(TIMEOUT);
  server.Execute(read);

  std::this_thread::sleep_for(std::chrono::seconds(2u));
  server.Disconnect();

  ASSERT_TRUE(read.get_result().error_code) << "we received something, and we shouldn't";
}

TEST(AsyncServer, DiscardFutures) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  constexpr auto numberOfIterations = 1000u;

  {
    auto result = server.Connect(PORT, TIMEOUT);
    ASSERT_FALSE(result.get()) << "missing echo client!";
  }

  std::vector<ReadTask<std::string>> reads;
  {
    std::string message;
    char c = 'a';
    for (auto i = 0u; i < numberOfIterations; ++i) {
      message += c++;
      WriteTask<std::string> write(TIMEOUT);
      write.set_message(message);
      server.Execute(write);
      reads.emplace_back(TIMEOUT);
      server.Execute(reads.back());
    }
  }
  {
    std::string message;
    char c = 'a';
    for (auto &read : reads) {
      message += c++;
      auto reading = read.get_result();
      ASSERT_FALSE(reading.error_code);
      ASSERT_EQ(message, reading.message);
    }
  }
}

static void ProfilingServer(size_t message_size) {
  CarlaEncoder encoder;
  AsyncTCPServer server(encoder);

  auto result = server.Connect(PORT, TIMEOUT);
  ASSERT_FALSE(result.get()) << "missing echo client!";
  for (auto i = 0u; i < 10u; ++i) {
    const std::string message = MakeRandomString(message_size);
    WriteTask<std::string> say_hello;
    {
      CARLA_PROFILE_SCOPE(Test_MainThread, Send);
      say_hello = WriteTask<std::string>(seconds(3));
      say_hello.set_message(message);
      server.Execute(say_hello);
    }
    ASSERT_FALSE(say_hello.get_result());
    Reading<std::string> reading;
    ReadTask<std::string> read_answer;
    {
      CARLA_PROFILE_SCOPE(Test_MainThread, Receive);
      read_answer = ReadTask<std::string>(seconds(3));
      server.Execute(read_answer);
      if (read_answer.IsReady()) {
        CARLA_PROFILE_SCOPE(Test_MainThread, GetReading);
        reading = read_answer.get_result();
      }
    }
    if (read_answer.valid())
        reading = read_answer.get_result();

    ASSERT_FALSE(reading.error_code);
    ASSERT_EQ(message.size(), reading.message.size());
    ASSERT_EQ(message, reading.message);
  }
}

TEST(AsyncServer, Message10KB) {
  auto f = std::async(std::launch::async, [](){ ProfilingServer(10u * 1024u); });
  f.get();
}

TEST(AsyncServer, Message1MB) {
  auto f = std::async(std::launch::async, [](){ ProfilingServer(1024u * 1024u); });
  f.get();
}

TEST(AsyncServer, Message10MB) {
  auto f = std::async(std::launch::async, [](){ ProfilingServer(10u * 1024u * 1024u); });
  f.get();
}

TEST(AsyncServer, Message20MB) {
  auto f = std::async(std::launch::async, [](){ ProfilingServer(20u * 1024u * 1024u); });
  f.get();
}
