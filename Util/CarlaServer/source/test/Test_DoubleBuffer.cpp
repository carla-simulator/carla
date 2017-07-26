#include <iostream>

#include <gtest/gtest.h>

#include <carla/Logging.h>
#include <carla/server/DoubleBuffer.h>

#include <atomic>
#include <cstring>
#include <future>
#include <string>

// #define CARLA_DOUBLEBUFFER_TEST_LOG
#ifdef CARLA_DOUBLEBUFFER_TEST_LOG
  template <typename ... Args>
  static inline void test_log(Args &&... args) {
    carla::detail::print_args(std::cout, "DEBUG:", std::forward<Args>(args)..., '\n');
  }
#else
  template <typename ... Args>
  static inline void test_log(Args &&...) {}
#endif

TEST(DoubleBuffer, WriteAndRead) {
  using namespace carla::server;

  DoubleBuffer<size_t> buffer;

  constexpr auto numberOfWrites = 50u;

  std::atomic_bool done{false};

  auto result_writer = std::async(std::launch::async, [&](){
    for (size_t i = 0u; i < numberOfWrites; ++i) {
      {
        auto writer = buffer.MakeWriter();
        ASSERT_TRUE(writer != nullptr);
        *writer = i;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(20u));
    };
    std::this_thread::sleep_for(std::chrono::milliseconds(200u));
    done = true;
  });

  const time_duration timeout = boost::posix_time::milliseconds(10u);

  auto result_reader = std::async(std::launch::async, [&](){
    auto readings = 0u;
    while (!done && (readings < numberOfWrites)) {
      size_t i;
      bool readed = false;
      const void* ptr = nullptr;
      {
        auto reader = buffer.TryMakeReader(timeout);
        if (reader != nullptr) {
          readed = true;
          i = *reader;
          ASSERT_EQ(readings, i);
          ++readings;
          ptr = reader.get();
        }
      }
      if (readed) {
        test_log("buffer", ptr, '=', i);
      } else {
        std::this_thread::yield();
      }
    };
  });

  result_reader.get();
  result_writer.get();
}

TEST(DoubleBuffer, LongMessage) {
  using namespace carla::server;

  const std::string message =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod"
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim"
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea"
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate"
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint"
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt"
      "mollit anim id est laborum."
      "Doloremque consequuntur quo perferendis ea aut neque ab. Corrupti"
      "tempora iusto voluptatem eum neque qui reiciendis iusto. Ut ipsa iste"
      "perspiciatis illum et cupiditate possimus. Vel omnis et sint. Voluptas"
      "nihil corrupti quasi et facilis. Autem doloribus autem accusantium"
      "quibusdam natus ut. Odit voluptatem similique dolor omnis aut."
      "Doloremque maxime et excepturi. Ullam in distinctio eligendi consequatur"
      "facilis assumenda. Vel dolores id similique blanditiis libero aut qui"
      "ea. Quod quam eligendi possimus quae et sed est. Neque ut nemo"
      "necessitatibus. Exercitationem molestiae minima sunt voluptate adipisci"
      "recusandae. Doloribus tenetur quas nostrum esse. Error at voluptas et"
      "pariatur accusantium nulla. Voluptatem repellat sit suscipit. Reiciendis"
      "sed officiis doloribus tempora quia. Tempora quam quasi cumque unde"
      "consequatur consequuntur. Nihil consequatur mollitia voluptas aspernatur"
      "voluptates expedita et enim. Est doloremque veritatis placeat ut. In at"
      "qui nemo consequatur omnis quasi. Dignissimos ullam et ut et dolores"
      "distinctio nisi. Aut quia et harum. Quam aspernatur illum cupiditate"
      "unde. Inventore est ut est ducimus libero.";

  DoubleBuffer<std::string> buffer;

  constexpr auto numberOfWrites = 50u;

  std::atomic_bool done{false};

  auto result_writer = std::async(std::launch::async, [&](){
    for (size_t i = 0u; i < numberOfWrites; ++i) {
      {
        auto writer = buffer.MakeWriter();
        ASSERT_TRUE(writer != nullptr);
        *writer = message;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1u));
    };
    std::this_thread::sleep_for(std::chrono::milliseconds(200u));
    done = true;
  });

  const time_duration timeout = boost::posix_time::milliseconds(10u);

  auto result_reader = std::async(std::launch::async, [&](){
    auto readings = 0u;
    while (!done && (readings < numberOfWrites)) {
      auto reader = buffer.TryMakeReader(timeout);
      if (reader != nullptr) {
        const std::string received = *reader;
        const void* ptr = reader.get();
        reader = nullptr; // release reader.

        ASSERT_EQ(received, message);
        test_log(readings, "buffer", ptr, '=', std::string(received.begin(), received.begin() + 11u), "...");
        ++readings;
      } else {
        std::this_thread::yield();
      }
    };
  });

  result_reader.get();
  result_writer.get();
}
